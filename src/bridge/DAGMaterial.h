//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "DAGNode.h"

namespace bridge {
	class DAGTexture;

	/**
	 * DAGMaterial
	 */
	class DAGMaterial : public DAGNode
	{
	private:
		MObjectHandle connectedShadingNode_;
		MCallbackId shadingNodeCallbackId_;
		MCallbackId shadingNodeDirtyCallbackId_;
		se::ShaderSet* shader_;
		std::vector<DAGTexture*> dagTextures_;
		MString technique_;
		MString	colorSet_;
		MString	tangentSet_;
		MString	binormalSet_;
		MStringArray texcoordSet_;
		bool updated_;

	private:
		static void ShadingNodeAttributeChangeCallcack(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug, void*);
		static void ShadingNodeDirtyPlugChangeCallcack(MObject& node, MPlug & plug, void*);

	private:
		void ConnectSurfaceShader(MObject shader);
		void DisconnectSurfaceShader();
		void ConnectTexture(MObject attr, MObject texture_node);
		void DisconnectTexture(MObject attr);

		void SetupDefaultShader();
		void SetTexture(int index, DAGTexture* texture);
		void SetParameterByPlug(MPlug plug);
		void SetTechniqueByPlug(MPlug name);

	public:
		DAGMaterial(MObject& object, bool connect = false);
		virtual ~DAGMaterial();

		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug) override;
		virtual void Update() override;
		virtual DAGType Type() const override { return DAGType::Material; }

		const se::ShaderSet* GetEngineShader() const { return shader_; }
		uint32_t GetDAGTextureNum() const { return static_cast<uint32_t>(dagTextures_.size()); }
		const DAGTexture* GetDAGTexture(uint32_t index) const { return dagTextures_[index]; }

		const MString& GetColorSet() const { return colorSet_; }
		const MString& GetTangentSet() const { return tangentSet_; }
		const MString& GetBinormalSet() const { return binormalSet_; }
		const MString& GetTexcoordSet(int index) { return texcoordSet_[index]; }
	};

}