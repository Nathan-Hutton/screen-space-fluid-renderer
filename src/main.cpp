#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Input.h"
#include "Camera.h"
#include "Particles.h"
#include "CacheHandler.h"
#include "EnvironmentMap.h"

#define _USE_MATH_DEFINES

Camera cam;
Camera lightCam;
Particles sim = Particles();
CacheHandler ch = CacheHandler();
cy::Matrix4f viewProjectionTransform;
cy::Matrix4f viewProjectionInverse;
EnvironmentMap environmentMap;
EnvironmentMap bgMap;   // will contain environment with the floor plane
GLuint bgMapID;
Model plane;
Model floorPlane;

cy::GLRenderDepth2D depthBuf;     // depth buffer texutre
cy::GLSLProgram    depthProg;    // program to render the depth buffer

cy::GLRenderDepth2D smoothBufs[2];
cy::GLRenderDepth2D smoothBuf;    // buffer for smoothed depth map
cy::GLSLProgram smoothProg;        // program to smooth the depth buffer

cy::Matrix4f lvp;
cy::Matrix4f lightProjInv;
cy::Matrix4f lightViewInv;
cy::GLRenderTexture2D posMapBuf;   // buffer for plane positions map
cy::GLSLProgram posMapProg;     // program to render the pos map
cy::GLRenderTexture2D lgtPrjNrmBuf;   // buffer for light projection normal map
cy::GLSLProgram lgtPrjNrmProg;     // program to render the light projection normal map
cy::GLSLProgram causticRenderProg;
cy::GLRenderTexture2D causticMap;
cy::GLRenderTexture2D quickBlur;
cy::GLSLProgram copyProg;

bool run = false;

GLuint renderCubeMap();
void renderScene();
void update();
void processInput(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y);

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    // const int screenWidth{ glutGet(GLUT_SCREEN_WIDTH) };
    // const int screenHeight{ glutGet(GLUT_SCREEN_HEIGHT) };
    const int screenWidth{ 2048 };
    const int screenHeight{ 1536 };
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Fluid rendering");
    // glutFullScreen();

    const GLenum err{ glewInit() };
    if (err != GLEW_OK) {
        std::cerr << "GLEW Init Error: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glPointSize(2.0);

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

    // floor plane setup
    floorPlane.LoadOBJFile("../data/Floor/Floor.obj");
    floorPlane.DifTexSetup();
    floorPlane.CompileShaders("../shaders/floorPlane.vert", "../shaders/floorPlane.frag");
    floorPlane.Initialize();

    // cubemap including ground plane for correct reflections and refractions
    bgMapID = renderCubeMap();
    bgMap.initFromExisting(bgMapID);

    // ----------------- caustics setup --------------------- //
    lightCam = Camera(screenWidth, screenHeight);
    lightCam.SetFarPlane(15.0);
    // light view parameters
    cy::Vec4f lightPos = cy::Vec4f(2.5, 2.0, -1.5, 1.0);
    cy::Matrix4f lightViewMatrix = cy::Matrix4f().View(lightPos.XYZ(), cy::Vec3f(0.5, 0.0, 0.5), cy::Vec3f(0.0, 1.0, 0.0));
    cy::Matrix4f lightProjMatrix = lightCam.GetProj();
    lvp = lightProjMatrix * lightViewMatrix;
    lightProjInv = lightProjMatrix.GetInverse();
    lightViewInv = lightViewMatrix.GetInverse();

    // position map buffer and program
    posMapBuf.Initialize(false);
    posMapBuf.Resize(4, screenWidth, screenHeight, cy::GL::TYPE_FLOAT);
    posMapProg.BuildFiles("../shaders/posMap.vert", "../shaders/posMap.frag");

    // caustic map buffer and program
    lgtPrjNrmBuf.Initialize(false, 3, screenWidth, screenHeight);
    // lgtPrjNrmBuf.SetTextureFilteringMode(GL_NEAREST, GL_NEAREST);
    lgtPrjNrmProg.BuildFiles("../shaders/lgtPrjNormals.vert", "../shaders/lgtPrjNormals.frag");

    causticRenderProg.BuildFiles("../shaders/causticRender.vert", "../shaders/causticRender.frag");
    causticMap.Initialize(false, 3, screenWidth, screenHeight);
    causticMap.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    quickBlur.Initialize(false, 3, screenWidth/4, screenHeight/4);
    quickBlur.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
    copyProg.BuildFiles("../shaders/copy.vert", "../shaders/copy.frag");

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glutDisplayFunc(renderScene);
    glutIdleFunc(update);
    glutReshapeFunc(resizeWindow);
    glutKeyboardFunc(processInput);

    glutMainLoop();

    return 0;
}

void update()
{
    if (run){
        ch.LoadNextFrame(&sim);
        glutPostRedisplay();
    }
}

GLuint renderCubeMap()
{
    unsigned int resolution = environmentMap.GetWidth();

    unsigned int CM_FBO;
    unsigned int textureID;
    glGenFramebuffers(1, &CM_FBO);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, CM_FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    cy::Vec3f position = cy::Vec3f(0.0, 0.5, 0.0);
    cy::Matrix4f project = cy::Matrix4f().Perspective(M_PI/2.0f, 1.0, 0.1, 1000);
    std::vector<cy::Matrix4f> views;
    views.push_back(cy::Matrix4f().View(position, position + cy::Vec3f(1.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, -1.0f, 0.0f)));
    views.push_back(cy::Matrix4f().View(position, position + cy::Vec3f(-1.0f, 0.0f, 0.0f), cy::Vec3f(0.0f, -1.0f, 0.0f)));
    views.push_back(cy::Matrix4f().View(position, position + cy::Vec3f(0.0f, 1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, 1.0f)));
    views.push_back(cy::Matrix4f().View(position, position + cy::Vec3f(0.0f, -1.0f, 0.0f), cy::Vec3f(0.0f, 0.0f, -1.0f)));
    views.push_back(cy::Matrix4f().View(position, position + cy::Vec3f(0.0f, 0.0f, 1.0f), cy::Vec3f(0.0f, -1.0f, 0.0f)));
    views.push_back(cy::Matrix4f().View(position, position + cy::Vec3f(0.0f, 0.0f, -1.0f), cy::Vec3f(0.0f, -1.0f, 0.0f)));

    // Render scene to cubemap
    // --------------------------------
    glViewport(0, 0, (int)resolution, (int)resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, CM_FBO);

    for (size_t i = 0; i < 6; i++)
    {
        cy::Matrix4f view = views[i];
        cy::Matrix4f vp = project * view;
        cy::Matrix4f vpInv = vp.GetInverse();

        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, textureID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render environment and plane
        environmentMap.render(vpInv);
        glClear(GL_DEPTH_BUFFER_BIT);

        cy::GLSLProgram* floorProg = floorPlane.GetProgram();
        floorProg->Bind();
        floorProg->SetUniformMatrix4("mvp", &vp[0]);
        floorProg->SetUniform("difTex", 0);
        floorPlane.GetDif().Bind(0);

        floorPlane.Bind();
        glDrawElements(GL_TRIANGLES, floorPlane.GetLength(), GL_UNSIGNED_INT, 0);
        floorPlane.Unbind();
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return textureID;
}

void renderScene()
{
    const int imWidth = cam.GetImgWidth();
    const int imHeight = cam.GetImgHeight();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ----------------------------- caustics --------------------------- //
    // floor plane positions map for caustics
    posMapBuf.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    posMapProg.Bind();
    posMapProg.SetUniformMatrix4("lvp", &lvp[0]);

    floorPlane.Bind();
    glDrawElements(GL_TRIANGLES, floorPlane.GetLength(), GL_UNSIGNED_INT, 0);
    floorPlane.Unbind();
    posMapBuf.Unbind();

    // create caustics texture, start with light depth buffer
    depthBuf.Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    
    depthProg.Bind();
    sim.Render(lvp, depthProg);
    depthBuf.Unbind();

    // lets not even smooth this bad boy and go straight to getting the normals
    // we can smooth later if we have to
    lgtPrjNrmBuf.Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    lgtPrjNrmProg.Bind();
    lgtPrjNrmProg.SetUniformMatrix4("invProjectionMatrix", &lightProjInv[0]);
    lgtPrjNrmProg.SetUniformMatrix4("invViewMatrix", &lightViewInv[0]);
    lgtPrjNrmProg.SetUniformMatrix4("lvp", &lvp[0]);
    lgtPrjNrmProg.SetUniform("depthTex", 0);
    lgtPrjNrmProg.SetUniform("posMap", 1);
    lgtPrjNrmProg.SetUniform("imgW", imWidth);
    lgtPrjNrmProg.SetUniform("imgH", imHeight);
    depthBuf.BindTexture(0);
    posMapBuf.BindTexture(1);

    plane.Bind();
    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();
    lgtPrjNrmBuf.Unbind();

    // let's draw some caustics baby
    causticMap.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    causticRenderProg.Bind();
    sim.RenderPoints(causticRenderProg, viewProjectionTransform, lvp, lgtPrjNrmBuf, posMapBuf);
    causticMap.Unbind();
    causticMap.BuildTextureMipmaps();

    // let's try some cheap blurring
    quickBlur.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    copyProg.Bind();
    copyProg.SetUniform("tex", 0);
    causticMap.BindTexture(0);

    plane.Bind();
    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();
    quickBlur.Unbind();
    quickBlur.BuildTextureMipmaps();

    causticMap.Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    quickBlur.BindTexture(0);
    plane.Bind();
    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();
    causticMap.Unbind();
    causticMap.BuildTextureMipmaps();

    // ------------------------------- finished caustics -------------------------- //
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the floor plane here
    cy::GLSLProgram* floorProg = floorPlane.GetProgram();
    floorProg->Bind();
    floorProg->SetUniformMatrix4("mvp", &viewProjectionTransform[0]);
    floorProg->SetUniform("difTex", 0);
    floorProg->SetUniform("causticMap", 1);
    floorProg->SetUniformMatrix4("lvp", &lvp[0]);
    floorPlane.GetDif().Bind(0);
    causticMap.BindTexture(1);

    floorPlane.Bind();
    glDrawElements(GL_TRIANGLES, floorPlane.GetLength(), GL_UNSIGNED_INT, 0);
    floorPlane.Unbind();

    /*// render the depth buffer
    depthBuf.Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    
    depthProg.Bind();
    sim.Render(viewProjectionTransform, depthProg);
    depthBuf.Unbind();

    // create a smoothed depth buffer
    bool horizontal{ true };
    float vertFov = cam.GetFov() * imHeight / imWidth;
    smoothBufs[0].Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    smoothProg.Bind();
    smoothProg.SetUniform("depthTex", 0);
    smoothProg.SetUniform("horizontal", !horizontal);
    smoothProg.SetUniform("verticalResolution", imHeight);
    smoothProg.SetUniform("verticalFOV", vertFov);
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
    cy::Matrix4f proj = cam.GetProj();
    cy::Matrix4f invProj = cam.GetProj().GetInverse();
    cy::Matrix4f invView = cy::Matrix4f().View(ch.m_from, ch.m_at, cy::Vec3f(0, 1, 0)).GetInverse();

    cy::GLSLProgram* program = plane.GetProgram();
    program->Bind();

    program->SetUniformMatrix4("invProjectionMatrix", &invProj[0]);
    program->SetUniformMatrix4("invViewMatrix", &invView[0]);
    program->SetUniformMatrix4("projMatrix", &proj[0]);
    program->SetUniform("depthTex", 0);
    program->SetUniform("env", 1);
    program->SetUniform("imgW", imWidth);
    program->SetUniform("imgH", imHeight);
    
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, bgMap.GetTextureID());
    smoothBufs[!horizontal].BindTexture(0);

    glDrawElements(GL_TRIANGLE_STRIP, plane.GetLength(), GL_UNSIGNED_INT, 0);
    plane.Unbind();*/

    // environmentMap.render(viewProjectionInverse);

    glutSwapBuffers();
}

void processInput(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y)
{
    switch (key) {
        case 27:
            glutLeaveMainLoop();
            break;
        case ' ':
            run = !run;
            break;
        case 's':
            ch.LoadNextFrame(&sim);
            break;
    }
    glutPostRedisplay();
}
