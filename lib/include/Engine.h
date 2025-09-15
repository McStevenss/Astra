#pragma once
#include <SDL2/SDL.h>
// #include <GL/glew.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <limits>
#include <cmath>
#include <Shader.hpp>
#include "TerrainChunk.hpp"
#include "TerrainMap.h"
#include "Camera.hpp"
//ImGui + SDL
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Player.h"
#include <unordered_map>
#include "Animation.h"
#include "Animator.h"
#include "Model.hpp"
#include "Mesh.hpp"

class Engine {

    public:
        Engine();
        
        void Initialize();
        void Start();
        
    private:
        
        Camera cam;
        Player player;
        void buildCircle(std::vector<glm::vec3>& out, float radius, int segments=64);
        void GenCircleGL();
        void HandleInput(float dt);
        void CreateFrameBuffer();
        void RescaleFramebuffer(float width, float height);
        void BindFramebuffer();
        void UnbindFramebuffer();
        float GetDeltaTime();
        ImVec2 RenderGUI();
        Shader* heightMapShader;
        Shader* heightMapColorShader;
        // TerrainChunk* terrainChunk;
        TerrainMap* terrainMap;
        Brush brush;

        int ScreenWidth=1920;
        int ScreenHeight=1080;
        SDL_Window* win;
        SDL_GLContext glctx;

        GLuint ringVBO=0;
        GLuint ringVAO=0;
        std::vector<glm::vec3> ringVerts;
        
                
        glm::vec3 playerPos;
        std::map<std::string, Animation> animations;

        
        glm::vec3 lightDir = glm::vec3(0.3,1.0,0.2);

        bool running=true;
        bool editMode=false;
        float aspect=ScreenWidth/ScreenHeight;
        bool wire=false;
        bool rmb=false; 
        bool lmb=false; 
        bool mmb=false; 
        bool shift=false;
        bool flatshade=false;
        bool showSlopes=false;
        bool projectCircle=true;
        float EditorWindowWidth;
        float EditorWindowHeight;

        int mx=0,my=0;
        // ------------ Config ------------
        const int   GRID_SIZE   = 256;          // 128x128 height samples
        const float TILE_SIZE   = 533.333f;     // WoW ADT ~533.333m, optional
        const float CELL_SIZE   = TILE_SIZE / (GRID_SIZE - 1);

        GLuint FBO = 0;
        GLuint texture_id = 0;
        GLuint RBO = 0;

};

