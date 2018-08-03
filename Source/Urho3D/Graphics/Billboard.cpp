#include "Graphics/Billboard.h"
#include "Core/Context.h"
#include "Graphics/Geometry.h"
#include "Graphics/Material.h"
#include "Resource/ResourceCache.h"
#include "Graphics/Technique.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/Texture2D.h"
#include "Scene/Node.h"

namespace Urho3D
{
	extern const char* GEOMETRY_CATEGORY;

	BillboardDrawable::BillboardDrawable(Context* context) :
		Drawable(context, DRAWABLE_GEOMETRY),
		fadeNearDistance_(0.f),
		fadeFarDistance_(1000.f),
		size_(10,10),
		visible_(true),
		onTop_(false),
		bindedModel_(nullptr)
	{
		batches_.Resize(1);
		batches_[0].geometry_ = new Geometry(context_);
		Material* mat = new Material(context);
		auto* cache = GetSubsystem<ResourceCache>();
		mat->SetTechnique(0, cache->GetResource<Technique>("Techniques/Billboard.xml")->Clone());
		SetMaterial(mat);
		SetDefaultTexture(cache->GetResource<Texture2D>("Textures/VkingLogo.png"));
		SetFadeFarDistance(1000);
		SetFadeNearDistance(0);
		SetSize(size_);

		CreateQuad();
	}

	BillboardDrawable::BillboardDrawable(Context* context, unsigned char) :
		Drawable(context, DRAWABLE_GEOMETRY),
		size_(100, 100),
		onTop_(false),
		bindedModel_(nullptr)
	{
		batches_.Resize(1);
		batches_[0].geometry_ = new Geometry(context_);
	}

	BillboardDrawable::~BillboardDrawable()
	{
		delete batches_[0].geometry_;
	}

	void BillboardDrawable::RegisterObject(Context* context)
	{
		context->RegisterFactory<BillboardDrawable>(GEOMETRY_CATEGORY);
	}

	void BillboardDrawable::SetDefaultTexture(Texture2D* tex)
	{
		defaultTexture_ = tex;
		batches_[0].material_->SetTexture(TU_DIFFUSE, defaultTexture_);
	}

	void BillboardDrawable::SetHoverTexture(Texture2D* tex)
	{
		hoverTexture_ = tex;
		batches_[0].material_->SetTexture(TU_DIFFUSE, defaultTexture_);
	}

	void BillboardDrawable::SetFadeNearDistance(float distance)
	{
		fadeNearDistance_ = distance;
		batches_[0].material_->SetShaderParameter("BillBoardFadeNearDistance", distance);
	}

	void BillboardDrawable::SetFadeFarDistance(float distance)
	{
		fadeFarDistance_ = distance;
		batches_[0].material_->SetShaderParameter("BillBoardFadeDistance", distance);
	}

	void BillboardDrawable::SetSize(Vector2 size)
	{
		size_ = size;
		batches_[0].material_->SetShaderParameter("BillboardSize", size);
	}
	
	void BillboardDrawable::SetOnTop(bool enable)
	{
		onTop_ = enable;
		if(onTop_)
			batches_[0].material_->GetTechnique(0)->GetPass("alpha")->SetDepthTestMode(CompareMode::CMP_ALWAYS);
	}

	void BillboardDrawable::BindModel(Node* node, Vector3 bindedOffset)
	{
		bindedModel_ = node;
		bindedOffset_ = bindedOffset;

		if(bindedModel_)
		{			
			node_->SetWorldPosition(bindedModel_->GetWorldPosition() + bindedOffset);
			bindedModel_->AddListener(this);
		}
	}
	void BillboardDrawable::OnMarkedDirty(Node* node)
	{
		if(node == node_)
			Drawable::OnMarkedDirty(node_);
		if(node == bindedModel_)
			node_->SetWorldPosition(bindedModel_->GetWorldPosition() + bindedOffset_);
	}

	void BillboardDrawable::SetVisible(bool enable)
	{
		if(visible_ != enable)
		{
			visible_ = enable;
			if(visible_)
			{
				SetOverrideViewMask(0xFFFFFFFF);
			}else
			{
				SetOverrideViewMask(0x00000000);
			}
		}
	}

	void BillboardDrawable::SetMaterial(Material* mat)
	{
		batches_[0].material_ = mat;

		MarkNetworkUpdate();
	}

	void BillboardDrawable::CreateQuad()
	{
		float width = 1.f;
		float height = 1.f;
		unsigned char vertexNum = 4;
		unsigned char indexNum = 6;

		SharedPtr<IndexBuffer> ib(new IndexBuffer(context_));
		SharedPtr<VertexBuffer> vb(new VertexBuffer(context_));
		unsigned int elementMask = MASK_POSITION;
		elementMask |= MASK_TEXCOORD1;
		vb->SetSize(vertexNum, elementMask);
		ib->SetSize(indexNum, false);

		boundingBox_.Clear();

		auto* dest = (unsigned char*)vb->Lock(0, vertexNum, true);	
		
		*((Vector3*)dest) = Vector3(-width / 2, -height / 2, 0);
		boundingBox_.Merge(*((Vector3*)dest));
		dest += sizeof(Vector3);
		*((Vector2*)dest) = Vector2(0, 1);
		dest += sizeof(Vector2);

		*((Vector3*)dest) = Vector3(width / 2, -height / 2, 0);
		boundingBox_.Merge(*((Vector3*)dest));
		dest += sizeof(Vector3);
		*((Vector2*)dest) = Vector2(1, 1);
		dest += sizeof(Vector2);

		*((Vector3*)dest) = Vector3(width / 2, height / 2, 0);
		boundingBox_.Merge(*((Vector3*)dest));
		dest += sizeof(Vector3);
		*((Vector2*)dest) = Vector2(1, 0);
		dest += sizeof(Vector2);

		*((Vector3*)dest) = Vector3(-width / 2, height / 2, 0);
		boundingBox_.Merge(*((Vector3*)dest));
		dest += sizeof(Vector3);
		*((Vector2*)dest) = Vector2(0, 0);
		dest += sizeof(Vector2);
		vb->Unlock();

		auto ibDest = (unsigned short*)ib->Lock(0, indexNum, true);
		*ibDest = 0;
		++ibDest;
		*ibDest = 2;
		++ibDest;
		*ibDest = 1;
		++ibDest;
		*ibDest = 0;
		++ibDest;
		*ibDest = 3;
		++ibDest;
		*ibDest = 2;
		++ibDest;
		ib->Unlock();
		
		batches_[0].geometry_->SetVertexBuffer(0, vb);
		batches_[0].geometry_->SetIndexBuffer(ib);
		batches_[0].geometry_->SetDrawRange(PrimitiveType::TRIANGLE_LIST, 0, indexNum, 0, vertexNum);
	}

	void BillboardDrawable::OnWorldBoundingBoxUpdate()
	{
		worldBoundingBox_ = boundingBox_.Transformed(node_->GetWorldTransform());
	}
	BillboardGUIDrawable::BillboardGUIDrawable(Context* context) :
		BillboardDrawable(context, 0),
		zOffset_(0.f),
		hoverEnlarge_(false)
	{
		Material* mat = new Material(context);
		auto* cache = GetSubsystem<ResourceCache>();
		mat->SetTechnique(0, cache->GetResource<Technique>("Techniques/BillboardGUI.xml")->Clone());
		SetMaterial(mat);
		SetDefaultTexture(cache->GetResource<Texture2D>("Textures/VkingLogo.png"));
		SetFadeFarDistance(1000);
		SetFadeNearDistance(0);
		SetSize(size_);

		CreateQuad();
	}

	BillboardGUIDrawable::~BillboardGUIDrawable()
	{
	}

	void BillboardGUIDrawable::RegisterObject(Context* context)
	{
		context->RegisterFactory<BillboardGUIDrawable>(GEOMETRY_CATEGORY);
	}

	void BillboardGUIDrawable::SetScreenOffset(Vector2 screenOffset)
	{
		screenOffset_ = screenOffset;
		batches_[0].material_->SetShaderParameter("BillBoardScreenOffset", screenOffset_);
	}

	void BillboardGUIDrawable::SetZOffset(float zOffset)
	{
		zOffset_ = zOffset;
		batches_[0].material_->SetShaderParameter("BillBoardZOffset", zOffset_);
	}
}
