#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>

void processInput(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y)
{
    if (key == 27)
        glutLeaveMainLoop();
}

void resizeWindow(int width, int height)
{
    glViewport(0, 0, width, height);
}
