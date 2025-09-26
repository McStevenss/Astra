#include "Engine.h"

Engine::Engine()
{
    Initialize();

    cam = Camera(60.0f,0.1f,1000.0f);
    cam.yaw = 4.7f; cam.pitch = 0.4f;

    player = Player(glm::vec3(278.0f,0.0f,124.0f), glm::vec3(1.0f), "models/vampire_base/Vampire A Lusth.dae");    
    cam.SetFocusPosition(&player.mPosition);

    heightMapShader = new Shader("shaders/hmap.vs","shaders/hmap.fs", "shaders/hmap.g");
    heightMapColorShader = new Shader("shaders/circle.vs","shaders/circle.fs");

    terrainMap = new TerrainMap(2,2,GRID_SIZE, CELL_SIZE);
    terrainMap->build();
        
    mapCircle.Init(brush.radius,64);
    editorFramebuffer.Init(ScreenWidth, ScreenHeight);
    // skyBox.Init();
    hdriBox.Init(ScreenWidth, ScreenHeight);

    terrainMap->load("saved");
}

void Engine::Initialize()
{
    // SDL init
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    win = SDL_CreateWindow("Mini WoW Sandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ScreenWidth, ScreenHeight, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    glctx = SDL_GL_CreateContext(win);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

     // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(win, glctx);
    ImGui_ImplOpenGL3_Init("#version 410");
    SDL_GL_SetSwapInterval(1);
    
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

}

float Engine::GetDeltaTime()
{
    static uint64_t lastTime = SDL_GetTicks64();
    uint64_t currentTime = SDL_GetTicks64();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    return deltaTime;
}

void Engine::Start()
{
    // glViewport(0, 0, ScreenWidth, ScreenHeight);
    uint32_t prevTicks = SDL_GetTicks();

    Shader PlayerShader("shaders/model.vs","shaders/model.fs");
    Shader SkyboxShader("shaders/skybox.vs","shaders/skybox.fs");
    Shader PBRShader("shaders/pbr_nt.vs","shaders/pbr_nt.fs");

    PBRShader.use();
    PBRShader.setInt("irradianceMap", 0);
    PBRShader.setInt("prefilterMap", 1);
    PBRShader.setInt("brdfLUT", 2);
    PBRShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
    PBRShader.setFloat("ao", 1.0f);




    Sphere test_sphere;
    glm::vec3 sphere_pos(275.0f,15.0f,130.2f);


    while(running)
    {
        float dt = GetDeltaTime();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImVec2 imgPos = RenderGUI(); // now it returns the top-left of the image inside window
        ImGui::Render();
        
        // --- Picking ---
        SDL_GetWindowSize(win,&ScreenWidth,&ScreenHeight);

        // BindFramebuffer();
        editorFramebuffer.BindFramebuffer();
        
        glm::vec2 localPos(mx - imgPos.x, my - imgPos.y);
        bool insideImage = (localPos.x >= 0 && localPos.x <= EditorWindowWidth && localPos.y >= 0 && localPos.y <= EditorWindowHeight);

        glm::mat4 View = cam.view(terrainMap);
        glm::mat4 Projection = cam.proj(EditorWindowWidth/(float)EditorWindowHeight);
        glm::mat4 VP = Projection*View; 
        bool hasHit = false;
        glm::vec3 hit;
        
        if(insideImage && editMode){
            
            glm::mat4 invVP = glm::inverse(VP);
            float xN = (2.0f * localPos.x / EditorWindowWidth - 1.0f);
            float yN = (1.0f - 2.0f * localPos.y / EditorWindowHeight);
            glm::vec4 p0 = invVP * glm::vec4(xN,yN,-1,1);
            glm::vec4 p1 = invVP * glm::vec4(xN,yN, 1,1);
            p1/=p1.w;
            p0/=p0.w;

            glm::vec3 ro = glm::vec3(p0); glm::vec3 rd = glm::normalize(glm::vec3(p1-p0));
            float closestT = 1e9f;
            for (auto& chunk : terrainMap->GetChunks()) 
            {
                glm::vec3 localRo = ro - chunk->position;  // move ray origin into chunk space
                glm::vec3 tmpHitLocal;

                if (chunk->rayHeightmapIntersect(localRo, rd, 4000.0f, tmpHitLocal)) {
                    glm::vec3 tmpHitWorld = tmpHitLocal + chunk->position; // convert back to world
                    float t = glm::length(tmpHitWorld - ro);
                    if (t < closestT) {
                        closestT = t;
                        hit = tmpHitWorld;
                        hasHit = true;
                    }
                }
            }

            if(hasHit && (lmb)){
                terrainMap->applyBrush(brush, hit, shift); 
            }
        }
        
        terrainMap->updateDirtyChunks();
        // --- Render ---
        glClearColor(0.52f,0.75f,0.95f,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        HandleInput(dt, insideImage);
        cam.Update(dt);

        // --- Render Player ---
        player.Update(dt, *terrainMap);
        PlayerShader.use();
        PlayerShader.setMat4("view",View);
        PlayerShader.setMat4("projection",Projection);
        PlayerShader.setVec3("lightDir",lightDir);
        player.Render(PlayerShader, cam, VP);
        
        // --- Render Test PBR Sphere ---
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sphere_pos);

        
        PBRShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, hdriBox.irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, hdriBox.prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, hdriBox.brdfLUTTexture);

        PBRShader.setMat4("view",View);
        PBRShader.setMat4("projection",Projection);
        PBRShader.setVec3("camPos",cam.positionWithCollision(terrainMap));
        PBRShader.setFloat("metallic", metallic);
        PBRShader.setFloat("roughness", roughness);
        PBRShader.setMat4("model", model);
        PBRShader.setBool("useIrradiance",useIrradiance);
        PBRShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));      
        for (unsigned int i = 0; i < 4; ++i)
        {
            PBRShader.setVec3("lightPositions[" + std::to_string(i) + "]", glm::vec3(266.0f-(i*5),29.0f,128.0f-(i*5)));
            PBRShader.setVec3("lightColors[" + std::to_string(i) + "]", glm::vec3(300.0f, 300.0f, 300.0f));
        }
        test_sphere.Render();

        // --- Render Terrain ---
        heightMapShader->use();
        heightMapShader->setBool("uFlatShading", flatshade);
        heightMapShader->setBool("uShowSlopes", showSlopes);
        heightMapShader->setVec3("uCamPos", cam.positionWithCollision(terrainMap));
        heightMapShader->setVec3("uLightDir", lightDir);
        terrainMap->render(*heightMapShader, Projection, View, wire);

        // Draw brush ring at hit position
        if(hasHit && editMode){
            mapCircle.RenderProjectedCircle(*heightMapColorShader,hit,VP,brush.radius,*terrainMap);
        }

        // --- Render Skybox ---
        // skyBox.Render(SkyboxShader,Projection, View);
        hdriBox.Render(View,Projection, showIrradianceMap);


        editorFramebuffer.UnbindFramebuffer();

        // --- Render GUI ---
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(win);
    }

}

void Engine::HandleInput(float dt, bool insideImage)
{
    SDL_GetMouseState(&mx,&my);
    SDL_Event e; 
    while(SDL_PollEvent(&e))
    {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if(e.type==SDL_QUIT) running=false;
        if(e.type==SDL_WINDOWEVENT && e.window.event==SDL_WINDOWEVENT_SIZE_CHANGED){ cam.recalculateViewport(e); }
        if(e.type==SDL_MOUSEBUTTONDOWN){ if(e.button.button==SDL_BUTTON_RIGHT) rmb=true; if(e.button.button==SDL_BUTTON_LEFT) lmb=true; if(e.button.button==SDL_BUTTON_MIDDLE) mmb=true; }
        if(e.type==SDL_MOUSEBUTTONUP){ if(e.button.button==SDL_BUTTON_RIGHT) rmb=false; if(e.button.button==SDL_BUTTON_LEFT) lmb=false; if(e.button.button==SDL_BUTTON_MIDDLE) mmb=false; }
        
        if(e.type==SDL_MOUSEWHEEL)
        {
            if(editMode)
            {
                if(e.wheel.y>0) brush.radius*=1.1f;
                if(e.wheel.y<0) brush.radius/=1.1f; 
                brush.radius = glm::clamp(brush.radius, 1.0f, 100.0f); 
            }
            else
            {
                cam.Zoom(e);
            }
        }
        if(e.type==SDL_KEYDOWN){
            if(e.key.keysym.sym==SDLK_ESCAPE) running=false;

            if(editMode)
            {
                if(e.key.keysym.sym==SDLK_1) {brush.mode = BrushMode::RaiseLower;};
                if(e.key.keysym.sym==SDLK_2) {brush.mode = BrushMode::Flat;};
                if(e.key.keysym.sym==SDLK_3) {brush.mode = BrushMode::Smooth;};
                if(e.key.keysym.sym==SDLK_LCTRL) brush.Falloff=false;
                if(e.key.keysym.sym==SDLK_v) brush.strength = glm::max(0.1f, brush.strength*0.9f);
                if(e.key.keysym.sym==SDLK_b) brush.strength = glm::min(10.0f, brush.strength*1.1f);
            }

            if(e.key.keysym.sym==SDLK_LSHIFT || e.key.keysym.sym==SDLK_RSHIFT) shift=true;
            if(e.key.keysym.sym==SDLK_TAB) flatshade=!flatshade;
            if(e.key.keysym.sym==SDLK_f){ wire=!wire; }
            if(e.key.keysym.sym==SDLK_F9){ terrainMap->load("saved");} 
            if(e.key.keysym.sym==SDLK_F10){ terrainMap->save("saved");} 
        }
        
        if(e.type==SDL_KEYUP){ if(e.key.keysym.sym==SDLK_LSHIFT || e.key.keysym.sym==SDLK_RSHIFT) shift=false; }
        if(e.type==SDL_KEYUP){ if(e.key.keysym.sym==SDLK_LCTRL) brush.Falloff=true; }

        if(!editMode && insideImage){
            cam.HandleInput(e,mx,my);
        }
    }

    player.HandleInput(dt,cam.forward,cam.right, rmb, lmb);
}

ImVec2 Engine::RenderGUI()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ScreenWidth*0.8f, ScreenHeight), ImGuiCond_Always);
    ImGui::Begin("Editor",
                 nullptr,
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoTitleBar);

    EditorWindowWidth = ImGui::GetContentRegionAvail().x;
    EditorWindowHeight = ImGui::GetContentRegionAvail().y;

    // rescale framebuffer
    editorFramebuffer.RescaleFramebuffer(EditorWindowWidth, EditorWindowHeight);
    glViewport(0, 0, EditorWindowWidth, EditorWindowHeight);

    // get correct image position **inside the window**
    ImVec2 imgPos = ImGui::GetCursorScreenPos();

    // add image
    ImGui::GetWindowDrawList()->AddImage(
        (ImTextureID)(intptr_t)editorFramebuffer.GetTextureID(),
        imgPos,
        ImVec2(imgPos.x + EditorWindowWidth, imgPos.y + EditorWindowHeight),
        ImVec2(0,1),
        ImVec2(1,0)
    );

    ImGui::End();

    // --- Settings Window (20%) ---
    ImGui::SetNextWindowPos(ImVec2(ScreenWidth * 0.8f, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ScreenWidth * 0.2f, ScreenHeight), ImGuiCond_Always);
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    //--------------------------------------------------------------------
    ImGui::SeparatorText("Status");
    if(ImGui::Button("Toggle Wireframe")) { wire = !wire; }
    ImGui::Checkbox("Edit Mode", &editMode);
    ImGui::Checkbox("Flat Shading", &flatshade);
    ImGui::Checkbox("Show Slopes", &showSlopes);
    ImGui::Checkbox("Project Circle", &projectCircle);

    //--------------------------------------------------------------------
    ImGui::SeparatorText("Camera");
    ImGui::Text("Player Position: (%.1f, %.1f, %.1f)", player.mPosition.x, player.mPosition.y, player.mPosition.z);
    ImGui::Text("Player Velocity: (%.1f, %.1f, %.1f)", player.mVelocity.x, player.mVelocity.y, player.mVelocity.z);
    ImGui::Text("Player Is Grounded: %s", player.isGrounded ? "true" : "false");
    ImGui::Text("Player Is Sliding: %s", player.isSliding ? "true" : "false");
    ImGui::Text("Yaw: %.1f", cam.yaw);
    ImGui::Text("Pitch: %.1f", cam.pitch);
    ImGui::Text("Distance: %.1f", cam.distance);
    //--------------------------------------------------------------------
    ImGui::SeparatorText("PBR Settings");
    ImGui::SliderFloat("Metallic", &metallic, 0.05f, 1.0f);
    ImGui::SliderFloat("Roughness", &roughness, 0.05f, 1.0f);
    ImGui::Checkbox("Irradiance", &useIrradiance);
    ImGui::Checkbox("Show Irradiance Map", &showIrradianceMap);

    
    //--------------------------------------------------------------------
    ImGui::SeparatorText("Brush Settings");
    ImGui::SliderFloat("Brush Radius", &brush.radius, 0.1f, 100.0f);
    ImGui::SliderFloat("Brush Strength", &brush.strength, 0.01f, 10.0f);
    const char* brushModes[] = {"Raise/Lower", "Smooth", "Flat"};
    int currentBrushMode = static_cast<int>(brush.mode); // keep track of selection
    if (ImGui::Combo("Brush Mode", &currentBrushMode, brushModes, IM_ARRAYSIZE(brushModes))) {
        brush.mode = static_cast<BrushMode>(currentBrushMode);
    }
    //--------------------------------------------------------------------
    ImGui::SeparatorText("Keybinds");
    ImGui::Text("[F] Wireframe toggle");
    ImGui::Text("[E/Q] Up/Down");
    ImGui::Text("[TAB] Smooth shade toggle");
    ImGui::Text("[SCRLWHL] Brush radius");
    ImGui::Text("[v/b] Brush strength");
    ImGui::Text("[MB1] Raise terrain");
    ImGui::Text("[Shift + MB1] Lower terrain");
    ImGui::Text("[MMB] Smooth terrain");
    ImGui::Text("[LCTRL + MB1] Raise terrain, no falloff");
    
    ImGui::End();

    return imgPos;
}