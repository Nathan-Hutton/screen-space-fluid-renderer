#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Input.h"
#include "Camera.h"
#include "Particles.h"
#include "CacheHandler.h"
#include "EnvironmentMap.h"
// #include "Plane.h"

#define _USE_MATH_DEFINES

Camera cam;
Particles sim = Particles();
CacheHandler ch = CacheHandler();
cy::Matrix4f viewProjectionTransform;
cy::Matrix4f viewProjectionInverse;
EnvironmentMap environmentMap;
Model plane;

cy::GLRenderDepth2D depthBuf;     // depth buffer texutre
cy::GLSLProgram    depthProg;    // program to render the depth buffer

cy::GLRenderDepth2D smoothBufs[2];
cy::GLRenderDepth2D smoothBuf;    // buffer for smoothed depth map
cy::GLSLProgram smoothProg;        // program to smooth the depth buffer


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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    // camera initialization
    cam = Camera(screenWidth, screenHeight);
    cam.SetPos(0.0f, 0.42f, -1.68f);

    // particle sim initialization
    sim = Particles();
    sim.LoadModel();

    // cache handler initializaiton
    ch.LoadSim("SphereDropGround");
    ch.LoadNextFrame(&sim);

    // render parameters initializaiton
    viewProjectionTransform = cam.GetProj() * cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0));
    viewProjectionInverse = (cam.GetProj() * cy::Matrix4f(cy::Matrix3f(cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0))))).GetInverse();

    // invironment map initializaiton
    environmentMap.init();

    // depth buffer initialization
    depthBuf.Initialize(
        false,
        screenWidth,
        screenHeight
    );
    depthProg.BuildFiles("../shaders/depth.vert", "../shaders/depth.frag");

    // smooth buffer initialization
    smoothBuf.Initialize(
        false,
        screenWidth,
        screenHeight
    );
    for (size_t i{ 0 }; i < 2; ++i)
    {
        smoothBufs[i].Initialize(
            false,
            screenWidth,
            screenHeight
        );
    }
    smoothProg.BuildFiles("../shaders/plane.vert", "../shaders/smooth.frag");

    // rendering plane initialization
    std::vector<float> planeVerts = {
        -1.0f, -1.0f, 0.0f, // Bottom left
        1.0f, -1.0f, 0.0f,  // Bottom right
        -1.0f, 1.0f, 0.0f,  // Top left
        1.0f, 1.0f, 0.0f,   // Top right
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
    bool horizontal{ true };
    smoothBufs[0].Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    smoothProg.Bind();
    smoothProg.SetUniform("depthTex", 0);
    smoothProg.SetUniform("horizontal", !horizontal);
    smoothProg.SetUniform("near", cam.getNearClip());
    smoothProg.SetUniform("far", cam.getFarClip());
    smoothProg.SetUniform("verticalResolution", glutGet(GLUT_WINDOW_HEIGHT));
    smoothProg.SetUniform("verticalFOV", cam.GetFov());
    depthBuf.BindTexture(0);

    plane.Bind();
    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    smoothBufs[0].Unbind();

    const int amount{ 5 };
    for (size_t i{ 0 }; i < amount; ++i)
    {
        smoothBufs[!horizontal].BindTexture(horizontal);
        smoothBufs[horizontal].Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        smoothProg.SetUniform("depthTex", horizontal);
        smoothProg.SetUniform("horizontal", horizontal);

        glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
        smoothBufs[horizontal].Unbind();
        horizontal = !horizontal;
    }

    // render the final texture
    const float scale = 2.0f * tan(cam.GetFov() / 2.0f);
    const int imWidth = cam.GetImgWidth();
    const int imHeight = cam.GetImgHeight();

    cy::GLSLProgram* program = plane.GetProgram();
    program->Bind();

    program->SetUniform("depthTex", 2);
    program->SetUniform("imgW", imWidth);
    program->SetUniform("imgH", imHeight);
    program->SetUniform("scale", scale);
    smoothBufs[!horizontal].BindTexture(2);

    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();

    glutSwapBuffers();
}
