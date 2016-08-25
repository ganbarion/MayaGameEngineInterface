//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once

#include "Common.h"
#include "bridge/DAGNode.h"

namespace bridge {

	/**
	 * MObjectHandleのHashオブジェクト
	 */
	struct MObjectHandleHash
	{
		std::size_t operator()(const MObjectHandle& key) const {
			return key.hashCode();
		}
	};


	typedef std::list<DAGNode*> DAGNodeList;
	typedef std::unordered_map<MObjectHandle, DAGNode*, MObjectHandleHash> DAGNodeMap;


	/**
	 * DAGManager
	 */
	class DAGManager
	{
	private:
		DAGManager();
		virtual ~DAGManager();

	public:
		static void CreateInstance() {
			assert(!instance_);
			instance_ = new DAGManager();
		}
		static void DestroyInstance() {
			assert(instance_);
			delete instance_;
		}
		static DAGManager* Get() { return instance_; }

	private:
		static DAGManager* instance_;

	private:
		enum CallbackType {
			CallBack_Transform,
			CallBack_Mesh,
			CallBack_ShadingEngine,
			CallBack_Texture,
			CallBack_Light,
			CallBack_Settings,

			CallBack_Max,
		};

	private:
		MCallbackId addNodeCallBackId_[CallBack_Max];
		MCallbackId removeNodeCallBackId_[CallBack_Max];
		MCallbackId timeChangedCallBackId_;

		DAGNode* settings_;
		DAGNodeList transformList_;
		DAGNodeList meshList_;
		DAGNodeList materialList_;
		DAGNodeList lightList_;

		DAGNodeMap nodeMap_;
		std::vector<DAGNode*> isolateSelectNode_;
		bool isIsolateSelected_;
		bool isTimeChanged_;

	private:
		void SetDrawFilter(MDagPath path);

	public:
		static void AddNodeCallback(MObject& node, void *clientData);
		static void RemoveNodeCallback(MObject& node, void *clientData);

	public:
		void UpdateNode();
		void DrawNode(se::GraphicsContext& context, ShadingPath path);
		void SetDrawFilter(MSelectionList list);
		void ClearDrawFilter();

		DAGNode* FindDAGNode(const MObject& object);
		DAGNode* GetSettingsNode() { return settings_; }
		bool IsIsolateSelected() const { return isIsolateSelected_; }
		void TimeChanged() { isTimeChanged_ = true; };
		bool IsTimeChanged() const { return isTimeChanged_; }

		void ForEach(std::function<void(DAGNode*)> func);
	};

}