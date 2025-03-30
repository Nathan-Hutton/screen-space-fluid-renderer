#pragma once

#include <stdlib.h>
#include <linux/limits.h>
#include <vector>
#include "Particles.h"

class CacheHandler
{
private:
    std::vector<unsigned char> m_VReadBuffer;
public:
    CacheHandler() {}

    void LoadFile(const char* filepath, Particles* sim)    // read in data from a fluid frame file
    {
        // get absolute path to the file
        char absCache[PATH_MAX];
        realpath(filepath, absCache);

        // open the file
        std::ifstream file(absCache, std::ios::binary | std::ios::ate);
        if(!file.is_open()) {
            // return false;
            fprintf(stderr, "Error: Could not open cache file\n");
            fprintf(stderr, absCache);
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

        std::cout << numParticles << std::endl;
        std::cout << particleRadius << std::endl;

        if(sim->GetCount() != numParticles) {
            sim->AllocatePositions(numParticles * 3);
        }

        memcpy(sim->GetPosBuffer()->data(), &m_VReadBuffer.data()[sizeof(unsigned int) + sizeof(float)], 3 * numParticles * sizeof(float));

        sim->SetFrameData(numParticles, particleRadius);

        fprintf(stdout, "Data copied!!\n");
    }
};