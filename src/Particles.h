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
    unsigned int        m_count;        // the number of particles
    float               m_radius;       // the radius of the the particles
    std::vector<float>  m_positions;    // the positions of all the particles
    Model*              m_sphere;       // sphere representing points
    GLuint              m_pointVBO;     // single point
    GLuint              m_pointVAO;

public:
    Particles()
    {
        m_count = 0;
        m_radius = 1.0f;
        m_positions = {};
        m_sphere = nullptr;
        m_pointVBO = 0;
        m_pointVAO = 0;
    }

    void Render(const cy::Matrix4f& viewProjectionTransform, cy::GLSLProgram &program) {
        program.Bind();
        // m_depthBuf.BindTexture(0);

        for (size_t i = 0; i < m_count; i++) {
            const cy::Vec3f translation = cy::Vec3f(m_positions[i*3], m_positions[i*3+1], m_positions[i*3+2]);
            const cy::Matrix4f mvp = viewProjectionTransform * cy::Matrix4f().Translation(translation) * cy::Matrix4f().Scale(m_radius);

            program.RegisterUniform(0, "mvp");
            program.SetUniformMatrix4(0, &mvp[0]);
            // program->SetUniform("depthTex", 0);

            m_sphere->Bind();

            glDrawElements(GL_TRIANGLES, m_sphere->GetLength(), GL_UNSIGNED_INT, 0);
            
            m_sphere->Unbind();
        }
    }

    void Render(const cy::Matrix4f& viewProjectionTransform, int imWidth, int imHeight, float scale) const
    {        
        cy::GLSLProgram* program = m_sphere->GetProgram();
        program->Bind();
        // m_depthBuf.BindTexture(0);

        for (size_t i = 0; i < m_count; i++) {
            const cy::Vec3f translation = cy::Vec3f(m_positions[i*3], m_positions[i*3+1], m_positions[i*3+2]);
            const cy::Matrix4f mvp = viewProjectionTransform * cy::Matrix4f().Translation(translation) * cy::Matrix4f().Scale(m_radius);

            program->RegisterUniform(0, "mvp");
            program->SetUniformMatrix4(0, &mvp[0]);
            program->SetUniform("depthTex", 0);
            program->SetUniform("imgW", imWidth);
            program->SetUniform("imgH", imHeight);
            program->SetUniform("scale", scale);

            m_sphere->Bind();

            glDrawElements(GL_TRIANGLES, m_sphere->GetLength(), GL_UNSIGNED_INT, 0);
            
            m_sphere->Unbind();
        }
    }

    void RenderPoints(cy::GLSLProgram &program, cy::Matrix4f mvp, cy::Matrix4f lvp, cy::GLRenderTexture2D &normalMap, cy::GLRenderTexture2D &posMap)
    {
        // cy::GLSLProgram* program = m_sphere->GetProgram();
        program.Bind();
        // m_depthBuf.BindTexture(0);
        program.SetUniformMatrix4("mvp", &mvp[0]);
        program.SetUniformMatrix4("lvp", &lvp[0]);
        program.SetUniform("normalMap", 0);
        program.SetUniform("posMap", 1);

        normalMap.BindTexture(0);
        posMap.BindTexture(1);

        for (size_t i = 0; i < m_count; i++) {
            const cy::Vec3f position = cy::Vec3f(m_positions[i*3], m_positions[i*3+1], m_positions[i*3+2]);
            const cy::Matrix4f translation = cy::Matrix4f().Translation(position);
            // const cy::Matrix4f mvp = viewProjectionTransform * cy::Matrix4f().Translation(translation) * cy::Matrix4f().Scale(m_radius);

            program.SetUniformMatrix4("translate", &translation[0]);

            glBindVertexArray(m_pointVAO);
            glDrawArrays(GL_POINTS, 0, 1);
            // glBindVertexArray(0);

            // m_sphere->Bind();

            // glDrawElements(GL_TRIANGLES, m_sphere->GetLength(), GL_UNSIGNED_INT, 0);
            
            // m_sphere->Unbind();
        }
    }

    void LoadModel()
    {
        m_sphere = new Model();
        m_sphere->LoadOBJFile("../data/sphere.obj");
        m_sphere->CompileShaders("../shaders/test.vert", "../shaders/test.frag");
        m_sphere->Initialize();

        // create vertex array of a single point lol
        float coords[3] = {0.0, 0.0, 0.0};
        // GLuint pointVBO;
        glGenVertexArrays(1, &m_pointVAO);

        glBindVertexArray(m_pointVAO);

        glGenBuffers(1, &m_pointVBO);

        glBindBuffer(GL_ARRAY_BUFFER, m_pointVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3, coords, GL_STATIC_DRAW);

        // Set vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0); // Vertex positions
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
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

    float GetRadius() { return m_radius; }
};
