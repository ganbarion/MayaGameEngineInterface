//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "DAGNode.h"

namespace bridge {

	/**
	 * DAGSettings
	 */
	class DAGSettings : public DAGNode
	{
	protected:
		bool initialized_;
		bool fxaaEnable_;

	protected:
		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug) override;

	protected:
		void SetParameter(MPlug& attr);

	public:
		DAGSettings(MObject& object);
		virtual ~DAGSettings();

		virtual void Update() override;
		virtual DAGType Type() const override { return DAGType::Settings; }

		bool IsEnableFXAA() const { return fxaaEnable_; }
	};

}