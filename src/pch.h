//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include <cstdint>
#include <stdio.h>
#include <assert.h>
#include <memory>
#include <functional>
#include <unordered_set>
#include <unordered_map>

#include <maya/MString.h>
#include <maya/MColor.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MRenderTargetManager.h>
#include <maya/MShaderManager.h>
#include <maya/MDGMessage.h>
#include <maya/MObject.h>
#include <maya/MObjectHandle.h>
#include <maya/MNodeMessage.h>
#include <maya/MDagMessage.h>
#include <maya/MStateManager.h>
#include <maya/MGeometryRequirements.h>
#include <maya/MHWGeometry.h>
#include <maya/MGeometryExtractor.h>
#include <maya/MDrawContext.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/M3dView.h>
#include <maya/MFloatMatrix.h>
#include <maya/MAngle.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MNodeClass.h>
#include <maya/MPxNode.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MUserData.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MDistance.h>
#include <maya/MDrawRegistry.h>
#include <maya/MEventMessage.h>
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MItInstancer.h>

#include <maya/MFnMesh.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnCamera.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnInstancer.h>
#include <maya/MFnSet.h>

#include "engine/engine.h"
#include "Common.h"
