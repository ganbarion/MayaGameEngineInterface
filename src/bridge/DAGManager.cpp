//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "bridge/DAGManager.h"
#include "bridge/DAGTransform.h"
#include "bridge/DAGMesh.h"
#include "bridge/DAGMaterial.h"
#include "bridge/DAGTexture.h"
#include "bridge/DAGLight.h"
#include "bridge/DAGSettings.h"

namespace bridge {

	namespace {
		void TraverseDraw(const std::list<DAGNode*>& list, se::GraphicsContext& context, ShadingPath path)
		{
			for (DAGNode* node : list) {
				node->Draw(context, path);
			}
		}

		void TraverseUpdate(const std::list<DAGNode*>& list)
		{
			for (DAGNode* node : list) {
				node->Update();
			}
		}
	}


	DAGManager* DAGManager::instance_ = nullptr;


	DAGManager::DAGManager()
		: isIsolateSelected_(false)
		, isTimeChanged_(false)
	{
		settings_ = nullptr;

		// ノードフックコールバック
		MStatus status;

		addNodeCallBackId_[CallBack_Transform] = MDGMessage::addNodeAddedCallback(AddNodeCallback, "transform", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeAddedCallback");

		removeNodeCallBackId_[CallBack_Transform] = MDGMessage::addNodeRemovedCallback(RemoveNodeCallback, "transform", &transformList_, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeRemovedCallback");

		addNodeCallBackId_[CallBack_Mesh] = MDGMessage::addNodeAddedCallback(AddNodeCallback, "mesh", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeAddedCallback");

		removeNodeCallBackId_[CallBack_Mesh] = MDGMessage::addNodeRemovedCallback(RemoveNodeCallback, "mesh", &meshList_, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeRemovedCallback");

		addNodeCallBackId_[CallBack_ShadingEngine] = MDGMessage::addNodeAddedCallback(AddNodeCallback, "shadingEngine", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeAddedCallback");

		removeNodeCallBackId_[CallBack_ShadingEngine] = MDGMessage::addNodeRemovedCallback(RemoveNodeCallback, "shadingEngine", &materialList_, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeRemovedCallback");

		addNodeCallBackId_[CallBack_Texture] = MDGMessage::addNodeAddedCallback(AddNodeCallback, "file", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeAddedCallback");

		removeNodeCallBackId_[CallBack_Texture] = MDGMessage::addNodeRemovedCallback(RemoveNodeCallback, "file", &materialList_, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeRemovedCallback");

		addNodeCallBackId_[CallBack_Light] = MDGMessage::addNodeAddedCallback(AddNodeCallback, "light", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeAddedCallback");

		removeNodeCallBackId_[CallBack_Light] = MDGMessage::addNodeRemovedCallback(RemoveNodeCallback, "light", &lightList_, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeRemovedCallback");

		addNodeCallBackId_[CallBack_Settings] = MDGMessage::addNodeAddedCallback(AddNodeCallback, "customViewportGlobals", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeAddedCallback");

		removeNodeCallBackId_[CallBack_Settings] = MDGMessage::addNodeRemovedCallback(RemoveNodeCallback, "customViewportGlobals", nullptr, &status);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed addNodeRemovedCallback");

		// グローバル時間が変更されたときをフック(アニメーション時のトランスフォーム更新用)
		timeChangedCallBackId_ = MEventMessage::addEventCallback("timeChanged", [](void* clientData) {
			DAGManager* mgr = reinterpret_cast<DAGManager*>(clientData);
			mgr->TimeChanged();
		}, this, &status);
		if (status != MStatus::kSuccess) {
			MDisplayError("Failed MEventMessage::addEventCallback\n");
		}

		// initialShadingGroupは追加コールバックが来ないので手動追加
		MSelectionList list;
		list.add("initialShadingGroup");
		if (list.length() > 0){
			MObject node;
			MStatus s = list.getDependNode(0, node);
			if (s == MStatus::kSuccess) {
				auto* mat = new DAGMaterial(node, true);
				materialList_.push_back(mat);
				nodeMap_[MObjectHandle(node)] = mat;
			}
		}
	}
	
	DAGManager::~DAGManager()
	{
		MStatus status;
		for (int32_t i = 0; i < CallBack_Max; i++) {
			status = MDGMessage::removeCallback(addNodeCallBackId_[i]);
			if (!status) MDisplayError("[MayaCustomViewport] / Failed removeNodeAddedCallback");
			status = MDGMessage::removeCallback(removeNodeCallBackId_[i]);
			if (!status) MDisplayError("[MayaCustomViewport] / Failed removeNodeRemovedCallback");
		}
		status = MDGMessage::removeCallback(timeChangedCallBackId_);
		if (!status) MDisplayError("[MayaCustomViewport] / Failed removeTimeChangedCallback");

		// リスト破棄
		for (DAGNode* node : transformList_) {
			delete node;
		}
		transformList_.clear();

		for (DAGNode* node : meshList_) {
			delete node;
		}
		meshList_.clear();

		for (DAGNode* node : materialList_) {
			delete node;
		}
		materialList_.clear();

		for (DAGNode* node : lightList_) {
			delete node;
		}
		lightList_.clear();

		// 設定ノード
		if (settings_) {
			delete settings_;
			settings_ = nullptr;
		}

		nodeMap_.clear();
	}
	
	void DAGManager::AddNodeCallback(MObject& node, void* clientData)
	{
		MFnDependencyNode nodeFn(node);
		DAGNode* dagNode = nullptr;
		MDisplayDebugInfo("Create / %s", MFnDagNode(node).name().asChar());

		// 独自DAGを生成する
		switch (node.apiType())
		{
		case MFn::kTransform:
			dagNode = new DAGTransform(node);
			instance_->transformList_.push_back(dagNode);
			break;
		case MFn::kMesh:
			dagNode = new DAGMesh(node);
			instance_->meshList_.push_back(dagNode);
			break;
		case MFn::kShadingEngine:
			dagNode = new DAGMaterial(node);
			instance_->materialList_.push_back(dagNode);
			break;
		case MFn::kFileTexture:
			dagNode = new DAGTexture(node);
			instance_->materialList_.push_back(dagNode);
			break;
		case MFn::kDirectionalLight:
			dagNode = new DAGLight(node, DAGLight::LightType::Directional);
			instance_->lightList_.push_back(dagNode);
			break;
		case MFn::kPointLight:
			dagNode = new DAGLight(node, DAGLight::LightType::Point);
			instance_->lightList_.push_back(dagNode);
			break;
		case MFn::kPluginDependNode:
			{
				// リファレンスしたシーンが持つものをスキップするため無名名前空間のもののみ追加する
				if (!instance_->settings_) {
					if (nodeFn.parentNamespace() == "") {
						dagNode = new DAGSettings(node);
						instance_->settings_ = dagNode;
					}
				}
				break;
			}
		}

		if (dagNode) {
			instance_->nodeMap_[MObjectHandle(node)] = dagNode;
		}
	}

	void DAGManager::RemoveNodeCallback(MObject& node, void *clientData)
	{
		// 独自DAG破棄
		if (clientData) {
			DAGNodeList& list = *reinterpret_cast<DAGNodeList*>(clientData);
			auto iter = list.begin();
			while (iter != list.end()) {
				DAGNode* curNode = *iter;
				if (*curNode == node) {
					delete curNode;
					list.erase(iter);
					break;
				}
				iter++;
			}
		}

		// 設定ノード
		if (instance_->settings_ && (*instance_->settings_) == node) {
			delete instance_->settings_;
			instance_->settings_ = nullptr;
		}

		MObjectHandle handle(node);
		auto iter = instance_->nodeMap_.find(handle);
		if (iter != instance_->nodeMap_.end()) {
			instance_->nodeMap_.erase(iter);
		}
	}

	void DAGManager::UpdateNode()
	{
		// トランスフォームの状態を他のノードが参照するので最初に更新
		TraverseUpdate(transformList_);

		if (settings_) settings_->Update();
		TraverseUpdate(materialList_);
		TraverseUpdate(meshList_);
		TraverseUpdate(lightList_);
		isTimeChanged_ = false;
	}

	void DAGManager::DrawNode(se::GraphicsContext& context, ShadingPath path)
	{
		if (!isIsolateSelected_) {
			// 描画の必要があるのはメッシュ、ライトのみ
			TraverseDraw(meshList_, context, path);
			TraverseDraw(lightList_, context, path);
		} else {
			// 選択項目の分離時
			for (DAGNode* node : isolateSelectNode_) {
				node->Draw(context, path);
			}
		}
	}

	void DAGManager::SetDrawFilter(MDagPath path)
	{
		MStatus status;
		MObjectHandle handle(path.node());
		auto iter = nodeMap_.find(handle);
		if (iter != nodeMap_.end()) {
			iter->second->SetIsolateSelected(true);
			isolateSelectNode_.push_back(iter->second);
		}

		// 子に対しても設定
		for (uint32_t ci = 0; ci < path.childCount(); ci++) {
			MObject obj = path.child(ci);
			MFnDagNode dagNode(obj, &status);
			if (status) {
				MDagPath childPath;
				dagNode.getPath(childPath);
				SetDrawFilter(childPath);
			}
		}
	}

	void DAGManager::SetDrawFilter(MSelectionList list)
	{
		for (uint32_t i = 0; i < list.length(); i++) {
			MDagPath path;
			list.getDagPath(i, path);
			SetDrawFilter(path);
		}
		isIsolateSelected_ = true;
	}

	void DAGManager::ClearDrawFilter()
	{
		for (DAGNode* node : isolateSelectNode_) {
			node->SetIsolateSelected(false);
		}
		isolateSelectNode_.clear();
		isIsolateSelected_ = false;
	}

	DAGNode* DAGManager::FindDAGNode(const MObject& object)
	{
		auto iter = nodeMap_.find(MObjectHandle(object));
		if (iter != nodeMap_.end()) {
			return iter->second;
		};
		return nullptr;
	}

	void DAGManager::ForEach(std::function<void(DAGNode*)> func)
	{
		for (auto& pair : nodeMap_) {
			func(pair.second);
		}
	}
}
