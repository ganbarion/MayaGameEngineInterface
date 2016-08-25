//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "bridge/DAGTransform.h"
#include "bridge/DAGManager.h"
#include <maya/MDagMessage.h>
#include "Utility.h"

namespace bridge {


	DAGTransform::DAGTransform(MObject& object)
		: DAGNode(object, false)
		, position_(0, 0, 0)
		, rotate_(0, 0, 0)
		, scale_(1, 1, 1)
		, visibility_(true)
		, updated_(true)
		, initialized_(false)
		, parent_(nullptr)
		, dagAddedChildCallback_(0)
		, dagRemovedChildCallback_(0)
	{
		MDagPath path;
		MFnDagNode(object).getPath(path);

		// DAG監視用のコールバックを登録
		dagAddedChildCallback_ = MDagMessage::addChildAddedDagPathCallback(path, [](MDagPath& child, MDagPath& parent, void* clientData) {
			DAGTransform* own = reinterpret_cast<DAGTransform*>(clientData);
			DAGNode* node = DAGManager::Get()->FindDAGNode(child.node());
			if (node) own->AddChild(node);
		}, this);

		dagRemovedChildCallback_ = MDagMessage::addChildRemovedDagPathCallback(path, [](MDagPath& child, MDagPath& parent, void* clientData) {
			DAGTransform* own = reinterpret_cast<DAGTransform*>(clientData);
			DAGNode* node = DAGManager::Get()->FindDAGNode(child.node());
			if (node) own->RemoveChild(node);
		}, this);
	}


	DAGTransform::~DAGTransform()
	{
		MDagMessage::removeCallback(dagAddedChildCallback_);
		MDagMessage::removeCallback(dagRemovedChildCallback_);
	}


	void DAGTransform::AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug)
	{
		MStatus s;
		if (msg & MNodeMessage::kAttributeSet) {
			MFnAttribute fnAttr(plug.attribute());
			MString sn = fnAttr.shortName();
			if (sn == "t") {
				GetVectorByPlug(position_.ToFloatArray(), plug);
			} else if (sn == "r") {
				GetVectorByPlug(rotate_.ToFloatArray(), plug);
			} else if (sn == "s") {
				GetVectorByPlug(scale_.ToFloatArray(), plug);
			} else if (sn == "tx") {
				position_.x = plug.asFloat();
			} else if (sn == "ty") {
				position_.y = plug.asFloat();
			} else if (sn == "tz") {
				position_.z = plug.asFloat();
			} else if (sn == "rx") {
				rotate_.x = plug.asFloat();
			} else if (sn == "ry") {
				rotate_.y = plug.asFloat();
			} else if (sn == "rz") {
				rotate_.z = plug.asFloat();
			} else if (sn == "sx") {
				scale_.x = plug.asFloat();
			} else if (sn == "sy") {
				scale_.y = plug.asFloat();
			} else if (sn == "sz") {
				scale_.z = plug.asFloat();
			} 
#if 0
			else if (sn == "v") {
				visibility_ = plug.asBool();
			}
#endif
			Updated();
		}
	}

	void DAGTransform::Updated()
	{
		updated_ = true;
		for (auto* node : childs_) {
			node->NotifyParentTransformUpdated(this);
		}
	}

	void DAGTransform::Update()
	{
		MFnDagNode dagFn(handle_.objectRef());

		// ノードの生成順の関係でTransformを生成したタイミングだと
		// DAGManagerにまだchildが登録されていない場合があるため一度セットアップする
		if (!initialized_) {
			for (uint32_t i = 0; i < dagFn.childCount(); i++) {
				DAGNode* node = DAGManager::Get()->FindDAGNode(dagFn.child(i));
				if (node) {
					AddChild(node);
				}
			}
			initialized_ = true;
		}

		// Visibilityだけは毎フレーム監視
		// レイヤーからの操作による切り替えにAttributeChangedでは対応できないため
		MDagPath path;
		if (!dagFn.getPath(path)) return;
		visibility_ = path.isVisible();

		// トランスフォーム更新
		// 親が計算済みかによらずMaya側から値を取得できるのでトランスフォーム間で更新順を気にする必要はない
		// タイムに変化がある場合アニメーションを反映させるため現在の値を再取得している
		// (アニメーションの場合AttributeChangedには来ず、DirtyPlugは重いため)
		// 可視性とワールドしか使用していないのでこれらのみ再取得しているが、必要に応じて他のパラメータも再取得する必要がある
		if (updated_ || DAGManager::Get()->IsTimeChanged()) {
			if (DAGManager::Get()->IsTimeChanged()) {
				Matrix44 cache = world_;
				GetWorldMatrixByDagPath(path, &world_);

				// タイム変更時はここでも子に更新を通知
				if (cache != world_) {
					for (auto* node : childs_) {
						node->NotifyParentTransformUpdated(this);
					}
				}
			} else {
				GetWorldMatrixByDagPath(path, &world_);
			}

			updated_ = false;
		}
	}

	void DAGTransform::LinkParent(const DAGNode* parent)
	{
		if (!parent_) {
			parent_ = static_cast<const DAGTransform*>(parent);
			Updated();
		} else {
			MDisplayError("[MayaCustomViewport] / 子にトランスフォームを持つトランスフォームのインスタンスはサポートされていません。");
		}
	}

	void DAGTransform::UnlinkParent(const DAGNode* parent)
	{
		if (parent_ == static_cast<const DAGTransform*>(parent)) {
			parent_ = nullptr;
			Updated();
		}
	}

	void DAGTransform::NotifyParentTransformUpdated(const DAGNode* parent)
	{
		Updated();
	}

	void DAGTransform::AddChild(DAGNode* node)
	{
		// 既に登録されていないかをチェック
		auto iter = childs_.begin();
		for (; iter != childs_.end(); iter++) {
			if (node == *iter) {
				return;
			}
		}
		childs_.push_back(node);
		node->LinkParent(this);
		MDisplayDebugInfo("AddChild / parent: %s, child: %s", MFnDagNode(Object()).name().asChar(), MFnDagNode(node->Object()).name().asChar());
	}

	void DAGTransform::RemoveChild(DAGNode* node)
	{
		auto iter = childs_.begin();
		for (; iter != childs_.end(); iter++) {
			if (node == *iter) {
				childs_.erase(iter);
				node->UnlinkParent(this);
				MDisplayDebugInfo("RemoveChild / parent: %s, child: %s", MFnDagNode(Object()).name().asChar(), MFnDagNode(node->Object()).name().asChar());
				break;
			}
		}
	}

}