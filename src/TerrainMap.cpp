#include "TerrainMap.h"


TerrainMap::TerrainMap(int chunksX, int chunksZ, int chunkSize, float cellSize)
    : chunksX(chunksX), chunksZ(chunksZ), chunkSize(chunkSize), cellSize(cellSize)
{
    chunks.reserve(chunksX * chunksZ);

    for (int cz = 0; cz < chunksZ; ++cz) {
        for (int cx = 0; cx < chunksX; ++cx) {
            float originX = cx * (chunkSize - 1) * cellSize;
            float originZ = cz * (chunkSize - 1) * cellSize;


            chunks.push_back(std::make_unique<TerrainChunk>(chunkSize, cellSize));
            TerrainChunk* chunk = chunks.back().get();
            chunk->position = {originX, 0.0f, originZ};
            chunk->gridX = cx;
            chunk->gridZ = cz;
         
        }
    }
}

void TerrainMap::build() {
    for (auto& chunk : chunks) {
        chunk->buildMesh();
    }
}

void TerrainMap::applyBrush(const Brush& b, const glm::vec3& hit, bool lower) {
    // Determine brush bounds in world coords
    float minX = hit.x - b.radius;
    float maxX = hit.x + b.radius;
    float minZ = hit.z - b.radius;
    float maxZ = hit.z + b.radius;

    for (auto& chunk : chunks) {
        // Compute this chunk’s world bounding box
        float cx0 = chunk->position.x;
        float cz0 = chunk->position.z;
        float cx1 = cx0 + (chunk->hm.size - 1) * cellSize;
        float cz1 = cz0 + (chunk->hm.size - 1) * cellSize;

        // If brush overlaps chunk
        if (maxX >= cx0 && minX <= cx1 &&
            maxZ >= cz0 && minZ <= cz1) {

            // Convert hit point to chunk-local coordinates
            glm::vec3 localHit = hit - chunk->position;
            chunk->applyBrush(b, localHit, lower);
        }
    }
}

void TerrainMap::render(bool wire) {
    for (auto& chunk : chunks) {
        chunk->Render(wire);
    }
}

std::vector<std::unique_ptr<TerrainChunk>>& TerrainMap::GetChunks()
{
    return chunks;
}

TerrainChunk* TerrainMap::getChunkAt(const glm::vec3& worldPos) {
    for (auto& chunkPtr : chunks) {
        if (chunkPtr->contains(worldPos.x, worldPos.z)) {
            return chunkPtr.get();
        }
    }
    return nullptr;
}

void TerrainMap::updateDirtyChunks()
{   
    for (auto& chunk : chunks) {
        chunk->updateMeshIfDirty();
    }
}


float TerrainMap::getHeightGlobal(float x, float z) {
    for (auto& chunkPtr : chunks) {
        if (chunkPtr->contains(x, z)) {
            glm::vec3 local = glm::vec3(x, 0, z) - chunkPtr->position;
            return chunkPtr->getHeightAt(local.x, local.z);
        }
    }
    return -1.0f;
}

glm::vec2 TerrainMap::getGradient(float x, float z, float sampleDist) {
    // Central difference approximation
    float hL = getHeightGlobal(x - sampleDist, z);
    float hR = getHeightGlobal(x + sampleDist, z);
    float hD = getHeightGlobal(x, z - sampleDist);
    float hU = getHeightGlobal(x, z + sampleDist);

    float dx = (hR - hL) / (2.0f * sampleDist);
    float dz = (hU - hD) / (2.0f * sampleDist);

    return glm::vec2(dx, dz);
}

bool TerrainMap::isTooSteep(float x, float z, float maxSlopeDegrees) {
    glm::vec2 grad = getGradient(x, z, cellSize);
    glm::vec3 normal(-grad.x, 1.0f, -grad.y);
    normal = glm::normalize(normal);

    // Compare slope with allowed slope
    float slopeAngle = glm::degrees(acos(glm::dot(normal, glm::vec3(0,1,0))));
    return slopeAngle > maxSlopeDegrees;
}

glm::vec3 TerrainMap::getSlideDirection(float x, float z) {
    glm::vec2 grad = getGradient(x, z, cellSize);
    // downhill direction = negative gradient
    glm::vec3 dir(-grad.x, 0.0f, -grad.y);
    return glm::normalize(dir);
}

glm::vec3 TerrainMap::getNormalGlobal(float x, float z) {
    for (auto& chunkPtr : chunks) {
        if (chunkPtr->contains(x, z)) {
            glm::vec3 local = glm::vec3(x, 0, z) - chunkPtr->position;
            return chunkPtr->hm.normalAtInterpolated(local.x, local.z);
            // if you don’t have interpolated version yet, bilinear interpolate
        }
    }
    return glm::vec3(0,1,0); // default flat
}

glm::vec3 TerrainMap::getDownhillAccelFromNormal(const glm::vec3& normal, float gravityConstant) {
    glm::vec3 gravity = glm::vec3(0.0f, gravityConstant, 0.0f);

    // Project gravity onto terrain tangent plane
    glm::vec3 tangent = gravity - glm::dot(gravity, normal) * normal;

    return tangent; // already scaled by gravityConstant
}

void TerrainMap::save(const std::string& folderPath) {
    namespace fs = std::filesystem;

    // Create the folder if it doesn't exist
    if (!fs::exists(folderPath)) {
        if (!fs::create_directories(folderPath)) {
            std::cerr << "Failed to create folder: " << folderPath << std::endl;
        }
    }

    for (auto& chunk : chunks) {
        // Build a filename for each chunk: e.g., "chunk_0_1.hmp"
        std::ostringstream filename;
        filename << folderPath << "/chunk_" << chunk->gridX << "_" << chunk->gridZ << ".hmap";

        if (!chunk->saveHMap(filename.str())) {
            std::cerr << "Failed to save chunk at (" << chunk->gridX 
                      << ", " << chunk->gridZ << ")" << std::endl;
        }
    }

    std::cout << "TerrainMap saved successfully to " << folderPath << std::endl;
}

void TerrainMap::load(const std::string& folderPath) {
    namespace fs = std::filesystem;

    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cerr << "Folder does not exist: " << folderPath << std::endl;
    }

    // Clear current chunks
    chunks.clear();
    int numErrors = 0;

    // Iterate over all .hmp files
    for (auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.path().extension() != ".hmap") continue;

        // auto chunk = std::make_unique<TerrainChunk>(chunkSize, cellSize);
        // TerrainChunk chunk(chunkSize, cellSize);
        auto chunk = std::make_unique<TerrainChunk>(chunkSize, cellSize);
        if (!chunk->loadHMap(entry.path().string())) {
            std::cerr << "Failed to load chunk: " << entry.path() << std::endl;
            numErrors++;
        }

        chunks.push_back(std::move(chunk));
    }

    if (!chunks.empty()) {
        std::cout << "chunks not empty"<< std::endl;
    
        int maxX = 0, maxZ = 0;
        for (auto& c : chunks) {
            if (c->gridX > maxX) maxX = c->gridX;
            if (c->gridZ > maxZ) maxZ = c->gridZ;
        }
        chunksX = maxX + 1;
        chunksZ = maxZ + 1;
    }

    if(numErrors > 0){
        std::cout << "TerrainMap failed to load " << numErrors << " chunks from: " << folderPath << std::endl;
    }
    else {
        build();
        updateDirtyChunks();
        std::cout << "TerrainMap loaded successfully from " << folderPath << std::endl;
    }
}
float TerrainMap::getCellSize()
{
    return cellSize;
}
// Optionally, update chunksX and chunksZ based on loaded data
// Update chunksX/Z