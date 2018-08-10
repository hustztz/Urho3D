// Urho3D LOD Info editor

Window@ editorLodInfoWindow;
ListView@ lodInfoList;
Serializable@ lodInfoSerializable;

LineEdit@ curTextWnd;
UIElement@ curLodLayerItem;

int M_LOD_ITEM    = 1;
int M_LOD_MODEL   = 2;
int M_LOD_MAT     = 3;
int M_LOD_MATDATA = 4;

void CreateLODInfoWindowEditor()
{
    if (editorLodInfoWindow !is null)
        return;
    
    editorLodInfoWindow = LoadEditorUI("UI/EditorLodInfoWindow.xml");
    ui.root.AddChild(editorLodInfoWindow);
    editorLodInfoWindow.opacity = uiMaxOpacity;
    
    lodInfoList = editorLodInfoWindow.GetChild("lodinfolist", true);
    lodInfoList.contentElement.layoutMode = LM_VERTICAL;
    SubscribeToEvent(editorLodInfoWindow.GetChild("cancelbtn", true),    "Released", "HideLODInfoWindowEditor");
    SubscribeToEvent(editorLodInfoWindow.GetChild("surebtn", true),      "Released", "HandleSaveLODInfoEvent");

    SubscribeToEvent(editorLodInfoWindow.GetChild("addlodbtn", true),    "Released", "HandleAddLodLayerEvent");
    SubscribeToEvent(editorLodInfoWindow.GetChild("deletelodbtn", true), "Released", "HandleDeleteLodLayerEvent");

    HideLODInfoWindowEditor();
}

void ShowLODInfoWindowEditor(Array<Serializable@>@ lodinfos)
{
    CreateLODInfoWindowEditor();   
    
    lodInfoList.RemoveAllItems();
    CenterDialog(editorLodInfoWindow);
    editorLodInfoWindow.visible = true;
    editorLodInfoWindow.BringToFront();

    for (uint i = 0; i < lodinfos.length; ++i)
    {
        Serializable@ target = lodinfos[i];
        Variant variant = target.GetAttribute("LodLevel");
        if (!variant.empty)
        {
            InitLodInfoListDisplay(target);
            return;
        }
    }
    HideLODInfoWindowEditor();
}

void InitLodInfoListDisplay(Serializable@ target)
{
    lodInfoSerializable = target;
    Variant variant     = target.GetAttribute("LodLevel");

    Array<Variant>@ infos = variant.GetVariantVector();
    for (uint i = 0; i < infos.length; ++i)
    {
        float dis = infos[i].GetFloat();
    
        i = i+1;
        Array<String>@ modelinfo = infos[i].GetString().Split(';', true);
    
        i = i+1;
        Array<String>@ matinfo   = infos[i].GetString().Split(';', true);

        LODFillDisplayData(dis, modelinfo[1], matinfo);
    }
}

void HandleAddLodLayerEvent()
{
//lod 层级 距离信息
    UIElement@ loditem = UIElement();
    loditem.vars["loditem"] = true;
    loditem.vars["lodinfotype"] = M_LOD_ITEM;
    loditem.layoutMode = LM_HORIZONTAL;
    loditem.SetFixedHeight(ATTR_HEIGHT);
    lodInfoList.AddItem(loditem);

    UIElement@ loaddescplaceholder = UIElement();
    loaddescplaceholder.SetFixedWidth(20);
    loditem.AddChild(loaddescplaceholder);

    Text@ loddesc = Text();
    loddesc.style = "FileSelectorListText";
    loddesc.text  = "LOD距离:";
    loditem.AddChild(loddesc);

    LineEdit@ diseditor = LineEdit(); 
    loditem.AddChild(diseditor);
    diseditor.name  = "lodtext";
    diseditor.text  = "60";
    diseditor.style = "EditorAttributeEdit";
    diseditor.SetFixedHeight(ATTR_HEIGHT);
    SubscribeToEvent(diseditor, "TextFinished", "HandleLODDistanceChangeEvent");

//lod 层级模型信息
    UIElement@ container = UIElement();
    container.vars["lodinfotype"] = M_LOD_MODEL;
    container.layoutMode = LM_HORIZONTAL;
    container.SetFixedHeight(ATTR_HEIGHT);
    lodInfoList.InsertItem(lodInfoList.numItems, container, loditem);

    UIElement@ placeholder = UIElement();
    placeholder.SetFixedWidth(30);
    container.AddChild(placeholder);

    Text@ desc = Text();
    desc.text  = "模型";
    desc.style = "EditorAttributeText";
    desc.SetFixedWidth(30);
    container.AddChild(desc);

    LineEdit@ path = LineEdit(); 
    path.style = "EditorAttributeEdit";
    path.name  = "modeltext";
    path.SetSize(120, ATTR_HEIGHT);
    container.AddChild(path);

//lod 层级材质信息
    Text@ matitem = Text();
    matitem.style = "FileSelectorListText";
    matitem.text  = "材质";
    matitem.vars["matnum"] = 0;
    matitem.vars["lodinfotype"] = M_LOD_MAT;

    Button@ pickButton = CreateLODResourcePickerButton(container, "选择");
    pickButton.vars["matitem"] = matitem;
    pickButton.vars["textwnd"] = path;
    
    SubscribeToEvent(pickButton, "Released", "LODPickModleResource");

    lodInfoList.InsertItem(lodInfoList.numItems, matitem, container);
}


void HandleLODDistanceChangeEvent(StringHash eventType, VariantMap& eventData)
{
    LineEdit@ line = cast<LineEdit@>(eventData["Element"].GetPtr());
    String text = eventData["Text"].GetString();
    float dis = text.ToFloat();
    if(dis < 0)
    {
        dis = 1;
    }
    line.text = String(dis);
}

void LODPickModleResource(StringHash eventType, VariantMap& eventData)
{
    Button@ btn     = cast<Button@>(eventData["Element"].GetPtr());
    curTextWnd      = cast<LineEdit@>(btn.vars["textwnd"].GetPtr());
    curLodLayerItem = cast<UIElement@>(btn.vars["matitem"].GetPtr());
    
    CreateFileSelector("选择模型资源", "确定", "取消", uiImportPath, {"*.mdl"}, uiImportFilter);
    SubscribeToEvent(uiFileSelector, "FileSelected", "LODPickModleResourceCallBack");
}

void LODPickModleResourceCallBack(StringHash eventType, VariantMap& eventData)
{
    if(eventData["coustomopen"].empty)
    {
        CloseFileSelector(uiImportFilter, uiImportPath);
    }
    if(eventData["Ok"].GetBool() == false) 
    {   
        return;
    }
    String modlepath = ExtractFileName(eventData);
    if (curTextWnd !is null)
    {
        curTextWnd.text = cache.SanitateResourceName(modlepath);
    }

    if(curLodLayerItem !is null)
    {
        int num   = curLodLayerItem.vars["matnum"].GetInt();
        int index = lodInfoList.FindItem(curLodLayerItem) +1;
        for(int i = 0; i < num; ++i)
        {
            lodInfoList.RemoveItem(index);
        }
    }

    Model@ res = cache.GetResource("Model", modlepath);
    if (res !is null)
    {
        if(curLodLayerItem !is null)
        {
            int num   = res.numGeometries;
            int index = lodInfoList.numItems;
            curLodLayerItem.vars["matnum"] = num;
            for(int i= 0; i < num; ++i, ++index)
            {
                UIElement@ matinfo = UIElement();
                matinfo.vars["lodinfotype"] = M_LOD_MATDATA;
                matinfo.layoutMode = LM_HORIZONTAL;
                matinfo.SetFixedHeight(ATTR_HEIGHT);
                lodInfoList.InsertItem(index, matinfo, curLodLayerItem);

                UIElement@ placeholder = UIElement();
                placeholder.SetFixedWidth(50);
                matinfo.AddChild(placeholder);

                LineEdit@ path = LineEdit(); 
                path.style = "EditorAttributeEdit";
                path.name  = "mattext";
                path.SetSize(120, ATTR_HEIGHT);
                matinfo.AddChild(path);
                
                Button@ pickButton = CreateLODResourcePickerButton(matinfo, "选择");
                pickButton.vars["textwnd"] = path;
                SubscribeToEvent(pickButton, "Released", "LODPicktMaterialResource");                
            }
        }
    }
}

void LODPicktMaterialResource(StringHash eventType, VariantMap& eventData)
{
    Button@ btn  = cast<Button@>(eventData["Element"].GetPtr());
    curTextWnd   = cast<LineEdit@>(btn.vars["textwnd"].GetPtr());

    CreateFileSelector("选择材质资源", "确定", "取消", uiImportPath, {"*.xml"}, uiImportFilter);
    SubscribeToEvent(uiFileSelector, "FileSelected", "LODPicktMaterialResourceCallBack");
}

void LODPicktMaterialResourceCallBack(StringHash eventType, VariantMap& eventData)
{
    CloseFileSelector(uiImportFilter, uiImportPath);
    if(eventData["Ok"].GetBool() == false) 
    {   
        return;
    }
    String matpath = ExtractFileName(eventData);

    if(curTextWnd !is null)
    {
        curTextWnd.text = cache.SanitateResourceName(matpath);
    }
}

Button@ CreateLODResourcePickerButton(UIElement@ container, String&in text)
{
    Button@ button = Button();
    button.style = AUTO_STYLE;
    button.SetFixedSize(36, ATTR_HEIGHT - 2);
    container.AddChild(button);

    Text@ buttonText = Text();
    buttonText.style = "EditorAttributeText";
    buttonText.SetAlignment(HA_CENTER, VA_CENTER);
    buttonText.text = text;
    button.AddChild(buttonText);

    return button;
}

void HandleDeleteLodLayerEvent()
{
    UIElement@ loditem = lodInfoList.selectedItem;
    if (loditem !is null && !loditem.vars["loditem"].empty)
    {
        lodInfoList.RemoveItem(loditem);
    }
}

void HideLODInfoWindowEditor()
{
    editorLodInfoWindow.visible = false;
}

void LODFillDisplayData(float dis, String modelpath, Array<String> matinfo)
{
    int index = lodInfoList.numItems;
    HandleAddLodLayerEvent();
    
    Array<UIElement@>@ items = lodInfoList.GetItems();
    
    curTextWnd      = cast<LineEdit@>(items[index+1].GetChild("modeltext", true));
    curLodLayerItem = items[index+2];

    VariantMap eventData;
    eventData["Ok"] = true;
    eventData["Filter"] = ".mdl";
    eventData["FileName"] = modelpath;
    eventData["coustomopen"] = true;

    StringHash eventType;
    LODPickModleResourceCallBack(eventType, eventData);

    items = lodInfoList.GetItems();
    Array<Variant> lodlayerinfo;
    int i=index;

    {//初始化lod距离
        LineEdit@ lodtext = cast<LineEdit@>(items[i].GetChild("lodtext", true));
        lodtext.text = String(dis);
        i = i + 1;
    }

    {//初始化lod模型路径
        LineEdit@ modeltext = cast<LineEdit@>(items[i].GetChild("modeltext", true));
        modeltext.text = modelpath;
        i = i + 1;
    }

    {//初始化lod材质
        int matnum = matinfo.length;
        items[i].vars["matnum"]= matnum -1;
        i = i + 1;

        int itemnum   = items.length;

        if(itemnum - i < matnum-1)
        {
            log.Warning("LOD:" + String(dis) + "材质数量过多; 只需要" + String(itemnum - i) + "个材质");
        }

        for(int j=1; j < matnum && i < itemnum; ++j, ++i)
        {
            LineEdit@ mattext = cast<LineEdit@>(items[i].GetChild("mattext", true));
            mattext.text = matinfo[j];
        }
    }
}

void HandleSaveLODInfoEvent()
{
    Array<UIElement@>@ items = lodInfoList.GetItems();
    Array<Variant> lodlayerinfo;
    for(int i=0; i < items.length; )
    {
        {//获取lod距离
            LineEdit@ lodtext = cast<LineEdit@>(items[i].GetChild("lodtext", true));
           // uint dis = lodtext.text;
            Variant var = lodtext.text.ToFloat();
            lodlayerinfo.Push(var);
            i = i + 1;
        }

        {//获取lod模型路径
            LineEdit@ modeltext = cast<LineEdit@>(items[i].GetChild("modeltext", true));
            String modelinfo = "Model;";
            modelinfo = modelinfo + modeltext.text;
            Variant var = modelinfo;
            lodlayerinfo.Push(var);
            i = i + 1;
        }

        {//获取lod材质
            String matinfo = "Material";
            int matnum = items[i].vars["matnum"].GetInt();
            i = i + 1;
            for(int j=0; j < matnum; ++j, ++i)
            {
                LineEdit@ mattext = cast<LineEdit@>(items[i].GetChild("mattext", true));
                matinfo = matinfo + ";" + mattext.text;
            }
            Variant var = matinfo;
            lodlayerinfo.Push(var);
        }
    }

    lodInfoSerializable.SetAttribute("LodLevel", Variant(lodlayerinfo));
    HideLODInfoWindowEditor();
}