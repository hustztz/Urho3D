#include "../Precompiled.h"
#include "../AngelScript/APITemplates.h"

#include "Graphics/Effects/ModelEffectUtil.h"
#include "Graphics/Effects/WeatherEffectUtil.h"
#include "Filter/OutlineFilter.h"
#include "Filter/SMAAFilter.h"
#include "Filter/UnshadedColorFilter.h"
#include "Filter/TranslucentFilter.h"
#include "Filter/HDRFilter.h"
#include "Filter/BloomHDRFilter.h"
#include "Filter/OuterElecEffectFilter.h"
#include "Filter/VolumeInfluenceFilter.h"
#include "Filter/Deferred/DeferredHBAOFilter.h"
#include "Graphics/Viewport.h"
#include "Graphics/Effects/DayNightWeatherControl.h"
#include "Filter/LensFlareFilter.h"

namespace Urho3D
{
	template <class T> void RegisterFilterBaseAPI(asIScriptEngine* engine, const char* className)
	{
		RegisterObject<T>(engine, className);
		engine->RegisterObjectMethod(className, "void set_enable(bool)", asMETHODPR(T, SetEnable, (bool), void), asCALL_THISCALL);
		engine->RegisterObjectMethod(className, "bool get_enable()", asMETHODPR(T, IsEnable, (), bool), asCALL_THISCALL);
		RegisterSubclass<Object, T>(engine, "Filter", className);
	}

	static void RegisterFilterBaseAPI(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<Filter>(engine, "Filter");
	}

	static OutlineFilter* ConstructOutlineFilter(Viewport* viewPort)
	{
		return new OutlineFilter(GetScriptContext(), viewPort);
	}

	static void RegisterOutlineFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<OutlineFilter>(engine, "OutlineFilter");
		engine->RegisterObjectBehaviour("OutlineFilter", asBEHAVE_FACTORY, "OutlineFilter@+ f(Viewport@+ )", asFUNCTION(ConstructOutlineFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("OutlineFilter", "void AddOutLineModel(Node@+)", asMETHODPR(OutlineFilter, AddOutLineModel, (Node*), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void AddOutLineModel(Node@+, const Color&in)", asMETHODPR(OutlineFilter, AddOutLineModel, (Node*, const Color&), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void AddOutLineModel(Node@+, const Color&in, float)", asMETHODPR(OutlineFilter, AddOutLineModel, (Node*, const Color&, float), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void AddOutLineModelTwinkle(Node@+)", asMETHODPR(OutlineFilter, AddOutLineModelTwinkle, (Node*), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void AddOutLineModelTwinkle(Node@+, const Color&in)", asMETHODPR(OutlineFilter, AddOutLineModelTwinkle, (Node*, const Color&), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void AddOutLineModelTwinkle(Node@+, const Color&in, float)", asMETHODPR(OutlineFilter, AddOutLineModelTwinkle, (Node*, const Color&, float), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void ClearOutlineModel(Node@+)", asMETHOD(OutlineFilter, ClearOutlineModel), asCALL_THISCALL);
		engine->RegisterObjectMethod("OutlineFilter", "void ClearOutlineModelTwinkle(Node@+)", asMETHOD(OutlineFilter, ClearOutlineModelTwinkle), asCALL_THISCALL);
	}
	
	static SMAAFilter* ConstructSMAAFilter(Viewport* viewPort)
	{
		return new SMAAFilter(GetScriptContext(), viewPort);
	}

	static void RegisterSMAAFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<SMAAFilter>(engine, "SMAAFilter");
		engine->RegisterObjectBehaviour("SMAAFilter", asBEHAVE_FACTORY, "SMAAFilter@+ f(Viewport@+ )", asFUNCTION(ConstructSMAAFilter), asCALL_CDECL);
	}

	static UnshadedColorFilter* ConstructUnshadedColorFilter(Viewport* viewPort)
	{
		return new UnshadedColorFilter(GetScriptContext(), viewPort);
	}

	static void RegisterUnshadedColorFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<UnshadedColorFilter>(engine, "UnshadedColorFilter");
		engine->RegisterObjectBehaviour("UnshadedColorFilter", asBEHAVE_FACTORY, "UnshadedColorFilter@+ f(Viewport@+ )", asFUNCTION(ConstructUnshadedColorFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("UnshadedColorFilter", "void AddUnshadedColorModel(Node@+, Color&in)", asMETHODPR(UnshadedColorFilter, AddUnshadedColorModel, (Node*, Color), void), asCALL_THISCALL);
		engine->RegisterObjectMethod("UnshadedColorFilter", "void ClearUnshadedColorModel(Node@+)", asMETHOD(UnshadedColorFilter, ClearUnshadedColorModel), asCALL_THISCALL);
	}

	static TranslucentFilter* ConstructTranslucentFilter(Viewport* viewPort)
	{
		return new TranslucentFilter(GetScriptContext(), viewPort);
	}

	static void RegisterTranslucentFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<Filter>(engine, "TranslucentFilter");
		engine->RegisterObjectBehaviour("TranslucentFilter", asBEHAVE_FACTORY, "TranslucentFilter@+ f(Viewport@+ )", asFUNCTION(ConstructTranslucentFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("TranslucentFilter", "void AddModel(Node@+)", asMETHOD(TranslucentFilter, AddModel), asCALL_THISCALL);
		engine->RegisterObjectMethod("TranslucentFilter", "void ClearModel(Node@+)", asMETHOD(TranslucentFilter, ClearModel), asCALL_THISCALL);
	}

	static HDRFilter* ConstructHDRFilter(Viewport* viewPort)
	{
		return new HDRFilter(GetScriptContext(), viewPort);
	}

	static void RegisterHDRFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<HDRFilter>(engine, "HDRFilter");
		engine->RegisterObjectBehaviour("HDRFilter", asBEHAVE_FACTORY, "HDRFilter@+ f(Viewport@+ )", asFUNCTION(ConstructHDRFilter), asCALL_CDECL);
	}
	static BloomHDRFilter* ConstructBloomHDRFilter(Viewport* viewPort)
	{
		return new BloomHDRFilter(GetScriptContext(), viewPort);
	}

	static void RegisterBloomHDRFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<BloomHDRFilter>(engine, "BloomHDRFilter");
		engine->RegisterObjectBehaviour("BloomHDRFilter", asBEHAVE_FACTORY, "BloomHDRFilter@+ f(Viewport@+ )", asFUNCTION(ConstructBloomHDRFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("BloomHDRFilter", "void SetBloomHDRThreshold(float)", asMETHOD(BloomHDRFilter, SetBloomHDRThreshold), asCALL_THISCALL);
	}
	
	static OuterElecEffectFilter* ConstructOuterElecEffectFilter(Viewport* viewPort)
	{
		return new OuterElecEffectFilter(GetScriptContext(), viewPort);
	}

	static void RegisterOuterElecEffectFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<OuterElecEffectFilter>(engine, "OuterElecEffectFilter");
		engine->RegisterObjectBehaviour("OuterElecEffectFilter", asBEHAVE_FACTORY, "OuterElecEffectFilter@+ f(Viewport@+ )", asFUNCTION(ConstructOuterElecEffectFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("OuterElecEffectFilter", "void AddOuterElecModel(Node@+, bool, float, float, Color, float)", asMETHOD(OuterElecEffectFilter, AddOuterElecModel), asCALL_THISCALL);
		engine->RegisterObjectMethod("OuterElecEffectFilter", "void ClearOuterElecModel(Node@+)", asMETHOD(OuterElecEffectFilter, ClearOuterElecModel), asCALL_THISCALL);
	}

	static VolumeInfluenceFilter* ConstructVolumeInfluenceFilter(Viewport* viewPort)
	{
		return new VolumeInfluenceFilter(GetScriptContext(), viewPort);
	}

	static void RegisterVolumeInfluenceFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<VolumeInfluenceFilter>(engine, "VolumeInfluenceFilter");
		engine->RegisterObjectBehaviour("VolumeInfluenceFilter", asBEHAVE_FACTORY, "VolumeInfluenceFilter@+ f(Viewport@+ )", asFUNCTION(ConstructVolumeInfluenceFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("VolumeInfluenceFilter", "void SetVolume(Node@+)", asMETHOD(VolumeInfluenceFilter, SetVolume), asCALL_THISCALL);
		engine->RegisterObjectMethod("VolumeInfluenceFilter", "void SetMarkColor(Color)", asMETHOD(VolumeInfluenceFilter, SetMarkColor), asCALL_THISCALL);
		engine->RegisterObjectMethod("VolumeInfluenceFilter", "Color GetMarkColor(Color)", asMETHOD(VolumeInfluenceFilter, GetMarkColor), asCALL_THISCALL);
	}

	static DeferredHBAOFilter* ConstructDeferredHBAOFilter(Viewport* viewPort)
	{
		return new DeferredHBAOFilter(GetScriptContext(), viewPort);
	}

	static void RegisterDeferredHBAOFilter(asIScriptEngine* engine)
	{
		engine->RegisterEnum("HBAOQuality");
		engine->RegisterEnumValue("HBAOQuality", "Low",    Low);
		engine->RegisterEnumValue("HBAOQuality", "Middle", Middle);
		engine->RegisterEnumValue("HBAOQuality", "High",   High);

		RegisterFilterBaseAPI<DeferredHBAOFilter>(engine, "DeferredHBAOFilter");
		engine->RegisterObjectBehaviour("DeferredHBAOFilter", asBEHAVE_FACTORY, "DeferredHBAOFilter@+ f(Viewport@+ )", asFUNCTION(ConstructDeferredHBAOFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("DeferredHBAOFilter", "void SetHBAOQuality(HBAOQuality)", asMETHOD(DeferredHBAOFilter, SetHBAOQuality), asCALL_THISCALL);
		engine->RegisterObjectMethod("DeferredHBAOFilter", "void SetHBAOIntensity(float)", asMETHOD(DeferredHBAOFilter, SetHBAOIntensity), asCALL_THISCALL);
	}
	
	static LensFlareFilter* ConstructLensFlareFilter(Viewport* viewPort)
	{
		return new LensFlareFilter(GetScriptContext(), viewPort);
	}

	static void RegisterLensFlareFilter(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI<LensFlareFilter>(engine, "LensFlareFilter");
		engine->RegisterObjectBehaviour("LensFlareFilter", asBEHAVE_FACTORY, "LensFlareFilter@+ f(Viewport@+ )", asFUNCTION(ConstructLensFlareFilter), asCALL_CDECL);
		engine->RegisterObjectMethod("LensFlareFilter", "void SetEnable(bool)", asMETHOD(LensFlareFilter, SetEnable), asCALL_THISCALL);
	}

	static void RegisterModelEffectUtilAPI(asIScriptEngine* engine)
	{
		engine->RegisterEnum("DynamicType");
		engine->RegisterEnumValue("DynamicType", "Dynamic", Dynamic);
		engine->RegisterEnumValue("DynamicType", "Static", Static);

		const char* np = engine->GetDefaultNamespace();
		engine->SetDefaultNamespace("ModelEffectUtil");

		engine->RegisterGlobalFunction("void SetOutlinePerspect(const Viewport@+, Node@+)", asFUNCTIONPR(&ModelEffectUtil::SetOutlinePerspect, (const Viewport*, Node*), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void SetOutlinePerspect(const Viewport@+, Node@+, Color)", asFUNCTIONPR(&ModelEffectUtil::SetOutlinePerspect, (const Viewport*, Node*, Color) ,void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelOutlinePerspect(const Viewport@+, Node@+)", asFUNCTION(&ModelEffectUtil::CancelOutlinePerspect), asCALL_CDECL);
		engine->RegisterGlobalFunction("void SetOutlinePerspectTwinkle(const Viewport@+, Node@+)", asFUNCTIONPR(&ModelEffectUtil::SetOutlinePerspectTwinkle, (const Viewport*, Node*), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void SetOutlinePerspectTwinkle(const Viewport@+, Node@+, Color)", asFUNCTIONPR(&ModelEffectUtil::SetOutlinePerspectTwinkle, (const Viewport*, Node*, Color), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelOutlinePerspectTwinkle(const Viewport@+, Node@+)", asFUNCTION(&ModelEffectUtil::CancelOutlinePerspectTwinkle), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetOuterElecEffect(const Viewport@+, Node@+)", asFUNCTIONPR(&ModelEffectUtil::SetOuterElecEffect, (const Viewport*, Node*), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void SetOuterElecEffect(const Viewport@+, Node@+, Color)", asFUNCTIONPR(&ModelEffectUtil::SetOuterElecEffect, (const Viewport*, Node*, Color), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void SetOuterElecEffect(const Viewport@+, Node@+, bool, float, float, Color, float)", asFUNCTIONPR(&ModelEffectUtil::SetOuterElecEffect, (const Viewport*, Node*, bool, float, float, Color, float), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelOuterElecEffect(const Viewport@+, Node@+)", asFUNCTION(&ModelEffectUtil::CancelOuterElecEffect), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetCollisionEffect(const Viewport@+, Node@+)", asFUNCTIONPR(&ModelEffectUtil::SetCollisionEffect, (const Viewport*, Node*), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void SetCollisionEffect(const Viewport@+, Node@+, Color)", asFUNCTIONPR(&ModelEffectUtil::SetCollisionEffect, (const Viewport*, Node*, Color), void), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelCollisionEffect(const Viewport@+)", asFUNCTION(&ModelEffectUtil::CancelCollisionEffect), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetDiffuse(Node@+, Color)", asFUNCTION(&ModelEffectUtil::SetDiffuse), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelDiffuse(Node@+)", asFUNCTION(&ModelEffectUtil::CancelDiffuse), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetUnshadedColor(const Viewport@+, Node@+, Color)", asFUNCTION(&ModelEffectUtil::SetUnshadedColor), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelUnshadedColor(const Viewport@+, Node@+)", asFUNCTION(&ModelEffectUtil::CancelUnshadedColor), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetModelTransparent(Node@+, float)", asFUNCTION(&ModelEffectUtil::SetModelTransparent), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelModelTransparent(Node@+)", asFUNCTION(&ModelEffectUtil::CancelModelTransparent), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetModelUnshadedTransparent(Node@+, Color)", asFUNCTION(&ModelEffectUtil::SetModelUnshadedTransparent), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelModelUnshadedTransparent(Node@+)", asFUNCTION(&ModelEffectUtil::CancelModelUnshadedTransparent), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetWireframe(Node@+)", asFUNCTION(&ModelEffectUtil::SetWireframe), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelWireframe(Node@+)", asFUNCTION(&ModelEffectUtil::CancelWireframe), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetTranslucent(const Viewport@+, Node@+)", asFUNCTION(&ModelEffectUtil::SetTranslucent), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelTranslucent(const Viewport@+, Node@+)", asFUNCTION(&ModelEffectUtil::CancelTranslucent), asCALL_CDECL);
		
		engine->RegisterGlobalFunction("void SetBloom(Node@+, Color)", asFUNCTION(&ModelEffectUtil::SetBloom), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelBloom(Node@+)", asFUNCTION(&ModelEffectUtil::CancelBloom), asCALL_CDECL);
		
		engine->RegisterGlobalFunction("void SetDynamicHint(Node@+, DynamicType)", asFUNCTION(&ModelEffectUtil::SetDynamicHint), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetModelVisible(Node@+, bool)", asFUNCTION(&ModelEffectUtil::SetModelVisible), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetTwinkle(Node@+, float, Color)", asFUNCTION(&ModelEffectUtil::SetTwinkle), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelTwinkle(Node@+)", asFUNCTION(&ModelEffectUtil::CancelTwinkle), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetNormalShowEffect(Node@+)", asFUNCTION(&ModelEffectUtil::SetNormalShowEffect), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelNormalShowEffect(Node@+)", asFUNCTION(&ModelEffectUtil::CancelNormalShowEffect), asCALL_CDECL);
		
		engine->SetDefaultNamespace(np);
	}

	static void RegisterWeatherEffectUtilAPI(asIScriptEngine* engine)
	{
		const char* np = engine->GetDefaultNamespace();
		engine->SetDefaultNamespace("WeatherEffectUtil");

		engine->RegisterGlobalFunction("void SetRainEffect(Camera@+)", asFUNCTION(&WeatherEffectUtil::SetRainEffect), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelRainEffect(Camera@+)", asFUNCTION(&WeatherEffectUtil::CancelRainEffect), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetSnowEffect(Camera@+)", asFUNCTION(&WeatherEffectUtil::SetSnowEffect), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelSnowEffect(Camera@+)", asFUNCTION(&WeatherEffectUtil::CancelSnowEffect), asCALL_CDECL);

		engine->RegisterGlobalFunction("void SetFogEffect(Zone@+, float, float, Color)", asFUNCTION(&WeatherEffectUtil::SetFogEffect), asCALL_CDECL);
		engine->RegisterGlobalFunction("void CancelFogEffect(Zone@+)", asFUNCTION(&WeatherEffectUtil::CancelFogEffect), asCALL_CDECL);

		engine->SetDefaultNamespace(np);
	}

	static DayNightWeatherControl* ConstructDayNightWeatherControl(Viewport* viewPort)
	{
		return new DayNightWeatherControl(GetScriptContext());
	}

	static void RegisterDayNightWeatherControl(asIScriptEngine* engine)
	{
		RegisterComponent<DayNightWeatherControl>(engine, "DayNightWeatherControl");
		engine->RegisterObjectBehaviour("DayNightWeatherControl", asBEHAVE_FACTORY, "DayNightWeatherControl@+ f()", asFUNCTION(ConstructDayNightWeatherControl), asCALL_CDECL);
		engine->RegisterObjectMethod("DayNightWeatherControl", "void SetWeatherRatio(float)", asMETHOD(DayNightWeatherControl, SetWeatherRatio), asCALL_THISCALL);
	}

	void RegisterFiltersAPI(asIScriptEngine* engine)
	{
		RegisterFilterBaseAPI(engine);

		RegisterOutlineFilter(engine);
		RegisterSMAAFilter(engine);
		RegisterUnshadedColorFilter(engine);
		RegisterTranslucentFilter(engine);
		RegisterHDRFilter(engine);
		RegisterBloomHDRFilter(engine);
		RegisterLensFlareFilter(engine);

		RegisterOuterElecEffectFilter(engine);
		RegisterVolumeInfluenceFilter(engine);
		RegisterDeferredHBAOFilter(engine);

		RegisterModelEffectUtilAPI(engine);
		RegisterWeatherEffectUtilAPI(engine);
		RegisterDayNightWeatherControl(engine);

	}
}