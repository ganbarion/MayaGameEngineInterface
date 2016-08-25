//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "bridge/DAGSettings.h"

namespace bridge {

	DAGSettings::DAGSettings(MObject& object)
		: DAGNode(object)
		, initialized_(false)
		, fxaaEnable_(true)
	{
	}


	DAGSettings::~DAGSettings()
	{
	}


	void DAGSettings::AttributeChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& otherPlug)
	{
		if (msg & MNodeMessage::kAttributeSet) {
			SetParameter(plug);
			// 再描画リクエスト
			M3dView::active3dView().refresh(true);
		}
	}


	void DAGSettings::Update()
	{
		if (!handle_.isValid()) return;

		// 初期化処理
		MStatus status;
		if (!initialized_) {
			MFnDependencyNode depNode(handle_.object(), &status);
			if (status) {
				// 全アトリビュートからパラメータをセット
				for (uint32_t i = 0; i < depNode.attributeCount(); i++) {
					MPlug plug(handle_.object(), depNode.attribute(i));
					SetParameter(plug);
				}
			}
			initialized_ = true;
		}
	}


	void DAGSettings::SetParameter(MPlug& plug)
	{
		MFnAttribute fnAttr(plug.attribute());
		MString sn = fnAttr.shortName();

		// ショートネームからパラメータを取得
		if (sn == "fae") {
			fxaaEnable_ = plug.asBool();
		}
	}

}