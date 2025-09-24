#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
#include "Shader.hpp"
// #include <vector>

class EditorFramebuffer{
    public:

        EditorFramebuffer() = default;
        void Init(float ScreenWidth, float ScreenHeight);
        void RescaleFramebuffer(float width, float height);
        void BindFramebuffer();
        void UnbindFramebuffer();
        GLuint GetTextureID() const;
    private:
        GLuint FBO = 0;
        GLuint texture = 0;
        GLuint RBO = 0;

};
