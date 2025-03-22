#pragma once

#include <array>
#include <GL/glew.h>
#include <GL/freeglut.h>

class Quad
{
    public:
        Quad(){}

        void init()
        {
            const std::array<GLfloat, 20> quadVertices
            {
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom left
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f, // Bottom right
                -1.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top left
                1.0f, 1.0f, 0.0f,   1.0f, 1.0f  // Top right
            };

            GLuint quadVBO;
            glGenVertexArrays(1, &m_VAO);

            glBindVertexArray(m_VAO);

            glGenBuffers(1, &quadVBO);

            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * quadVertices.size(), quadVertices.data(), GL_STATIC_DRAW);

            // Set vertex attributes
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0); // Vertex positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(sizeof(GL_FLOAT) * 3)); // Texture coordinates
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
        
        void draw() const
        {
            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }

    private:
        GLuint m_VAO;
};
