#include "MapCircle.h"

// MapCircle::MapCircle(float radius, int segments)
// {
//     pRadius = radius;
//     pSegments = segments;
//     buildCircle(ring, pRadius, pSegments); 


//     glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
//     glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, Vertices.size()*sizeof(glm::vec3), Vertices.data(), GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0); glBindVertexArray(0);   
// }

void MapCircle::Init(float radius, int segments)
{
    pRadius = radius;
    pSegments = segments;
    buildCircle(ring, pRadius, pSegments); 

    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, Vertices.size()*sizeof(glm::vec3), Vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0); glBindVertexArray(0);  
}

void MapCircle::Render(Shader &shader, glm::mat4 VP, glm::vec3 position)
{
    // std::vector<glm::vec3> ring;
    // buildCircle(ring, 0.5f, 24);    
    // glm::mat4 Mring = glm::mat4(1.0f); // identity

    // for(auto& v : ring){ v.y = 0.0f; }
    Mring = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y + 0.05f, position.z));
  
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, ring.size()*sizeof(glm::vec3), ring.data(), GL_DYNAMIC_DRAW);

    shader.use();
    shader.setMat4("uVP", VP);
    shader.setMat4("uM", Mring);
    shader.setVec4("uColor", pColor);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, (GLint)ring.size());
    glBindVertexArray(0);
}

void MapCircle::RenderProjectedCircle(Shader &shader, glm::vec3 hit, glm::mat4 VP, float radius, TerrainMap& terrainMap)
{
    std::vector<glm::vec3> ring; buildCircle(ring, radius, 96);
            

    for(auto& v : ring)
    {                    
        float worldX = v.x + hit.x;
        float worldZ = v.z + hit.z;
        TerrainChunk* tempChunk = terrainMap.getChunkAt(glm::vec3(worldX,0.0f,worldZ));
        if (tempChunk) {
            glm::vec3 local = glm::vec3(worldX, 0.0f, worldZ) - tempChunk->position;
            v.y = tempChunk->getHeightAt(local.x, local.z) + tempChunk->circleOffset;
        } else {
            v.y = 0.0f;
        } 
        // update vertex to world-space XZ
        v.x = worldX;
        v.z = worldZ;
    }                

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, ring.size()*sizeof(glm::vec3), ring.data(), GL_DYNAMIC_DRAW);



    shader.use();
    shader.setMat4("uVP", VP);
    shader.setMat4("uM", Mring);
    shader.setVec4("uColor", glm::vec4(0.0f,0.0f,0.0f,1.0f));
    

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, (GLint)ring.size());
    glBindVertexArray(0);
}

void MapCircle::buildCircle(std::vector<glm::vec3>& out, float radius, int segments)
{
    out.clear(); out.reserve(segments);
    for(int i=0;i<segments;++i){
        float a = (i/(float)segments)*6.2831853f;
        out.emplace_back(radius*cosf(a), 0.0f, radius*sinf(a));
    }

    for(auto& v : out){
        v.y = 0.0f; 
    }
}

void MapCircle::SetColor(glm::vec3 color)
{
    pColor = glm::vec4(color,1.0f);
}
