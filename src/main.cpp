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
Model floorPlane;

cy::GLRenderDepth2D depthBuf;     // depth buffer texutre
cy::GLSLProgram    depthProg;    // program to render the depth buffer

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

    floorPlane.LoadOBJFile("../data/Floor/Floor.obj");
    floorPlane.DifTexSetup();
    floorPlane.CompileShaders("../shaders/floorPlane.vert", "../shaders/floorPlane.frag");
    floorPlane.Initialize();

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

    // render the floor plane here??
    glClear(GL_DEPTH_BUFFER_BIT);
    cy::GLSLProgram* floorProg = floorPlane.GetProgram();
    floorProg->Bind();
    floorProg->SetUniformMatrix4("mvp", &viewProjectionTransform[0]);
    floorProg->SetUniform("difTex", 0);
    floorPlane.GetDif().Bind(0);

    floorPlane.Bind();
    glDrawElements(GL_TRIANGLES, floorPlane.GetLength(), GL_UNSIGNED_INT, 0);
    floorPlane.Unbind();

    // render the depth buffer
    depthBuf.Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    
    depthProg.Bind();
    sim.Render(viewProjectionTransform, depthProg);
    depthBuf.Unbind();

    // create a smoothed depth buffer
    smoothBuf.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float myPi = M_PI;
    float myE = M_E;

    smoothProg.Bind();
    smoothProg.SetUniform("depthTex", 0);
    smoothProg.SetUniform("e", myPi);
    smoothProg.SetUniform("pi", myE);
    depthBuf.BindTexture(0);

    plane.Bind();
    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();
    smoothBuf.Unbind();

    // render the final texture
    float scale = 2.0f * tan(cam.GetFov() / 2.0f);
    int imWidth = cam.GetImgWidth();
    int imHeight = cam.GetImgHeight();
    // cy::Vec4f lightView = viewProjectionTransform * cy::Vec4f(-3.0, 8.0, 7.0, 1.0);
    cy::Matrix4f invProj = cam.GetProj().GetInverse();
    cy::Matrix4f invView = cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0)).GetInverse();

    // printf("%f, %f, %f, %f\n", invView[0], invView[4], invView[8], invView[12]);
    // printf("%f, %f, %f, %f\n", invView[1], invView[5], invView[9], invView[13]);
    // printf("%f, %f, %f, %f\n", invView[2], invView[6], invView[10], invView[14]);
    // printf("%f, %f, %f, %f\n", invView[3], invView[7], invView[11], invView[15]);

    cy::GLSLProgram* program = plane.GetProgram();
    program->Bind();

    // program->RegisterUniform(0, "invProjectionMatrix");
    // program->RegisterUniform(1, "invViewMatrix");
    program->SetUniformMatrix4("invProjectionMatrix", &invProj[0]);
    program->SetUniformMatrix4("invViewMatrix", &invView[0]);
    program->SetUniform("depthTex", 1);
    program->SetUniform("env", 2);
    program->SetUniform("imgW", imWidth);
    program->SetUniform("imgH", imHeight);
    // program->SetUniform("scale", scale);
    // program->RegisterUniform(0, "lightView");
    // program->SetUniform4("lightView", &lightView[0]);
    
    smoothBuf.BindTexture(1);

    //bind environment map
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap.GetTextureID());

    plane.Bind();
    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();

    glutSwapBuffers();
}