#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Input.h"
#include "ShaderHandler.h"
#include "Camera.h"
#include "Particles.h"

Camera cam;
Particles sim = Particles();

void resizeWindow(int width, int height);
void renderScene();
void update();

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    const int screenWidth{ glutGet(GLUT_SCREEN_WIDTH) };
    const int screenHeight{ glutGet(GLUT_SCREEN_HEIGHT) };
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Volume rendering");
    glutFullScreen();

    const GLenum err{ glewInit() };
    if (err != GLEW_OK) {
        std::cerr << "GLEW Init Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // compileShaders();
    cam = Camera(screenWidth, screenHeight);
    sim.LoadModel();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glutDisplayFunc(renderScene);
    glutIdleFunc(update);
    glutReshapeFunc(resizeWindow);
    glutKeyboardFunc(processInput);

    glutMainLoop();

    return 0;
}

void update()
{
    // Put something here
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    sim.Render(&cam);

    glutSwapBuffers();
}

void resizeWindow(int width, int height)
{
    glViewport(0, 0, width, height);
}

