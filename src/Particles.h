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
    // Model*              m_screenPlane;  // plane for viewing the depth buffer
    // cy::GLRenderDepth2D* m_depthBuf;     // depth buffer texutre
    // cy::GLSLProgram*    m_depthProg;    // program to render the depth buffer


public:
    Particles()
    {
        m_count = 0;
        m_radius = 1.0f;
        m_positions = {};

        // m_depthBuf.Initialize(
        //     false,
        //     screenWidth,
        //     screenHeight
        // );
        // m_depthBuf = new cy::GLRenderDepth2D();
        // GLenum err;
        // while((err = glGetError()) != GL_NO_ERROR) {}   // clears any previous errors
        // m_depthBuf->Initialize(false, 1, screenWidth, screenHeight);
        // while((err = glGetError()) != GL_NO_ERROR) {
        //     printf("OpenGL error: %d\n", err);
        // }
        // // m_depthBuf.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
    
        // m_depthProg = new cy::GLSLProgram();
        // m_depthProg->BuildFiles("../shaders/depth.vert", "../shaders/depth.frag");
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

    void Render(const cy::Matrix4f& viewProjectionTransform) const
    {        
        // render the depth buffer
        // printf("framebuffer IsComplete?: %d\n", (int)m_depthBuf->IsComplete());
        // m_depthBuf->Bind();
        // GLenum status = glCheckFramebufferStatus(m_depthBuf->GetID());
        // printf("frame buffer status: %d\n", status);
        // glClear(GL_DEPTH_BUFFER_BIT);

        // m_depthProg->Bind();

        // for (size_t i = 0; i < m_count; i++) {
        //     const cy::Vec3f translation = cy::Vec3f(m_positions[i*3], m_positions[i*3+1], m_positions[i*3+2]);
        //     const cy::Matrix4f mvp = viewProjectionTransform * cy::Matrix4f().Translation(translation) * cy::Matrix4f().Scale(m_radius);

        //     m_depthProg->RegisterUniform(0, "mvp");
        //     m_depthProg->SetUniformMatrix4(0, &mvp[0]);

        //     m_sphere->Bind();

        //     glDrawElements(GL_TRIANGLES, m_sphere->GetLength(), GL_UNSIGNED_INT, 0);

        //     m_sphere->Unbind();
        // }

        // m_depthBuf->Unbind();

        // glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // // render a plane instead of the particles
        // cy::GLSLProgram* program = m_screenPlane->GetProgram();
        // program->Bind();
        // program->SetUniform("depthTex", 0);
        // m_depthBuf->BindTexture(0);

        // m_screenPlane->Bind();

        // glDrawElements(GL_TRIANGLE_STRIP, m_screenPlane->GetLength(), GL_UNSIGNED_INT, 0);

        // m_screenPlane->Unbind();
        
        cy::GLSLProgram* program = m_sphere->GetProgram();
        program->Bind();
        // m_depthBuf.BindTexture(0);

        for (size_t i = 0; i < m_count; i++) {
            const cy::Vec3f translation = cy::Vec3f(m_positions[i*3], m_positions[i*3+1], m_positions[i*3+2]);
            const cy::Matrix4f mvp = viewProjectionTransform * cy::Matrix4f().Translation(translation) * cy::Matrix4f().Scale(m_radius);

            program->RegisterUniform(0, "mvp");
            program->SetUniformMatrix4(0, &mvp[0]);
            // program->SetUniform("depthTex", 0);

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

        // m_screenPlane = new Model();
        // std::vector<float> planeVerts = {
        //     -1.0f, -1.0f, 0.999f, // Bottom left
        //     1.0f, -1.0f, 0.999f,  // Bottom right
        //     -1.0f, 1.0f, 0.999f,  // Top left
        //     1.0f, 1.0f, 0.999f,   // Top right
        // };
        // m_screenPlane->SetVerts(planeVerts);
        // std::vector<float> planeNorms = {
        //     0.0f,   0.0f,   1.0f,
        //     0.0f,   0.0f,   1.0f,
        //     0.0f,   0.0f,   1.0f,
        //     0.0f,   0.0f,   1.0f,
        // };
        // m_screenPlane->SetNorms(planeNorms);
        // std::vector<float> planeTexts = {
        //     0.0f, 0.0f,
        //     1.0f, 0.0f,
        //     0.0f, 1.0f,
        //     1.0f, 1.0f
        // };
        // m_screenPlane->SetTexts(planeTexts);
        // m_screenPlane->CompileShaders("../shaders/plane.vert", "../shaders/plane.frag");
        // m_screenPlane->Initialize();
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
