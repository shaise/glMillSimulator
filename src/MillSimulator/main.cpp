
//
// main.cpp
//
// simulating cnc mill operation using Sequenced Convex Subtraction (SCS) by Nigel Stewart et al.
//


#include "GlUtils.h"
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "MillSimulation.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define gEyeStep (PI / 36)

MillSim::MillSimulation gMillSimulator;

GLFWwindow* glwind;

EndMillFlat endMillFlat01(1, 3.175f, 16);
EndMillFlat endMillFlat02(2, 1.5f, 16);
EndMillBall endMillBall03(4, 1, 16, 4, 0.2f);
EndMillTaper endMillTaper04(3, 1, 16, 90, 0.2f);

// test section - remove!
GLuint tprogram, tmodel, tview, tprojection, tarray;

int gLastX = 0, gLastY = 0;
bool gIsDragging;


void ShowStats() {
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    glLoadIdentity();
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-1.0f, -1.0f);
    glDisable(GL_LIGHTING);
    glEnable(GL_LIGHTING);
    //glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

// add stuff here to show over simulation
void display()
{
    //ShowStats();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (action != GLFW_PRESS)
        return;
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_UP:
        gMillSimulator.TiltEye(gEyeStep);
        break;
    case GLFW_KEY_DOWN:
        gMillSimulator.TiltEye(- gEyeStep);
        break;
    default:
        gMillSimulator.HandleKeyPress(key);
        break;
    }
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    std::cout << "Button:" << button << ", state:" << action << ", pos:" << x << "," << y << std::endl;
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        gIsDragging = action == GLFW_PRESS;
        if (gIsDragging)
        {
            gLastX = (int)x;
            gLastY = (int)y;
        }
    }
}


void check_drag(GLFWwindow* window)
{
    double fx, fy;
    int x, y, dx, dy;
    if (gIsDragging)
    {
        glfwGetCursorPos(window, &fx, &fy);
        int x = (int)fx;
        int y = (int)fy;
        int dx = x - gLastX;
        int dy = y - gLastY;
        if (dx != 0 || dy != 0)
        {
            gMillSimulator.DragView(dx, dy);
            gLastX = x;
            gLastY = y;
        }
    }
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


int main(int argc, char **argv)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);


    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glwind = glfwCreateWindow(800, 600, "OpenGL Triangle", NULL, NULL);
    if (!glwind)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(glwind, key_callback);
    glfwSetMouseButtonCallback(glwind, mouse_callback);

    glfwMakeContextCurrent(glwind);
    glewInit();
    glfwSwapInterval(1);

    std::cout << glGetString(GL_VERSION) << std::endl;
    gMillSimulator.LoadGCodeFile("cam_test1.txt");
    gMillSimulator.AddTool(&endMillFlat01);
    gMillSimulator.AddTool(&endMillFlat02);
    gMillSimulator.AddTool(&endMillBall03);
    gMillSimulator.AddTool(&endMillTaper04);
    gMillSimulator.InitSimulation();
    gMillSimulator.SetBoxStock(0, 0, -8.7f, 50, 50, 8.7f);
    gMillSimulator.InitDisplay();
    
    while (!glfwWindowShouldClose(glwind))
    {
        gMillSimulator.ProcessSim((unsigned int)(glfwGetTime() * 1000));
        display();
        glfwSwapBuffers(glwind);
        glfwPollEvents();
        check_drag(glwind);
    }

    glfwDestroyWindow(glwind);

    glfwTerminate();
    exit(EXIT_SUCCESS);


    return 0;
}

