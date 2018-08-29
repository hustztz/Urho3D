funcdef void DrawEffectFunc(Variant&);
funcdef void CancelEffectFunc(void);

bool ginscene = false;

class DrawEffectDesc
{
    bool PostEffect = false;
    bool mIsAble;
    String mEffectName;
    Variant mArgMap;
    DrawEffectFunc@   mDrawEffectFunc;
    CancelEffectFunc@ mClearEffectFunc;
    EditorSelectEffectInfoData@ mSaveData;
    float titlew  = 80;
    float offsetx = 40;
    float width   = 250;
    float height  = 24;

    Array<LineEdit@> mColorWnd;

    DrawEffectDesc(String effectname, DrawEffectFunc@ drawfunc, CancelEffectFunc@ clearfunc, Variant var = Variant(), bool able = false)
    {
        mEffectName = effectname;
        @mDrawEffectFunc = @drawfunc;
        @mClearEffectFunc = @clearfunc;
        bool preable;
        Variant prevar;
        mSaveData = gEditorSelectEffectInfoDataMgr.havaEffectInfo(this, effectname, preable, prevar);
        if(mSaveData is null)
        {
            mArgMap = var;
            mIsAble = able;
            mSaveData = gEditorSelectEffectInfoDataMgr.AddEffectInfo(this, effectname, able, var);
        }
        else
        {
            mIsAble = preable;
            mArgMap = prevar;
        }
        SetAble(mIsAble);
    }

    void SetAble(bool able)
    {
        execClearEffect();
        mIsAble = able;
        execDrawEffect();
    }

    void execDrawEffect()
    {
        if(mIsAble)
        {
            mDrawEffectFunc(mArgMap);
        }
    }

    void execClearEffect()
    {
        if(mIsAble)
        {
            mClearEffectFunc();
        }
    }

    Array<UIElement@>@ CreateArgWnd()
    {
        Array<UIElement@> arglist;
        
        if (mArgMap.type == VAR_COLOR)
        {
            UIElement@ agrwnd = CreateColorWnd(mArgMap.GetColor());
            arglist.Push(agrwnd);
        }
        else if(mArgMap.type == VAR_VARIANTMAP)
        {
            VariantMap map = mArgMap.GetVariantMap();
            int i=1;
            float posx = 0;

            while(true)
            {
                Variant argvar = map["Arg"+i];
                VariantType argtype = argvar.type;
                UIElement@ agrwnd;
                if(argtype == VAR_FLOAT)
                {
                   agrwnd = CreateFloatWnd(map["ArgName"+i].GetString(), argvar.GetFloat(), i);
                   arglist.Push(agrwnd);
                }
                else if(argtype == VAR_COLOR)
                {
                   agrwnd = CreateColorWnd(argvar.GetColor(), i);
                   arglist.Push(agrwnd);
                }
                else if(argtype == VAR_BOOL)
                {
                    agrwnd = CreateBoolWnd(map["ArgName"+i].GetString(), argvar.GetBool(), i);
                    arglist.Push(agrwnd);
                }
                else
                {
                    break;
                }
                ++i;
            }
        }
        
        return arglist;
    }
    
    UIElement@ CreateFloatWnd(String argname, float value, int argindex)
    {
        UIElement@ agrwnd = UIElement();
        agrwnd.SetFixedSize(width, height);
        agrwnd.defaultStyle = uiStyle;

        Text@ title = CreateTitleWnd(agrwnd, argname);
        title.SetPosition(offsetx, 0);
        LineEdit@ text = CreateAttributeLineEdit(agrwnd);
        text.SetFixedWidth(80);
        text.SetPosition(titlew+offsetx, 0);
        text.text = String(value);
        text.vars["ArgIndex"] = argindex;
        SubscribeToEvent(text, "TextFinished", "HandleFloatArgChangeEvent");

        return agrwnd;
    }

    UIElement@ CreateColorWnd(Color color, int argindex = -1)
    {
        UIElement@ agrwnd = UIElement();
        agrwnd.SetFixedSize(width, height);
        agrwnd.defaultStyle = uiStyle;
        
        float space = 45;
        LineEdit@ text;
        Text@ title = CreateTitleWnd(agrwnd, "颜色:");
        title.SetPosition(offsetx, 0);
        float posx = titlew+ offsetx;

        text = CreateAttributeLineEdit(agrwnd);
        text.SetPosition(space*0 + posx, 0);
        text.text = String(color.r);
        text.vars["ArgIndex"] = argindex;
        SubscribeToEvent(text, "TextFinished", "HandleInputColorChangeEvent");
        mColorWnd.Push(text);

        text = CreateAttributeLineEdit(agrwnd);
        text.SetPosition(space*1 + posx, 0);
        text.text = String(color.g);
        text.vars["ArgIndex"] = argindex;
        SubscribeToEvent(text, "TextFinished", "HandleInputColorChangeEvent");
        mColorWnd.Push(text);

        text = CreateAttributeLineEdit(agrwnd);
        text.SetPosition(space*2 + posx, 0);
        text.text = String(color.b);
        text.vars["ArgIndex"] = argindex;
        SubscribeToEvent(text, "TextFinished", "HandleInputColorChangeEvent");
        mColorWnd.Push(text);

        return agrwnd;
    }

    UIElement@ CreateBoolWnd(String argname, bool able, int argindex = -1)
    {
        UIElement@ agrwnd = UIElement();
        agrwnd.SetFixedSize(width, height);
        Text@ title = CreateTitleWnd(agrwnd, argname);
        title.SetPosition(offsetx, 0);
        
        CheckBox@ ablebox = CheckBox();
        agrwnd.AddChild(ablebox);

        ablebox.SetPosition(titlew + offsetx, 0);
        ablebox.defaultStyle = uiStyle;
        ablebox.SetStyleAuto();
        ablebox.checked = able;
        ablebox.vars["ArgIndex"] = argindex;
        SubscribeToEvent(ablebox, "Toggled", "HandleBoolToggled");
        return agrwnd;
    }

    Text@ CreateTitleWnd(UIElement@ parent, String title)
    {
        Text@ text = Text();
        text.text = title;
        text.fontSize = 11; 
        parent.AddChild(text);
        return text;
    }

    LineEdit@ CreateAttributeLineEdit(UIElement@ parent)
    {
        LineEdit@ attrEdit = LineEdit();
        parent.AddChild(attrEdit);
        
        attrEdit.style = "EditorAttributeEdit";
        attrEdit.SetFixedWidth(40);
        attrEdit.SetFixedHeight(ATTR_HEIGHT - 2);
        return attrEdit;
    }

    void HandleInputColorChangeEvent(StringHash eventType, VariantMap& eventData)
    {
        LineEdit@ line = cast<LineEdit@>(eventData["Element"].GetPtr());
        String text = eventData["Text"].GetString();
        float dis = text.ToFloat();
        if(dis < 0.0)
        {
            dis = 0.0;
        }
        line.text = String(dis);
        float r = mColorWnd[0].text.ToFloat();
        float g = mColorWnd[1].text.ToFloat();
        float b = mColorWnd[2].text.ToFloat();

        int argindex = line.vars["ArgIndex"].GetInt();
        RefreshColorValue(r, g, b, argindex);
    }

    void RefreshColorValue(float r, float g, float b, int argindex)
    {
        if (mArgMap.type == VAR_COLOR)
        {
            execClearEffect();
            mArgMap = Color(r, g, b);                
            execDrawEffect();
        }
        else if(mArgMap.type == VAR_VARIANTMAP && argindex > 0)
        {
            execClearEffect();
            VariantMap map = mArgMap.GetVariantMap();
            map["Arg"+argindex] = Color(r, g, b);
            mArgMap = map;
            execDrawEffect();
        }
        mSaveData.RefreshVar(mArgMap);
    }

    void HandleFloatArgChangeEvent(StringHash eventType, VariantMap& eventData)
    {
        LineEdit@ line = cast<LineEdit@>(eventData["Element"].GetPtr());
        int argindex = line.vars["ArgIndex"].GetInt();
        String text = eventData["Text"].GetString();
        float value = text.ToFloat();
        line.text = String(value);
        if(mArgMap.type == VAR_VARIANTMAP && argindex > 0)
        {
            execClearEffect();
            VariantMap map = mArgMap.GetVariantMap();
            map["Arg"+argindex] = value;
            mArgMap = map;            
            execDrawEffect();
        }
        mSaveData.RefreshVar(mArgMap);
    }

    void HandleBoolToggled(StringHash eventType, VariantMap& eventData)
    {
        CheckBox@ ablebox = cast<CheckBox@>(eventData["Element"].GetPtr());
        int argindex = ablebox.vars["ArgIndex"].GetInt();        

        if(mArgMap.type == VAR_VARIANTMAP && argindex > 0)
        {
            execClearEffect();
            VariantMap map = mArgMap.GetVariantMap();
            map["Arg"+argindex] = ablebox.checked;
            mArgMap = map;            
            execDrawEffect();
        }
        mSaveData.RefreshVar(mArgMap);
    }

}

class EditorSelectEffect
{
    Viewport@ mViewport;
    Camera@ mCamera;
    OutlineFilter@ mOutlineFilter;
    OuterElecEffectFilter@ mOuterElecEffectFilter;
    UnshadedColorFilter@ mUnshadedColorFilter;
    TranslucentFilter@ mTranslucentFilter;
    VolumeInfluenceFilter@ mVolumeInfluenceFilter;
    AfterHDRANDBloomFilter@ mAfterHDRANDBloomFilter;
    
    HDRFilter@ mHDRFilter;
    SMAAFilter@ mSMAAFilter;
    TemporalAAFilter@ mTemporalAAFilter;
    BloomHDRFilter@ mBloomHDRFilter;
    DeferredHBAOFilter@ mDeferredHBAOFilter;
    DeferredHBAOTemporalFilter@ mDeferredHBAOTemporalFilter;
    LensFlareFilter@ mLensFlareFilter;
    TranslucentAfterHDRFilter@ mTranslucentAfterHDRFilter;
    DepthOfFieldFilter@ mDepthOfFieldFilter;
    EDLFilter@ mEDLFilter;
    VignetteFilter@ mVignetteFilter;
    
    Node@ mCurEffectNode;

    Array<DrawEffectDesc@> mEffectDescList;
    Array<DrawEffectDesc@> mDefferDescList;
    
    EditorSelectEffect(ViewportContext@ vpcontext)
    {
        Viewport@ viewport = vpcontext.viewport;
        @mViewport = @viewport;
        @mCamera = vpcontext.camera;

        mDeferredHBAOFilter = DeferredHBAOFilter(viewport);
        viewport.AddFilter(mDeferredHBAOFilter);
        mDeferredHBAOFilter.enable = false;

        mDeferredHBAOTemporalFilter = DeferredHBAOTemporalFilter(viewport);
      //  viewport.AddFilter(mDeferredHBAOTemporalFilter);
        mDeferredHBAOTemporalFilter.enable = false;

        mTranslucentFilter = TranslucentFilter(viewport);
        viewport.AddFilter(mTranslucentFilter);

        mBloomHDRFilter = BloomHDRFilter(viewport);
        viewport.AddFilter(mBloomHDRFilter);
    //  mBloomHDRFilter.enable = false;

        mLensFlareFilter = LensFlareFilter(viewport);
        viewport.AddFilter(mLensFlareFilter);
        mLensFlareFilter.enable = false;

        mHDRFilter = HDRFilter(viewport);
        viewport.AddFilter(mHDRFilter);
        mHDRFilter.enable = false;

        mAfterHDRANDBloomFilter = AfterHDRANDBloomFilter(viewport);
        viewport.AddFilter(mAfterHDRANDBloomFilter);
    
        mOutlineFilter = OutlineFilter(viewport);
        viewport.AddFilter(mOutlineFilter);

        mOuterElecEffectFilter = OuterElecEffectFilter(viewport);
        viewport.AddFilter(mOuterElecEffectFilter);

        mUnshadedColorFilter = UnshadedColorFilter(viewport);
        viewport.AddFilter(mUnshadedColorFilter);

        mVolumeInfluenceFilter = VolumeInfluenceFilter(viewport);
        viewport.AddFilter(mVolumeInfluenceFilter);

        mTranslucentAfterHDRFilter = TranslucentAfterHDRFilter(viewport);
        viewport.AddFilter(mTranslucentAfterHDRFilter);

        mDepthOfFieldFilter = DepthOfFieldFilter(viewport);
        viewport.AddFilter(mDepthOfFieldFilter);
        mDepthOfFieldFilter.enable = false;

        mSMAAFilter = SMAAFilter(viewport);
        viewport.AddFilter(mSMAAFilter);
        mSMAAFilter.enable = false;

        mTemporalAAFilter = TemporalAAFilter(viewport);
        viewport.AddFilter(mTemporalAAFilter);
        mTemporalAAFilter.enable = false;
        
        mVignetteFilter = VignetteFilter(viewport);
        viewport.AddFilter(mVignetteFilter);
        mVignetteFilter.enable = false;

        mEDLFilter = EDLFilter(viewport);
        viewport.AddFilter(mEDLFilter);
        mEDLFilter.enable = false;

        

        InitEffectDescList();

        if (editorSelectEffectWindow !is null)
        {
            editorSelectEffectWindow.RefreshUI();
        }
    }

    ~EditorSelectEffect()
    {
        ClearModelEffect();
    }

    void DrawModelEffect(Node@ node)
    {
        if(node is mCurEffectNode) return;

        ClearModelEffect();
        
        mCurEffectNode = node;

        InternalDrawModelEffect();
    }

    void InternalDrawModelEffect()
    {
        if(mCurEffectNode !is null)
        {
            for(int i=0; i < mEffectDescList.length; ++i)
            {
                mEffectDescList[i].execDrawEffect();
            }
        }
    }

    void ClearModelEffect()
    {
        if(mCurEffectNode !is null)
        {
            for(int i=0; i < mEffectDescList.length; ++i)
            {
                mEffectDescList[i].execClearEffect();
            }
        }
        mCurEffectNode = null;
    }
 
    void InitEffectDescList()
    {
    ///描边效果
        mEffectDescList.Push(DrawEffectDesc("描边", DrawEffectFunc(this.SetOutlinePerspect), CancelEffectFunc(this.CancelOutlinePerspect), Variant(Color(1,0.5,0)) , true));
    ///闪烁描边效果
        mEffectDescList.Push(DrawEffectDesc("闪烁描边", DrawEffectFunc(this.SetOutlinePerspectTwinkle), CancelEffectFunc(this.CancelOutlinePerspectTwinkle), Variant(Color(1,0.5,0))));
    ///电线外面的电流效果    
    {
        VariantMap map;
        map["ArgName1"] = "X方向:";
        map["Arg1"]     = false;

        map["ArgName2"] = "边距:";
        map["Arg2"]     = float(0.01);
        
        map["Arg3"]     = float(1);
        map["ArgName3"] = "速度:";

        map["Arg4"]     = Color(0,0,1);
        map["Arg5"]     = float(2.0);
        map["ArgName5"] = "采样数:";

        // bool xDir, float entend, float speed, Color color, float sample
        mEffectDescList.Push(DrawEffectDesc("电流", DrawEffectFunc(this.SetOuterElecEffect), CancelEffectFunc(this.CancelOuterElecEffect), Variant(map)));
    }
    ///相交部分高亮闪烁
        mEffectDescList.Push(DrawEffectDesc("相交部分高亮闪烁", DrawEffectFunc(this.SetCollisionEffect), CancelEffectFunc(this.CancelCollisionEffect), Variant(Color(1,0.5,0))));
    ///通过改变模型diffuse颜色，是模型外观泛出这种颜色
        mEffectDescList.Push(DrawEffectDesc("diffuse", DrawEffectFunc(this.SetDiffuse), CancelEffectFunc(this.CancelDiffuse), Variant(Color(1,0.5,0))));
    ///模型unshaded着色
        mEffectDescList.Push(DrawEffectDesc("模型纯色", DrawEffectFunc(this.SetUnshadedColor), CancelEffectFunc(this.CancelUnshadedColor), Variant(Color(1,0.5,0))));
    ///模型透明 
    {
        VariantMap map;
        map["Arg1"]     = float(0.5);
        map["ArgName1"] = "Alpha:";
        mEffectDescList.Push(DrawEffectDesc("模型透明", DrawEffectFunc(this.SetModelTransparent), CancelEffectFunc(this.CancelModelTransparent), Variant(map)));
    }
    ///模型纯色透明
    {
        VariantMap map;
        map["Arg1"]     = Color(1,0.5,0);
        map["Arg2"]     = float(0.5);
        map["ArgName2"] = "Alpha:";
        mEffectDescList.Push(DrawEffectDesc("模型纯色透明", DrawEffectFunc(this.SetModelUnshadedTransparent), CancelEffectFunc(this.CancelModelUnshadedTransparent), Variant(map)));
    }
    ///线框
        mEffectDescList.Push(DrawEffectDesc("线框", DrawEffectFunc(this.SetWireframe), CancelEffectFunc(this.CancelWireframe)));
    ///穿透显示
        mEffectDescList.Push(DrawEffectDesc("穿透显示", DrawEffectFunc(this.SetTranslucent), CancelEffectFunc(this.CancelTranslucent), Variant(Color(1,0.5,0))));
    ///发光效果
        mEffectDescList.Push(DrawEffectDesc("泛光(Bloom)", DrawEffectFunc(this.SetBloom), CancelEffectFunc(this.CancelBloom),Variant(Color(10,5,0))));
    ///设置闪烁效果
    {
        VariantMap map;
        map["Arg1"]     = float(0.5);
        map["ArgName1"] = "周期:";
        map["Arg2"] = Color(1,0.5,0);
        mEffectDescList.Push(DrawEffectDesc("闪烁", DrawEffectFunc(this.SetTwinkle), CancelEffectFunc(this.CancelTwinkle), Variant(map)));
    }
    ///显示法线
        mEffectDescList.Push(DrawEffectDesc("法线", DrawEffectFunc(this.SetNormalShowEffect), CancelEffectFunc(this.CancelNormalShowEffect)));

    ///SMAA 抗锯齿
        mDefferDescList.Push(DrawEffectDesc("抗锯齿(SMAA)", DrawEffectFunc(this.OpenSMAAEffect), CancelEffectFunc(this.CloseSMAAEffect), Variant(), true));
    ///SMAA 抗锯齿
        mDefferDescList.Push(DrawEffectDesc("抗锯齿(TAA)", DrawEffectFunc(this.OpenTemporalAAFilter), CancelEffectFunc(this.CloseTemporalAAFilter)));
    ///环境遮蔽
    {
        VariantMap map;
        map["Arg1"]     = float(1.0);
        map["ArgName1"] = "AO强度:";
        
        map["Arg2"]     = float(20.0);
        map["ArgName2"] = "AO半径:";
        
        map["Arg3"]     = float(8.0);
        map["ArgName3"] = "圆周采样:";
                
        map["Arg4"]     = float(4.0);
        map["ArgName4"] = "径向采样:";
        
        map["Arg5"]     = float(1.0);
        map["ArgName5"] = "AO衰减:";
        
        map["Arg6"]     = float(30);
        map["ArgName6"] = "AO阈值°:";
        
        map["Arg7"]     = float(6.0);
        map["ArgName7"] = "滤波半径:";

        mDefferDescList.Push(DrawEffectDesc("环境遮蔽(AO)", DrawEffectFunc(this.OpenDeferredHBAOEffect), CancelEffectFunc(this.CloseDeferredHBAOEffect), Variant(map)));
    }

    ///环境遮蔽 TAO
    {
        VariantMap map;
        map["Arg1"]     = float(1.0);
        map["ArgName1"] = "AO强度:";
        
        map["Arg2"]     = float(20.0);
        map["ArgName2"] = "AO半径:";
        
        map["Arg3"]     = float(8.0);
        map["ArgName3"] = "圆周采样:";
                
        map["Arg4"]     = float(4.0);
        map["ArgName4"] = "径向采样:";
        
        map["Arg5"]     = float(1.0);
        map["ArgName5"] = "AO衰减:";
        
        map["Arg6"]     = float(30);
        map["ArgName6"] = "AO阈值°:";
        
        map["Arg7"]     = float(6.0);
        map["ArgName7"] = "滤波半径:";

        mDefferDescList.Push(DrawEffectDesc("环境遮蔽(TemporalAO)", DrawEffectFunc(this.OpenDeferredHBAOTemporalEffect), CancelEffectFunc(this.CloseDeferredHBAOTemporalEffect), Variant(map)));
    }
    ///高动态范围
        mDefferDescList.Push(DrawEffectDesc("高动态范围(HDR)", DrawEffectFunc(this.OpenHDREffect), CancelEffectFunc(this.CloseHDREffect), Variant() , true));
    ///乏光+高动态范围
    {
        VariantMap map;
        map["Arg1"]     = float(7.5);
        map["ArgName1"] = "阀值:";
        mDefferDescList.Push(DrawEffectDesc("泛光+高动态范围(BloomHDR)", DrawEffectFunc(this.OpenBloomHDREffect), CancelEffectFunc(this.CloseBloomHDREffect), Variant(map), true));
    }
    ///景深
    {
        VariantMap map;
        map["Arg1"]     = float(14);
        map["ArgName1"] = "闪光圈max:";
        
        map["Arg2"]     = float(100);
        map["ArgName2"] = "成像距离:";
        
        map["Arg3"]     = float(0.3);
        map["ArgName3"] = "焦距:";
        
        map["Arg4"]     = float(1.4);
        map["ArgName4"] = "光圈:";

        mDefferDescList.Push(DrawEffectDesc("景深", DrawEffectFunc(this.OpenDepthOfFieldFilter), CancelEffectFunc(this.CloseDepthOfFieldFilter), Variant(map)));
    }

    ///镜头光晕
        mDefferDescList.Push(DrawEffectDesc("镜头光晕(LensFlare)", DrawEffectFunc(this.OpenLensFlareffect), CancelEffectFunc(this.CloseLensFlareEffect)));
    ///下雨
        mDefferDescList.Push(DrawEffectDesc("下雨", DrawEffectFunc(this.OpenRainEffect), CancelEffectFunc(this.CloseRainEffect)));
    ///下雪
        mDefferDescList.Push(DrawEffectDesc("下雪", DrawEffectFunc(this.OpenSnowEffect), CancelEffectFunc(this.CloseSnowEffect)));
    ///静态阴影
        mDefferDescList.Push(DrawEffectDesc("静态阴影", DrawEffectFunc(this.OpenStaticShadow), CancelEffectFunc(this.CloseStaticShadow), Variant(), true));
    ///EDL
    {
        VariantMap map;
        map["Arg1"]     = float(1);
        map["ArgName1"] = "范围:";
        
        map["Arg2"]     = float(300);
        map["ArgName2"] = "增益:";
        mDefferDescList.Push(DrawEffectDesc("点云(EDL)", DrawEffectFunc(this.OpenEDLFilter), CancelEffectFunc(this.CloseEDLFilter), Variant(map)));
    }
    ///VignetteFilter
    {
        VariantMap map;
        map["Arg1"]     = float(0.3);
        map["ArgName1"] = "衰减:";

        mDefferDescList.Push(DrawEffectDesc("Vignette", DrawEffectFunc(this.OpenVignetteFilter), CancelEffectFunc(this.CloseVignetteFilter), Variant(map)));
    }

    ///LogDepth
        mDefferDescList.Push(DrawEffectDesc("LogDepth", DrawEffectFunc(this.OpenLogDepth), CancelEffectFunc(this.CloseLogDepth), Variant(), true));

        for(int i=0; i < mDefferDescList.length; ++i)
        {
            mDefferDescList[i].PostEffect = true;
        }

        
    }

///AO
    void OpenDeferredHBAOEffect(Variant& var)
    {
        VariantMap map = var.GetVariantMap();

        mDeferredHBAOFilter.SetHBAOIntensity(map["Arg1"].GetFloat());
        mDeferredHBAOFilter.SetAORadius(map["Arg2"].GetFloat());
        mDeferredHBAOFilter.SetAONumDir(map["Arg3"].GetFloat());
        mDeferredHBAOFilter.SetAONumSteps(map["Arg4"].GetFloat());
        mDeferredHBAOFilter.SetAOAttenuation(map["Arg5"].GetFloat());
        mDeferredHBAOFilter.SetAOAngleBias(map["Arg6"].GetFloat());
        mDeferredHBAOFilter.SetBilateralBlurRadius(map["Arg7"].GetFloat());

        mDeferredHBAOFilter.enable = true;
    }

    void CloseDeferredHBAOEffect()
    {
        mDeferredHBAOFilter.enable = false;
    }

///TemporalAO
    void OpenDeferredHBAOTemporalEffect(Variant& var)
    {
        VariantMap map = var.GetVariantMap();

        mDeferredHBAOTemporalFilter.SetHBAOIntensity(map["Arg1"].GetFloat());
        mDeferredHBAOTemporalFilter.SetAORadius(map["Arg2"].GetFloat());
        mDeferredHBAOTemporalFilter.SetAONumDir(map["Arg3"].GetFloat());
        mDeferredHBAOTemporalFilter.SetAONumSteps(map["Arg4"].GetFloat());
        mDeferredHBAOTemporalFilter.SetAOAttenuation(map["Arg5"].GetFloat());
        mDeferredHBAOTemporalFilter.SetAOAngleBias(map["Arg6"].GetFloat());
        mDeferredHBAOTemporalFilter.SetBilateralBlurRadius(map["Arg7"].GetFloat());

        mDeferredHBAOTemporalFilter.enable = true;
    }

    void CloseDeferredHBAOTemporalEffect()
    {
        mDeferredHBAOTemporalFilter.enable = false;
    }

///HDR
    void OpenHDREffect(Variant& var)
    {
        mHDRFilter.enable = true;
    }

    void CloseHDREffect()
    {
        mHDRFilter.enable = false;
    }

///BloomHDR
    void OpenBloomHDREffect(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        mBloomHDRFilter.SetBloomHDRThreshold(map["Arg1"].GetFloat());
        mBloomHDRFilter.enable = true;
    }

    void CloseBloomHDREffect()
    {
        mBloomHDRFilter.enable = false;
    }

///LensFlare
    void OpenLensFlareffect(Variant& var)
    {
        mLensFlareFilter.SetEnable(true);
    }

    void CloseLensFlareEffect()
    {
        mLensFlareFilter.SetEnable(false);
    }

///DepthOfFieldFilter
    void OpenDepthOfFieldFilter(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        mDepthOfFieldFilter.SetMaxCoc(map["Arg1"].GetFloat());
        mDepthOfFieldFilter.SetFocusDistance(map["Arg2"].GetFloat());
        mDepthOfFieldFilter.SetFocalLength(map["Arg3"].GetFloat());
        mDepthOfFieldFilter.SetFNumber(map["Arg4"].GetFloat());
        mDepthOfFieldFilter.enable = true;
    }

    void CloseDepthOfFieldFilter()
    {
        mDepthOfFieldFilter.enable = false;
    }

///SMAA
    void OpenSMAAEffect(Variant& var)
    {
        mSMAAFilter.enable = true;
    }

    void CloseSMAAEffect()
    {
        mSMAAFilter.enable = false;
    }

///TAA
    void OpenTemporalAAFilter(Variant& var)
    {
        mTemporalAAFilter.enable = true;
    }

    void CloseTemporalAAFilter()
    {
        mTemporalAAFilter.enable = false;
    }

///Vignette
    void OpenVignetteFilter(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        mVignetteFilter.SetFalloff(map["Arg1"].GetFloat());
        mVignetteFilter.enable = true;
    }

    void CloseVignetteFilter()
    {
        mVignetteFilter.enable = false;
    }


///EDL
    void OpenEDLFilter(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        mEDLFilter.SetPixScale(map["Arg1"].GetFloat());
        mEDLFilter.SetExpScale(map["Arg2"].GetFloat());
        mEDLFilter.enable = true;
    }

    void CloseEDLFilter()
    {
        mEDLFilter.enable = false;
    }

///LogDepth
    void OpenLogDepth(Variant& var)
    {
        renderer.ableLogDepth = true;
    }

    void CloseLogDepth()
    {
        renderer.ableLogDepth = false;
    }

    void AddCameraLookAtNodeToScene()
    {
        if(ginscene == false)
        {
            editorScene.AddChild(gcameraLookAtNode);
            ginscene = true;
            DayNightWeatherControl@ weather = editorScene.GetComponent("DayNightWeatherControl");
            if(weather !is null)
            {
                weather.SetWeatherRatio(1.0);
            }
        }
    }

    void RemoveCameraLookAtNodeFromScene()
    {
        if(ginscene)
        {
            editorScene.RemoveChild(gcameraLookAtNode);
            ginscene = false;
            DayNightWeatherControl@ weather = editorScene.GetComponent("DayNightWeatherControl");
            if(weather !is null)
            {
                weather.SetWeatherRatio(0.5);
            }
            mViewport.UpdateStaticShadow();
        }
    }

///下雨效果
    void OpenRainEffect(Variant& var)
    {
        AddCameraLookAtNodeToScene();
        WeatherEffectUtil::SetRainEffect(mCamera);

    }

    void CloseRainEffect()
    {
        RemoveCameraLookAtNodeFromScene();
        WeatherEffectUtil::CancelRainEffect(mCamera);
        
    }

///下雪效果
    void OpenSnowEffect(Variant& var)
    {
        AddCameraLookAtNodeToScene();
        WeatherEffectUtil::SetSnowEffect(mCamera);
    }

    void CloseSnowEffect()
    {
        RemoveCameraLookAtNodeFromScene();
        WeatherEffectUtil::CancelSnowEffect(mCamera);
    }

///静态阴影
    void OpenStaticShadow(Variant& var)
    {
        mViewport.EnableStaticShadow(true);
        mViewport.UpdateStaticShadow();
    }

    void CloseStaticShadow()
    {
        mViewport.EnableStaticShadow(false);
    }

///穿透描边效果
    void SetOutlinePerspect(Variant& var)
    {
        ModelEffectUtil::SetOutlinePerspect(mViewport, mCurEffectNode, var.GetColor());
    }

    void CancelOutlinePerspect()
    {
        ModelEffectUtil::CancelOutlinePerspect(mViewport, mCurEffectNode);
    }

    void SetOutlinePerspectTwinkle(Variant& var)
    {
        ModelEffectUtil::SetOutlinePerspectTwinkle(mViewport, mCurEffectNode, var.GetColor());
    }

    void CancelOutlinePerspectTwinkle()
    {
        ModelEffectUtil::CancelOutlinePerspectTwinkle(mViewport, mCurEffectNode);
    }
///电线外面的电流效果
    void SetOuterElecEffect(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        ModelEffectUtil::SetOuterElecEffect(mViewport, mCurEffectNode,
            map["Arg1"].GetBool(),
            map["Arg2"].GetFloat(),
            map["Arg3"].GetFloat(),
            map["Arg4"].GetColor(),
            map["Arg5"].GetFloat());
    }

    void CancelOuterElecEffect()
    {
        ModelEffectUtil::CancelOuterElecEffect(mViewport, mCurEffectNode);
    }
///相交部分高亮闪烁
    void SetCollisionEffect(Variant& var)
    {
        ModelEffectUtil::SetCollisionEffect(mViewport, mCurEffectNode, var.GetColor());
    }

    void CancelCollisionEffect()
    {
        ModelEffectUtil::CancelCollisionEffect(mViewport);
    }

///通过改变模型diffuse颜色，是模型外观泛出这种颜色
    void SetDiffuse(Variant& var)
    {
        ModelEffectUtil::SetDiffuse(mCurEffectNode, var.GetColor());
    }

    void CancelDiffuse()
    {
        ModelEffectUtil::CancelDiffuse(mCurEffectNode);
    }

///模型unshaded着色
    void SetUnshadedColor(Variant& var)
    {
        ModelEffectUtil::SetUnshadedColor(mViewport, mCurEffectNode, var.GetColor());
    }

    void CancelUnshadedColor()
    {
        ModelEffectUtil::CancelUnshadedColor(mViewport, mCurEffectNode);
    }

///模型透明
    void SetModelTransparent(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        ModelEffectUtil::SetModelTransparent(mCurEffectNode, map["Arg1"].GetFloat());
    }

    void CancelModelTransparent()
    {
        ModelEffectUtil::CancelModelTransparent(mCurEffectNode);
    }

///模型纯色透明
    void SetModelUnshadedTransparent(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        Color color = map["Arg1"].GetColor();
        float alpha = map["Arg2"].GetFloat();
        color.a = alpha;
        ModelEffectUtil::SetModelUnshadedTransparent(mCurEffectNode, color);
    }

    void CancelModelUnshadedTransparent()
    {
        ModelEffectUtil::CancelModelUnshadedTransparent(mCurEffectNode);
    }

///线框（wire）显示,假定这里只有FILL_WIREFRAME和FILL_SOLID两种填充方式，不考虑FILL_POINTS填充方式
    void SetWireframe(Variant& var)
    {
        ModelEffectUtil::SetWireframe(mCurEffectNode);
    }

    void CancelWireframe()
    {
        ModelEffectUtil::CancelWireframe(mCurEffectNode);
    }

///穿透显示,这里假定batches的技术都是一样的
    void SetTranslucent(Variant& var)
    {
        ModelEffectUtil::SetTranslucent(mViewport, mCurEffectNode);
    }

    void CancelTranslucent()
    {
        ModelEffectUtil::CancelTranslucent(mViewport, mCurEffectNode);
    }

///发光效果,注意color会和场景光线作用的模型外观显示颜色已经叠加来起作用，并和hdr是否开启有关，开启后color值可以设置超过1.
    void SetBloom(Variant& var)
    {
        ModelEffectUtil::SetBloom(mCurEffectNode, var.GetColor());
    }

    void CancelBloom()
    {
        ModelEffectUtil::CancelBloom(mCurEffectNode);
    }

///设置闪烁效果
    void SetTwinkle(Variant& var)
    {
        VariantMap map = var.GetVariantMap();
        ModelEffectUtil::SetTwinkle(mCurEffectNode, map["Arg1"].GetFloat(), map["Arg2"].GetColor());
    }

    void CancelTwinkle()
    {
        ModelEffectUtil::CancelTwinkle(mCurEffectNode);
    }

///显示法线
    void SetNormalShowEffect(Variant& var)
    {
        ModelEffectUtil::SetNormalShowEffect(mCurEffectNode);
    }

    void CancelNormalShowEffect()
    {
        ModelEffectUtil::CancelNormalShowEffect(mCurEffectNode);
    }

    UIElement@ CreateOutlineArgWnd()
    {
        UIElement@ agrwnd = UIElement();
        agrwnd.defaultStyle = uiStyle;
        for(int i=0; i < 3; ++i)
        {
            LineEdit@ text = CreateAttributeLineEdit(agrwnd);
            text.SetPosition(25*i, 0);
        }
        agrwnd.SetPosition(60, 0);
        return agrwnd;
    }

    LineEdit@ CreateAttributeLineEdit(UIElement@ parent)
    {
        LineEdit@ attrEdit = LineEdit();
        parent.AddChild(attrEdit);
        
        attrEdit.style = "EditorAttributeEdit";
        attrEdit.SetFixedWidth(20);
        attrEdit.SetFixedHeight(ATTR_HEIGHT - 2);
        return attrEdit;
    }
}