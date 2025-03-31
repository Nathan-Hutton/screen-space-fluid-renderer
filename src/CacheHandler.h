#pragma once

#include <stdlib.h>
#include <linux/limits.h>
#include <vector>
#include "Particles.h"
#include "utils.h"

class CacheHandler
{
private:
    unsigned int startFrame;
    unsigned int endFrame;
    unsigned int currentFrame;
    char simFolder[PATH_MAX];
    std::vector<unsigned char> m_VReadBuffer;
public:
    cy::Vec3f from;
    cy::Vec3f at;

    CacheHandler()
    {
        startFrame = endFrame = currentFrame = 0;
        simFolder[0] = '\0';
        strcat(simFolder, "../data/");
        from = cy::Vec3f();
        at = cy::Vec3f();
    }

    void LoadFile(const char* filepath, Particles* sim)    // read in data from a fluid frame file
    {
        // get absolute path to the file
        char absCache[PATH_MAX];
        if (realpath(filepath, absCache) == nullptr)
            perror("realpath failed");

        // open the file
        std::ifstream file(absCache, std::ios::binary | std::ios::ate);
        if(!file.is_open()) {
            // return false;
            fprintf(stderr, "Error: Could not open cache file\n");
            fprintf(stderr, "%s", absCache);
            return;
        }

        size_t fileSize = (size_t)file.tellg();
        m_VReadBuffer.resize(fileSize);
        file.seekg(0, std::ios::beg);
        file.read((char*)m_VReadBuffer.data(), fileSize);
        file.close();

        unsigned int numParticles;
        float particleRadius;

        memcpy(&numParticles,   m_VReadBuffer.data(),                        sizeof(unsigned int));
        memcpy(&particleRadius, &m_VReadBuffer.data()[sizeof(unsigned int)], sizeof(float));

        // std::cout << numParticles << std::endl;
        // std::cout << particleRadius << std::endl;

        if(sim->GetCount() != numParticles) {
            sim->AllocatePositions(numParticles * 3);
        }

        memcpy(sim->GetPosBuffer()->data(), &m_VReadBuffer.data()[sizeof(unsigned int) + sizeof(float)], 3 * numParticles * sizeof(float));

        sim->SetFrameData(numParticles, particleRadius);

        // fprintf(stdout, "Data copied!!\n");
    }

    void LoadSim(const char* simName)
    {
        if (std::strcmp(simName, "SphereDropGround") == 0) {
            startFrame = 1;
            endFrame = 160;
            currentFrame = 0;

            from = cy::Vec3f(0.93f, 0.47f, 1.51f);
            at = cy::Vec3f(0.42f, 0.07f, 0.54f);
        }
        else if (strcmp(simName, "Armadillo")) {
            startFrame = 139;
            endFrame = 240;
            currentFrame = 138;

            from = cy::Vec3f(0.0f, 0.42f, -1.68f);
            at = cy::Vec3f(0.0f, 0.38f, 0.0f);
        }
        else if (strcmp(simName, "DamBreakLucy")) {
            startFrame = 1;
            endFrame = 360;
            currentFrame = 0;

            from = cy::Vec3f(-3.86f, 2.6f, 5.7f);
            at = cy::Vec3f(0.22f, 2.08f, 0.12f);
        }
        strcat(simFolder, simName);
    }

    bool LoadNextFrame(Particles* sim)
    {
        currentFrame++;
        if (currentFrame > endFrame || currentFrame < startFrame) {
            return false;
        }

        char currentFramePath[PATH_MAX];
        memccpy(currentFramePath, simFolder, 0, (size_t)PATH_MAX);
        //char* padded = intToPadZeroes(currentFrame);

        strcat(currentFramePath, "/FluidFrame/frame.");
        strcat(currentFramePath, intToPadZeroes(currentFrame));
        strcat(currentFramePath, ".pos");

        // strcat(currentFramePath, "/FluidFrame/frame.0001.pos");

        LoadFile(currentFramePath, sim);

        return true;
    }
};
