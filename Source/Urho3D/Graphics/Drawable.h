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

#pragma once

#include "../Graphics/GraphicsDefs.h"
#include "../Graphics/Technique.h"
#include "../Math/BoundingBox.h"
#include "../Scene/Component.h"
#include <iostream>
#include <stack>
#include<algorithm>
#include <stdio.h>
#include <stdlib.h>
#include "../Resource/Resource.h"

using namespace std;

namespace Urho3D
{

static const unsigned DRAWABLE_UNDEFINED = 0x0;
static const unsigned DRAWABLE_GEOMETRY = 0x1;
static const unsigned DRAWABLE_LIGHT = 0x2;
static const unsigned DRAWABLE_ZONE = 0x4;
static const unsigned DRAWABLE_GEOMETRY2D = 0x8;
static const unsigned DRAWABLE_POINTCLOUD = 0x10;
static const unsigned DRAWABLE_ANY = 0xff;
static const unsigned DEFAULT_VIEWMASK = M_MAX_UNSIGNED;
static const unsigned DEFAULT_LIGHTMASK = M_MAX_UNSIGNED;
static const unsigned DEFAULT_SHADOWMASK = M_MAX_UNSIGNED;
static const unsigned DEFAULT_ZONEMASK = M_MAX_UNSIGNED;
static const int MAX_VERTEX_LIGHTS = 4;
static const float ANIMATION_LOD_BASESCALE = 2500.0f;

class Camera;
class File;
class Geometry;
class Light;
class Material;
class OcclusionBuffer;
class Octant;
class RayOctreeQuery;
class Zone;
struct RayQueryResult;
struct WorkItem;
class Pass;
class MaterialShaderParameter;
class Drawable;
class XMLFile;
class JSONFile;

/// Geometry update type.
enum UpdateGeometryType
{
    UPDATE_NONE = 0,
    UPDATE_MAIN_THREAD,
    UPDATE_WORKER_THREAD
};

enum DynamicType
{
	Dynamic,
	Static
};

/// Rendering frame update parameters.
struct FrameInfo
{
    /// Frame number.
    unsigned frameNumber_;
    /// Time elapsed since last frame.
    float timeStep_;
    /// Viewport size.
    IntVector2 viewSize_;
    /// Camera being used.
    Camera* camera_;
};

enum VertexType
{
	VERTEX_Min = 0,
	VERTEX_Max = 1,
	VERTEX_V1 = 2,
	VERTEX_V2 = 3,
	VERTEX_V3 = 4
};

class BIHNode;
struct URHO3D_API BIHStackData
{
	BIHNode* node;
	float min, max;
	BIHStackData(BIHNode* Node, float Min, float Max) {
		this->node = Node;
		this->min = Min;
		this->max = Max;
	}
	BIHStackData& operator=(const BIHStackData& s)//опнпн╦╦сич  
	{
		this->node = s.node;
	}
};

class URHO3D_API BIHTree : public Object
{
	URHO3D_OBJECT(BIHTree, Object);

public:
	/// Construct.
	BIHTree(Context* context);
	~BIHTree();
	void Init(int l, int r);
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
		}
	}
	int intersectWhere(Ray r, float sceneMin, float sceneMax, PODVector<RayQueryResult>& results);
	int sortTriangles(int l, int r, float split, int axis);
	void swapTriangles(int index1, int index2);
	void getTriangle(int index, Vector3& v1, Vector3& v2, Vector3& v3);
	BoundingBox* createBox(int l, int r);
	BIHNode* createNode(int l, int r, BoundingBox* nodeBbox, int depth);
	void setMinMax(BoundingBox* bbox, bool doMin, int axis, float value);
	void buildTree(Geometry* geometry);
	bool getIsBuildTree() { return m_IsBuildTree; }
	void setIsBuildTree(bool isBuildTree) { m_IsBuildTree = isBuildTree; }

	PODVector<Vector3> tempVars;
	stack<BIHStackData*> stackData;
	PODVector<Vector3> originalVertices;
private:
	int maxTrisPerNode = 21;
	int max_tree_depth = 100;
	float ONE_THIRD = 1.0f / 3.0f;
	int triangleNum = 0;
	bool m_IsBuildTree;
	BIHNode* root;
};

class URHO3D_API Tree : public Resource
{
	URHO3D_OBJECT(Tree, Resource);

public:
	/// Construct.
	explicit Tree(Context* context);
	/// Destruct.
	~Tree() override;
	/// Register object factory.
	static void RegisterObject(Context* context);
	/// Load resource from stream. May be called from a worker thread. Return true if successful.
	bool BeginLoad(Deserializer& source) override;
	/// Finish resource loading. Always called from the main thread. Return true if successful.
	bool EndLoad() override;
	/// Save resource. Return true if successful.
	bool Save(Serializer& dest) const override;

	/// Load from an XML element. Return true if successful.
	bool Load(const XMLElement& source);
	/// Save to an XML element. Return true if successful.
	bool Save(XMLElement& dest) const;

	/// Load from a JSON value. Return true if successful.
	bool Load(const JSONValue& source);
	/// Save to a JSON value. Return true if successful.
	bool Save(JSONValue& dest) const;
	/// Set index buffers.
	void SetBIHTrees(const Vector<SharedPtr<BIHTree> >& tree);
	Vector<SharedPtr<BIHTree> >& GetBIHTrees();
private:
	/// bihTree.
	Vector<SharedPtr<BIHTree> > bihTrees_;
};

class URHO3D_API BIHNode
{
public:
	BIHNode(int axis) {
		this->axis = axis;
	}
	BIHNode(int l, int r)
	{
		leftIndex = l;
		rightIndex = r;
		axis = 3; // indicates leaf
	}
	//~BIHNode();
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
	int leftIndex, rightIndex;
	float leftPlane;
	float rightPlane;
	BIHNode* left;
	BIHNode* right;
	int axis;

};

/// Source data for a 3D geometry draw call.
struct URHO3D_API SourceBatch
{
    /// Construct with defaults.
	SourceBatch();
    /// Copy-construct.
    SourceBatch(const SourceBatch& batch);
    /// Destruct.
	~SourceBatch();

    /// Assignment operator.
    SourceBatch& operator =(const SourceBatch& rhs);

    /// Distance from camera.
    float distance_{};
    /// Geometry.
    Geometry* geometry_{};
    /// Material.
    SharedPtr<Material> material_;
    /// World transform(s). For a skinned model, these are the bone transforms.
    const Matrix3x4* worldTransform_{&Matrix3x4::IDENTITY};
    /// Number of world transforms.
    unsigned numWorldTransforms_{1};
    /// Per-instance data. If not null, must contain enough data to fill instancing buffer.
    void* instancingData_{};
    /// %Geometry type.
    GeometryType geometryType_{GEOM_STATIC};
	
	float createCollisionData(Ray& ray, BoundingBox& boundingBox, PODVector<RayQueryResult>& results, Vector<SharedPtr<BIHTree> >& trees);
	//BIHTree* tree;
	SharedPtr<BIHTree> tree_;
	/*SharedPtr<Material> oldMaterial_;
	SharedPtr<Technique> oldTechnique_;*/
};

/// drawable's override shader parameter animation instance
class OverrideShaderParameterAnimationInfo : public ValueAnimationInfo
{
public:
	/// Construct.
	OverrideShaderParameterAnimationInfo
		(Drawable* drawalbe, const String& name, ValueAnimation* attributeAnimation, WrapMode wrapMode, float speed);
	/// Copy construct.
	OverrideShaderParameterAnimationInfo(const OverrideShaderParameterAnimationInfo& other);
	/// Destruct.
	~OverrideShaderParameterAnimationInfo() override;

	/// Return shader parameter name.
	const String& GetName() const {
		return name_;
	}

protected:
	/// Apply new animation value to the target object. Called by Update().
	void ApplyValue(const Variant& newValue) override;

private:
	/// Shader parameter name.
	String name_;
};

/// Base class for visible components.
class URHO3D_API Drawable : public Component
{
    URHO3D_OBJECT(Drawable, Component);

    friend class Octant;
    friend class Octree;
    friend void UpdateDrawablesWork(const WorkItem* item, unsigned threadIndex);

public:
    /// Construct.
    Drawable(Context* context, unsigned char drawableFlags);
    /// Destruct.
    ~Drawable() override;
    /// Register object attributes. Drawable must be registered first.
    static void RegisterObject(Context* context);

    /// Handle enabled/disabled state change.
    void OnSetEnabled() override;
    /// Process octree raycast. May be called from a worker thread.
    virtual void ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results);
    /// Update before octree reinsertion. Is called from a worker thread
    virtual void Update(const FrameInfo& frame) { }
    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    virtual void UpdateBatches(const FrameInfo& frame);
    /// Prepare geometry for rendering.
    virtual void UpdateGeometry(const FrameInfo& frame) { }

    /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
    virtual UpdateGeometryType GetUpdateGeometryType() { return UPDATE_NONE; }

    /// Return the geometry for a specific LOD level.
    virtual Geometry* GetLodGeometry(unsigned batchIndex, unsigned level);

    /// Return number of occlusion geometry triangles.
    virtual unsigned GetNumOccluderTriangles() { return 0; }

    /// Draw to occlusion buffer. Return true if did not run out of triangles.
    virtual bool DrawOcclusion(OcclusionBuffer* buffer);
    /// Visualize the component as debug geometry.
    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

    /// Set draw distance.
    void SetDrawDistance(float distance);
    /// Set shadow draw distance.
    void SetShadowDistance(float distance);
    /// Set LOD bias.
    void SetLodBias(float bias);
    /// Set view mask. Is and'ed with camera's view mask to see if the object should be rendered.
    void SetViewMask(unsigned mask);
	/// Set override mask
	void SetOverrideViewMask(unsigned mask);
	/// 取消 override view mask，即置所有bit为1
	void CancelOverrideViewMask();
    /// Set light mask. Is and'ed with light's and zone's light mask to see if the object should be lit.
    void SetLightMask(unsigned mask);
    /// Set shadow mask. Is and'ed with light's light mask and zone's shadow mask to see if the object should be rendered to a shadow map.
    void SetShadowMask(unsigned mask);
    /// Set zone mask. Is and'ed with zone's zone mask to see if the object should belong to the zone.
    void SetZoneMask(unsigned mask);
	void SetDynamicType(DynamicType type);
    /// Set maximum number of per-pixel lights. Default 0 is unlimited.
    void SetMaxLights(unsigned num);
    /// Set shadowcaster flag.
    void SetCastShadows(bool enable);
    /// Set occlusion flag.
    void SetOccluder(bool enable);
    /// Set occludee flag.
    void SetOccludee(bool enable);
    /// Mark for update and octree reinsertion. Update is automatically queued when the drawable's scene node moves or changes scale.
    void MarkForUpdate();

    /// Return local space bounding box. May not be applicable or properly updated on all drawables.
    const BoundingBox& GetBoundingBox() const { return boundingBox_; }

    /// Return world-space bounding box.
    const BoundingBox& GetWorldBoundingBox();

    /// Return drawable flags.
    unsigned char GetDrawableFlags() const { return drawableFlags_; }

    /// Return draw distance.
    float GetDrawDistance() const { return drawDistance_; }

    /// Return shadow draw distance.
    float GetShadowDistance() const { return shadowDistance_; }

    /// Return LOD bias.
    float GetLodBias() const { return lodBias_; }

    /// Return view mask.
    unsigned GetViewMask() const { return viewMask_&overrideViewMask_; }

    /// Return light mask.
    unsigned GetLightMask() const { return lightMask_; }

    /// Return shadow mask.
    unsigned GetShadowMask() const { return shadowMask_; }

    /// Return zone mask.
    unsigned GetZoneMask() const { return zoneMask_; }

	/// Return dynamic type.
	DynamicType GetDynamicType() const { return dynamicType_; }

    /// Return maximum number of per-pixel lights.
    unsigned GetMaxLights() const { return maxLights_; }

    /// Return shadowcaster flag.
    bool GetCastShadows() const { return castShadows_; }

    /// Return occluder flag.
    bool IsOccluder() const { return occluder_; }

    /// Return occludee flag.
    bool IsOccludee() const { return occludee_; }

    /// Return whether is in view this frame from any viewport camera. Excludes shadow map cameras.
    bool IsInView() const;
    /// Return whether is in view of a specific camera this frame. Pass in a null camera to allow any camera, including shadow map cameras.
    bool IsInView(Camera* camera) const;

    /// Return draw call source data.
    const Vector<SourceBatch>& GetBatches() const { return batches_; }

    /// Set new zone. Zone assignment may optionally be temporary, meaning it needs to be re-evaluated on the next frame.
    void SetZone(Zone* zone, bool temporary = false);
    /// Set sorting value.
    void SetSortValue(float value);

    /// Set view-space depth bounds.
    void SetMinMaxZ(float minZ, float maxZ)
    {
        minZ_ = minZ;
        maxZ_ = maxZ;
    }

    /// Mark in view. Also clear the light list.
    void MarkInView(const FrameInfo& frame);
    /// Mark in view without specifying a camera. Used for shadow casters.
    void MarkInView(unsigned frameNumber);
    /// Sort and limit per-pixel lights to maximum allowed. Convert extra lights into vertex lights.
    void LimitLights();
    /// Sort and limit per-vertex lights to maximum allowed.
    void LimitVertexLights(bool removeConvertedLights);

    /// Set base pass flag for a batch.
    void SetBasePass(unsigned batchIndex) { basePassFlags_ |= (1u << batchIndex); }

    /// Return octree octant.
    Octant* GetOctant() const { return octant_; }

    /// Return current zone.
    Zone* GetZone() const { return zone_; }

    /// Return whether current zone is inconclusive or dirty due to the drawable moving.
    bool IsZoneDirty() const { return zoneDirty_; }

    /// Return distance from camera.
    float GetDistance() const { return distance_; }

    /// Return LOD scaled distance from camera.
    float GetLodDistance() const { return lodDistance_; }

    /// Return sorting value.
    float GetSortValue() const { return sortValue_; }

    /// Return whether is in view on the current frame. Called by View.
    bool IsInView(const FrameInfo& frame, bool anyCamera = false) const;

    /// Return whether has a base pass.
    bool HasBasePass(unsigned batchIndex) const { return (basePassFlags_ & (1u << batchIndex)) != 0; }

    /// Return per-pixel lights.
    const PODVector<Light*>& GetLights() const { return lights_; }

    /// Return per-vertex lights.
    const PODVector<Light*>& GetVertexLights() const { return vertexLights_; }

    /// Return the first added per-pixel light.
    Light* GetFirstLight() const { return firstLight_; }

    /// Return the minimum view-space depth.
    float GetMinZ() const { return minZ_; }

    /// Return the maximum view-space depth.
    float GetMaxZ() const { return maxZ_; }

    /// Add a per-pixel light affecting the object this frame.
    void AddLight(Light* light)
    {
        if (!firstLight_)
            firstLight_ = light;

        // Need to store into the light list only if the per-pixel lights are being limited
        // Otherwise recording the first light is enough
        if (maxLights_)
            lights_.Push(light);
    }

    /// Add a per-vertex light affecting the object this frame.
    void AddVertexLight(Light* light)
    {
        vertexLights_.Push(light);
    }

	Pass* AddPass(const String& passName, const String& vsName, const String& psName, const String& vsDefines = "", const String& psDefines = "");
	//Pass* AddPass(unsigned int index, const String& passName, const String& vsName, const String& psName, const String& vsDefines = "", const String& psDefines = "");
	bool RemovePass(const String& passName);
	//bool RemovePass(unsigned int index, const String& passName);
	Pass* GetPass(const String& passName);

	/// Set override shader parameter.
	void SetOverrideShaderParameter(const String& name, const Variant& value);
	/// Remove override shader parameter.
	bool RemoveOverrideShaderParameter(const String& name);
	/// 返回override shader parameters
	HashMap<StringHash, MaterialShaderParameter>& GetOverrideShaderParameters() { return overrideShaderParameters_; }
	/// 设置override technique
	void SetOverrideTechnique(Technique* overrideTechnique){ overrideTechnique_ = overrideTechnique; }
	/// 返回override technique
	Technique* GetOverrideTechnique() { return overrideTechnique_; }
	///取消override technique
	void CancelOverrideTechnique() { overrideTechnique_ = nullptr; }
	/// 设置override render state（fillmode）
	void SetOverrideFillMode(FillMode fillMode) { overrideFillMode_ = fillMode; }
	/// 返回override render state（fillmode）
	FillMode GetOverrideFillMode() { return overrideFillMode_; }
	/// Set Override shader parameter animation.
	void SetOverrideShaderParameterAnimation(const String& name, ValueAnimation* animation, WrapMode wrapMode = WM_LOOP, float speed = 1.0f);
	OverrideShaderParameterAnimationInfo* GetOverrideShaderParameterAnimationInfo(const String& name) const;
private:
	void UpdateEventSubscription();
	void HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData);
protected:
    /// Handle node being assigned.
    void OnNodeSet(Node* node) override;
    /// Handle scene being assigned.
    void OnSceneSet(Scene* scene) override;
    /// Handle node transform being dirtied.
    void OnMarkedDirty(Node* node) override;
    /// Recalculate the world-space bounding box.
    virtual void OnWorldBoundingBoxUpdate() = 0;

    /// Handle removal from octree.
    virtual void OnRemoveFromOctree() { }

    /// Add to octree.
    void AddToOctree();
    /// Remove from octree.
    void RemoveFromOctree();

    /// Move into another octree octant.
    void SetOctant(Octant* octant) { octant_ = octant; }

    /// World-space bounding box.
    BoundingBox worldBoundingBox_;
    /// Local-space bounding box.
    BoundingBox boundingBox_;
    /// Draw call source data.
    Vector<SourceBatch> batches_;
    /// Drawable flags.
    unsigned char drawableFlags_;
    /// Bounding box dirty flag.
    bool worldBoundingBoxDirty_;
    /// Shadowcaster flag.
    bool castShadows_;
    /// Occluder flag.
    bool occluder_;
    /// Occludee flag.
    bool occludee_;
    /// Octree update queued flag.
    bool updateQueued_;
    /// Zone inconclusive or dirtied flag.
    bool zoneDirty_;
    /// Octree octant.
    Octant* octant_;
    /// Current zone.
    Zone* zone_;
    /// View mask.
    unsigned viewMask_;
	/// Override View Mask, 主要用于和viewMask做并，来产生相应的可见性效果，去掉后（即置override view mask全bit为1），可以返回到viewMask的可见性效果
	unsigned overrideViewMask_;
    /// Light mask.
    unsigned lightMask_;
    /// Shadow mask.
    unsigned shadowMask_;
    /// Zone mask.
    unsigned zoneMask_;
    /// Last visible frame number.
    unsigned viewFrameNumber_;
    /// Current distance to camera.
    float distance_;
    /// LOD scaled distance.
    float lodDistance_;
    /// Draw distance.
    float drawDistance_;
    /// Shadow distance.
    float shadowDistance_;
    /// Current sort value.
    float sortValue_;
    /// Current minimum view space depth.
    float minZ_;
    /// Current maximum view space depth.
    float maxZ_;
    /// LOD bias.
    float lodBias_;
    /// Base pass flags, bit per batch.
    unsigned basePassFlags_;
    /// Maximum per-pixel lights.
    unsigned maxLights_;
    /// List of cameras from which is seen on the current frame.
    PODVector<Camera*> viewCameras_;
    /// First per-pixel light added this frame.
    Light* firstLight_;
    /// Per-pixel lights affecting this drawable.
    PODVector<Light*> lights_;
    /// Per-vertex lights affecting this drawable.
    PODVector<Light*> vertexLights_;
	///模型是否有运动，用于区分是否可以用于静态阴影
	DynamicType dynamicType_;
	///为了实现特效，一些drawable需要做一些额外的渲染，这些单独的pass就是做这个的。
	HashMap<String, SharedPtr<Pass> > separatePasses_;
	///为了lod材质切换时，材质lod切换，shader参数的变化能保留下来，在drawable级别存下这些shader参数
	HashMap<StringHash, MaterialShaderParameter> overrideShaderParameters_;
	///有的特效需要切换技术来实现，所以增加overrideTechnique属性，该属性会覆盖材质的技术
	SharedPtr<Technique> overrideTechnique_;
	///override render state
	FillMode overrideFillMode_;
private:
	bool subscribed_{};
	HashMap<StringHash, SharedPtr<OverrideShaderParameterAnimationInfo> > overrideShaderParameterAnimationInfos_;
};

inline bool CompareDrawables(Drawable* lhs, Drawable* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

URHO3D_API bool WriteDrawablesToOBJ(PODVector<Drawable*> drawables, File* outputFile, bool asZUp, bool asRightHanded, bool writeLightmapUV = false);

}
