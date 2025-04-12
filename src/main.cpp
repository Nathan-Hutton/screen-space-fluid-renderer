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
EnvironmentMap environmentMap;
Model plane;

cy::GLRenderDepth2D depthBuf;     // depth buffer texutre
cy::GLSLProgram    depthProg;    // program to render the depth buffer

void renderScene();
void update();

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
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
	glDepthFunc(GL_LEQUAL);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    printf("screen res: %d / %d\n", screenWidth, screenHeight);

    cam = Camera(screenWidth, screenHeight);
    cam.SetPos(0.0f, 0.42f, -1.68f);

    sim = Particles();
    sim.LoadModel();

    ch.LoadSim("SphereDropGround");
    ch.LoadNextFrame(&sim);
    viewProjectionTransform = cam.GetProj() * cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0));
    viewProjectionInverse = (cam.GetProj() * cy::Matrix4f(cy::Matrix3f(cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0))))).GetInverse();

    environmentMap.init();

    depthBuf.Initialize(
        false,
        screenWidth,
        screenHeight
    );

    depthProg.BuildFiles("../shaders/depth.vert", "../shaders/depth.frag");

    // plane = Model();
    std::vector<float> planeVerts = {
        -1.0f, -1.0f, 0.999f, // Bottom left
        1.0f, -1.0f, 0.999f,  // Bottom right
        -1.0f, 1.0f, 0.999f,  // Top left
        1.0f, 1.0f, 0.999f,   // Top right
    };
    plane.SetVerts(planeVerts);
    std::vector<float> planeNorms = {
        0.0f,   0.0f,   1.0f,
        0.0f,   0.0f,   1.0f,
        0.0f,   0.0f,   1.0f,
        0.0f,   0.0f,   1.0f,
    };
    plane.SetNorms(planeNorms);
    std::vector<float> planeTexts = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    plane.SetTexts(planeTexts);
    plane.CompileShaders("../shaders/plane.vert", "../shaders/plane.frag");
    plane.Initialize();

    printf("setup complete! (ish)\n");

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

    environmentMap.render(viewProjectionInverse);

    // render the depth buffer
    depthBuf.Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    
    depthProg.Bind();
    sim.Render(viewProjectionTransform, depthProg);
    depthBuf.Unbind();

    // create a smoothed depth buffer
    // let try using a plane?

    // render the final texture
    float scale = 2 * tan(cam.GetFov() / 2);
    int imWidth = cam.GetImgWidth();
    int imHeight = cam.GetImgHeight();

    depthBuf.BindTexture(0);

    sim.Render(viewProjectionTransform, imWidth, imHeight, scale);
    glutSwapBuffers();
}

