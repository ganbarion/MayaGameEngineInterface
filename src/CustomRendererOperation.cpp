//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "CustomRendererOperation.h"
#include "bridge/DAGManager.h"
#include "bridge/DAGSettings.h"
#include "nodes/CustomViewportGlobals.h"


namespace {
	/**
	 * 生成時のステートをキャッシングし、デストラクタでステート復帰する
	 */
	class DX11StateBuckup
	{
	private:
		ID3D11DeviceContext* context_;
		ID3D11PixelShader* psShader_;
		ID3D11VertexShader* vsShader_;
		ID3D11Buffer* vertexBuffer_[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11Buffer* indexBuffer_;
		uint32_t vOffset_[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		uint32_t vStride_[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];
		uint32_t iOffset_;
		DXGI_FORMAT iFormat_;
		ID3D11ShaderResourceView* backup_srv_[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
		ID3D11Buffer* vs_cbuf_[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
		ID3D11Buffer* ps_cbuf_[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
		D3D11_PRIMITIVE_TOPOLOGY topology_;
		ID3D11InputLayout* layout_;

	public:
		DX11StateBuckup(ID3D11DeviceContext* context)
		{
			context_ = context;
			context_->AddRef();
			context->PSGetShader(&psShader_, nullptr, 0);
			context->VSGetShader(&vsShader_, nullptr, 0);
			context->IAGetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, vertexBuffer_, vOffset_, vStride_);
			context->IAGetIndexBuffer(&indexBuffer_, &iFormat_, &iOffset_);
			context->IAGetInputLayout(&layout_);
			context->PSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, backup_srv_);
			context->VSGetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, vs_cbuf_);
			context->PSGetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, ps_cbuf_);
			context->IAGetPrimitiveTopology(&topology_);
		}

		~DX11StateBuckup()
		{
			context_->PSSetShader(psShader_, nullptr, 0);
			context_->VSSetShader(vsShader_, nullptr, 0);
			context_->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, vertexBuffer_, vOffset_, vStride_);
			context_->IASetIndexBuffer(indexBuffer_, iFormat_, iOffset_);
			context_->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, backup_srv_);
			context_->VSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, vs_cbuf_);
			context_->PSSetConstantBuffers(0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT, ps_cbuf_);
			context_->IASetPrimitiveTopology(topology_);
			context_->IASetInputLayout(layout_);
			context_->Release();
		}
	};
}


CustomRenderOperation::CustomRenderOperation(const MString &name)
	: MUserRenderOperation(name)
	, targets_(nullptr)
	, fxaaEnable_(true)
{
	samplerState_.Create(se::FILTER_BILINEAR);

	uint16_t indeces[] = { 0, 2, 1 };
	quadFillIndexBuffer_.Create(indeces, sizeof(indeces), se::INDEX_BUFFER_STRIDE_U16);
}

CustomRenderOperation::~CustomRenderOperation()
{
	targets_ = nullptr;
	renderTarget_.Destroy();
	samplerState_.Destroy();
	quadFillIndexBuffer_.Destroy();
}

/**
 * 自前の描画エンジンを使用したビューポートへの描画ルーチン
 * ここでメッシュの描画、ポストエフェクト等をすべて行い、結果をMayaの内部ターゲットに出力する
 */
MStatus CustomRenderOperation::execute(const MHWRender::MDrawContext& drawContext)
{
	if (!targets_) return MStatus::kSuccess;

	// カメラ取得
	M3dView mView;
	MDagPath cameraPath;
	if (panelName_.length() && (M3dView::getM3dViewFromModelPanel(panelName_, mView) == MStatus::kSuccess)) {
		mView.getCamera(cameraPath);
	}

	auto& context = se::GraphicsCore::GetImmediateContext();
	ID3D11DeviceContext* dxContext = context.GetDeviceContext();
	// 現在のステートをバックアップ
	DX11StateBuckup dx11backup(dxContext);
	dxContext->ClearState();

	// Maya内部ターゲットの取得
	// 毎回作りなおしているのはMayaのテクスチャオブジェクトが変わっている可能性があるため
	se::ColorBuffer colorBuffer;
	se::DepthStencilBuffer depthBuffer;
	colorBuffer.CreateFromRTV(static_cast<ID3D11RenderTargetView*>(targets_[0]->resourceHandle()));
	depthBuffer.CreateFromDSV(static_cast<ID3D11DepthStencilView*>(targets_[1]->resourceHandle()));

	// スムーズシェード以外はクリアしてスキップ
	context.ClearDepthStencil(depthBuffer);
	if (mView.displayStyle() != M3dView::kGouraudShaded) {
		context.ClearRenderTarget(colorBuffer, se::float4(0.35f));
		return MStatus::kSuccess;
	}

	// エンジン描画
	{
		updateRenderSettings();

		// 選択項目の分離による描画リストのフィルタリング
		updateIsolateSelect();

		se::ColorBuffer& target = fxaaEnable_ ? renderTarget_ : colorBuffer;
		context.ClearRenderTarget(target, se::float4(0.35f));
		context.SetRenderTarget(&target, 1, &depthBuffer);
		se::Rect rect(0, 0, colorBuffer.GetWidth(), colorBuffer.GetHeight());
		context.SetViewport(rect);
		context.SetScissorRect(rect);

		// シーン描画
		scene_.Update(drawContext, cameraPath);
		scene_.Draw(context);

		// FXAA
		if (fxaaEnable_) {
			context.SetRenderTarget(&colorBuffer, 1, nullptr);
			se::ShaderSet* fxaa = se::ShaderManager::Get().Find("FXAA");
			context.SetVertexShader(fxaa->GetVS());
			context.SetPixelShader(fxaa->GetPS());
			context.SetPrimitiveType(se::PRIMITIVE_TYPE_TRIANGLE_LIST);
			context.SetInputLayout(se::VertexInputLayout());
			context.SetVertexBuffer(0, se::VertexBuffer());
			context.SetIndexBuffer(quadFillIndexBuffer_);
			context.SetPSResource(0, renderTarget_);
			context.SetPSSamplerState(0, samplerState_);
			context.SetBlendState(se::BlendState::Get(se::BlendState::Opacity));
			context.SetDepthStencilState(se::DepthStencilState::Get(se::DepthStencilState::Disable));
			context.SetRasterizerState(se::RasterizerState::Get(se::RasterizerState::BackFaceCull));
			context.DrawIndexed(0, 3);
		}

		// 描画フィルタをクリア
		bridge::DAGManager::Get()->ClearDrawFilter();
	}

	return MStatus::kSuccess;
}



MayaRenderTargets CustomRenderOperation::targetOverrideList(unsigned int& listSize)
{
	if (targets_) {
		listSize = 2;
		return &targets_[0];
	}
	return nullptr;
}

void CustomRenderOperation::setRenderTargets(MHWRender::MRenderTarget **targets, int32_t num)
{
	targets_ = targets;
	if (num == 0) return;

	// ターゲットのリサイズ
	// 複数パネルがある場合1回の更新で各パネル分リサイズがかかるので
	// ここでは簡略化のため毎回作り直しているが、各パネルごとにターゲットを持つほうが効率的
	MHWRender::MRenderTargetDescription desc;
	targets[0]->targetDescription(desc);
	if(!renderTarget_.GetResource()
		|| (renderTarget_.GetWidth() != desc.width() || renderTarget_.GetHeight() != desc.height()) )
	{
		uint32_t w = se::Max<uint32_t>(8, desc.width());
		uint32_t h = se::Max<uint32_t>(8, desc.height());
		renderTarget_.Destroy();
		renderTarget_.Create2D(DXGI_FORMAT_R8G8B8A8_UNORM, w, h);
	}
}


void CustomRenderOperation::updateRenderSettings()
{
	MStatus status;

	// customViewportGlobalsノードがあるか確認してなければ作る
	MSelectionList list;
	list.add("customViewportGlobals");
	MObject settingsNode;
	if (list.length() > 0) {
		status = list.getDependNode(0, settingsNode);
		if (!status) return;
	} else {
		MFnDependencyNode fnNode;
		settingsNode = fnNode.create("customViewportGlobals", "customViewportGlobals", &status);
		if (!status) {
			status.perror("createNodeNode / customViewportGlobals.");
			return;
		}
		// ロックする
		MFnDependencyNode node(settingsNode);
		node.setLocked(true);
	}

	// settingsノードからカレントの設定を取得
	auto* settings = static_cast<bridge::DAGSettings*>(bridge::DAGManager::Get()->GetSettingsNode());
	if (settings) {
		fxaaEnable_ = settings->IsEnableFXAA();
	}
}


void CustomRenderOperation::updateIsolateSelect()
{
	// 選択項目の分離がなされているかのチェック
	// C++でmelを呼び出したくないが現状M3dViewからisolateSelectedの状態がとれないので
	// 頂点単位の選択項目の分離は非対応

	MStatus status;
	MString cmdString = "isolateSelect -q -state ";
	cmdString += panelName_;
	int cmdResult;
	if (!MGlobal::executeCommand(cmdString, cmdResult)) {
		MDisplayError("[MayaCustomViewport] / Failed isolateSelect.");
	}
	if (cmdResult) {
		// isolateSelectの表示中オブジェクトセット名を取得
		cmdString = "isolateSelect -q -viewObjects ";
		cmdString += panelName_;
		MString stringResult;
		if (!MGlobal::executeCommand(cmdString, stringResult)) {
			MDisplayError("[MayaCustomViewport] / Failed isolateSelect.");
		}

		// オブジェクトセットのメンバリストを取得
		MSelectionList list;
		if (stringResult != "") {
			list.add(stringResult);
			MObject groupSet;
			if (list.getDependNode(0, groupSet)) {
				MFnSet set(groupSet);
				set.getMembers(list, true);
			}
		}
		bridge::DAGManager::Get()->SetDrawFilter(list);
	}
}
