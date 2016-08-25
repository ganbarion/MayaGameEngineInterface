//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "DAGNode.h"

namespace bridge {


	/**
	 * DAGTexture
	 */
	class DAGTexture : public DAGNode
	{
	protected:
		MString					filePath_;
		MHWRender::MTexture*	texture_;
		se::Texture*			engineTexture_;
		se::SamplerState*		engineSampler_;
		bool mirror_u_;
		bool mirror_v_;
		bool wrap_u_;
		bool wrap_v_;

	protected:
		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug);

	private:
		void CreateTexture(MString path);
		void EvaluateAddressingMode();

	public:
		DAGTexture(MObject& object);
		virtual ~DAGTexture();

		virtual void Update() override;
		virtual DAGType Type() const override { return DAGType::Texture; }

		const se::Texture* GetEngineTexture() const { return engineTexture_; }
		const se::SamplerState* GetEngineSampler() const { return engineSampler_; }
	};

}