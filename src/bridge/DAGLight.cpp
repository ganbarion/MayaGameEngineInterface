//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "bridge/DAGLight.h"
#include "bridge/DAGTransform.h"
#include "bridge/DAGManager.h"
#include "Utility.h"

namespace bridge {

	DAGLight::DAGLight(MObject& object, LightType type)
		: DAGNode(object)
		, lightType_(type)
		, direction_(0, 0, 1)
		, color_(1, 1, 1)
		, intensity_(1)
		, range_(0)
		, updated_(true)
	{
	}


	DAGLight::~DAGLight()
	{
		// TODO: ライトをエンジンから削除
	}


	void DAGLight::AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug)
	{
		MStatus status;
		MObject node = plug.node();

		if (msg & MNodeMessage::kAttributeSet) {
			MObject attr = plug.attribute();
			MFnAttribute fnAttr(attr);
			MString sn = fnAttr.shortName();
			if (sn == "cl") {			// color
				GetVectorByPlug(color_.ToFloatArray(), plug);
				updated_ = true;
			} else if (sn == "in") {	// intensity
				intensity_ = plug.asFloat();
				updated_ = true;
			} else if (sn == "ra") {	// range
				range_ = plug.asFloat();
				updated_ = true;
			}
		} else if ((msg & MNodeMessage::kConnectionMade) && (msg & MNodeMessage::kOtherPlugSet)) {
			// シーン読み込み時は初期パラメータを取得する
			MObject attr = plug.attribute();
			MFnAttribute fnAttr(attr);
			MString asn = fnAttr.shortName();
			MFnDagNode dag(node);
			MPlug clPlug = dag.findPlug("cl", &status);
			if (!status) return;
			MPlug inPlug = dag.findPlug("in", &status);
			if (!status) return;
			GetVectorByPlug(color_.ToFloatArray(), clPlug);
			intensity_ = inPlug.asFloat();
			MPlug raPlug = dag.findPlug("ra", &status);
			if (status) {
				range_ = raPlug.asFloat();
			}
			updated_ = true;
		}
	}

	void DAGLight::NodeDirtyPlug(MObject& node, MPlug& plug)
	{
		MFnAttribute fnAttr(plug.attribute());
		MString sn = fnAttr.shortName();

		if (sn == "cl") {			// color
			GetVectorByPlug(color_.ToFloatArray(), plug);
			updated_ = true;
		} else if (sn == "in") {	// intensity
			intensity_ = plug.asFloat();
			updated_ = true;
		} else if (sn == "ra") {	// range
			range_ = plug.asFloat();
			updated_ = true;
		}
	}


	void DAGLight::Update()
	{
		if (updated_) {

			switch (lightType_)
			{
			case LightType::Directional:
				{
					if (transform_) {
						Matrix44 world = transform_->GetWorldMatrix();
						direction_ = Vector3((float)-world.m[2][0], (float)-world.m[2][1], (float)-world.m[2][2]);
						// TODO: 設定を描画エンジンに通知
					}
				}
				break;

			case LightType::Point:
				{
					// TODO: 設定を描画エンジンに通知
				}
				break;
			}

			updated_ = false;
		}
	}

	void DAGLight::LinkParent(const DAGNode* parent)
	{
		transform_ = static_cast<const DAGTransform*>(parent);
		updated_ = true;
	}

	void DAGLight::UnlinkParent(const DAGNode* parent)
	{
		if (transform_ == static_cast<const DAGTransform*>(parent)) {
			transform_ = nullptr;
		}
		updated_ = true;
	}

	void DAGLight::NotifyParentTransformUpdated(const DAGNode* parent)
	{
		updated_ = true;
	}

}
