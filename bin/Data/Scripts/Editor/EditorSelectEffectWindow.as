
funcdef void TraverseEffectInfoDataFunc(EditorSelectEffectInfoData@);

class EditorSelectEffectInfoData
{
    bool mAble;
    Variant mVar;
    String mEffectName;
    DrawEffectDesc@ mDrawEffectDesc;

    EditorSelectEffectInfoData(DrawEffectDesc@ draweffectdesc, String effectname, bool able, Variant var)
    {
        mAble = able;
        mVar  = var;
        mEffectName = effectname;
        @mDrawEffectDesc = @draweffectdesc;
    }

    bool isEffect(String effectname)
    {
        return mEffectName.Compare(effectname) == 0;
    }

    void RefreshDrawEffectDesc(DrawEffectDesc@ draweffectdesc)
    {
        mDrawEffectDesc = draweffectdesc;
    }

    void SetAble(bool able)
    {
        mAble = able;
        mDrawEffectDesc.SetAble(able);
    }

    bool GetAble()
    {
        return mAble;
    }

    Variant GetVar()
    {
        return mVar;
    }

    String GetEffectName()
    {
        return mEffectName;
    }

    Array<UIElement@>@ CreateArgWnd()
    {
        return mDrawEffectDesc.CreateArgWnd();
    }

    void RefreshVar(Variant var)
    {
        mVar = var;
    }
}

class EditorSelectEffectInfoDataMgr
{
    Array<EditorSelectEffectInfoData@> mInfoList;
    EditorSelectEffectInfoData@ havaEffectInfo(DrawEffectDesc@ draweffectdesc, String effectname, bool& able, Variant& var)
    {
        for(int i=0; i < mInfoList.length; ++i)
        {
            if(mInfoList[i].isEffect(effectname))
            {
                able = mInfoList[i].GetAble();
                var  = mInfoList[i].GetVar();
                mInfoList[i].RefreshDrawEffectDesc(draweffectdesc);
                return mInfoList[i];
            }
        }
        return null;
    }

    EditorSelectEffectInfoData@ AddEffectInfo(DrawEffectDesc@ draweffectdesc, String effectname, bool able, Variant var)
    {
        EditorSelectEffectInfoData@ data = EditorSelectEffectInfoData(draweffectdesc, effectname, able, var);
        mInfoList.Push(data);
        return data;
    }

    void Traverse(TraverseEffectInfoDataFunc@ func)
    {
        for(int i=0; i < mInfoList.length; ++i)
        {
            func(mInfoList[i]);
        }
    }

    void ChangeAbleState(String effectname, bool able)
    {
        for(int i=0; i < mInfoList.length; ++i)
        {
            if(mInfoList[i].isEffect(effectname))
            {
                mInfoList[i].SetAble(able);
                return;
            }
        }
    }
}

EditorSelectEffectInfoDataMgr@ gEditorSelectEffectInfoDataMgr = EditorSelectEffectInfoDataMgr();

EditorSelectEffectWindow@ editorSelectEffectWindow;
class EditorSelectEffectWindow
{
    Window@ mSelectEffectWindow;
    ListView@ mEffectList;

    EditorSelectEffectWindow()
    {
        mSelectEffectWindow = LoadEditorUI("UI/EditorSelectEffectWindow.xml");
        ui.root.AddChild(mSelectEffectWindow);
        mEffectList = mSelectEffectWindow.GetChild("effectlist", true);
        mEffectList.hierarchyMode = true;
        mEffectList.contentElement.layoutMode = LM_VERTICAL;
        mSelectEffectWindow.opacity = uiMaxOpacity;
        mSelectEffectWindow.visible = false;
        RegistBtnEvents();
        InitDisplayList();
    }

    void RefreshUI()
    {
        int numItems = mEffectList.numItems;
        for(int i=0; i < numItems; ++i)
        {
            mEffectList.RemoveItem(0);
        }
        InitDisplayList();
    }

    void InitDisplayList()
    {
        gEditorSelectEffectInfoDataMgr.Traverse(TraverseEffectInfoDataFunc(this.AddListItem));
        int numItems = mEffectList.numItems;
        for(int i=0; i < numItems; ++i)
        {
            mEffectList.Expand(i, false);
        }
    }

    void AddListItem(EditorSelectEffectInfoData@ infodata)
    {
        bool able = infodata.GetAble();
        String effecname = infodata.GetEffectName();
        
        UIElement@ item = UIElement();
        item.SetMinSize(340, 20);
        item.indent       = 20;
        item.defaultStyle = uiStyle;

        Text@ text = Text();
        text.position = IntVector2(20,0);
        text.text = effecname;
        text.fontSize = 13; 
        item.AddChild(text);

        CheckBox@ ablebox = CheckBox();
        item.AddChild(ablebox);
        ablebox.defaultStyle = uiStyle;
        ablebox.SetStyleAuto();
        ablebox.checked = able;
        ablebox.vars["EffectName"] = effecname;
        ablebox.position = IntVector2(340, 0);
        SubscribeToEvent(ablebox, "Toggled", "HandleEffectToggled");
        
        Array<UIElement@>@ argwnds = infodata.CreateArgWnd();
        
        mEffectList.InsertItem(mEffectList.numItems, item, null);
        int index = mEffectList.numItems;
        for(int i=0; i < argwnds.length; ++i)
        {
            argwnds[i].position = IntVector2(40,0);
            mEffectList.InsertItem(index + i, argwnds[i], item);
        }
    }

    void RegistBtnEvents()
    {
        SubscribeToEvent(mSelectEffectWindow.GetChild("closebtn", true), "Released", "HideWindow");
    }

    void HandleEffectToggled(StringHash eventType, VariantMap& eventData)
    {
        CheckBox@ ablebox = cast<CheckBox@>(eventData["Element"].GetPtr());
        gEditorSelectEffectInfoDataMgr.ChangeAbleState(ablebox.vars["EffectName"].GetString(), ablebox.checked);
    }

    bool IsShow()
    {
        return mSelectEffectWindow.visible;
    }

    void HideWindow()
    {
        mSelectEffectWindow.visible = false;
    }

    void ShowWindow()
    {
        mSelectEffectWindow.visible = true;
        mSelectEffectWindow.BringToFront();
        CenterDialog(mSelectEffectWindow);
    }
}

bool ToggleEditorSelectEffectWindow()
{
    if (editorSelectEffectWindow is null)
    {
        @editorSelectEffectWindow = EditorSelectEffectWindow();
    }
    if (editorSelectEffectWindow.IsShow())
    {
        editorSelectEffectWindow.HideWindow();
    }
    else
    {
        editorSelectEffectWindow.ShowWindow();
    }
    return true;
}
