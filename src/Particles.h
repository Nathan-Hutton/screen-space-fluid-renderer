#pragma once

// #include <filesystem>
#include <vector>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "Model.h"
#include "Camera.h"

// using path = std::filesystem::path;

/* Stores the current state of the fluid particles at 1 frame */
class Particles
{
private:
    unsigned int        count;      // the number of particles
    float               radius;     // the radius of the the particles
    std::vector<float>  positions;  // the positions of all the particles
    Model*              sphere;     // sphere representing points


public:
    Particles()
    {
        count = 0;
        radius = 1.0f;
        positions = {};
    }

    void Render(Camera* cam)
    {
        if (positions.empty()) {
            cy::Matrix4f mvp = cam->GetProj() * cam->GetView();

            cy::GLSLProgram* program = sphere->GetProgram();

            program->Bind();
            program->RegisterUniform(0, "mvp");
            program->SetUniformMatrix4(0, &mvp[0]);

            sphere->Bind();

            glDrawElements(GL_TRIANGLES, sphere->GetLength(), GL_UNSIGNED_INT, 0);
            
            sphere->Unbind();
        }
    }

    void LoadModel()
    {
        sphere = new Model();
        sphere->LoadOBJFile("../data/sphere.obj");
        sphere->CompileShaders("../shaders/test.vert", "../shaders/test.frag");
        sphere->Initialize();
    }

    void SetFrameData(unsigned int numParticles, float nRadius)
    {
        count = numParticles;
        radius = nRadius;
    }
    unsigned int GetCount() { return count; }
    std::vector<float>* GetPosBuffer() { return &positions; }

    void AllocatePositions(unsigned int numParticles)
    {
        positions.resize(numParticles);
    }
};