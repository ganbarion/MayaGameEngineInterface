//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#include "CustomViewportGlobals.h"

MTypeId CustomViewportGlobals::id(0x7fff0);
MObject CustomViewportGlobals::fxaaEnable_;


CustomViewportGlobals::CustomViewportGlobals()
{
}


CustomViewportGlobals::~CustomViewportGlobals()
{
}


MStatus CustomViewportGlobals::compute(const MPlug& plug, MDataBlock& data)
{
	/* Do nothing. */
	return MS::kSuccess;
}


void* CustomViewportGlobals::creator()
{
	return new CustomViewportGlobals();
}


MStatus CustomViewportGlobals::initialize()
{
	MStatus s;

	MFnNumericAttribute fnAttr;
	fxaaEnable_ = fnAttr.create("fxaaEnable", "fae", MFnNumericData::kBoolean, true, &s);
	MFnAttribute fnNewAttr(fxaaEnable_);
	fnNewAttr.setStorable(true);
	fnNewAttr.setKeyable(true);
	fnNewAttr.setAffectsAppearance(true);
	addAttribute(fxaaEnable_);

	return MS::kSuccess;
}
