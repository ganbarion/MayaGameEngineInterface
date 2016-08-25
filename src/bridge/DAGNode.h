//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"

namespace bridge {
	class DAGNode;

	/**
	 * DAGType
	 */
	enum class DAGType {
		Transform,
		Mesh,
		Material,
		Texture,
		Light,
		Settings,
	};


	/**
	 * DAGConnection
	 */
	class DAGConnection
	{
	private:
		DAGNode* node_;
		int32_t refCount_;

	public:
		DAGConnection(DAGNode* node) {
			node_ = node;
			refCount_ = 1;
		}

		DAGNode* node() const { return node_; }
		int32_t refCount() const { return refCount_; }
		void AddRef() { refCount_++; }
		void DecRef() { refCount_--; }
	};


	/**
	 * DAGNode
	 */
	class DAGNode
	{
	protected:
		MObjectHandle handle_;
		MCallbackId attrChangeCallBackId_;
		MCallbackId nodeDirtyPlugCallBackId_;
		std::list<DAGConnection> connections_;
		bool isIsolateSelected_;

	protected:
		static void AttributeChangeCallcack(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void*);
		static void NodeDirtyPlugCallcack(MObject& node, MPlug& plug, void* data);

	protected:
		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug) {};
		virtual void NodeDirtyPlug(MObject& node, MPlug & plug) {};

	public:
		bool operator==(const MObject& object) const {
			return handle_ == object;
		}
		bool operator!=(const MObject& object) const {
				return handle_ != object;
		}

	public:
		DAGNode(MObject& object, bool useDirtyCallback = false);
		virtual ~DAGNode();

		virtual DAGType Type() const = 0;
		virtual void Update() {}
		virtual void Draw(se::GraphicsContext& context, ShadingPath path) {}
		virtual void LinkParent(const DAGNode* parent) {}						// 親トランスフォーム接続用
		virtual void UnlinkParent(const DAGNode* parent) {}						// 親トランスフォーム接続解除用
		virtual void NotifyParentTransformUpdated(const DAGNode* parent) {}		// 親トランスフォーム更新通知
		virtual void NotifyUpdateConnection(const DAGNode* node) {}

		void Connect(DAGNode* node);
		void Disconnect(DAGNode* node);
		void NotifyUpdateConnectionAll();
		DAGNode* FindConnectedItem(MObject obj);
		void SetIsolateSelected(bool selected) { isIsolateSelected_ = selected; }
		bool IsIsolateSelected() const { return isIsolateSelected_; }

		MObject Object() const { return handle_.object(); }
		uint32_t Hash() const { return handle_.hashCode(); }
		MString Name() const { return MFnDependencyNode(handle_.object()).name(); }
	};

}