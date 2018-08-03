#pragma once

#include "../Container/ArrayPtr.h"
#include "../Core/Object.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Math/Boundingbox.h"
#include "../Graphics/OctreeQuery.h"
#include "../Graphics/Geometry.h"
#include "../Graphics/Drawable.h"
#include "../Graphics/Model.h"
#include <iostream>
#include <stack>
#include <vector>
#include<algorithm>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

namespace Urho3D
{

	class IndexBuffer;
	class Ray;
	class Graphics;
	class VertexBuffer;
	class BIHTree;
	class BIHNode;
	class Model;
	enum VertexType
	{
		VERTEX_Min = 0,
		VERTEX_Max = 1,
		VERTEX_V1 = 2,
		VERTEX_V2 = 3,
		VERTEX_V3 = 4
	};
	struct URHO3D_API BIHStackData
	{
		BIHNode* node;
		float min, max;
		BIHStackData(BIHNode* Node, float Min, float Max) {
			this->node = Node;
			this->min = Min;
			this->max = Max;
		}
		/*
		BIHStackData& operator=(const BIHStackData& s)//опнпн╦╦сич  
		{
			this->node = s.node;
		}
		*/
	};
	//自定义“小于”
	inline bool CustomCompare(RayQueryResult& a, RayQueryResult& b) {
		return a.distance_ < b.distance_;
	}
	/// Vector of BIHNode.
	using BIHNodeVector = Vector<BIHNode>;

	class URHO3D_API BIHTree : public Component
	{
		URHO3D_OBJECT(BIHTree, Component);
	public:
		//BIHTree();
		/// Construct.
		explicit BIHTree(Context* context);
		~BIHTree() override;
		/// Register object factory.
		static void RegisterObject(Context* context);
		void Init(int l, int r);
		float CreateBihCollide(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results);
		float CollideWithRay(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results);
		float getMinMax(BoundingBox* bbox, bool doMin, int axis);
		float getVectorValueByIndex(Vector3 &vec, int index)
		{
			switch (index)
			{
			case 0:
				return vec.x_;
			case 1:
				return vec.y_;
			case 2:
				return vec.z_;
			default:
				return 0;
			}			
		}
		int intersectWhere(Ray r, float sceneMin, float sceneMax, PODVector<RayQueryResult>& results);
		int sortTriangles(int l, int r, float split, int axis);
		void getTriangle(int index, Vector3& v1, Vector3& v2, Vector3& v3);
		BoundingBox* createBox(int l, int r);
		BIHNode* createNode(int l, int r, BoundingBox* nodeBbox, int depth);
		void setMinMax(BoundingBox* bbox, bool doMin, int axis, float value);
		void SetModel(Model* model);
		/// Return model.
		Model* GetModel() const { return model_; }
		void SetNumGeometries(unsigned num);
		void ResetLodLevels();
		void SetBoundingBox(const BoundingBox& box);
		void HandleModelReloadFinished(StringHash eventType, VariantMap& eventData);
		void RaycastSingle(RayOctreeQuery& query, PODVector<RayQueryResult>& results);
		float createCollisionData(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results);
		void construct();
		void SetBinFileName(String str) { m_binFileName = str; };
		String GetBinFileName() const { return m_binFileName; };
		/// Set model attribute.
		void SetModelAttr(const ResourceRef& value);
		/// Return model attribute.
		ResourceRef GetModelAttr() const;

		/// Return all the obstacle avoidance types configured in the crowd as attribute.
		VariantVector GetBIHTreeTypesAttr() const;
		void SetBIHTreeTypesAttr(const VariantVector& value);

		vector<Vector3> tempVars;
		stack<BIHStackData*> stackData;
		vector<Vector3> originalVertices;
		PODVector<BIHNode*> bihNodeList;
		String m_binFileName;
	protected:
		/// Model.
		SharedPtr<Model> model_;
		/// Draw call source data.
		Vector<SourceBatch> batches_;
		/// All geometries.
		Vector<Vector<SharedPtr<Geometry> > > geometries_;
		/// Local-space bounding box.
		BoundingBox boundingBox_;
		/// LOD scaled distance.
		float lodDistance_;
		bool m_IsBuildTree;
	private:
		int maxTrisPerNode = 21;
		int max_tree_depth = 100;
		float ONE_THIRD = 1.0f / 3.0f;
		BIHNode* root;
	};
	class URHO3D_API BIHNode
	{
	public:
		BIHNode(int axis) {
			this->left = nullptr;
			this->right = nullptr;
			this->axis = axis;
		}
		BIHNode(int l, int r)
		{
			this->left = nullptr;
			this->right = nullptr;
			this->leftIndex = l;
			this->rightIndex = r;
			this->axis = 3; // indicates leaf
		}
		BIHNode(int l, int r,int axis)
		{
			this->left = nullptr;
			this->right = nullptr;
			this->leftIndex = l;
			this->rightIndex = r;
			this->axis = axis; // indicates leaf
		}
		BIHNode* getLeftChild() {
			return this->left;
		}

		void setLeftChild(BIHNode* left) {
			this->left = left;
		}

		float getLeftPlane() {
			return leftPlane;
		}

		void setLeftPlane(float leftPlane) {
			this->leftPlane = leftPlane;
		}

		BIHNode* getRightChild() {
			return this->right;
		}

		void setRightChild(BIHNode* right) {
			this->right = right;
		}

		float getRightPlane() {
			return rightPlane;
		}

		void setRightPlane(float rightPlane) {
			this->rightPlane = rightPlane;
		}

		Vector3 ComputeTriangleNormal(Vector3 v1, Vector3 v2, Vector3 v3);

		/*
		const Vector3 getVertex01() {
			return vertex01;
		}
		*/

		/// Construct from a variant vector.
		BIHNode(const BIHNodeVector& value) // NOLINT(google-explicit-constructor)
		{
			*this = value;
		}

		/// Assign from a BIHNode vector.
		BIHNode& operator =(const BIHNodeVector& rhs)
		{
			return *this;
		}

		/// Empty variant vector.
		static const BIHNodeVector emptyBIHNodeVector;
		
		int leftIndex, rightIndex;
		float leftPlane;
		float rightPlane;
		BIHNode* left;
		BIHNode* right;
		Vector3 vertex01;
		Vector3 vertex02;
		Vector3 vertex03;
		int axis;
	private:

	};

}