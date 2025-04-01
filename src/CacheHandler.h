#pragma once

#include <stdlib.h>
#include <linux/limits.h>
#include <vector>
#include "Particles.h"
#include "utils.h"

class CacheHandler
{
private:
    unsigned int m_startFrame;
    unsigned int m_endFrame;
    unsigned int m_currentFrame;
    char m_simFolder[PATH_MAX];
    std::vector<unsigned char> m_VReadBuffer;
public:
    cy::Vec3f m_from;
    cy::Vec3f m_at;

    CacheHandler()
    {
        m_startFrame = m_endFrame = m_currentFrame = 0;
        m_simFolder[0] = '\0';
        strcat(m_simFolder, "../data/");
        m_from = cy::Vec3f();
        m_at = cy::Vec3f();
    }

    void LoadFile(const char* filepath, Particles* sim)    // read in data from a fluid frame file
    {
        // get absolute path to the file
        char absCache[PATH_MAX];
        if (realpath(filepath, absCache) == nullptr)
            printf("realpath failed in CacheHandler::LoadFile(): No such file or directory: %s\n", filepath);

        // open the file
        std::ifstream file(absCache, std::ios::binary | std::ios::ate);
        if(!file.is_open()) {
            // return false;
            fprintf(stderr, "Error: Could not open cache file\n");
            fprintf(stderr, "%s", absCache);
            printf("\n\n");
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
            m_startFrame = 1;
            m_endFrame = 160;
            m_currentFrame = 0;

            m_from = cy::Vec3f(0.93f, 0.47f, 1.51f);
            m_at = cy::Vec3f(0.42f, 0.07f, 0.54f);
        }
        else if (strcmp(simName, "Armadillo")) {
            m_startFrame = 139;
            m_endFrame = 240;
            m_currentFrame = 138;

            m_from = cy::Vec3f(0.0f, 0.42f, -1.68f);
            m_at = cy::Vec3f(0.0f, 0.38f, 0.0f);
        }
        else if (strcmp(simName, "DamBreakLucy")) {
            m_startFrame = 1;
            m_endFrame = 360;
            m_currentFrame = 0;

            m_from = cy::Vec3f(-3.86f, 2.6f, 5.7f);
            m_at = cy::Vec3f(0.22f, 2.08f, 0.12f);
        }
        strcat(m_simFolder, simName);
    }

    bool LoadNextFrame(Particles* sim)
    {
        m_currentFrame++;
        if (m_currentFrame > m_endFrame || m_currentFrame < m_startFrame) {
            return false;
        }

        char padded[5];
        snprintf(padded, sizeof(padded), "%04d", m_currentFrame);

        char currentFramePath[PATH_MAX];
        if (snprintf(currentFramePath, PATH_MAX, "%s/FluidFrame/frame.%s.pos", m_simFolder, padded) >= PATH_MAX) {
            fprintf(stderr, "Warning: Path was truncated!\n");
            return false;
        }

        LoadFile(currentFramePath, sim);

        return true;
    }

};
