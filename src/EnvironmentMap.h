#pragma once

#include "ShaderHandler.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

class EnvironmentMap
{
    public:
    EnvironmentMap() {}
    void init()
    {
        // *******
        // Texture
        // *******
        unsigned int imageWidth, imageHeight;
        std::vector<unsigned char> positiveX;
        lodepng::decode(positiveX, imageWidth, imageHeight, "../assets/cubemap/cubemap_posx.png");
        std::vector<unsigned char> negativeX;
        lodepng::decode(negativeX, imageWidth, imageHeight, "../assets/cubemap/cubemap_negx.png");
        std::vector<unsigned char> positiveY;
        lodepng::decode(positiveY, imageWidth, imageHeight, "../assets/cubemap/cubemap_posy.png");
        std::vector<unsigned char> negativeY;
        lodepng::decode(negativeY, imageWidth, imageHeight, "../assets/cubemap/cubemap_negy.png");
        std::vector<unsigned char> positiveZ;
        lodepng::decode(positiveZ, imageWidth, imageHeight, "../assets/cubemap/cubemap_posz.png");
        std::vector<unsigned char> negativeZ;
        lodepng::decode(negativeZ, imageWidth, imageHeight, "../assets/cubemap/cubemap_negz.png");

        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, positiveX.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, negativeX.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, positiveY.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, negativeY.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, positiveZ.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, negativeZ.data());

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // ***************************
        // Plane to render the texture
        // ***************************
        const std::vector<GLfloat> environmentMapPlaneVertices
        {
            -1.0f, -1.0f, 0.999f, // Bottom left
            1.0f, -1.0f, 0.999f,  // Bottom right
            -1.0f, 1.0f, 0.999f,  // Top left
            1.0f, 1.0f, 0.999f,   // Top right
        };

        GLuint environmentMapPlaneVBO;
        glGenVertexArrays(1, &m_planeVAO);

        glBindVertexArray(m_planeVAO);

        glGenBuffers(1, &environmentMapPlaneVBO);

        glBindBuffer(GL_ARRAY_BUFFER, environmentMapPlaneVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * environmentMapPlaneVertices.size(), environmentMapPlaneVertices.data(), GL_STATIC_DRAW);

        // Set vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0); // Vertex positions
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // ***********
        // Make shader
        // ***********
        compileShader(m_shader, { "../shaders/environment.vert", "../shaders/environment.frag" });

        glUseProgram(m_shader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
        glUniform1i(glGetUniformLocation(m_shader, "env"), 0);
    }

    void render(const cy::Matrix4f& viewProjectionInverse)
    {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glUseProgram(m_shader);

        glUniformMatrix4fv(glGetUniformLocation(m_shader, "invViewProjection"), 1, GL_FALSE,  &viewProjectionInverse[0]);

        glBindVertexArray(m_planeVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }

    private:
        GLuint m_textureID;
        GLuint m_planeVAO;
        GLuint m_shader;
};
