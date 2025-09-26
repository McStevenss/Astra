#include "HDRIBox.h"

void HDRIBox::Init(float ScreenWidth, float ScreenHeight)
{

    glEnable(GL_DEPTH_TEST);


    equirectangularToCubemapShader = new Shader("shaders/HDRI/cubemap.vs","shaders/HDRI/equirectangular.fs");
    irradianceShader = new Shader("shaders/HDRI/cubemap.vs","shaders/HDRI/irradiance_conv.fs");
    backgroundShader = new Shader("shaders/HDRI/background.vs","shaders/HDRI/background.fs");

    envBox = new Cube();

    std::cout << "[HDRIBox] Creating framebuffer/renderobjects" << std::endl;
    CreateFramebuffers();
    
    std::cout << "[HDRIBox] Loading textures" << std::endl;
    LoadHDRITexture();
    RenderProjections();
    
    std::cout << "[HDRIBox] Creating Irradiance maps" << std::endl;
    CreateIrradianceMap();
    CalculateIrradiance(ScreenWidth, ScreenHeight);

}

void HDRIBox::Render(glm::mat4 View, glm::mat4 Projection, bool showIrradianceMap)
{
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    backgroundShader->use();
    backgroundShader->setMat4("view", View);
    backgroundShader->setMat4("projection", Projection);
    backgroundShader->setInt("environmentMap", 0);
    glActiveTexture(GL_TEXTURE0);
    if(showIrradianceMap){
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    }
    else{
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    }
    envBox->Render();
}

void HDRIBox::LoadHDRITexture()
{
    //--- Load HDRI Texture ---
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    // float *data = stbi_loadf("textures/qwantani_moon_noon_puresky_4k.hdr", &width, &height, &nrComponents, 0);
    float *data = stbi_loadf("textures/horn-koppe_spring_2k.hdr", &width, &height, &nrComponents, 0);
    
    if (data)
    {
        // --- Clamp HDR values to a reasonable range (0.0 - 50.0 for example) ---
        int pixelCount = width * height;
        for (int i = 0; i < pixelCount * 3; ++i) // multiply by 3 for RGB
        {
            if (data[i] > HDRImageColorCap) data[i] = HDRImageColorCap;
            if (data[i] < 0.0f) data[i] = 0.0f; // optional: clamp negative values
        }


        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); 

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "[HDRIBox] Successfully loaded HDR image." << std::endl;

    }
    else
    {
        std::cout << "[HDRIBox] Failed to load HDR image." << std::endl;
    }  

    //--- Create environment cubemap ---
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, cubemapSize, cubemapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void HDRIBox::CreateFramebuffers()
{
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapSize, cubemapSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
}

void HDRIBox::RenderProjections()
{
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    equirectangularToCubemapShader->use();
    equirectangularToCubemapShader->setInt("equirectangularMap", 0);
    equirectangularToCubemapShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);

    glViewport(0, 0, cubemapSize, cubemapSize); // don't forget to configure the viewport to the capture dimensions. (512x512)
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        envBox->Render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HDRIBox::CreateIrradianceMap()
{
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceSize, irradianceSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceSize, irradianceSize);
}

void HDRIBox::CalculateIrradiance(float ScreenWidth, float ScreenHeight)
{
    glDepthFunc(GL_LEQUAL); 
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    irradianceShader->use();
    irradianceShader->setInt("environmentMap",0);
    irradianceShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, irradianceSize, irradianceSize); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        envBox->Render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // glViewport(0, 0, ScreenWidth, ScreenHeight);

}
