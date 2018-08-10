// Urho3D import asset editor

Window@ editorImportAssetWindow;

LineEdit@ importAssetInputPath;
LineEdit@ importAssetOutputPath;
LineEdit@ importAssetOutFileName;
 
ListView@ modelSplitLodInfoList;

funcdef void ImportAssetHandleFunction();
Array<ImportAssetHandleFunction@> importAssethandleMap;
Array<String> scanSuffixList = {"*.obj", "*.fbx"};
Array<String> singlefileList = {"单个文件输入", "文件夹输入"};

Dictionary importAssetTypeDesc;

int ToNodeType        = 0;
int ToModleType       = 1;
int ToAnimationType   = 2;
int ToNodeAndLODType = 3;
int ToSceneType       = 4;
int ToSceneLODType    = 5;

bool importDirectoryMode = false;

void RegisterImportAssetHandle()
{
    importAssethandleMap.Resize(6);
    importAssethandleMap[ToNodeType]        = @HandleSwitchToNode;
    importAssethandleMap[ToModleType]       = @HandleSwitchToModel;
    importAssethandleMap[ToAnimationType]   = @HandleImportAnimatin;
    importAssethandleMap[ToNodeAndLODType] = @HandleSwitchToNodeAppendLOD;
    importAssethandleMap[ToSceneType]       = @HandleSwitchToModelSaveScene;
    importAssethandleMap[ToSceneLODType]    = @HandleSwitchToModelSaveSceneAppendLOD;
    
    importAssetTypeDesc["输出节点"]             = ToNodeType;
    importAssetTypeDesc["输出模型"]             = ToModleType;
    importAssetTypeDesc["输出动画"]             = ToAnimationType;
    importAssetTypeDesc["输出带LOD信息的节点"]   = ToNodeAndLODType;
    importAssetTypeDesc["输出场景"]             = ToSceneType;
    importAssetTypeDesc["输出带LOD信息的场景"]   = ToSceneLODType;
}

void CreateSwitchAssetWindowEditor()
{
    if (editorImportAssetWindow !is null)
        return;
    
    RegisterImportAssetHandle();
    editorImportAssetWindow = LoadEditorUI("UI/EditorSwitchAssetWindow.xml");
    ui.root.AddChild(editorImportAssetWindow);
    editorImportAssetWindow.opacity = uiMaxOpacity;
    CenterDialog(editorImportAssetWindow);
    
    modelSplitLodInfoList  = editorImportAssetWindow.GetChild("lodlist", true);

    DropDownList@ droplist = editorImportAssetWindow.GetChild("typelist", true);

    Array<String> keys = importAssetTypeDesc.keys;
    for(int i=0; i < keys.length; ++i)
    {
        Text@ choice = Text();
        choice.text  = keys[i];
        choice.style = "EditorEnumAttributeText";
        int index = 0;
        importAssetTypeDesc.Get(keys[i], index);
        choice.vars["handleindex"] = index;
        droplist.AddItem(choice);
    }

    DropDownList@ suffixlist = editorImportAssetWindow.GetChild("suffixlist", true);

    for(int i=0; i < scanSuffixList.length; ++i)
    {
        Text@ choice = Text();
        choice.text  = scanSuffixList[i];
        choice.style = "EditorEnumAttributeText";
        suffixlist.AddItem(choice);
    }

    DropDownList@ singlefile = editorImportAssetWindow.GetChild("singlefile", true);

    for(int i=0; i < singlefileList.length; ++i)
    {
        Text@ choice = Text();
        choice.text  = singlefileList[i];
        choice.style = "EditorEnumAttributeText";
        singlefile.AddItem(choice);
    }

    importAssetInputPath   = editorImportAssetWindow.GetChild("inputpath", true);
    importAssetOutputPath  = editorImportAssetWindow.GetChild("outputpath", true);
    importAssetOutFileName = editorImportAssetWindow.GetChild("outfilename", true);

    SubscribeToEvent(editorImportAssetWindow.GetChild("CloseButton", true), "Released", "HideImportAssetEditor");
    SubscribeToEvent(editorImportAssetWindow.GetChild("cancelbtn", true), "Released", "HideImportAssetEditor");

    SubscribeToEvent(droplist, "ItemSelected", "HandleSelectTypeChange");
    SubscribeToEvent(singlefile, "ItemSelected", "HandleSelectSingleFileChange");

    SubscribeToEvent(editorImportAssetWindow.GetChild("surebtn", true), "Click", "HandleImportAsset");
    SubscribeToEvent(editorImportAssetWindow.GetChild("clickinputpath", true), "Click", "SelectAssetInputPath");
    SubscribeToEvent(editorImportAssetWindow.GetChild("clickoutputpath", true), "Click", "SelectAssetOutputPath");
    
    SubscribeToEvent(editorImportAssetWindow.GetChild("addlodbtn", true),    "Released", "HandleAddSpliteLodLayerEvent");
    SubscribeToEvent(editorImportAssetWindow.GetChild("deletelodbtn", true), "Released", "HandleDeleteSplitLodLayerEvent");
    InitCutMeshLODInfo();
    HideImportAssetEditor();
}

void InitCutMeshLODInfo()
{
    Array<float> loddis  = {5, 10, 15};
    Array<float> cutmesh = {1, 0.5, 0.25};

    for(int i=0; i < loddis.length; ++i)
    {
        HandleAddSpliteLodLayerEvent();
        UIElement@ item = modelSplitLodInfoList.items[i];
        LineEdit@ lodtext = cast<LineEdit@>(item.GetChild("lodtext", true));
        lodtext.text = String(loddis[i]);

        LineEdit@ lodcutmeshtext = cast<LineEdit@>(item.GetChild("lodcutmeshtext", true));
        lodcutmeshtext.text = String(cutmesh[i]);
    }
}

void HandleAddSpliteLodLayerEvent()
{
//lod 层级 距离信息
    UIElement@ loditem = UIElement();
    loditem.vars["loditem"] = true;
    loditem.vars["lodinfotype"] = M_LOD_ITEM;
    loditem.layoutMode = LM_HORIZONTAL;
    loditem.SetFixedHeight(ATTR_HEIGHT);
    modelSplitLodInfoList.AddItem(loditem);

    UIElement@ loaddescplaceholder = UIElement();
    loaddescplaceholder.SetFixedWidth(20);
    loditem.AddChild(loaddescplaceholder);
    {
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
    }

    {
        Text@ loddesc = Text();
        loddesc.style = "FileSelectorListText";
        loddesc.text  = "模型减面力度:";
        loditem.AddChild(loddesc);
        LineEdit@ diseditor = LineEdit(); 
        loditem.AddChild(diseditor);
        diseditor.name  = "lodcutmeshtext";
        diseditor.text  = "0.5";
        diseditor.style = "EditorAttributeEdit";
        diseditor.SetFixedHeight(ATTR_HEIGHT);
        SubscribeToEvent(diseditor, "TextFinished", "HandleLODCutMeshChangeEvent");
    }
}

void HandleLODCutMeshChangeEvent(StringHash eventType, VariantMap& eventData)
{
    LineEdit@ line = cast<LineEdit@>(eventData["Element"].GetPtr());
    String text    = eventData["Text"].GetString();
    float dis      = text.ToFloat();
    if(dis < 0)
    {
        dis = 0;
    }
    if(dis > 1)
    {
        dis = 1;
    }
    line.text = String(dis);
}

void HandleDeleteSplitLodLayerEvent()
{
    UIElement@ loditem = modelSplitLodInfoList.selectedItem;
    if (loditem !is null && !loditem.vars["loditem"].empty)
    {
        modelSplitLodInfoList.RemoveItem(loditem);
    }
}

String GetLODDistanceLayer()
{
    String lodinfo;
    Array<UIElement@>@ items = modelSplitLodInfoList.GetItems();

    for(int i=0; i < items.length; ++i)
    {
        if(i > 0) lodinfo = lodinfo + "|";
        LineEdit@ lodtext = cast<LineEdit@>(items[i].GetChild("lodtext", true));
        LineEdit@ cutmesh = cast<LineEdit@>(items[i].GetChild("lodcutmeshtext", true));
        lodinfo = lodinfo + lodtext.text + ":" + cutmesh.text;
    }
    return lodinfo;
}

void ShowSwitchAssetEditor()
{
    DropDownList@ droplist = editorImportAssetWindow.GetChild("typelist", true);
    droplist.selection = 0;

    CenterDialog(editorImportAssetWindow);
    editorImportAssetWindow.visible = true;
    editorImportAssetWindow.BringToFront();
}

void HideImportAssetEditor()
{
    importAssetInputPath.text  = "";
    importAssetOutputPath.text = "";
    importAssetOutFileName.text = "";
    editorImportAssetWindow.visible = false;
    editorImportAssetWindow.GetChild("lodsetpanel", true).visible  = false;
    editorImportAssetWindow.GetChild("typedesctips", true).visible = true;
}

void HandleSelectTypeChange(StringHash eventType, VariantMap& eventData)
{
    DropDownList@ droplist = editorImportAssetWindow.GetChild("typelist", true);
    int handleindex =  droplist.selectedItem.vars["handleindex"].GetInt();

    if(handleindex == ToNodeAndLODType || handleindex == ToSceneLODType)
    {
        editorImportAssetWindow.GetChild("lodsetpanel", true).visible  = true;
        editorImportAssetWindow.GetChild("typedesctips", true).visible = false;
    }
    else
    {
        editorImportAssetWindow.GetChild("lodsetpanel", true).visible  = false;
        editorImportAssetWindow.GetChild("typedesctips", true).visible = true;
    }

}

void HandleSelectSingleFileChange(StringHash eventType, VariantMap& eventData)
{
    DropDownList@ droplist = editorImportAssetWindow.GetChild("singlefile", true);
    int handleindex =  droplist.selection;

    DropDownList@ suffixlist = editorImportAssetWindow.GetChild("suffixlist", true);

    if(handleindex == 0)
    {
     // suffixlist.visible  = false;
        importDirectoryMode = false;
    }
    else
    {
    //  suffixlist.visible  = true;
        importDirectoryMode = true;
    }
    importAssetInputPath.text  = "";
    importAssetOutFileName.text = "";
}

void SelectAssetInputPath()
{
    if(importDirectoryMode)
    {
        CreateFileSelector("选择资源路径或资源", "确定", "取消", uiImportPath, {".选择路径"}, uiImportFilter);
        SubscribeToEvent(uiFileSelector, "FileSelected", "HandleSelectAssetInputPath");
        uiFileSelector.directoryMode = importDirectoryMode;
    }
    else
    {
        DropDownList@ suffixlist = editorImportAssetWindow.GetChild("suffixlist", true);
        
        CreateFileSelector("选择资源路径或资源", "确定", "取消", uiImportPath, {scanSuffixList[suffixlist.selection]}, uiImportFilter);
        SubscribeToEvent(uiFileSelector, "FileSelected", "HandleSelectAssetInputPath");
        uiFileSelector.directoryMode = false;
    }
}

void SelectAssetOutputPath()
{
    CreateFileSelector("选择输出路径", "确定", "取消", uiImportPath, {".选择路径"}, 0);
    uiFileSelector.directoryMode = true;
    SubscribeToEvent(uiFileSelector, "FileSelected", "HandleSelectAssetOutputPath");
}

void HandleSelectAssetInputPath(StringHash eventType, VariantMap& eventData)
{
    String filename = uiFileSelector.fileName;
    CloseFileSelector(uiImportFilter, uiImportPath);

    if(eventData["Ok"].GetBool() == false) 
    {   
        return;
    }
    
    if(importDirectoryMode)
    {
        String inputpath = ExtractFileName(eventData);
        uint pos = inputpath.FindLast("/");
        if(pos > 0)
        {
            String dir = inputpath.Substring(0, pos);
            pos = dir.FindLast("/");
            if(pos > 0)
            {
                importAssetOutFileName.text = dir.Substring(pos+1);
            }
        }
        importAssetInputPath.text = inputpath;
    }
    else
    {
        importAssetOutFileName.text = GetFileName(filename);
        importAssetInputPath.text   = ExtractFileName(eventData);
    }
}

void HandleSelectAssetOutputPath(StringHash eventType, VariantMap& eventData)
{
    CloseFileSelector(uiImportFilter, uiImportPath);
    if(eventData["Ok"].GetBool() == false) 
    {   
        return;
    }
    importAssetOutputPath.text = ExtractFileName(eventData);
}

void HandleImportAsset(StringHash eventType, VariantMap& eventData)
{
    String inpath  = importAssetInputPath.text;
    String outpath = importAssetOutputPath.text;
    DropDownList@ droplist = editorImportAssetWindow.GetChild("typelist", true);
    int handleindex =  droplist.selectedItem.vars["handleindex"].GetInt();

    if(!fileSystem.DirExists(outpath))
    {
        log.Error("输出路径不存在");
        MessageBox("输出路径不存在", "Error");
        return;
    }

    if(importDirectoryMode)
    {
        if(!fileSystem.DirExists(inpath))
        {
            log.Error("输入路径不存在");
            MessageBox("输入路径不存在", "Error");
            return;
        }
    }
    else
    {
        if(!fileSystem.FileExists(inpath))
        {
            log.Error("输入资源不存在");
            MessageBox("输入资源不存在", "Error");
            return;
        }
    }
    if(importDirectoryMode && (handleindex == ToModleType || handleindex == ToAnimationType))
    {

    }
    else
    {
        if(importAssetOutFileName.text.empty)
        {
            log.Error("保存的文件名不能为空");
            MessageBox("保存的文件名不能为空", "Error");
            return;
        }
    }

    importAssethandleMap[handleindex]();
}

void HandleSwitchToNode()
{
    log.Info("HandleSwitchToNode");

    RunSwitchAsset(ToNodeType, "node", ".xml");
}

void HandleSwitchToModel()
{
    log.Info("HandleSwitchToModel");

    RunSwitchAsset(ToModleType, "model", ".mdl");
}

void HandleImportAnimatin()
{
    log.Info("HandleImportAnimatin");

    RunSwitchAsset(ToAnimationType, "anim", ".ani");
}

void HandleSwitchToNodeAppendLOD()
{
    log.Info("HandleSwitchToNodeAppendLOD");
    Array<String> lodapp;
    lodapp.Push("-lod");
    String lodinfo = GetLODDistanceLayer();
    if(lodapp.length > 0)
    {
        lodinfo = "#" + lodinfo;
        lodapp.Push(lodinfo);
    }
    RunSwitchAsset(ToNodeAndLODType, "node", ".xml", lodapp);
}

void HandleSwitchToModelSaveScene()
{
    log.Info("HandleSwitchToModelSaveScene");

    RunSwitchAsset(ToSceneType, "scene", ".xml");
}

void HandleSwitchToModelSaveSceneAppendLOD()
{
    log.Info("HandleSwitchToModelSaveSceneAppendLOD");

    Array<String> lodapp;
    lodapp.Push("-lod");
    String lodinfo = GetLODDistanceLayer();
    if(lodapp.length > 0)
    {
        lodinfo = "#" + lodinfo;
        lodapp.Push(lodinfo);
    }

    RunSwitchAsset(ToSceneLODType, "scene", ".xml", lodapp);
}

void RunSwitchAsset(int handlemod, const String&in mod, const String&in suffix, Array<String>@ lodapp = null)
{
    Array<String> args;
    String inpath      = importAssetInputPath.text;
    String outpath     = importAssetOutputPath.text;
    String outFileName = importAssetOutFileName.text;

    if(importDirectoryMode)
    {
        DropDownList@ suffixlist = editorImportAssetWindow.GetChild("suffixlist", true);
        inpath = inpath + scanSuffixList[suffixlist.selection];
    }

    if(importDirectoryMode)
    {
        if(handlemod == ToAnimationType || handlemod == ToModleType)
        {
            outFileName = outpath + "/";
        }
        else
        {
            outFileName = outpath + "/" + outFileName + suffix;
        }
    }
    else
    {
        outFileName = outpath + "/" + outFileName + suffix;
    }
    
    args.Push(mod);
    args.Push("\"" + inpath + "\"");
    args.Push("\"" + outFileName + "\"");
/*
    Array<String> options = importOptions.Trimmed().Split(' ');
    for (uint i = 0; i < options.length; ++i)
        args.Push(options[i]);
    // If material lists are to be applied, make sure the option to create them exists
    if (applyMaterialList)
        args.Push("-l");
*/
    if(lodapp !is null)
    {
        for (uint i = 0; i < lodapp.length; ++i)
        {
            args.Push(lodapp[i]);
        }
    }

    if (ExecuteAssetImporter(args) != 0)
    {
        log.Error("Failed to execute AssetImporter to import model");
        for(int i=0; i < args.length; ++i)
        {
            log.Error(args[i]);
        }
    }
    else
    {
        log.Info("success to switch asset !!");
    }
    HideImportAssetEditor();
}
