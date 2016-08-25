//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "DAGNode.h"

namespace bridge {
	class DAGMaterial;
	class DAGTransform;

	struct TransformData
	{
		se::TUniformParameter<se::ObjectParameterData> uniforms;
		bool updated;
	};

	typedef std::unordered_map<const DAGTransform*, TransformData> NodeUniformMap;

	/**
	 * DAGMesh
	 */
	class DAGMesh : public DAGNode
	{
	private:
		// メッシュリソース
		struct Mesh 
		{
			std::vector<se::VertexBuffer> vertexBuffers;
			se::IndexBuffer indexBuffer;
			const se::VertexInputLayout* layout;
			DAGMaterial* material;
		};

	private:
		std::vector<Mesh> meshes_;
		NodeUniformMap uniformMap_;
		bool updated_;

	private:
		void UpdateGeometry();

	protected:
		virtual void AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug) override;
		virtual void NodeDirtyPlug(MObject& node, MPlug& plug) override;

	public:
		DAGMesh(MObject& object);
		virtual ~DAGMesh();

		virtual DAGType Type() const override { return DAGType::Mesh; }
		virtual void Update() override;
		virtual void Draw(se::GraphicsContext& context, ShadingPath path) override;
		virtual void NotifyUpdateConnection(const DAGNode* node) override;
		virtual void LinkParent(const DAGNode* parent) override;
		virtual void UnlinkParent(const DAGNode* parent) override;
		virtual void NotifyParentTransformUpdated(const DAGNode* parent) override;
	};

}