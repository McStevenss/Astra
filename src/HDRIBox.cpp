#include "HDRIBox.h"

void HDRIBox::Init(float ScreenWidth, float ScreenHeight)
{
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);  
    glEnable(GL_DEPTH_TEST);


    equirectangularToCubemapShader = new Shader("shaders/HDRI/cubemap.vs","shaders/HDRI/equirectangular.fs");
    irradianceShader = new Shader("shaders/HDRI/cubemap.vs","shaders/HDRI/irradiance_conv.fs");
    backgroundShader = new Shader("shaders/HDRI/background.vs","shaders/HDRI/background.fs");
    prefilterShader = new Shader("shaders/HDRI/cubemap.vs","shaders/HDRI/prefilter.fs");
    brdfShader = new Shader("shaders/HDRI/brdf.vs","shaders/HDRI/brdf.fs");

    envBox = new Cube();
    envQuad = new Quad();

    std::cout << "[HDRIBox] Creating framebuffer/renderobjects" << std::endl;
    CreateFramebuffers();
    
    std::cout << "[HDRIBox] Loading textures" << std::endl;
    LoadHDRITexture();
    RenderProjections();
    
    std::cout << "[HDRIBox] Creating Irradiance maps" << std::endl;
    CreateIrradianceMap();
    CreateLUTTexture();
    CreatePrefilterMap();

    CalculateIrradiance(ScreenWidth, ScreenHeight);
    CalculatePrefilter();

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

void HDRIBox::CreatePrefilterMap()
{
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilterSize, prefilterSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void HDRIBox::CreateLUTTexture()
{
    glGenTextures(1, &brdfLUTTexture);
    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, cubemapSize, cubemapSize, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubemapSize, cubemapSize);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, cubemapSize, cubemapSize);
    brdfShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    envQuad->Render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void HDRIBox::CalculatePrefilter()
{
    glDepthFunc(GL_LEQUAL); 
    prefilterShader->use();
    prefilterShader->setInt("environmentMap", 0);
    prefilterShader->setMat4("projection", captureProjection);
    prefilterShader->setFloat("resolution", (float)cubemapSize);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth  = static_cast<unsigned int>(prefilterSize * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(prefilterSize * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            prefilterShader->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            envBox->Render();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
