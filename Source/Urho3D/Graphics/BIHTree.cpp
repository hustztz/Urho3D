#include "../Precompiled.h"

#include "../Graphics/Geometry.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/IndexBuffer.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/Log.h"
#include "../Math/Ray.h"
#include "../Graphics/BIHTree.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"
#include "../core/Object.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Core/Context.h"
#include "../DebugNew.h"

#include "../Core/CoreEvents.h"
#include "../Core/Profiler.h"
#include "../Core/Thread.h"
#include "../Core/WorkQueue.h"
#include "../Graphics/DebugRenderer.h"
//#include "../Graphics/Octree.h"
#include "../Scene/Scene.h"
#include "../Scene/SceneEvents.h"
#include "../Graphics/StaticModel.h"
#include "../Container/Vector.h"

#include<iostream>
#include<stdio.h>
#include<stack>
#include<queue>
#include<malloc.h>
using namespace std;

namespace Urho3D
{
const char* BIHTREE_CATEGORY = "Geometry";
const BIHNodeVector BIHNode::emptyBIHNodeVector{};
static const StringVector BIHTreeTypesStructureElementNames =
{
	"leftIndex",
	"rightIndex",
	"axis",
	"vertex01.x_",
	"vertex01.y_",
	"vertex01.z_",
	"vertex02.x_",
	"vertex02.y_",
	"vertex02.z_",
	"vertex03.x_",
	"vertex03.y_",
	"vertex03.z_"
};

BIHTree::BIHTree(Context* context):
Component(context),
model_(nullptr),
lodDistance_(0.0f),
m_IsBuildTree(false),
maxTrisPerNode(21),
max_tree_depth(100),
ONE_THIRD(1.0f / 3.0f),
root(nullptr)
{
	//root = NULL;
	//m_IsBuildTree = false;
	//memset(&bihNodeList, 0, sizeof(BIHNode*));
}
BIHTree::~BIHTree() = default;

void BIHTree::RegisterObject(Context* context)
{
	context->RegisterFactory<BIHTree>(BIHTREE_CATEGORY);
	URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
	URHO3D_MIXED_ACCESSOR_ATTRIBUTE("Model", GetModelAttr, SetModelAttr, ResourceRef, ResourceRef(Model::GetTypeStatic()), AM_DEFAULT);
	URHO3D_COPY_BASE_ATTRIBUTES(BIHTree);
	URHO3D_ACCESSOR_ATTRIBUTE("BIHTreeTypes", GetBIHTreeTypesAttr, SetBIHTreeTypesAttr, VariantVector, Variant::emptyVariantVector, AM_BINONLY)
		.SetMetadata(AttributeMetadata::P_VECTOR_STRUCT_ELEMENTS, BIHTreeTypesStructureElementNames);
	URHO3D_ACCESSOR_ATTRIBUTE("binFile", GetBinFileName, SetBinFileName, String, String::EMPTY, AM_DEFAULT);
}

void BIHTree::SetModelAttr(const ResourceRef& value)
{
	auto* cache = GetSubsystem<ResourceCache>();
	SetModel(cache->GetResource<Model>(value.name_));
}

ResourceRef BIHTree::GetModelAttr() const
{
	return GetResourceRef(model_, Model::GetTypeStatic());
}

VariantVector BIHTree::GetBIHTreeTypesAttr() const
{
	VariantVector ret;
	queue<BIHNode*> queue;
	BIHNode* temp = new BIHNode(0);
	*temp = *root;
	queue.push(temp);

	while (!queue.empty()) { 

		temp = queue.front();
		ret.Push(temp->leftIndex);
		ret.Push(temp->rightIndex);
		ret.Push(temp->axis);

		ret.Push(temp->vertex01.x_);
		ret.Push(temp->vertex01.y_);
		ret.Push(temp->vertex01.z_);
		ret.Push(temp->vertex02.x_);
		ret.Push(temp->vertex02.y_);
		ret.Push(temp->vertex02.z_);
		ret.Push(temp->vertex03.x_);
		ret.Push(temp->vertex03.y_);
		ret.Push(temp->vertex03.z_);
		
		VectorBuffer ret01;
		ret01.WriteVector3(temp->vertex01);
		ret01.WriteVector3(temp->vertex02);
		ret01.WriteVector3(temp->vertex03);
		//ret.Push(ret01.GetBuffer());

		queue.pop();

		if (temp->left != nullptr) {
			queue.push(temp->left);
		}

		if (temp->right != nullptr) {
			queue.push(temp->right);
		}
	}
	return ret;
}

void BIHTree::SetBIHTreeTypesAttr(const VariantVector& value)
{
	unsigned index = 0;
	unsigned bihType = 0;
	bihNodeList.Clear();
	//int numBihTreeTypes_ = index < value.Size() ? Min(value[index++].GetUInt(), (unsigned)8) : 0;
	//int numBihTreeTypes_ = value.Size();
	//while (bihType < numBihTreeTypes_)
	{
		while (index + 11 <= value.Size())
		{
			BIHNode* params = new BIHNode(0);       // NOLINT(hicpp-member-init)
			params->leftIndex = value[index++].GetInt();
			params->rightIndex = value[index++].GetInt();
			params->axis = value[index++].GetInt();
			params->vertex01.x_ = value[index++].GetFloat();
			params->vertex01.y_ = value[index++].GetFloat();
			params->vertex01.z_ = value[index++].GetFloat();
			params->vertex02.x_ = value[index++].GetFloat();
			params->vertex02.y_ = value[index++].GetFloat();
			params->vertex02.z_ = value[index++].GetFloat();
			params->vertex03.x_ = value[index++].GetFloat();
			params->vertex03.y_ = value[index++].GetFloat();
			params->vertex03.z_ = value[index++].GetFloat();
			//memcpy(&bihNodeList[bihType], params, sizeof(BIHNode*));
			bihNodeList.Push(params);
		}
		//++bihType;
	}
}

void BIHTree::SetModel(Model* model)
{
	if (model == model_)
		return;

	if (!node_)
	{
		URHO3D_LOGERROR("Can not set model while model component is not attached to a scene node");
		return;
	}

	// Unsubscribe from the reload event of previous model (if any), then subscribe to the new
	if (model_)
		UnsubscribeFromEvent(model_, E_RELOADFINISHED);

	model_ = model;
	if (model)
	{
		SubscribeToEvent(model, E_RELOADFINISHED, URHO3D_HANDLER(BIHTree, HandleModelReloadFinished));

		// Copy the subgeometry & LOD level structure
		SetNumGeometries(model->GetNumGeometries());
		const Vector<Vector<SharedPtr<Geometry> > >& geometries = model->GetGeometries();
		const PODVector<Vector3>& geometryCenters = model->GetGeometryCenters();
		const Matrix3x4* worldTransform = node_ ? &node_->GetWorldTransform() : nullptr;
		for (unsigned i = 0; i < geometries.Size(); ++i)
		{
			batches_[i].worldTransform_ = worldTransform;
			geometries_[i] = geometries[i];
			//geometryData_[i].center_ = geometryCenters[i];
		}

		SetBoundingBox(model->GetBoundingBox());
		ResetLodLevels();
	}
	else
	{
		SetNumGeometries(0);
		SetBoundingBox(BoundingBox());
	}

	MarkNetworkUpdate();
	construct();
}

void BIHTree::SetNumGeometries(unsigned num)
{
	batches_.Resize(num);
	geometries_.Resize(num);
	//geometryData_.Resize(num);
	ResetLodLevels();
}

void BIHTree::ResetLodLevels()
{
	// Ensure that each subgeometry has at least one LOD level, and reset the current LOD level
	for (unsigned i = 0; i < batches_.Size(); ++i)
	{
		if (!geometries_[i].Size())
			geometries_[i].Resize(1);
		batches_[i].geometry_ = geometries_[i][0];
		//geometryData_[i].lodLevel_ = 0;
	}

	// Find out the real LOD levels on next geometry update
	lodDistance_ = M_INFINITY;
}

void BIHTree::SetBoundingBox(const BoundingBox& box)
{
	boundingBox_ = box;
	OnMarkedDirty(node_);
}

void BIHTree::HandleModelReloadFinished(StringHash eventType, VariantMap& eventData)
{
	Model* currentModel = model_;
	model_.Reset(); // Set null to allow to be re-set
	SetModel(currentModel);
}

void BIHTree::RaycastSingle(RayOctreeQuery& query, PODVector<RayQueryResult>& results) 
{
	Matrix3x4 inverse(node_->GetWorldTransform().Inverse());
	Ray localRay = query.ray_.Transformed(inverse);
	float distance = localRay.HitDistance(boundingBox_);
	Vector3 normal = -query.ray_.direction_;
	Vector2 geometryUV;
	unsigned hitBatch = M_MAX_UNSIGNED;

	if (distance < query.maxDistance_)
	{
		distance = M_INFINITY;

		//for (unsigned i = 0; i < batches_.Size(); ++i)
		{
			//Geometry* geometry = batches_[i].geometry_;
			//if (geometry)
			{
				//Vector3 geometryNormal;

				int resultCount = createCollisionData(localRay, boundingBox_, results);
				URHO3D_LOGDEBUG(String("resultCount= ") + resultCount);
				if (resultCount > 0)
				{
					int index = 1;
					for (PODVector<RayQueryResult>::Iterator j = results.Begin(); j != results.End(); ++j)
					{
						RayQueryResult& result = *j;
						result.position_ = query.ray_.origin_ + result.distance_ * query.ray_.direction_;
						//result.drawable_ = this;
						result.node_ = node_;
						//Model* model = node_->GetComponent<StaticModel>()->GetModel();
						URHO3D_LOGDEBUG(String("index_= ") + (index));
						URHO3D_LOGDEBUG(String("result.distance_= ") + (result.distance_));
						URHO3D_LOGDEBUG(String("query.ray_.origin_.x_= ") + (query.ray_.origin_.x_));
						URHO3D_LOGDEBUG(String("query.ray_.origin_.y_= ") + (query.ray_.origin_.y_));
						URHO3D_LOGDEBUG(String("query.ray_.origin_.z_= ") + (query.ray_.origin_.z_));
						URHO3D_LOGDEBUG(String("query.ray_.direction_.x_= ") + (query.ray_.direction_.x_));
						URHO3D_LOGDEBUG(String("query.ray_.direction_.y_= ") + (query.ray_.direction_.y_));
						URHO3D_LOGDEBUG(String("query.ray_.direction_.z_= ") + (query.ray_.direction_.z_));
						URHO3D_LOGDEBUG(String("result.position_.x_= ") + (result.position_.x_));
						URHO3D_LOGDEBUG(String("result.position_.y_= ") + (result.position_.y_));
						URHO3D_LOGDEBUG(String("result.position_.z_= ") + (result.position_.z_));
						index++;
					}
				}
			}
		}
	}
}

float BIHTree::createCollisionData(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results)
{
	construct();
	return CollideWithRay(ray, boundingBox, results);

}

void BIHTree::construct()
{
	const unsigned char* vertexData;
	const unsigned char* indexData;
	unsigned vertexSize;
	unsigned indexSize;
	const PODVector<VertexElement>* elements;
	if (node_->GetComponent<StaticModel>())
	{
		Vector<SourceBatch> batch = node_->GetComponent<StaticModel>()->GetBatches();
		batches_ = batch;
	}
	//for (unsigned i = 0; i < geometries_.Size(); ++i)
	for (unsigned i = 0; i < batches_.Size(); ++i)
	{
		Geometry* geometry = batches_[i].geometry_;
		//Geometry* geometry = geometries_[i][0];
		geometry->GetRawData(vertexData, vertexSize, indexData, indexSize, elements);
		if (!vertexData || !elements || VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR3, SEM_POSITION) != 0)
			return;
		if (!m_IsBuildTree)
		{
			//tree = new BIHTree();
			float nearest = M_INFINITY;
			const auto* vertices = (const unsigned char*)vertexData;

			const unsigned short* indices = ((const unsigned short*)indexData) + geometry->GetIndexStart();
			const unsigned short* indicesEnd = indices + geometry->GetIndexCount();
			const unsigned short* nearestIndices = nullptr;
			while (indices < indicesEnd)
			{
				Vector3& v0 = *((Vector3*)(&vertices[indices[0] * vertexSize]));
				Vector3& v1 = *((Vector3*)(&vertices[indices[1] * vertexSize]));
				Vector3& v2 = *((Vector3*)(&vertices[indices[2] * vertexSize]));
				originalVertices.push_back(v0);
				originalVertices.push_back(v1);
				originalVertices.push_back(v2);
				indices += 3;
			}
			Init(0, geometry->GetIndexCount() / 3 - 1);
			m_IsBuildTree = true;
		}
	}
}

void BIHTree::Init(int l, int r)
{
	BoundingBox* sceneBbox = createBox(l, r);
	root = createNode(0, r, sceneBbox, 0);
}
	
float BIHTree::CollideWithRay(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results)
{
	float tMin = ray.HitDistance(boundingBox);
	float tMax = ray.HitDistanceFarthest(boundingBox);
	if (tMax <= 0)
	{
		tMax = M_INFINITY;
	}
	else if (tMin == tMax)
	{
		tMin = 0;
	}
	if (tMin <= 0)
	{
		tMin = 0;
	}
	return intersectWhere(ray, tMin, tMax, results);
}

void BIHTree::setMinMax(BoundingBox* bbox, bool doMin, int axis, float value)
{
	//Vector3* min = bbox->getMin();
	//Vector3* max = bbox->getMax();
	Vector3 min;
	bbox->getMin(min);
	Vector3 max;
	bbox->getMax(max);

	if (doMin)
	{
		min.set(axis, value);
	}
	else
	{
		max.set(axis, value);
	}

	bbox->setMinMax(min, max);
}

float BIHTree::getMinMax(BoundingBox* bbox, bool doMin, int axis)
{
	if (doMin)
	{
		Vector3 vec;
		bbox->getMin(vec);
		return getVectorValueByIndex(vec, axis);
	}
	else
	{
		Vector3 vec;
		bbox->getMax(vec);
		return getVectorValueByIndex(vec, axis);
	}
}

BoundingBox* BIHTree::createBox(int l, int r)
{
	Vector3* min = new Vector3(M_INFINITY, M_INFINITY, M_INFINITY);
	Vector3* max = new Vector3(-M_INFINITY, -M_INFINITY, -M_INFINITY);
	/*
	Vector3* v1 = new Vector3(Vector3::ZERO);
	Vector3* v2 = new Vector3(Vector3::ZERO);
	Vector3* v3 = new Vector3(Vector3::ZERO);
	*/
	Vector3 v1, v2, v3;
	if (tempVars.empty())
	{
		tempVars.push_back(*min);
		tempVars.push_back(*max);
		tempVars.push_back(v1);
		tempVars.push_back(v2);
		tempVars.push_back(v3);
	}
	else
	{
		tempVars[VertexType::VERTEX_Min] = *min;
		tempVars[VertexType::VERTEX_Max] = *max;
		tempVars[VertexType::VERTEX_V1] = v1;
		tempVars[VertexType::VERTEX_V2] = v2;
		tempVars[VertexType::VERTEX_V3] = v3;
	}
	for (int i = l; i <= r; i++)
	{
		getTriangle(i, v1, v2, v3);
		BoundingBox::checkMinMax(*min, *max, v1);
		BoundingBox::checkMinMax(*min, *max, v2);
		BoundingBox::checkMinMax(*min, *max, v3);
	}

	BoundingBox* bbox = new BoundingBox(*min, *max);
	return bbox;
}

int BIHTree::sortTriangles(int l, int r, float split, int axis)
{
	int pivot = l;
	int j = r;
	Vector3 v1, v2, v3;
	//Vector3* v2 = new Vector3(Vector3::ZERO);
	//Vector3* v3 = new Vector3(Vector3::ZERO);
	tempVars[VertexType::VERTEX_V1] = v1;
	tempVars[VertexType::VERTEX_V2] = v2;
	tempVars[VertexType::VERTEX_V3] = v3;
	while (pivot <= j)
	{
		getTriangle(pivot, v1, v2, v3);
		v1 += v2; v1 += v3; v1 *= ONE_THIRD;
		if (getVectorValueByIndex(v1, axis) > split)
		{
			//swapTriangles(pivot, j);
			--j;
		}
		else
		{
			++pivot;
		}
	}
	pivot = (pivot == l && j < pivot) ? j : pivot;
	return pivot;
}

void BIHTree::getTriangle(int index, Vector3& v1, Vector3& v2, Vector3& v3)
{
	int pointIndex = index * 3;
	v1 = originalVertices[pointIndex++];
	v2 = originalVertices[pointIndex++];
	v3 = originalVertices[pointIndex++];
}

BIHNode* BIHTree::createNode(int l, int r, BoundingBox* nodeBbox, int depth)
{
	if ((r - l) < maxTrisPerNode || depth > max_tree_depth)
	{
		return (new BIHNode(l, r));
	}
	BoundingBox* currentBox = createBox(l, r);
	Vector3 exteriorExt = nodeBbox->Extent();
	Vector3 interiorExt = currentBox->Extent();
	exteriorExt -= interiorExt;
	int axis = 0;
	if (exteriorExt.x_ > exteriorExt.y_)
	{
		if (exteriorExt.x_ > exteriorExt.z_)
		{
			axis = 0;
		}
		else
		{
			axis = 2;
		}
	}
	else
	{
		if (exteriorExt.y_ > exteriorExt.z_)
		{
			axis = 1;
		}
		else
		{
			axis = 2;
		}
	}
	if (exteriorExt == (Vector3::ZERO))
	{
		axis = 0;
	}
	float split = getVectorValueByIndex(currentBox->Center(), axis);
	int pivot = sortTriangles(l, r, split, axis);
	if (pivot == l || pivot == r)
	{
		pivot = (r + l) / 2;
	}
	// If one of the partitions is empty, continue with recursion: same level but different bbox
	if (pivot < l)
	{
		// Only right
		BoundingBox* rbbox = new BoundingBox(currentBox);
		setMinMax(rbbox, true, axis, split);
		return createNode(l, r, rbbox, depth + 1);
		//createNode(node,l, r, rbbox, depth + 1);
	}
	else if (pivot > r)
	{
		// Only left
		BoundingBox* lbbox = new BoundingBox(currentBox);
		setMinMax(lbbox, false, axis, split);
		return createNode(l, r, lbbox, depth + 1);
		//createNode(node,l, r, lbbox, depth + 1);
	}
	else
	{
		// Build the node
		//BIHNode* node = new BIHNode(axis);
		BIHNode* node = new BIHNode(l,r,axis);
		Vector3 v1, v2, v3;
		getTriangle(pivot, v1, v2, v3);
		node->vertex01 = v1;
		node->vertex02 = v2;
		node->vertex03 = v3;

		// Left child
		BoundingBox* lbbox = new BoundingBox(currentBox);
		setMinMax(lbbox, false, axis, split);

		// The left node right border is the plane most right
		node->setLeftPlane(getMinMax(createBox(l, Max(l, pivot - 1)), false, axis));
		node->setLeftChild(createNode(l, Max(l, pivot - 1), lbbox, depth + 1)); // Recursive call

		BoundingBox* rbbox = new BoundingBox(currentBox);
		setMinMax(rbbox, true, axis, split);
		// The right node left border is the plane most left
		node->setRightPlane(getMinMax(createBox(pivot, r), true, axis));
		node->setRightChild(createNode(pivot, r, rbbox, depth + 1)); // Recursive call
																		//node->parent = node;
		return node;
	}
}

Vector3 BIHNode::ComputeTriangleNormal(Vector3 v1, Vector3 v2, Vector3 v3)
{
	Vector3 vec;
	vec.x_ = (v2.y_ - v1.y_)*(v3.z_ - v1.z_) - (v2.z_ - v1.z_) * (v3.y_ - v1.y_);
	vec.y_ = (v2.z_ - v1.z_)*(v3.x_ - v1.x_) - (v2.x_ - v1.x_) * (v3.z_ - v1.z_);
	vec.z_ = (v2.x_ - v1.x_)*(v3.y_ - v1.y_) - (v2.y_ - v1.y_) * (v3.x_ - v1.x_);
	return vec;
}

int BIHTree::intersectWhere(Ray r, float sceneMin, float sceneMax, PODVector<RayQueryResult>& results)
{
	Vector3 o = r.origin_;
	Vector3 d = r.direction_;

	float origins[] = { r.origin_.x_,
		r.origin_.y_,
		r.origin_.z_ };

	float invDirections[] = { 1.0f / r.direction_.x_,
		1.0f / r.direction_.y_,
		1.0f / r.direction_.z_ };

	r.direction_.Normalized();

	Vector3* v1 = new Vector3(Vector3::ZERO);
	Vector3* v2 = new Vector3(Vector3::ZERO);
	Vector3* v3 = new Vector3(Vector3::ZERO);
	tempVars[VertexType::VERTEX_V1] = *v1;
	tempVars[VertexType::VERTEX_V2] = *v2;
	tempVars[VertexType::VERTEX_V3] = *v3;
	int cols = 0;
	BIHNode* temp = new BIHNode(0);
	*temp = *root;
	stackData.push(new BIHStackData(root, sceneMin, sceneMax));
	while (stackData.size() > 0) {

		//BIHStackData data = stack.remove(stack.size() - 1);
		BIHStackData* data = stackData.top();
		stackData.pop();
		BIHNode* node = data->node;
		float tMin = data->min,
			tMax = data->max;

		if (tMax < tMin) {
			continue;
		}

		while (node->axis != 3) { // while node is not a leaf
			int a = node->axis;

			// find the origin and direction value for the given axis
			float origin = origins[a];
			float invDirection = invDirections[a];

			float tNearSplit, tFarSplit;
			BIHNode* nearNode;
			BIHNode* farNode;

			tNearSplit = (node->leftPlane - origin) * invDirection;
			tFarSplit = (node->rightPlane - origin) * invDirection;
			nearNode = node->left;
			farNode = node->right;

			if (invDirection < 0) {
				float tmpSplit = tNearSplit;
				tNearSplit = tFarSplit;
				tFarSplit = tmpSplit;

				BIHNode tmpNode = *nearNode;
				nearNode = farNode;
				farNode = &tmpNode;
			}

			if (tMin > tNearSplit && tMax < tFarSplit) {
				continue;
			}

			if (tMin > tNearSplit) {
				tMin = Max(tMin, tFarSplit);
				*node = *farNode;
			}
			else if (tMax < tFarSplit) {
				tMax = Min(tMax, tNearSplit);
				*node = *nearNode;
			}
			else {
				stackData.push(new BIHStackData(farNode, Max(tMin, tFarSplit), tMax));
				//stack.add(new BIHStackData(farNode, Max(tMin, tFarSplit), tMax));
				tMax = Min(tMax, tNearSplit);
				*node = *nearNode;
			}
		}
		for (int i = node->leftIndex; i <= node->rightIndex; i++) {
			Vector3 v1, v2, v3;
			getTriangle(i, v1, v2, v3);
			Vector3 normal;
			Vector2 outUV;
			float distance = r.Intersect(v1, v2, v3, normal, outUV);
			if (distance != M_INFINITY) {
				Vector3 contactNormal = node->ComputeTriangleNormal(v1, v2, v3);
				Vector3 temp;
				temp = d * distance + o;
				Vector3 contactPoint = temp;
				Vector2 geometryUV = Vector2::ZERO;
				RayQueryResult cr;
				cr.distance_ = distance;
				cr.normal_ = normal;
				cr.position_ = contactPoint;
				cr.textureUV_ = outUV;
				cr.subObject_ = cols;
				results.Push(cr);
				cols++;
			}
		}
	}
	r.origin_ = o;
	r.direction_ = d;
	*root = *temp;
	return cols;
}

}