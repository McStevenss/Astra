#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "glad/glad.h"
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <limits>
#include <cmath>
#include "CustomFileHeaders.h"

struct VertexPNUV {
    glm::vec3 p, n;
    glm::vec2 uv;
};

struct HeightMap {
    int size;
    float cell;
    std::vector<float> h;

    HeightMap(int s, float c) : size(s), cell(c), h(s*s, 0.0f) {}

    float& at(int x,int z){ return h[z*size + x]; }
    float  at(int x,int z) const { return h[z*size + x]; }
    bool inBounds(int x,int z) const { return x>=0 && z>=0 && x<size && z<size; }

    
    // Bilinear sample height at world-space XZ
    float sampleHeight(float wx, float wz) const {
        float gx = wx / cell; // grid space
        float gz = wz / cell;
        int x0 = (int)floorf(gx); int z0 = (int)floorf(gz);
        int x1 = x0 + 1; int z1 = z0 + 1;
        if(x0 < 0 || z0 < 0 || x1 >= size || z1 >= size) return 0.0f;
        float tx = gx - x0; float tz = gz - z0;
        float h00 = at(x0,z0), h10 = at(x1,z0), h01 = at(x0,z1), h11 = at(x1,z1);
        float hx0 = h00*(1-tx) + h10*tx;
        float hx1 = h01*(1-tx) + h11*tx;
        return hx0*(1-tz) + hx1*tz;
    }

    glm::vec3 normalAt(int x,int z) const {
        float hL = inBounds(x-1,z)? at(x-1,z) : at(x,z);
        float hR = inBounds(x+1,z)? at(x+1,z) : at(x,z);
        float hD = inBounds(x,z-1)? at(x,z-1) : at(x,z);
        float hU = inBounds(x,z+1)? at(x,z+1) : at(x,z);
        return glm::normalize(glm::vec3(-(hR-hL)/(2*cell),1.0f,-(hU-hD)/(2*cell)));
    }

    float sampleHeightTri(float wx, float wz) const {
        float gx = wx / cell;
        float gz = wz / cell;
        int x0 = (int)floorf(gx);
        int z0 = (int)floorf(gz);
        int x1 = x0 + 1;
        int z1 = z0 + 1;

        if(x0 < 0 || z0 < 0 || x1 >= size || z1 >= size) return 0.0f;

        float tx = gx - x0;
        float tz = gz - z0;

        // Heights of the quad corners
        float h00 = at(x0,z0);
        float h10 = at(x1,z0);
        float h01 = at(x0,z1);
        float h11 = at(x1,z1);

        if(tx + tz < 1.0f) {
            // Tri A: (x0,z0), (x1,z0), (x0,z1)
            // Barycentric interpolation
            return h00 * (1 - tx - tz) + h10 * tx + h01 * tz;
        } else {
            // Tri B: (x1,z0), (x1,z1), (x0,z1)
            float u = 1 - tx;
            float v = 1 - tz;
            return h11 * (tx + tz - 1) + h10 * v + h01 * u;
        }
    }

    glm::vec3 computeVertexNormal(int x, int z) const {
        float hL = at(x-1, z);
        float hR = at(x+1, z);
        float hD = at(x, z-1);
        float hU = at(x, z+1);
        glm::vec3 n = glm::vec3(hL - hR, 2.0f*cell, hD - hU);
        return glm::normalize(n);
    }

    glm::vec3 triangleNormalA(int x, int z) const {
        // vertices in grid space
        glm::vec3 p00(x*cell, at(x,z), z*cell);
        glm::vec3 p10((x+1)*cell, at(x+1,z), z*cell);
        glm::vec3 p01(x*cell, at(x,z+1), (z+1)*cell);

        glm::vec3 e1 = p10 - p00;
        glm::vec3 e2 = p01 - p00;
        return glm::normalize(glm::cross(e1, e2));
    }

    glm::vec3 triangleNormalB(int x, int z) const {
        glm::vec3 p10((x+1)*cell, at(x+1,z), z*cell);
        glm::vec3 p11((x+1)*cell, at(x+1,z+1), (z+1)*cell);
        glm::vec3 p01(x*cell, at(x,z+1), (z+1)*cell);

        glm::vec3 e1 = p11 - p10;
        glm::vec3 e2 = p01 - p10;
        return glm::normalize(glm::cross(e1, e2));
    }

    glm::vec3 sampleNormalTri(float wx, float wz) const {
        // Convert world coordinates to grid coordinates
        float gx = wx / cell;
        float gz = wz / cell;
        int x0 = (int)floorf(gx);
        int z0 = (int)floorf(gz);

        // Out-of-bounds check
        if (x0 < 0 || z0 < 0 || x0 + 1 >= size || z0 + 1 >= size)
            return glm::vec3(0, 1, 0); // default up

        float tx = gx - x0;
        float tz = gz - z0;
        
        glm::vec3 normal;
        if (tx + tz < 1.0f) {
            // Point is in Tri A
            normal = triangleNormalA(x0, z0);
        } else {
            // Point is in Tri B
            normal =  triangleNormalB(x0, z0);
        }

        if (normal.y < 0) normal = -normal;

        return normal;
    }


    glm::vec3 normalAtInterpolated(float wx, float wz) const {
        // Convert world space to grid space
        float gx = wx / cell;
        float gz = wz / cell;

        int x0 = (int)floorf(gx);
        int z0 = (int)floorf(gz);
        int x1 = x0 + 1;
        int z1 = z0 + 1;

        if (!inBounds(x0, z0) || !inBounds(x1, z1))
            return glm::vec3(0,1,0);

        // Fractional offset in cell
        float tx = gx - x0;
        float tz = gz - z0;

        // Get grid normals at the 4 corners
        glm::vec3 n00 = normalAt(x0, z0);
        glm::vec3 n10 = normalAt(x1, z0);
        glm::vec3 n01 = normalAt(x0, z1);
        glm::vec3 n11 = normalAt(x1, z1);

        // Bilinear interpolation
        glm::vec3 n0 = glm::mix(n00, n10, tx);
        glm::vec3 n1 = glm::mix(n01, n11, tx);
        glm::vec3 n  = glm::mix(n0, n1, tz);

        return glm::normalize(n);
    }
};

//We remove any padding here that the compiler might add, so we can successfully load it straight from memory into RAM and it "autoparses" it to the correct HMapHeader!
// #pragma pack(push,1)
// struct HMapHeader {
//     char magic[4];
//     uint32_t size;
//     float cell; 
//     uint32_t gridX;
//     uint32_t gridZ;
// };
// #pragma pack(pop)

enum class BrushMode { RaiseLower, Smooth, Flat};
struct Brush {
    float radius=6.0f;
    bool Falloff=true;
    float strength=1.0f;
    BrushMode mode=BrushMode::RaiseLower;
};

class TerrainChunk {

    public:
        TerrainChunk(int gridSize=128, float cellSize=1.0f) : hm(gridSize, cellSize) {}
        ~TerrainChunk(){ mesh.destroy(); }
        // CPU access
        float heightAt(int x,int z) const { return hm.at(x,z); }
        float getHeightAt(float x, float y) const;
        glm::vec3 getTriNormalAt(float x, float z) const;
        float getTriHeightAt(float x, float z) const;
        glm::vec3 normalAt(int x,int z) const { return hm.normalAt(x,z); }
        bool inBounds(int x,int z) const { return hm.inBounds(x,z); }
        bool rayHeightmapIntersect(const glm::vec3& rayOrigin, const glm::vec3& rayDistance, float maxDist, glm::vec3& outHit);
        bool contains(float wx, float wz);
        bool saveHMap(const std::string& path);
        bool loadHMap(const std::string& path);
        glm::vec3 getNormalAtQuad(float x, float z);
        // GPU mesh
        void buildMesh();
        void updateMeshIfDirty();
        void resetHeightMap();
        void Render(bool wire=false);
        
        // Brush editing
        void applyBrush(const Brush& b, const glm::vec3& hit, bool lower=false);
        
        float circleOffset = 0.15f;
        HeightMap hm;

        glm::vec3 position;
        int gridX;
        int gridZ;


    private:
        void drawMesh();
     
        struct TerrainGL {
            GLuint vao=0, vbo=0, ibo=0; GLsizei indexCount=0;
            void destroy(){
                if(ibo) glDeleteBuffers(1,&ibo);
                if(vbo) glDeleteBuffers(1,&vbo);
                if(vao) glDeleteVertexArrays(1,&vao);
                vao=vbo=ibo=0; indexCount=0;
            }
        };

        TerrainGL mesh;
        bool dirty = true;

    };
