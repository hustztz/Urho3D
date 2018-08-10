EditorNormalViewWindow@ editorNormalViewWindow;

class EditorNormalViewWindow
{
    Window@ normalViewWindow;
    Scene@ previewScene;
    View3D@ normalpreview;
    Node@ previewModelNode;
    LineEdit@ modelPathWnd;
    Node@ lookPointNode;
    Node@ cameraNode;
    Node@ cameraLookAtNode;
    Camera@ camera;
    Node@ gridNode;
    int cameraYaw = 0;
    int cameraPitch = 0;
    bool isupdateing = false;

    EditorNormalViewWindow()
    {
        normalViewWindow = LoadEditorUI("UI/EditorNormalViewWindow.xml");
        ui.root.AddChild(normalViewWindow);
        normalViewWindow.opacity = uiMaxOpacity;
        normalViewWindow.visible = false;
        InitNormalViewPreview();
        RegistBtnEvents();
    }

    void RegistBtnEvents()
    {
        @modelPathWnd = normalViewWindow.GetChild("modelpath", true);
        SubscribeToEvent(modelPathWnd, "TextChanged", "ModelPathChangeEvent");
        SubscribeToEvent(normalViewWindow.GetChild("closebtn", true), "Released", "HideWindow");
        SubscribeToEvent(normalViewWindow.GetChild("selectmodel", true), "Released", "SelectModelEvent");
        SubscribeToEvent(normalViewWindow.GetChild("resetcamerabtn", true), "Released", "ResetCamera");
        //normalViewWindow.GetChild("toggellookatbtn", true).visible = false;
        SubscribeToEvent(normalViewWindow.GetChild("toggellookatbtn", true), "Released", "ToggelLooKPointVisible");
        
    }

    void InitNormalViewPreview()
    {
        previewScene = Scene("PreviewScene");
        previewScene.CreateComponent("Octree");
        
        Skybox@ skybox  = previewScene.CreateComponent("Skybox");
        skybox.model    = cache.GetResource("Model", "Models/Box.mdl");
        skybox.material = cache.GetResource("Material", "Materials/DynamicSkybox.xml");

        cameraLookAtNode = previewScene.CreateChild("PreviewCamera");
        cameraLookAtNode.position = Vector3(0, 0, -2);
        cameraNode = Node();
        cameraLookAtNode.AddChild(cameraNode);

        camera = cameraNode.CreateComponent("Camera");
        camera.nearClip = 0.1f;
        camera.farClip = 10000000.0f;

        normalpreview = normalViewWindow.GetChild("normalpreview", true);
        normalpreview.SetView(previewScene, camera);
        normalpreview.viewport.renderPath = renderPath;
       // normalpreview.autoUpdate = false;

        lookPointNode = previewScene.CreateChild("PreviewModel");
        lookPointNode.enabled  = false;
        lookPointNode.SetScale(0.2);
        lookPointNode.rotation = Quaternion(0, 0, 0);
        StaticModel@ boxModel = lookPointNode.CreateComponent("StaticModel");
        boxModel.model = cache.GetResource("Model", "Models/Sphere.mdl");
        //boxModel.model = cache.GetResource("Model", "Models/Box.mdl");
        //ConfigNormalMat(lookPointNode);
        SubscribeToEvent(normalpreview, "DragMove", "RotateMaterialPreview");

        CreateGrid();
    }

    void CreateGrid()
    {
        if (gridNode !is null)
            gridNode.Remove();

        gridNode = Node();
        gridNode.name = "EditorGrid";
        grid = gridNode.CreateComponent("CustomGeometry");
        grid.numGeometries = 1;
        grid.material = cache.GetResource("Material", "Materials/VColUnlit.xml");
        grid.viewMask = 0x80000000; // Editor raycasts use viewmask 0x7fffffff
        grid.occludee = false;

        UpdateGrid();
    }

    void UpdateGrid(bool updateGridGeometry = true)
    {
       // showGrid ? ShowGrid() : HideGrid();
        gridNode.scale = Vector3(gridScale, gridScale, gridScale);
        previewScene.octree.AddManualDrawable(grid);
        
        if (!updateGridGeometry)
            return;

        uint size = uint(Floor(gridSize / 2) * 2);
        float halfSizeScaled = size / 2;
        float scale = 1.0;
        uint subdivisionSize = uint(gridSubdivisions);

        if (subdivisionSize > 0)
        {
        // size *= subdivisionSize;
        //  scale /= subdivisionSize;
        }

        uint halfSize = size / 2;

        grid.BeginGeometry(0, LINE_LIST);
        float lineOffset = -halfSizeScaled;
        for (uint i = 0; i <= size; ++i)
        {
            bool lineCenter = i == halfSize;
            bool lineSubdiv = !Equals(Mod(i, subdivisionSize), 0.0);

            if (!grid2DMode)
            {
                grid.DefineVertex(Vector3(lineOffset, 0.0, halfSizeScaled));
                grid.DefineColor(lineCenter ? gridZColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
                grid.DefineVertex(Vector3(lineOffset, 0.0, -halfSizeScaled));
                grid.DefineColor(lineCenter ? gridZColor : (lineSubdiv ? gridSubdivisionColor : gridColor));

                grid.DefineVertex(Vector3(-halfSizeScaled, 0.0, lineOffset));
                grid.DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
                grid.DefineVertex(Vector3(halfSizeScaled, 0.0, lineOffset));
                grid.DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
            }
            else
            {
                grid.DefineVertex(Vector3(lineOffset, halfSizeScaled, 0.0));
                grid.DefineColor(lineCenter ? gridYColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
                grid.DefineVertex(Vector3(lineOffset, -halfSizeScaled, 0.0));
                grid.DefineColor(lineCenter ? gridYColor : (lineSubdiv ? gridSubdivisionColor : gridColor));

                grid.DefineVertex(Vector3(-halfSizeScaled, lineOffset, 0.0));
                grid.DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
                grid.DefineVertex(Vector3(halfSizeScaled, lineOffset, 0.0));
                grid.DefineColor(lineCenter ? gridXColor : (lineSubdiv ? gridSubdivisionColor : gridColor));
            }

            lineOffset  += scale;
        }
        grid.Commit();
    }

    void ToggelLooKPointVisible()
    {
        lookPointNode.enabled = !lookPointNode.enabled;
        normalpreview.QueueUpdate();
    }

    void ShowNormalDisplay(Node@ node = null)
    {
        if(previewModelNode is null)
        {
            CreateDefaultData();        
        }
        ConfigNormalMat(previewModelNode);
        normalpreview.QueueUpdate();
    }

    void ConfigNormalMat(Node@ node)
    {
        if(node is null) return;
        ModelEffectUtil::SetNormalShowEffect(node);
        /*
        Array<Component@> modelcomplist = node.GetComponents("StaticModel", true);
        Material@ normalviewmat = cache.GetResource("Material", "Materials/DebugNormal.xml");
        for(int i=0; i < modelcomplist.length; ++i)
        {
            StaticModel@ stcomp = cast<StaticModel@>(modelcomplist[i]);
            int matnum = stcomp.numGeometries;
            for(int j=0; j < matnum; ++j)
            {
                stcomp.materials[j] = normalviewmat;
            }
        }
        Array<Node@>@ childs = node.GetChildren();
        for(int i=0; i < childs.length; ++i)
        {
            ConfigNormalMat(childs[i]);
        }
        */
    }

    void CreateDefaultData()
    {
        previewModelNode = previewScene.CreateChild("PreviewModel");
        previewModelNode.rotation = Quaternion(0, 0, 0);
        StaticModel@ boxModel = previewModelNode.CreateComponent("StaticModel");
        boxModel.model = cache.GetResource("Model", "Models/Box.mdl");
    }

    void RemovePreNormalDisplay()
    {
        if(previewScene !is null)
        {
            previewScene.RemoveChild(previewModelNode);
            previewModelNode = null;
        }
    }

    bool IsShow()
    {
        return normalViewWindow.visible;
    }

    void HideWindow()
    {
        normalViewWindow.visible = false;
        EnableUpdate(false);
    }
    
    void ShowWindow(bool shotcut)
    {
        EnableUpdate(true);
        if(shotcut)
        {
            RemovePreNormalDisplay();
            Array<XMLFile@> sceneCopyBuffer;
            for (uint i = 0; i < selectedNodes.length; ++i)
            {
                    // Skip the root scene node as it cannot be copied
                if (selectedNodes[i] is editorScene)
                    continue;

                XMLFile@ xml = XMLFile();
                XMLElement rootElem = xml.CreateRoot("node");
                selectedNodes[i].SaveXML(rootElem);
                sceneCopyBuffer.Push(xml);
            }

            previewModelNode = previewScene.CreateChild("PreviewModel");
            for (uint i = 0; i < sceneCopyBuffer.length; ++i)
            {
                XMLElement rootElem = sceneCopyBuffer[i].root;
                Node@ newNode = previewModelNode.CreateChild("", LOCAL);
                newNode.LoadXML(rootElem);
                newNode.position = Vector3(0,0,0);
                newNode.rotation = Quaternion(0, 0, 0);
            }
        }

        normalViewWindow.visible = true;
        normalViewWindow.BringToFront();
        CenterDialog(normalViewWindow);
        ShowNormalDisplay();
        ResetCamera();
    }

    void SelectModelEvent()
    {
        CreateFileSelector("选择模型", "OK", "Cancel", uiImportPath, {"*.mdl"}, uiImportFilter);
        SubscribeToEvent(uiFileSelector, "FileSelected", "PickModelDone");
    }

    void PickModelDone(StringHash eventType, VariantMap& eventData)
    {
        CloseFileSelector();
        if (!eventData["OK"].GetBool())
        {
            return;
        }

        String resourceName = eventData["FileName"].GetString();
        modelPathWnd.text = resourceName;
    }

    void ModelPathChangeEvent()
    {
        Model@ model =  cache.GetResource("Model", modelPathWnd.text);
        if(model is null)
        {
            return;
        }

        RemovePreNormalDisplay();
        previewModelNode = previewScene.CreateChild("PreviewModel");
        previewModelNode.rotation = Quaternion(0, 0, 0);
        StaticModel@ boxModel = previewModelNode.CreateComponent("StaticModel");
        boxModel.model = model;
        ConfigNormalMat(previewModelNode);

        normalpreview.QueueUpdate();
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        float timeStep = eventData["TimeStep"].GetFloat();
        MoveCameraPanel(timeStep);
        CameraRotate();
        Zoom();
    }

    void MoveCameraPanel(float timeStep)
    {
        int cameraBaseSpeed = 4.0;
      //  Node@ cameraNode = cameraLookAtNode;
        if (input.keyDown[KEY_W] || input.keyDown[KEY_UP])
        {
            Vector3 dir = cameraNode.direction;
            dir.Normalize();

            CameraPan(dir * timeStep * cameraBaseSpeed);
        }
        if (input.keyDown[KEY_S] || input.keyDown[KEY_DOWN])
        {
            Vector3 dir = cameraNode.direction;
            dir.Normalize();

            CameraPan(-dir * timeStep * cameraBaseSpeed);
        }
        if (input.keyDown[KEY_A] || input.keyDown[KEY_LEFT])
        {
            Vector3 dir = cameraNode.right;
            dir.Normalize();

            CameraPan(-dir * timeStep * cameraBaseSpeed);
        }
        if (input.keyDown[KEY_D] || input.keyDown[KEY_RIGHT])
        {
            Vector3 dir = cameraNode.right;
            dir.Normalize();

            CameraPan(dir * timeStep * cameraBaseSpeed);
        }
        if (input.keyDown[KEY_E] || input.keyDown[KEY_PAGEUP])
        {
            Vector3 dir = cameraNode.up;
            dir.Normalize();

            CameraPan(dir * timeStep * cameraBaseSpeed);
        }
        if (input.keyDown[KEY_Q] || input.keyDown[KEY_PAGEDOWN])
        {
            Vector3 dir = cameraNode.up;
            dir.Normalize();

            CameraPan(-dir * timeStep * cameraBaseSpeed);
        }
    }

    void CameraPan(Vector3 trans)
    {
        cameraLookAtNode.Translate(trans);
        lookPointNode.position = cameraLookAtNode.position;
        normalpreview.QueueUpdate();
    }

    void CameraRotate()
    {
        bool changeCamViewButton = false;
        changeCamViewButton = input.mouseButtonDown[MOUSEB_MIDDLE] || input.mouseButtonDown[MOUSEB_RIGHT] || input.mouseButtonDown[MOUSEB_LEFT];
        if (changeCamViewButton)
        {
            IntVector2 mouseMove = input.mouseMove;
            if (mouseMove.x != 0 || mouseMove.y != 0)
            {
                cameraYaw   += mouseMove.x*0.5;
                cameraPitch -= mouseMove.y*0.5;
                Quaternion rot = Quaternion(cameraPitch, cameraYaw, 0);
                    /*
                if (input.mouseButtonDown[MOUSEB_MIDDLE]) 
                {
                    CameraRotateAroundLookAt(rot);
                }
                else 
                {
                    CameraRotateAroundCenter(rot);
                }*/
                CameraRotateAroundLookAt(rot);
            }
        }
    }

    void Zoom()
    {
        if (input.mouseMoveWheel != 0)
        {
            float distance = cameraNode.position.length;
            float ratio = distance / 40.0f;
            float factor = ratio < 1.0f ? ratio : 1.0f;
            if(factor < 0.001 && factor > 0)  
            {
                factor = 0.01;
            }
            Vector3 dir = cameraNode.direction;
            dir.Normalize();
            dir *= input.mouseMoveWheel * 8 * factor;

            CameraMoveForward(dir);
        }
    }

    void CameraMoveForward(Vector3 trans)
    {
        cameraNode.Translate(trans, TS_PARENT);
        normalpreview.QueueUpdate();
    }

    void CameraRotateAroundLookAt(Quaternion rot)
    {
        cameraNode.rotation = rot;
        Vector3 dir = cameraNode.direction;
        dir.Normalize();

        float dist = cameraNode.position.length;

        cameraNode.position = -dir * dist;

        normalpreview.QueueUpdate();
    }

    void CameraRotateAroundCenter(Quaternion rot)
    {
        cameraNode.rotation = rot;
        Vector3 dir    = cameraNode.worldDirection;
        Vector3 oldPos = cameraNode.worldPosition;
        dir.Normalize();

        float dist = cameraNode.position.length;

        cameraLookAtNode.worldPosition = cameraNode.worldPosition + dir * dist;
        cameraNode.worldPosition = oldPos;

        normalpreview.QueueUpdate();
    }

    void RotateMaterialPreview(StringHash eventType, VariantMap& eventData)
    {
        /*
        int dx = eventData["DX"].GetInt();
        int dy = eventData["DY"].GetInt();
        
        if (normalpreview.height > 0 && normalpreview.width > 0)
        {
            cameraYaw   += dy*0.5;
            cameraPitch -= dx*0.5;

            cameraNode.rotation = cameraNode.rotation.Slerp(Quaternion(cameraYaw, cameraPitch, 0), 0.1);
            Vector3 dir = cameraNode.direction;
            dir.Normalize();
            float dist = cameraNode.position.length;
            cameraNode.position = -dir * dist;
            
            normalpreview.QueueUpdate();
        }*/
    }

    void ResetCamera()
    {

        /*
        BoundingBox box;
        Array<Component@> visitedComponents;

        if(previewModelNode !is null)
        {
            MergeNodeBoundingBox(box, visitedComponents, previewModelNode);
        }
            
        Sphere sphere = Sphere(box);
        cameraLookAtNode.worldPosition = sphere.center;
        */
        cameraLookAtNode.position = Vector3(0, 0, 0);
        cameraLookAtNode.rotation = Quaternion();
        lookPointNode.position    = cameraLookAtNode.position;


        cameraNode.position = Vector3(0, 5, -9);
        cameraNode.rotation = Quaternion(Vector3(0, 0, 1), -cameraNode.position);

        cameraYaw   = cameraNode.rotation.yaw;
        cameraPitch = cameraNode.rotation.pitch;

        normalpreview.QueueUpdate();
    }

    void EnableUpdate(bool able)
    {
        if(isupdateing != able)
        {
            isupdateing = able;
            if(able)
            {
                SubscribeToEvent("Update", "HandleUpdate");
            }
            else
            {
                UnsubscribeFromEvent("Update");
            }
        }
    }
}

bool ToggleEditorNormalViewWindow()
{
    if (editorNormalViewWindow is null)
    {
        @editorNormalViewWindow = EditorNormalViewWindow();
    }
    if (editorNormalViewWindow.IsShow())
    {
        HideEditorNormalViewWindow();
    }
    else
    {
        ShowEditorNormalViewWindow();
    }
    return true;
}

void ShowEditorNormalViewWindow(bool shotcut = false)
{
    if (editorNormalViewWindow is null)
    {
        @editorNormalViewWindow = EditorNormalViewWindow();
    }
    editorNormalViewWindow.ShowWindow(shotcut);
}

void HideEditorNormalViewWindow()
{
    editorNormalViewWindow.HideWindow();
}