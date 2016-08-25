//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include <bridge/DAGNode.h>

namespace bridge {

	DAGNode::DAGNode(MObject& object, bool useDirtyCallback)
		: attrChangeCallBackId_(0)
		, nodeDirtyPlugCallBackId_(0)
		, isIsolateSelected_(false)
	{
		handle_ = object;

		// アトリビュートの変更時のコールバック登録
		MStatus status;
		attrChangeCallBackId_ = MNodeMessage::addAttributeChangedCallback(object, AttributeChangeCallcack, this, &status);
		if (status != MStatus::kSuccess) {
			MDisplayError("[MayaCustomViewport] / Failed MNodeMessage::addAttributeChangedCallback.\n");
		}
		if (useDirtyCallback) {
			nodeDirtyPlugCallBackId_ = MNodeMessage::addNodeDirtyPlugCallback(object, NodeDirtyPlugCallcack, this, &status);
			if (status != MStatus::kSuccess) {
				MDisplayError("[MayaCustomViewport] / Failed MNodeMessage::addNodeDirtyPlugCallback.\n");
			}
		}
	}


	DAGNode::~DAGNode()
	{
		MStatus status = MMessage::removeCallback(attrChangeCallBackId_);
		if (status != MStatus::kSuccess){
			MDisplayError("[MayaCustomViewport] / Failed MMessage::removeCallback.\n");
		}

		if (nodeDirtyPlugCallBackId_) {
			MStatus status = MMessage::removeCallback(nodeDirtyPlugCallBackId_);
			if (status != MStatus::kSuccess){
				MDisplayError("[MayaCustomViewport] / Failed MMessage::removeCallback.\n");
			}
		}

		// 本来ここに来るときにはすべて連結が解除されてないといけないが念のため
		connections_.clear();
	}


	void DAGNode::AttributeChangeCallcack(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void* obj)
	{
		auto* node = static_cast<DAGNode*>(obj);
		node->AttributeChanged(msg, plug, otherPlug);
	}

	void DAGNode::NodeDirtyPlugCallcack(MObject& node, MPlug& plug, void* data)
	{
		auto* own = static_cast<DAGNode*>(data);
		own->NodeDirtyPlug(node, plug);
	}


	void DAGNode::Connect(DAGNode* node)
	{
		for (auto& c : connections_) {
			if (c.node() == node) {
				c.AddRef();
				return;
			}
		}
		connections_.push_back(DAGConnection(node));
	}


	void DAGNode::Disconnect(DAGNode* node)
	{
		auto iter = connections_.begin();
		while (iter != connections_.end()) {
			if (iter->node() == node) {
				iter->DecRef();
				if (iter->refCount() == 0) {
					connections_.erase(iter);
				}
				return;
			}
			iter++;
		}
	}

	DAGNode* DAGNode::FindConnectedItem(MObject obj)
	{
		auto iter = connections_.begin();
		for (auto& c : connections_) {
			if (*c.node() == obj) {
				return c.node();
			}
		}
		return nullptr;
	}


	void DAGNode::NotifyUpdateConnectionAll()
	{
		for (auto& c : connections_) {
			c.node()->NotifyUpdateConnection(this);
		}
	}

}
