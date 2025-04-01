#pragma once

// #include <filesystem>
#include <vector>
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "Model.h"
#include "Camera.h"

/* Stores the current state of the fluid particles at 1 frame */
class Particles
{
private:
    unsigned int        m_count;      // the number of particles
    float               m_radius;     // the radius of the the particles
    std::vector<float>  m_positions;  // the positions of all the particles
    Model*              m_sphere;     // sphere representing points


public:
    Particles()
    {
        m_count = 0;
        m_radius = 1.0f;
        m_positions = {};
    }

    void Render(Camera* cam) const
    {
        const cy::Vec3f from = cy::Vec3f(-3.96f, 2.52f, 3.56f);
        const cy::Vec3f at = cy::Vec3f(0.0f, 0.46f, -0.1f);

        cy::GLSLProgram* program = m_sphere->GetProgram();
        program->Bind();

        if (m_positions.empty()) {
            const cy::Matrix4f mvp = cam->GetProj() * cam->GetView();

            program->RegisterUniform(0, "mvp");
            program->SetUniformMatrix4(0, &mvp[0]);

            m_sphere->Bind();

            glDrawElements(GL_TRIANGLES, m_sphere->GetLength(), GL_UNSIGNED_INT, 0);
            
            m_sphere->Unbind();

            return;
        }

        for (size_t i = 0; i < m_count; i++) {
            const cy::Vec3f translation = cy::Vec3f(m_positions[i*3], m_positions[i*3+1], m_positions[i*3+2]);
            const cy::Matrix4f mvp = cam->GetProj() * cam->GetView(from, at) * cy::Matrix4f().Translation(translation) * cy::Matrix4f().Scale(m_radius);

            program->RegisterUniform(0, "mvp");
            program->SetUniformMatrix4(0, &mvp[0]);

            m_sphere->Bind();

            glDrawElements(GL_TRIANGLES, m_sphere->GetLength(), GL_UNSIGNED_INT, 0);
            
            m_sphere->Unbind();
        }
    }

    void LoadModel()
    {
        m_sphere = new Model();
        m_sphere->LoadOBJFile("../data/sphere.obj");
        m_sphere->CompileShaders("../shaders/test.vert", "../shaders/test.frag");
        m_sphere->Initialize();
    }

    void SetFrameData(unsigned int numParticles, float nRadius)
    {
        m_count = numParticles;
        m_radius = nRadius;
    }
    unsigned int GetCount() const { return m_count; }
    std::vector<float>* GetPosBuffer() { return &m_positions; }

    void AllocatePositions(unsigned int numParticles)
    {
        m_positions.resize(numParticles);
    }
};
