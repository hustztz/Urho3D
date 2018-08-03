//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Core/Profiler.h"
#include "../Graphics/LodModel.h"
#include "../Graphics/GeomModel.h"
#include "../Graphics/Batch.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Geometry.h"
#include "../Graphics/Material.h"
#include "../Graphics/OcclusionBuffer.h"
#include "../Graphics/OctreeQuery.h"
#include "../Graphics/VertexBuffer.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/ResourceEvents.h"
#include "../Scene/Node.h"

#include "../DebugNew.h"

namespace Urho3D
{

extern const char* GEOMETRY_CATEGORY;

LodModel::LodModel(Context* context) :
    Drawable(context, DRAWABLE_GEOMETRY),
    materialsAttr_(Material::GetTypeStatic())
{
}

LodModel::~LodModel() = default;

void LodModel::RegisterObject(Context* context)
{
    /*context->RegisterFactory<LodModel>(GEOMETRY_CATEGORY);

    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
    URHO3D_MIXED_ACCESSOR_ATTRIBUTE("GeomModel", GetModelAttr, SetModelAttr, ResourceRef, ResourceRef(GeomModel::GetTypeStatic()), AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Material", GetMaterialsAttr, SetMaterialsAttr, ResourceRefList, ResourceRefList(Material::GetTypeStatic()),
        AM_DEFAULT);
    URHO3D_ATTRIBUTE("Is Occluder", bool, occluder_, false, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Can Be Occluded", IsOccludee, SetOccludee, bool, true, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Cast Shadows", bool, castShadows_, false, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Draw Distance", GetDrawDistance, SetDrawDistance, float, 0.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Shadow Distance", GetShadowDistance, SetShadowDistance, float, 0.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("LOD Bias", GetLodBias, SetLodBias, float, 1.0f, AM_DEFAULT);
    URHO3D_COPY_BASE_ATTRIBUTES(Drawable);*/
}

void LodModel::ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results)
{
    RayQueryLevel level = query.level_;

    switch (level)
    {
    case RAY_AABB:
        Drawable::ProcessRayQuery(query, results);
        break;

    case RAY_OBB:
    case RAY_TRIANGLE:
    case RAY_TRIANGLE_UV:
        Matrix3x4 inverse(node_->GetWorldTransform().Inverse());
        Ray localRay = query.ray_.Transformed(inverse);
        float distance = localRay.HitDistance(boundingBox_);
        Vector3 normal = -query.ray_.direction_;
        Vector2 geometryUV;
        unsigned hitBatch = M_MAX_UNSIGNED;

        if (level >= RAY_TRIANGLE && distance < query.maxDistance_)
        {
            distance = M_INFINITY;

            for (unsigned i = 0; i < batches_.Size(); ++i)
            {
                Geometry* geometry = batches_[i].geometry_;
                if (geometry)
                {
                    Vector3 geometryNormal;
                    float geometryDistance = level == RAY_TRIANGLE ? geometry->GetHitDistance(localRay, &geometryNormal) :
                        geometry->GetHitDistance(localRay, &geometryNormal, &geometryUV);
                    if (geometryDistance < query.maxDistance_ && geometryDistance < distance)
                    {
                        distance = geometryDistance;
                        normal = (node_->GetWorldTransform() * Vector4(geometryNormal, 0.0f)).Normalized();
                        hitBatch = i;
                    }
                }
            }
        }

        if (distance < query.maxDistance_)
        {
            RayQueryResult result;
            result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
            result.normal_ = normal;
            result.textureUV_ = geometryUV;
            result.distance_ = distance;
            result.drawable_ = this;
            result.node_ = node_;
            result.subObject_ = hitBatch;
            results.Push(result);
        }
        break;
    }
}

void LodModel::UpdateBatches(const FrameInfo& frame)
{
    const BoundingBox& worldBoundingBox = GetWorldBoundingBox();
    distance_ = frame.camera_->GetDistance(worldBoundingBox.Center());

    if (batches_.Size() == 1)
        batches_[0].distance_ = distance_;
    else
    {
        const Matrix3x4& worldTransform = node_->GetWorldTransform();
		for (unsigned i = 0; i < batches_.Size(); ++i)
		{
			GeomModel* model = GetModel(0);
			if (!model)
				continue;
			const PODVector<Vector3>& geometryCenters = model->GetGeometryCenters();
			batches_[i].distance_ = frame.camera_->GetDistance(worldTransform * geometryCenters[i]);
		}
    }

    float scale = worldBoundingBox.Size().DotProduct(DOT_SCALE);
    float newLodDistance = frame.camera_->GetLodDistance(distance_, scale, lodBias_);

    if (newLodDistance != lodDistance_)
    {
        lodDistance_ = newLodDistance;
        CalculateLodLevels();
    }
}

Geometry* LodModel::GetLodGeometry(unsigned batchIndex, unsigned level)
{
	// If level is out of range, use visible geometry
	if (level >= models_.Size())
		return BaseClassName::GetLodGeometry(batchIndex, level);

	const GeomModel* currentModel = GetModel(level);
	if (!currentModel)
		return nullptr;

	return currentModel->GetGeometry(batchIndex);
    
}

void LodModel::SetModel(GeomModel* model)
{
	if (!node_)
    {
        URHO3D_LOGERROR("Can not set model while model component is not attached to a scene node");
        return;
    }

	float lodDistance = model->GetLodDistance();
	ModelList::Iterator iter;
	for (iter = models_.Begin(); iter != models_.End(); ++iter)
	{
		if (model == iter->Get())
			return;
		if (lodDistance == iter->Get()->GetLodDistance())
		{
			// Unsubscribe from the reload event of previous model (if any), then subscribe to the new
			UnsubscribeFromEvent(iter->Get(), E_RELOADFINISHED);
			models_.Erase(iter);
			break;
		}
		else if (lodDistance < iter->Get()->GetLodDistance())
		{
			break;
		}
	}
	iter--;
	models_.Insert(iter, SharedPtr<GeomModel>(model));
	
    if (model)
    {
        SubscribeToEvent(model, E_RELOADFINISHED, URHO3D_HANDLER(LodModel, HandleModelReloadFinished));

        // Copy the subgeometry & LOD level structure
        SetNumGeometries(model->GetNumGeometries());
        const PODVector<Vector3>& geometryCenters = model->GetGeometryCenters();
        const Matrix3x4* worldTransform = node_ ? &node_->GetWorldTransform() : nullptr;
        for (unsigned i = 0; i < model->GetNumGeometries(); ++i)
        {
            batches_[i].worldTransform_ = worldTransform;
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
}

void LodModel::SetMaterial(Material* material)
{
    for (unsigned i = 0; i < batches_.Size(); ++i)
        batches_[i].material_ = material;

    MarkNetworkUpdate();
}

bool LodModel::SetMaterial(unsigned index, Material* material)
{
    if (index >= batches_.Size())
    {
		URHO3D_LOGERROR("Material index out of bounds");
		return false;
    }

    batches_[index].material_ = material;
    MarkNetworkUpdate();
    return true;
}

void LodModel::ApplyMaterialList(const String& fileName)
{
    String useFileName = fileName;
	if (useFileName.Trimmed().Empty() && !models_.Empty())
	{
		GeomModel* model = GetModel(0);
		if(model)
			useFileName = ReplaceExtension(model->GetName(), ".txt");
	}

    auto* cache = GetSubsystem<ResourceCache>();
    SharedPtr<File> file = cache->GetFile(useFileName, false);
    if (!file)
        return;

    unsigned index = 0;
    while (!file->IsEof() && index < batches_.Size())
    {
        auto* material = cache->GetResource<Material>(file->ReadLine());
        if (material)
            SetMaterial(index, material);

        ++index;
    }
}

Material* LodModel::GetMaterial(unsigned index) const
{
    return index < batches_.Size() ? batches_[index].material_ : nullptr;
}

unsigned LodModel::GetNumGeometries(unsigned level) const
{
	return GetModel(level) ? GetModel(level)->GetNumGeometries() : 0;
}

bool LodModel::IsInside(const Vector3& point) const
{
    if (!node_)
        return false;

    Vector3 localPosition = node_->GetWorldTransform().Inverse() * point;
    return IsInsideLocal(localPosition);
}

bool LodModel::IsInsideLocal(const Vector3& point) const
{
    // Early-out if point is not inside bounding box
    if (boundingBox_.IsInside(point) == OUTSIDE)
        return false;

    Ray localRay(point, Vector3(1.0f, -1.0f, 1.0f));

    for (unsigned i = 0; i < batches_.Size(); ++i)
    {
        Geometry* geometry = batches_[i].geometry_;
        if (geometry)
        {
            if (geometry->IsInside(localRay))
                return true;
        }
    }

    return false;
}

void LodModel::SetBoundingBox(const BoundingBox& box)
{
    boundingBox_ = box;
    OnMarkedDirty(node_);
}

void LodModel::SetNumGeometries(unsigned num)
{
    batches_.Resize(num);
    ResetLodLevels();
}

void LodModel::SetModelAttr(const ResourceRef& value)
{
	SubscribeToEvent(E_RESOURCEBACKGROUNDLOADED, URHO3D_HANDLER(LodModel, HandleResourceBackgroundLoaded));

    auto* cache = GetSubsystem<ResourceCache>();
	cache->BackgroundLoadResource<GeomModel>(value.name_, true, nullptr, id_);
    //SetModel(cache->GetResource<Model>(value.name_));
}

void LodModel::SetMaterialsAttr(const ResourceRefList& value)
{
    auto* cache = GetSubsystem<ResourceCache>();
	for (unsigned i = 0; i < value.names_.Size(); ++i)
	{
		if(i >= batches_.Size())
			SetNumGeometries(i + 1);
		SetMaterial(i, cache->GetResource<Material>(value.names_[i]));
	} 
}

ResourceRef LodModel::GetModelAttr(unsigned level) const
{
    return GetResourceRef(GetModel(level), GeomModel::GetTypeStatic());
}

const ResourceRefList& LodModel::GetMaterialsAttr() const
{
    materialsAttr_.names_.Resize(batches_.Size());
    for (unsigned i = 0; i < batches_.Size(); ++i)
        materialsAttr_.names_[i] = GetResourceName(GetMaterial(i));

    return materialsAttr_;
}

void LodModel::OnWorldBoundingBoxUpdate()
{
    worldBoundingBox_ = boundingBox_.Transformed(node_->GetWorldTransform());
}

GeomModel* LodModel::GetModel(unsigned level) const
{
	if (models_.Empty())
		return nullptr;

	ModelList::ConstIterator iter = models_.Begin();
	for (unsigned i = 0; i < level; i ++, iter ++)
	{
		if(iter == models_.End())
			break;
	}
	return iter->Get();
}

unsigned LodModel::GetLodLevel(float distance) const
{
	unsigned level = 0;
	for (ModelList::ConstIterator iter = models_.Begin(); iter != models_.End(); ++iter, level++)
	{
		GeomModel* model = iter->Get();
		if (model && distance <= model->GetLodDistance())
			break;
	}
	return level;
}

void LodModel::ResetLodLevels()
{
	if (models_.Empty())
		return;

	GeomModel* model = GetModel(0);
	if (!model)
		return;
    // Ensure that each subgeometry has at least one LOD level, and reset the current LOD level
    for (unsigned i = 0; i < batches_.Size(); ++i)
    {
        batches_[i].geometry_ = model->GetGeometry(i);
    }

    // Find out the real LOD levels on next geometry update
    lodDistance_ = M_INFINITY;
}

void LodModel::CalculateLodLevels()
{
	// If only one LOD geometry, no reason to go through the LOD calculation
	if (models_.Size() <= 1)
		return;

	unsigned level = GetLodLevel(lodDistance_);
	GeomModel* model = GetModel(level);
	if (!model)
		return;

	for (unsigned i = 0; i < batches_.Size(); ++i)
	{
		batches_[i].geometry_ = model->GetGeometry(i);
	}
}

void LodModel::HandleModelReloadFinished(StringHash eventType, VariantMap& eventData)
{
	for (ModelList::Iterator iter = models_.Begin(); iter != models_.End(); ++iter)
	{
		GeomModel* currentModel = iter->Get();
		if(!currentModel)
			continue;
		iter->Reset(); // Set null to allow to be re-set
		SetModel(currentModel);
	}
}

void LodModel::HandleResourceBackgroundLoaded(StringHash eventType, VariantMap& eventData)
{
	using namespace ResourceBackgroundLoaded;

	GeomModel* resource = dynamic_cast<GeomModel*>(eventData[P_RESOURCE].GetPtr());
	if (resource)
	{
		if (eventData[P_OWNERID].GetUInt() == id_)
		{
			SetModel(resource);
			UnsubscribeFromEvent(E_RESOURCEBACKGROUNDLOADED);
		}
	}
}

}
