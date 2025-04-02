#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Input.h"
#include "Camera.h"
#include "Particles.h"
#include "CacheHandler.h"
#include "EnvironmentMap.h"

Camera cam;
Particles sim = Particles();
CacheHandler ch = CacheHandler();
cy::Matrix4f viewProjectionTransform;
cy::Matrix4f viewProjectionInverse;

void renderScene();
void update();

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    const int screenWidth{ glutGet(GLUT_SCREEN_WIDTH) };
    const int screenHeight{ glutGet(GLUT_SCREEN_HEIGHT) };
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Fluid rendering");
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


    cam = Camera(screenWidth, screenHeight);
    cam.SetPos(0.0f, 0.42f, -1.68f);
    sim.LoadModel();

    ch.LoadSim("SphereDropGround");
    ch.LoadNextFrame(&sim);
    viewProjectionTransform = cam.GetProj() * cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0));
    viewProjectionInverse = (cam.GetProj() * cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0))).GetInverse();

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
    ch.LoadNextFrame(&sim);
    glutPostRedisplay();
}


void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    sim.Render(viewProjectionTransform);
    glutSwapBuffers();
}

