//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "DAGNode.h"

namespace bridge {

	/**
	 * DAGTransform
	 */
	class DAGTransform : public DAGNode
	{
	protected:
		Vector3 position_;
		Vector3 rotate_;
		Vector3 scale_;
		Matrix44 world_;
		bool visibility_;
		bool updated_;
		bool initialized_;

		const DAGTransform* parent_;
		std::vector<DAGNode*> childs_;
		MCallbackId dagAddedChildCallback_;
		MCallbackId dagRemovedChildCallback_;

	protected:
		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug) override;
		void Updated();

	public:
		DAGTransform(MObject& object);
		virtual ~DAGTransform();

		virtual DAGType Type() const override { return DAGType::Transform; }
		virtual void Update() override;
		virtual void LinkParent(const DAGNode* parent) override;
		virtual void UnlinkParent(const DAGNode* parent) override;
		virtual void NotifyParentTransformUpdated(const DAGNode* parent) override;

		void AddChild(DAGNode* node);
		void RemoveChild(DAGNode* node);

		const Matrix44& GetWorldMatrix() const { return world_; }
		bool IsVisible() const { return visibility_ && (!parent_ || parent_->IsVisible()); }
	};

}