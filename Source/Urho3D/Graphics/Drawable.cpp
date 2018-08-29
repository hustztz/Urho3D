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
#include "../Graphics/Camera.h"
#include "../Graphics/DebugRenderer.h"
#include "../IO/File.h"
#include "../Graphics/Geometry.h"
#include "../Graphics/Material.h"
#include "../Graphics/Octree.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/VertexBuffer.h"
#include "../Graphics/Zone.h"
#include "../IO/Log.h"
#include "../Scene/Scene.h"
//#include "../Graphics/Technique.h"
#include "../Core/Thread.h"
#include "../Core/CoreEvents.h"
//#include "../Resource/XMLFile.h"
//#include "../Resource/JSONFile.h"
#include "../Graphics/Graphics.h"
#include "../Resource/XMLFile.h"
#include "../Resource/JSONFile.h"

#include "../DebugNew.h"

#ifdef _MSC_VER
#pragma warning(disable:6293)
#endif

namespace Urho3D
{
///V7.0迁移Mesh
const char*	V7_MESH_CATEGORY = "V7_Mesh";

const char* GEOMETRY_CATEGORY = "Geometry";

//SourceBatch::SourceBatch() = default;
SourceBatch::SourceBatch()
{
	tree_ = NULL;
}

//SourceBatch::SourceBatch(const SourceBatch& batch) = default;
SourceBatch::SourceBatch(const SourceBatch& batch)
{
	tree_ = NULL;
}

SourceBatch::~SourceBatch() = default;

SourceBatch& SourceBatch::operator =(const SourceBatch& rhs)= default;

OverrideShaderParameterAnimationInfo::OverrideShaderParameterAnimationInfo(Drawable* drawable, const String& name, int batchIndex, ValueAnimation* attributeAnimation,
	WrapMode wrapMode, float speed) :
	ValueAnimationInfo(drawable, attributeAnimation, wrapMode, speed),
	name_(name),
	batchIndex_(batchIndex)
{
}

OverrideShaderParameterAnimationInfo::OverrideShaderParameterAnimationInfo(const OverrideShaderParameterAnimationInfo& other) = default;

OverrideShaderParameterAnimationInfo::~OverrideShaderParameterAnimationInfo() = default;

void OverrideShaderParameterAnimationInfo::ApplyValue(const Variant& newValue)
{
	static_cast<Drawable*>(target_.Get())->SetOverrideShaderParameter(batchIndex_, name_, newValue);
}

Drawable::Drawable(Context* context, unsigned char drawableFlags) :
	Component(context),
	boundingBox_(0.0f, 0.0f),
	drawableFlags_(drawableFlags),
	worldBoundingBoxDirty_(true),
	castShadows_(false),
	occluder_(false),
	occludee_(true),
	updateQueued_(false),
	zoneDirty_(false),
	octant_(nullptr),
	zone_(nullptr),
	viewMask_(DEFAULT_VIEWMASK),
	overrideViewMask_(DEFAULT_VIEWMASK),
	lightMask_(DEFAULT_LIGHTMASK),
	shadowMask_(DEFAULT_SHADOWMASK),
	zoneMask_(DEFAULT_ZONEMASK),
	viewFrameNumber_(0),
	distance_(0.0f),
	lodDistance_(0.0f),
	drawDistance_(0.0f),
	shadowDistance_(0.0f),
	sortValue_(0.0f),
	minZ_(0.0f),
	maxZ_(0.0f),
	lodBias_(1.0f),
	basePassFlags_(0),
	maxLights_(0),
	firstLight_(nullptr),
	dynamicType_(Static),
	overrideTechnique_(nullptr),
	overrideFillMode_(FILL_SOLID)
{
    if (drawableFlags == DRAWABLE_UNDEFINED || drawableFlags > DRAWABLE_ANY)
    {
        URHO3D_LOGERROR("Drawable with undefined drawableFlags");
    }
}

Drawable::~Drawable()
{
    RemoveFromOctree();
}

const char* ObjectDynamicType[] =
{
	"Dynamic",
	"Static",
	nullptr
};


void Drawable::RegisterObject(Context* context)
{
	URHO3D_ENUM_ATTRIBUTE("Dynamic Type", dynamicType_, ObjectDynamicType, Static, AM_FILE);
    URHO3D_ATTRIBUTE("Max Lights", int, maxLights_, 0, AM_DEFAULT);
    URHO3D_ATTRIBUTE("View Mask", int, viewMask_, DEFAULT_VIEWMASK, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Light Mask", int, lightMask_, DEFAULT_LIGHTMASK, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Shadow Mask", int, shadowMask_, DEFAULT_SHADOWMASK, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Zone Mask", GetZoneMask, SetZoneMask, unsigned, DEFAULT_ZONEMASK, AM_DEFAULT);
}

void Drawable::OnSetEnabled()
{
    bool enabled = IsEnabledEffective();

    if (enabled && !octant_)
        AddToOctree();
    else if (!enabled && octant_)
        RemoveFromOctree();
}

void Drawable::ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results)
{
    float distance = query.ray_.HitDistance(GetWorldBoundingBox());
    if (distance < query.maxDistance_)
    {
        RayQueryResult result;
        result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
        result.normal_ = -query.ray_.direction_;
        result.distance_ = distance;
        result.drawable_ = this;
        result.node_ = GetNode();
        result.subObject_ = M_MAX_UNSIGNED;
        results.Push(result);
    }
}

void Drawable::UpdateBatches(const FrameInfo& frame)
{
    const BoundingBox& worldBoundingBox = GetWorldBoundingBox();
    const Matrix3x4& worldTransform = node_->GetWorldTransform();
    distance_ = frame.camera_->GetDistance(worldBoundingBox.Center());

    for (unsigned i = 0; i < batches_.Size(); ++i)
    {
        batches_[i].distance_ = distance_;
        batches_[i].worldTransform_ = &worldTransform;
    }

    float scale = worldBoundingBox.Size().DotProduct(DOT_SCALE);
    float newLodDistance = frame.camera_->GetLodDistance(distance_, scale, lodBias_);

    if (newLodDistance != lodDistance_)
        lodDistance_ = newLodDistance;
}

Geometry* Drawable::GetLodGeometry(unsigned batchIndex, unsigned level)
{
    // By default return the visible batch geometry
    if (batchIndex < batches_.Size())
        return batches_[batchIndex].geometry_;
    else
        return nullptr;
}

bool Drawable::DrawOcclusion(OcclusionBuffer* buffer)
{
    return true;
}

void Drawable::DrawDebugGeometry(DebugRenderer* debug, bool depthTest)
{
    if (debug && IsEnabledEffective())
        debug->AddBoundingBox(GetWorldBoundingBox(), Color::GREEN, depthTest);
}

void Drawable::SetDrawDistance(float distance)
{
    drawDistance_ = distance;
    MarkNetworkUpdate();
}

void Drawable::SetShadowDistance(float distance)
{
    shadowDistance_ = distance;
    MarkNetworkUpdate();
}

void Drawable::SetLodBias(float bias)
{
    lodBias_ = Max(bias, M_EPSILON);
    MarkNetworkUpdate();
}

void Drawable::SetViewMask(unsigned mask)
{
    viewMask_ = mask;
    MarkNetworkUpdate();
}

void Drawable::SetOverrideViewMask(unsigned mask)
{
	overrideViewMask_ = mask;
	MarkNetworkUpdate();
}

void Drawable::CancelOverrideViewMask()
{
	overrideViewMask_ = DEFAULT_VIEWMASK;
	MarkNetworkUpdate();
}

	void Drawable::SetLightMask(unsigned mask)
{
    lightMask_ = mask;
    MarkNetworkUpdate();
}

void Drawable::SetShadowMask(unsigned mask)
{
    shadowMask_ = mask;
    MarkNetworkUpdate();
}

void Drawable::SetZoneMask(unsigned mask)
{
    zoneMask_ = mask;
    // Mark dirty to reset cached zone
    OnMarkedDirty(node_);
    MarkNetworkUpdate();
}
void Drawable::SetDynamicType(DynamicType type)
{
	dynamicType_ = type;
	MarkNetworkUpdate();
}
void Drawable::SetMaxLights(unsigned num)
{
    maxLights_ = num;
    MarkNetworkUpdate();
}

void Drawable::SetCastShadows(bool enable)
{
    castShadows_ = enable;
    MarkNetworkUpdate();
}

void Drawable::SetOccluder(bool enable)
{
    occluder_ = enable;
    MarkNetworkUpdate();
}

void Drawable::SetOccludee(bool enable)
{
    if (enable != occludee_)
    {
        occludee_ = enable;
        // Reinsert to octree to make sure octant occlusion does not erroneously hide this drawable
        if (octant_ && !updateQueued_)
            octant_->GetRoot()->QueueUpdate(this);
        MarkNetworkUpdate();
    }
}

void Drawable::MarkForUpdate()
{
    if (!updateQueued_ && octant_)
        octant_->GetRoot()->QueueUpdate(this);
}

const BoundingBox& Drawable::GetWorldBoundingBox()
{
    if (worldBoundingBoxDirty_)
    {
        OnWorldBoundingBoxUpdate();
        worldBoundingBoxDirty_ = false;
    }

    return worldBoundingBox_;
}

bool Drawable::IsInView() const
{
    // Note: in headless mode there is no renderer subsystem and no view frustum tests are performed, so return
    // always false in that case
    auto* renderer = GetSubsystem<Renderer>();
    return renderer && viewFrameNumber_ == renderer->GetFrameInfo().frameNumber_ && !viewCameras_.Empty();
}

bool Drawable::IsInView(Camera* camera) const
{
    auto* renderer = GetSubsystem<Renderer>();
    return renderer && viewFrameNumber_ == renderer->GetFrameInfo().frameNumber_ && (!camera || viewCameras_.Contains(camera));
}

bool Drawable::IsInView(const FrameInfo& frame, bool anyCamera) const
{
    return viewFrameNumber_ == frame.frameNumber_ && (anyCamera || viewCameras_.Contains(frame.camera_));
}

void Drawable::SetZone(Zone* zone, bool temporary)
{
    zone_ = zone;

    // If the zone assignment was temporary (inconclusive) set the dirty flag so that it will be re-evaluated on the next frame
    zoneDirty_ = temporary;
}

void Drawable::SetSortValue(float value)
{
    sortValue_ = value;
}

void Drawable::MarkInView(const FrameInfo& frame)
{
    if (frame.frameNumber_ != viewFrameNumber_)
    {
        viewFrameNumber_ = frame.frameNumber_;
        viewCameras_.Resize(1);
        viewCameras_[0] = frame.camera_;
    }
    else
        viewCameras_.Push(frame.camera_);

    basePassFlags_ = 0;
    firstLight_ = nullptr;
    lights_.Clear();
    vertexLights_.Clear();
}

void Drawable::MarkInView(unsigned frameNumber)
{
    if (frameNumber != viewFrameNumber_)
    {
        viewFrameNumber_ = frameNumber;
        viewCameras_.Clear();
    }
}

void Drawable::LimitLights()
{
    // Maximum lights value 0 means unlimited
    if (!maxLights_ || lights_.Size() <= maxLights_)
        return;

    // If more lights than allowed, move to vertex lights and cut the list
    const BoundingBox& box = GetWorldBoundingBox();
    for (unsigned i = 0; i < lights_.Size(); ++i)
        lights_[i]->SetIntensitySortValue(box);

    Sort(lights_.Begin(), lights_.End(), CompareDrawables);
    vertexLights_.Insert(vertexLights_.End(), lights_.Begin() + maxLights_, lights_.End());
    lights_.Resize(maxLights_);
}

void Drawable::LimitVertexLights(bool removeConvertedLights)
{
    if (removeConvertedLights)
    {
        for (unsigned i = vertexLights_.Size() - 1; i < vertexLights_.Size(); --i)
        {
            if (!vertexLights_[i]->GetPerVertex())
                vertexLights_.Erase(i);
        }
    }

    if (vertexLights_.Size() <= MAX_VERTEX_LIGHTS)
        return;

    const BoundingBox& box = GetWorldBoundingBox();
    for (unsigned i = 0; i < vertexLights_.Size(); ++i)
        vertexLights_[i]->SetIntensitySortValue(box);

    Sort(vertexLights_.Begin(), vertexLights_.End(), CompareDrawables);
    vertexLights_.Resize(MAX_VERTEX_LIGHTS);
}

void Drawable::SetOverrideShaderParameterAnimation(const String & name, ValueAnimation * animation, WrapMode wrapMode, float speed)
{
	for (int i = 0; i < batches_.Size(); ++i)
	{
		SetOverrideShaderParameterAnimation(i, name, animation, wrapMode, speed);
	}
}

void Drawable::SetOverrideShaderParameterAnimation(int index, const String& name, ValueAnimation* animation, WrapMode wrapMode, float speed)
{
	if (index >= batches_.Size())
	{
		URHO3D_LOGERROR(String("Drawable#SetOverrideShaderParameter的index参数超出batch的总数数!!!!!!!!"));
		return;
	}

	OverrideShaderParameterAnimationInfo* info = GetOverrideShaderParameterAnimationInfo(index, name);

	if (animation)
	{
		if (info && info->GetAnimation() == animation)
		{
			info->SetWrapMode(wrapMode);
			info->SetSpeed(speed);
			return;
		}

		StringHash nameHash(name);
		batches_[index].overrideShaderParameterAnimationInfos_[nameHash] = new OverrideShaderParameterAnimationInfo(this, name, index, animation, wrapMode, speed);
		UpdateEventSubscription();
	} else
	{
		if (info)
		{
			StringHash nameHash(name);
			batches_[index].overrideShaderParameterAnimationInfos_.Erase(nameHash);
			RemoveOverrideShaderParameter(index, name);
			UpdateEventSubscription();
		}
	}
}

OverrideShaderParameterAnimationInfo* Drawable::GetOverrideShaderParameterAnimationInfo(int index, const String& name) const
{
	if (index >= batches_.Size())
	{
		URHO3D_LOGERROR(String("Drawable#SetOverrideShaderParameter的index参数超出batch的总数数!!!!!!!!"));
		return nullptr;
	}

	StringHash nameHash(name);
	HashMap<StringHash, SharedPtr<OverrideShaderParameterAnimationInfo> >::ConstIterator i = batches_[index].overrideShaderParameterAnimationInfos_.Find(nameHash);
	if (i == batches_[index].overrideShaderParameterAnimationInfos_.End())
		return nullptr;
	return i->second_;
}

void Drawable::UpdateEventSubscription()
{
	for (int i = 0; i < batches_.Size(); i++)
	{
		if (batches_[i].overrideShaderParameterAnimationInfos_.Size() && !subscribed_)
		{
			SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Drawable, HandleAttributeAnimationUpdate));
			subscribed_ = true;
		}
		else if (subscribed_ && batches_[i].overrideShaderParameterAnimationInfos_.Empty())
		{
			UnsubscribeFromEvent(E_UPDATE);
			subscribed_ = false;
		}
	}
}

void Drawable::HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData)
{
	// Timestep parameter is same no matter what event is being listened to
	float timeStep = eventData[Update::P_TIMESTEP].GetFloat();

	// Keep weak pointer to self to check for destruction caused by event handling
	WeakPtr<Object> self(this);

	for (int j = 0; j < batches_.Size(); ++j)
	{
		Vector<String> finishedNames;
		for (HashMap<StringHash, SharedPtr<OverrideShaderParameterAnimationInfo> >::ConstIterator i = batches_[j].overrideShaderParameterAnimationInfos_.Begin();
			i != batches_[j].overrideShaderParameterAnimationInfos_.End(); ++i)
		{
			bool finished = i->second_->Update(timeStep);
			// If self deleted as a result of an event sent during animation playback, nothing more to do
			if (self.Expired())
				return;

			if (finished)
				finishedNames.Push(i->second_->GetName());
		}


		// Remove finished animations
		for (unsigned i = 0; i < finishedNames.Size(); ++i)
			SetOverrideShaderParameterAnimation(j, finishedNames[i], nullptr);
	}
}

void Drawable::OnNodeSet(Node* node)
{
    if (node)
        node->AddListener(this);
}

void Drawable::OnSceneSet(Scene* scene)
{
    if (scene)
        AddToOctree();
    else
        RemoveFromOctree();
}

void Drawable::OnMarkedDirty(Node* node)
{
    worldBoundingBoxDirty_ = true;
    if (!updateQueued_ && octant_)
        octant_->GetRoot()->QueueUpdate(this);

    // Mark zone assignment dirty when transform changes
    if (node == node_)
        zoneDirty_ = true;
}

void Drawable::AddToOctree()
{
    // Do not add to octree when disabled
    if (!IsEnabledEffective())
        return;

    Scene* scene = GetScene();
    if (scene)
    {
        auto* octree = scene->GetComponent<Octree>();
        if (octree)
            octree->InsertDrawable(this);
        else
            URHO3D_LOGERROR("No Octree component in scene, drawable will not render");
    }
    else
    {
        // We have a mechanism for adding detached nodes to an octree manually, so do not log this error
        //URHO3D_LOGERROR("Node is detached from scene, drawable will not render");
    }
}

void Drawable::RemoveFromOctree()
{
    if (octant_)
    {
        Octree* octree = octant_->GetRoot();
        if (updateQueued_)
            octree->CancelUpdate(this);

        // Perform subclass specific deinitialization if necessary
        OnRemoveFromOctree();

        octant_->RemoveDrawable(this);
    }
}

Pass* Drawable::AddPass(const String& passName, const String& vsName, const String& psName, const String& vsDefines, const String& psDefines)
{
#ifdef 	URHO3D_THREADING
	if(!Thread::IsMainThread())
	{
		URHO3D_LOGERROR("Drawable#AddPass was called in non-main thread!!!!!!!!");
		return nullptr;
	}
#endif
	HashMap<String, SharedPtr<Pass>>::Iterator i = separatePasses_.Find(passName);
	if (i != separatePasses_.End())
	{
		//URHO3D_LOGDEBUG("When Adding a pass Material index out of bounds");
		return nullptr;
	}		
	else
	{
		separatePasses_[passName] = new Pass(passName);
		separatePasses_[passName]->SetVertexShader(vsName);
		separatePasses_[passName]->SetPixelShader(psName);
		separatePasses_[passName]->SetVertexShaderDefines(vsDefines);
		separatePasses_[passName]->SetPixelShaderDefines(psDefines);
		return separatePasses_[passName];
	}
}

bool Drawable::RemovePass(const String& passName)
{
#ifdef 	URHO3D_THREADING
	if (!Thread::IsMainThread())
	{
		URHO3D_LOGERROR("Drawable#RemovePass was called in non-main thread!!!!!!!!");
		return false;
	}
#endif
	separatePasses_.Erase(passName);
	return true;
}

Pass* Drawable::GetPass(const String& passName)
{
	HashMap<String, SharedPtr<Pass>>::Iterator i = separatePasses_.Find(passName);
	if (i != separatePasses_.End())
	{
		return i->second_;
	} 
	
	return nullptr;
}

void Drawable::SetOverrideShaderParameter(const String & name, const Variant & value)
{
	for (int i = 0; i < batches_.Size(); ++i)
	{
		SetOverrideShaderParameter(i, name, value);
	}
}

void Drawable::SetOverrideShaderParameter(int index, const String& name, const Variant& value)
{
#ifdef 	URHO3D_THREADING
	if (!Thread::IsMainThread())
	{
		URHO3D_LOGERROR("Drawable#SetOverrideShaderParameter was called in non-main thread!!!!!!!!");
		return;
	}
#endif
	if (index >= batches_.Size())
	{
		URHO3D_LOGERROR(String("Drawable#SetOverrideShaderParameter的index参数超出batch的总数数!!!!!!!!"));
		return;
	}

	MaterialShaderParameter newParam;
	newParam.name_ = name;
	newParam.value_ = value;

	StringHash nameHash(name);
	
	batches_[index].overrideShaderParameters_[nameHash] = newParam;
}

bool Drawable::RemoveOverrideShaderParameter(const String & name)
{
	bool state = true;
	for (int i = 0; i < batches_.Size(); ++i)
	{
		state &= RemoveOverrideShaderParameter(i, name);
	}
	return state;
}

bool Drawable::RemoveOverrideShaderParameter(int index, const String& name)
{
#ifdef 	URHO3D_THREADING
	if (!Thread::IsMainThread())
	{
		URHO3D_LOGERROR("Drawable#RemoveOverrideShaderParameter was called in non-main thread!!!!!!!!");
		return false;
	}
#endif
	if (index >= batches_.Size())
	{
		URHO3D_LOGERROR(String("Drawable#SetOverrideShaderParameter的index参数超出batch的总数数!!!!!!!!"));
		return false;
	}

	StringHash nameHash(name);
	HashMap<StringHash, MaterialShaderParameter>::Iterator iter = batches_[index].overrideShaderParameters_.Find(nameHash);
	if (iter != batches_[index].overrideShaderParameters_.End())
	{
		batches_[index].overrideShaderParameters_.Erase(nameHash);
	}
	return true;
}

HashMap<StringHash, MaterialShaderParameter>& Drawable::GetOverrideShaderParameters(int index)
{
	if (index >= batches_.Size())
	{
		URHO3D_LOGERROR(String("Drawable#SetOverrideShaderParameter的index参数超出batch的总数数!!!!!!!!"));
		return batches_[batches_.Size()-1].overrideShaderParameters_;
	}
	return batches_[index].overrideShaderParameters_;
}

	bool WriteDrawablesToOBJ(PODVector<Drawable*> drawables, File* outputFile, bool asZUp, bool asRightHanded, bool writeLightmapUV)
{
    // Must track indices independently to deal with potential mismatching of drawables vertex attributes (ie. one with UV, another without, then another with)
    unsigned currentPositionIndex = 1;
    unsigned currentUVIndex = 1;
    unsigned currentNormalIndex = 1;
    bool anythingWritten = false;

    // Write the common "I came from X" comment
    outputFile->WriteLine("# OBJ file exported from Urho3D");

    for (unsigned i = 0; i < drawables.Size(); ++i)
    {
        Drawable* drawable = drawables[i];

        // Only write enabled drawables
        if (!drawable->IsEnabledEffective())
            continue;

        Node* node = drawable->GetNode();
        Matrix3x4 transMat = drawable->GetNode()->GetWorldTransform();
        Matrix3x4 n = transMat.Inverse();
        Matrix3 normalMat = Matrix3(n.m00_, n.m01_, n.m02_, n.m10_, n.m11_, n.m12_, n.m20_, n.m21_, n.m22_);
        normalMat = normalMat.Transpose();

        const Vector<SourceBatch>& batches = drawable->GetBatches();
        for (unsigned geoIndex = 0; geoIndex < batches.Size(); ++geoIndex)
        {
            Geometry* geo = drawable->GetLodGeometry(geoIndex, 0);
            if (geo == nullptr)
                continue;
            if (geo->GetPrimitiveType() != TRIANGLE_LIST)
            {
                URHO3D_LOGERRORF("%s (%u) %s (%u) Geometry %u contains an unsupported geometry type %u", node->GetName().Length() > 0 ? node->GetName().CString() : "Node", node->GetID(), drawable->GetTypeName().CString(), drawable->GetID(), geoIndex, (unsigned)geo->GetPrimitiveType());
                continue;
            }

            // If we've reached here than we're going to actually write something to the OBJ file
            anythingWritten = true;

            const unsigned char* vertexData;
            const unsigned char* indexData;
            unsigned elementSize, indexSize;
            const PODVector<VertexElement>* elements;
            geo->GetRawData(vertexData, elementSize, indexData, indexSize, elements);
            if (!vertexData || !elements)
                continue;

            bool hasPosition = VertexBuffer::HasElement(*elements, TYPE_VECTOR3, SEM_POSITION);
            if (!hasPosition)
            {
                URHO3D_LOGERRORF("%s (%u) %s (%u) Geometry %u contains does not have Vector3 type positions in vertex data", node->GetName().Length() > 0 ? node->GetName().CString() : "Node", node->GetID(), drawable->GetTypeName().CString(), drawable->GetID(), geoIndex);
                continue;
            }

            bool hasNormals = VertexBuffer::HasElement(*elements, TYPE_VECTOR3, SEM_NORMAL);
            bool hasUV = VertexBuffer::HasElement(*elements, TYPE_VECTOR2, SEM_TEXCOORD, 0);
            bool hasLMUV = VertexBuffer::HasElement(*elements, TYPE_VECTOR2, SEM_TEXCOORD, 1);

            if (elementSize > 0 && indexSize > 0)
            {
                unsigned vertexStart = geo->GetVertexStart();
                unsigned vertexCount = geo->GetVertexCount();
                unsigned indexStart = geo->GetIndexStart();
                unsigned indexCount = geo->GetIndexCount();

                // Name NodeID DrawableType DrawableID GeometryIndex ("Geo" is included for clarity as StaticModel_32_2 could easily be misinterpreted or even quickly misread as 322)
                // Generated object name example: Node_5_StaticModel_32_Geo_0 ... or ... Bob_5_StaticModel_32_Geo_0
                outputFile->WriteLine(String("o ").AppendWithFormat("%s_%u_%s_%u_Geo_%u", node->GetName().Length() > 0 ? node->GetName().CString() : "Node", node->GetID(), drawable->GetTypeName().CString(), drawable->GetID(), geoIndex));

                // Write vertex position
                unsigned positionOffset = VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR3, SEM_POSITION);
                for (unsigned j = 0; j < vertexCount; ++j)
                {
                    Vector3 vertexPosition = *((const Vector3*)(&vertexData[(vertexStart + j) * elementSize + positionOffset]));
                    vertexPosition = transMat * vertexPosition;

                    // Convert coordinates as requested
                    if (asRightHanded)
                        vertexPosition.x_ *= -1;
                    if (asZUp)
                    {
                        float yVal = vertexPosition.y_;
                        vertexPosition.y_ = vertexPosition.z_;
                        vertexPosition.z_ = yVal;
                    }
                    outputFile->WriteLine("v " + String(vertexPosition));
                }

                if (hasNormals)
                {
                    unsigned normalOffset = VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR3, SEM_NORMAL);
                    for (unsigned j = 0; j < vertexCount; ++j)
                    {
                        Vector3 vertexNormal = *((const Vector3*)(&vertexData[(vertexStart + j) * elementSize + normalOffset]));
                        vertexNormal = normalMat * vertexNormal;
                        vertexNormal.Normalize();

                        if (asRightHanded)
                            vertexNormal.x_ *= -1;
                        if (asZUp)
                        {
                            float yVal = vertexNormal.y_;
                            vertexNormal.y_ = vertexNormal.z_;
                            vertexNormal.z_ = yVal;
                        }

                        outputFile->WriteLine("vn " + String(vertexNormal));
                    }
                }

                // Write TEXCOORD1 or TEXCOORD2 if it was chosen
                if (hasUV || (hasLMUV && writeLightmapUV))
                {
                    // if writing Lightmap UV is chosen, only use it if TEXCOORD2 exists, otherwise use TEXCOORD1
                    unsigned texCoordOffset = (writeLightmapUV && hasLMUV) ? VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR2, SEM_TEXCOORD, 1) :
                        VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR2, SEM_TEXCOORD, 0);
                    for (unsigned j = 0; j < vertexCount; ++j)
                    {
                        Vector2 uvCoords = *((const Vector2*)(&vertexData[(vertexStart + j) * elementSize + texCoordOffset]));
                        outputFile->WriteLine("vt " + String(uvCoords));
                    }
                }

                // If we don't have UV but have normals then must write a double-slash to indicate the absence of UV coords, otherwise use a single slash
                const String slashCharacter = hasNormals ? "//" : "/";

                // Amount by which to offset indices in the OBJ vs their values in the Urho3D geometry, basically the lowest index value
                // Compensates for the above vertex writing which doesn't write ALL vertices, just the used ones
                unsigned indexOffset = M_MAX_INT;
                for (unsigned indexIdx = indexStart; indexIdx < indexStart + indexCount; indexIdx++)
                {
                    if (indexSize == 2)
                        indexOffset = Min(indexOffset, (unsigned)*((unsigned short*)(indexData + indexIdx * indexSize)));
                    else
                        indexOffset = Min(indexOffset, *((unsigned*)(indexData + indexIdx * indexSize)));
                }

                for (unsigned indexIdx = indexStart; indexIdx < indexStart + indexCount; indexIdx += 3)
                {
                    // Deal with 16 or 32 bit indices
                    unsigned longIndices[3];
                    if (indexSize == 2)
                    {
                        //16 bit indices
                        unsigned short indices[3];
                        memcpy(indices, indexData + (indexIdx * indexSize), (size_t)indexSize * 3);
                        longIndices[0] = indices[0] - indexOffset;
                        longIndices[1] = indices[1] - indexOffset;
                        longIndices[2] = indices[2] - indexOffset;
                    }
                    else
                    {
                        //32 bit indices
                        unsigned indices[3];
                        memcpy(indices, indexData + (indexIdx * indexSize), (size_t)indexSize * 3);
                        longIndices[0] = indices[0] - indexOffset;
                        longIndices[1] = indices[1] - indexOffset;
                        longIndices[2] = indices[2] - indexOffset;
                    }

                    String output = "f ";
                    if (hasNormals)
                    {
                        output.AppendWithFormat("%l/%l/%l %l/%l/%l %l/%l/%l",
                            currentPositionIndex + longIndices[0],
                            currentUVIndex + longIndices[0],
                            currentNormalIndex + longIndices[0],
                            currentPositionIndex + longIndices[1],
                            currentUVIndex + longIndices[1],
                            currentNormalIndex + longIndices[1],
                            currentPositionIndex + longIndices[2],
                            currentUVIndex + longIndices[2],
                            currentNormalIndex + longIndices[2]);
                    }
                    else if (hasNormals || hasUV)
                    {
                        unsigned secondTraitIndex = hasNormals ? currentNormalIndex : currentUVIndex;
                        output.AppendWithFormat("%l%s%l %l%s%l %l%s%l",
                            currentPositionIndex + longIndices[0],
                            slashCharacter.CString(),
                            secondTraitIndex + longIndices[0],
                            currentPositionIndex + longIndices[1],
                            slashCharacter.CString(),
                            secondTraitIndex + longIndices[1],
                            currentPositionIndex + longIndices[2],
                            slashCharacter.CString(),
                            secondTraitIndex + longIndices[2]);
                    }
                    else
                    {
                        output.AppendWithFormat("%l %l %l",
                            currentPositionIndex + longIndices[0],
                            currentPositionIndex + longIndices[1],
                            currentPositionIndex + longIndices[2]);
                    }
                    outputFile->WriteLine(output);
                }

                // Increment our positions based on what vertex attributes we have
                currentPositionIndex += vertexCount;
                currentNormalIndex += hasNormals ? vertexCount : 0;
                // is it possible to have TEXCOORD2 but not have TEXCOORD1, assume anything
                currentUVIndex += (hasUV || hasLMUV) ? vertexCount : 0;
            }
        }
    }
    return anythingWritten;
}

float SourceBatch::createCollisionData(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results, Vector<SharedPtr<BIHTree> >& trees)
{
	if(!tree_)
	{
		tree_ = new BIHTree(geometry_->GetContext());
		trees.Push(tree_);
		if (!tree_->getIsBuildTree())
		{
			tree_->buildTree(geometry_);
		}
	}
	else
	{
		tree_->Init(0, geometry_->GetIndexCount() / 3 - 1);
	}
	return tree_->CollideWithRay(ray, boundingBox, results);
}

BIHTree::BIHTree(Context* context) :
	Object(context)
{
	root = NULL;
	m_IsBuildTree = false;
}
BIHTree::~BIHTree() = default;
//BIHTree::BIHTree() 
//{
//	root = NULL;
//	m_IsBuildTree = false;
//}

//BIHTree::~BIHTree() = default;

//BIHTree::BIHTree()
//{
//	root = NULL;
//	m_IsBuildTree = false;
//}

void BIHTree::buildTree(Geometry* geometry)
{
	const unsigned char* vertexData;
	const unsigned char* indexData;
	unsigned vertexSize;
	unsigned indexSize;
	const PODVector<VertexElement>* elements;
	geometry->GetRawData(vertexData, vertexSize, indexData, indexSize, elements);
	if (!vertexData || !elements || VertexBuffer::GetElementOffset(*elements, TYPE_VECTOR3, SEM_POSITION) != 0)
		return;
	float nearest = M_INFINITY;
	const auto* vertices = (const unsigned char*)vertexData;

	const unsigned short* indices = ((const unsigned short*)indexData) + geometry->GetIndexStart();
	const unsigned short* indicesEnd = indices + geometry->GetIndexCount();
	const unsigned short* nearestIndices = nullptr;

	// 16-bit indices
	if (indexSize == sizeof(unsigned short))
	{
		const unsigned short* indices = ((const unsigned short*)indexData) + geometry->GetIndexStart();
		const unsigned short* indicesEnd = indices + geometry->GetIndexCount();
		const unsigned short* nearestIndices = nullptr;
		while (indices < indicesEnd)
		{
			Vector3& v0 = *((Vector3*)(&vertices[indices[0] * vertexSize]));
			Vector3& v1 = *((Vector3*)(&vertices[indices[1] * vertexSize]));
			Vector3& v2 = *((Vector3*)(&vertices[indices[2] * vertexSize]));
			originalVertices.Push(v0);
			originalVertices.Push(v1);
			originalVertices.Push(v2);
			indices += 3;
		}
	}
	// 32-bit indices
	else
	{
		const unsigned* indices = ((const unsigned*)indexData) + geometry->GetIndexStart();
		const unsigned* indicesEnd = indices + geometry->GetIndexCount();
		const unsigned* nearestIndices = nullptr;
		while (indices < indicesEnd)
		{
			Vector3& v0 = *((Vector3*)(&vertices[indices[0] * vertexSize]));
			Vector3& v1 = *((Vector3*)(&vertices[indices[1] * vertexSize]));
			Vector3& v2 = *((Vector3*)(&vertices[indices[2] * vertexSize]));
			originalVertices.Push(v0);
			originalVertices.Push(v1);
			originalVertices.Push(v2);
			indices += 3;
		}
	}
	triangleNum = (geometry->GetIndexCount() / 3) - 1;
	Init(0, triangleNum);
	setIsBuildTree(true);
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
	if (tempVars.Empty())
	{
		tempVars.Push(*min);
		tempVars.Push(*max);
		tempVars.Push(v1);
		tempVars.Push(v2);
		tempVars.Push(v3);
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
			swapTriangles(pivot, j);
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

void BIHTree::swapTriangles(int index1, int index2)
{
	// swap indices
	Vector3 tmp = originalVertices[index1];
	originalVertices[index1] = originalVertices[index2];
	originalVertices[index2] = tmp;
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
		BIHNode* node = new BIHNode(axis);

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

Tree::Tree(Context* context) :
	Resource(context)
{
}

Tree::~Tree() = default;

void Tree::RegisterObject(Context* context)
{
	context->RegisterFactory<Tree>();
}

void Tree::SetBIHTrees(const Vector<SharedPtr<BIHTree> >& tree)
{
	bihTrees_ = tree;
}

Vector<SharedPtr<BIHTree> >& Tree::GetBIHTrees()
{
	return bihTrees_;
}

bool Tree::BeginLoad(Deserializer& source)
{
	// Check ID
	String fileID = source.ReadFileID();
	if(!fileID.Contains("BT3"))	
	{
		URHO3D_LOGERROR(source.GetName() + " is not a valid bihTree file");
		return true;
	}
	URHO3D_LOGDEBUG("fileID="+ String(fileID));
	bihTrees_.Clear();

	unsigned memoryUse = sizeof(Tree);
	bool async = GetAsyncLoadState() == ASYNC_LOADING;

	// Read bihTree num
	unsigned numBihTrees = source.ReadUInt();
	URHO3D_LOGDEBUG("numBihTrees=" + String(numBihTrees));
	//numBihTrees = 0;
	bihTrees_.Reserve(numBihTrees);
	int verticesNum = 0;
	for (unsigned i = 0; i < numBihTrees; ++i)
	{
		SharedPtr<BIHTree> tree(new BIHTree(context_));
		//SharedPtr<BIHTree>& desc = bihTrees_[i];
		verticesNum = source.ReadUInt();
		for (int j = 0; j < verticesNum; ++j)
		{
			Vector3 vec = source.ReadVector3();
			tree->originalVertices.Push(vec);
		}
		bihTrees_.Push(tree);
		memoryUse += sizeof(Vector3) * verticesNum;
	}

	SetMemoryUse(memoryUse);
	return true;
}

bool Tree::EndLoad()
{
	return true;
}

/// Save resource. Return true if successful.
bool Tree::Save(Serializer& dest)const
{
	// Write ID
	if (!dest.WriteFileID("BT3"))
		return false;
	
    // Write BIHTree  attribute
	dest.WriteUInt(bihTrees_.Size());
	for (unsigned i = 0; i < bihTrees_.Size(); ++i)
	{
		SharedPtr<BIHTree> info = bihTrees_[i];
		// Write vertices coordinate
		dest.WriteUInt(info->originalVertices.Size());
		for (unsigned j = 0; j < info->originalVertices.Size(); ++j)
		{
			dest.WriteVector3(info->originalVertices[j]);
		}
	}
	
	return true;
}

}
