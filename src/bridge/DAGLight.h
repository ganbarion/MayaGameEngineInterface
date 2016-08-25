//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "DAGNode.h"

namespace bridge {
	class DAGTransform;

	/**
	 * DAGLight
	 */
	class DAGLight : public DAGNode
	{
	public:
		enum class LightType {
			Directional,
			Point,
		};

	protected:
		const DAGTransform* transform_;
		LightType lightType_;
		Vector3 direction_;
		Vector3 color_;
		float intensity_;
		float range_;
		bool updated_;

	protected:
		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug) override;
		virtual void NodeDirtyPlug(MObject& node, MPlug& plug) override;

	public:
		DAGLight(MObject& object, LightType type);
		virtual ~DAGLight();

		virtual DAGType Type() const override { return DAGType::Light; }
		virtual void Update() override;
		virtual void LinkParent(const DAGNode* parent) override;
		virtual void UnlinkParent(const DAGNode* parent) override;
		virtual void NotifyParentTransformUpdated(const DAGNode* parent) override;
	};

}