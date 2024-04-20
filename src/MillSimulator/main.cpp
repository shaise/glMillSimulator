
//
// main.cpp
//
// simulating cnc mill operation using Sequenced Convex Subtraction (SCS) by Nigel Stewart et al.
//


#include "GlUtils.h"
#include <GLFW/glfw3.h>
#include "linmath.h"
#include "MillPathSegment.h"
#include "StockObject.h"
#include "MillOperation.h"
#include "SimShapes.h"
#include "EndMillFlat.h"
#include "EndMillTaper.h"
#include "EndMillBall.h"
#include "Shader.h"
#include "GCodeParser.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

bool spin = true;
bool simulate = false;
bool singleStep = false;
int debug = 0;
bool isRotate = true;
float rot = 0.0f;
std::ostringstream fpsStream;

MillMotion gZeroPos = { 0, 0,  10 };
MillMotion gCurMotion;
MillMotion gDestMotion;
int gLastToolId = -1;
int gcurstep = 0;
int gnsteps = 0;
int gPathStep = 0;
int gnPathSteps = 0;
bool gIsInStock;
//int gToolId = -1;
float eyeHeight = 30;
#define STOCK_HEIGHT 2.0f
GLFWwindow* glwind;
CSShader shader3D, shaderInv3D, shaderFlat;

std::vector<MillSim::MillPathSegment*> MillPathSegments;
MillSim::GCodeParser gcodeParser;

MillMotion flatMillMotions[] = {
    {-0.7f, -0.7f, 10},
    {-0.7f, -0.7f, 1},
    {0.7f, 0.7f, 1, 0.7f, 0.7f, 0},
    {0.7f, 0.7f, 10},

    {-3, -3, 10},
    {-3, -3, 0.5},
    {3, 3, 0.5, 3, 3, -1},
    {3, 3, 10},

    {15, 15, 10},
    { 15, 15, 1.5f},
    {15, -15, 1.5f},
    {-15, -15, 1.5f},
    {-15, 15, 1.5f},
    {15, 15, 1.5f},

    {15, 15, 1},
    {15, -15, 1},
    {-15, -15, 1},
    {-15, 15, 1},
    {15, 15, 1},

    {15, 15, 0.5f},
    {15, -15, 0.5f},
    {-15, -15, 0.5f},
    {-15, 15, 0.5f},
    {15, 15, 0.5f},

    {15, 15, 0},
    {15, -15, 0},
    {-15, -15, 0},
    {-15, 15, 0},
    {15, 15, 0},

    {15, 15, 10},

    {8, 8, 10},
    {8, 8, 1.5f},
    {8, -8, 1.5f},
    {6.1f, -8, 1.5f},
    {6.1f, 8, 1.5f},
    {4.2f, 8, 1.5f},
    {4.2f, -8, 1.5f},
    {2.3f, -8, 1.5f},
    {2.3f, 8, 1.5f},
    {0.4f, 8, 1.5f},
    {0.4f, -8, 1.5f},
    {-1.5f, -8, 1.5f},
    {-1.5f, 8, 1.5f},
    {-3.4f, 8, 1.5f},
    {-3.4f, -8, 1.5f},
    {-5.3f, -8, 1.5f},
    {-5.3f, 8, 1.5f},
    {-7.2f, 8, 1.5f},
    {-7.2f, -8, 1.5f},
    {-8,  -8, 1.5f},
    {-8,  8, 1.5f},
    { 8,  8, 1.5f},
    { 8,  -8, 1.5f},
    {-8,  -8, 1.5f},

    {-8,  -8, 10},
};

MillMotion taperMillMotions[] = {
    {14.2f, 14.2f, 10},
    {14.2f, 14.2f, 1.5f},
    {14.2f, -14.2f, 1.5f},
    {-14.2f, -14.2f, 1.5f},
    {-14.2f, 14.2f, 1.5f},
    {14.2f, 14.2f, 1.5f},
    {14.2f, 14.2f, 10},
    {0, 0, 10},
};

MillMotion ballMillMotions[] = {
    {12, 12, 10},
    {12, 12, 1.5f},
    {12, -12, 2.5f},
    {-12, -12, 1.5f},
    {-12, 12, 2.5f},
    {12, 12, 1.5f},
    {12, 12, 10},
    {0, 0, 10},
};

#define NUM_FLAT_MOTIONS (sizeof(flatMillMotions) / sizeof(MillMotion))
#define NUM_TAPER_MOTIONS (sizeof(taperMillMotions) / sizeof(MillMotion))
#define NUM_BALL_MOTIONS (sizeof(ballMillMotions) / sizeof(MillMotion))

EndMillFlat endMillFlat01(3.175, 16);
EndMillFlat endMillFlat02(1.5, 16);
EndMillTaper endMillTaper02(1, 16, 90, 0.2f);
EndMillBall endMillBall03(1, 16, 4, 0.2f);

MillOperation millOperations[] = {
    {&endMillFlat01, {0, 0, 10}, flatMillMotions, NUM_FLAT_MOTIONS },
    {&endMillFlat02, {0, 0, 10}, flatMillMotions, NUM_FLAT_MOTIONS },
    {&endMillBall03, {0, 0, 10}, ballMillMotions, NUM_BALL_MOTIONS },
    {&endMillTaper02, {0, 0, 10}, taperMillMotions, NUM_TAPER_MOTIONS },
};

#define TOOL_TABLE_SIZE (sizeof(millOperations) / sizeof(MillOperation))

vec3 lightColor = { 1.0, 1.0, 0.9 };
vec3 lightPos = { 20.0, 20.0, 10.0 };
vec3 ambientCol = { 0.3, 0.3, 0.1 };

vec3 eye = { 0, 100, 50 };
vec3 target = { 0, 0, 0 };
vec3 upvec = { 0, 0, 1 };

vec3 stockColor = { 0.7, 0.7, 0.7 };
vec3 cutColor = { 0.4, 0.7, 0.4 };
vec3 toolColor = { 0.4, 0.4, 0.7 };


MillOperation* curMillOperation = nullptr;
MillSim::StockObject* gStockObject;
MillSim::StockObject* glightObject;

// test section - remove!
GLuint tprogram, tmodel, tview, tprojection, tarray;

void clearMillPathSegments() {
    for (std::vector<MillSim::MillPathSegment*>::const_iterator i = MillPathSegments.begin(); i != MillPathSegments.end(); ++i) {
        MillSim::MillPathSegment* p = *i;
        delete p;
    }
    
    MillPathSegments.clear();
}

float random(float from, float to)
{
    double diff = to - from;
    double frand = diff * ((double)rand() / RAND_MAX);
    return (float)(from + frand);
}

void InitSimulation()
{
    clearMillPathSegments();
    curMillOperation = nullptr;

    gDestMotion = gZeroPos;
    //gDestPos = curMillOperation->startPos;
    gcurstep = 0;
    gnsteps = -1;
    gLastToolId = -1;
    gnPathSteps = gcodeParser.Operations.size();;
    gPathStep = 0;
}

void SetTool(int tool) {
    //curMillOpIx = 0;
    if (tool <= TOOL_TABLE_SIZE)
        curMillOperation = &millOperations[tool - 1];
    else
        curMillOperation = nullptr;
    gLastToolId = tool;
}

/*void GenetateTool(MillOperation* op)
{
    if (gToolId >= 0)
        glDeleteLists(gToolId, 1);
    gToolId = glGenLists(1);
    glNewList(gToolId, GL_COMPILE);
    op->RenderTool();
    glEndList();
}*/

void SimNext()
{
    static int simDecim = 0;

    simDecim++;
    if (simDecim < 1)
        return;

    simDecim = 0;

    if (gcurstep > gnsteps)
    {
        if (gPathStep >= gnPathSteps)
            return;

        //if (gPathStep == 0)
        //    GenetateTool(curMillOperation);

        gCurMotion = gDestMotion;
        gDestMotion = gcodeParser.Operations[gPathStep];
        if (gDestMotion.tool != gLastToolId)
        {
            SetTool(gDestMotion.tool);
        }

        if (curMillOperation != nullptr)
        {
            MillSim::MillPathSegment* segment = new MillSim::MillPathSegment(curMillOperation->endmill, &gCurMotion, &gDestMotion);
            gnsteps = segment->numSimSteps;
            gcurstep = 0;
            MillPathSegments.push_back(segment);
        }

        gPathStep++;
    }
    gcurstep++;
}


void ShowStats() {
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    //glPushMatrix();
    glLoadIdentity();
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-1.0f, -1.0f);
    glDisable(GL_LIGHTING);
    std::string s = fpsStream.str();
    for (unsigned int i=0; i<s.size(); ++i) {
        //glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
    }
    glEnable(GL_LIGHTING);
    //glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

int renderTime = 0, renderModel = 0;

float vMillProfile[] = { 0.1f, 1, 0.1f, 0.1f, 0, 0, -0.1f, 0.1f, -0.1f, 1 };

void GlsimStart()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void GlsimToolStep1(void)
{
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_ZERO, GL_ZERO, GL_REPLACE);
    glDepthMask(GL_FALSE);
}


void GlsimToolStep2(void)
{
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDepthFunc(GL_GREATER);
    glCullFace(GL_FRONT);
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
}

void GlsimClipBack(void)
{
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_ZERO);
    glDepthFunc(GL_LESS);
    glCullFace(GL_FRONT);
    //glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
}


void GlsimRenderStock(void)
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glDepthFunc(GL_EQUAL);
    glCullFace(GL_BACK);
}

void GlsimRenderTools(void)
{
    glCullFace(GL_FRONT);
}

void GlsimEnd(void)
{
    glCullFace(GL_BACK);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

int nsegs = 0;

void renderSegmentForward(int iSeg)
{
    MillSim::MillPathSegment* p = MillPathSegments.at(iSeg);
    int step = iSeg == (MillPathSegments.size() - 1) ? gcurstep : p->numSimSteps;
    int start = p->isMultyPart ? 1 : step;
    for (int i = start; i <= step; i++)
    {
        GlsimToolStep1();
        p->render(i);
        GlsimToolStep2();
        p->render(i);
    }
}

void renderSegmentReversed(int iSeg)
{
    MillSim::MillPathSegment* p = MillPathSegments.at(iSeg);
    int step = iSeg == (MillPathSegments.size() - 1) ? gcurstep : p->numSimSteps;
    int end = p->isMultyPart ? 1 : step;
    for (int i = step; i >= end; i--)
    {
        GlsimToolStep1();
        p->render(i);
        GlsimToolStep2();
        p->render(i);
    }
}


void display()
{
    mat4x4 matLookAt, model;
    mat4x4_identity(model);
    eye[2] = eyeHeight;
    mat4x4_look_at(matLookAt, eye, target, upvec);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    
    unsigned int msec = glfwGetTime() * 1000;

    if (isRotate)
        mat4x4_rotate_Z(matLookAt, matLookAt, rot);

    shaderFlat.Activate();
    shaderFlat.UpdateViewMat(matLookAt);
    
    GlsimStart();
    gStockObject->render();

    int len = (int)MillPathSegments.size();

    GlsimToolStep2();

    for (int i = 0; i < len; i++)
        renderSegmentForward(i);

    for (int i = len - 1; i >= 0; i--)
        renderSegmentForward(i);

    for (int i = 0; i < len; i++)
        renderSegmentReversed(i);

    for (int i = len - 1; i >= 0; i--)
        renderSegmentReversed(i);

    GlsimClipBack();
    gStockObject->render();

    // start coloring
    shader3D.Activate();
    shader3D.UpdateViewMat(matLookAt);
    shader3D.UpdateObjColor(stockColor);
    GlsimRenderStock();
    gStockObject->render();
    GlsimRenderTools();

    // render cuts (back faces of tools)

    shaderInv3D.Activate();
    shaderInv3D.UpdateViewMat(matLookAt);
    shaderInv3D.UpdateObjColor(cutColor);
    for (int i = 0; i < len; i++)
    {
        MillSim::MillPathSegment* p = MillPathSegments.at(i);
        int step = (i == (len - 1)) ? gcurstep : p->numSimSteps;
        int start = p->isMultyPart ? 1 : step;
        for (int j = start; j <= step; j++)
            MillPathSegments.at(i)->render(j);
    }

    GlsimEnd();
        
    glEnable(GL_CULL_FACE);

    if (curMillOperation && curMillOperation->endmill)
    {
        Vector3 toolPos;
        toolPos.FromMillMotion(&gDestMotion);
        if (len > 0)
        {
            MillSim::MillPathSegment* p = MillPathSegments.at(len - 1);
            toolPos = *p->GetHeadPosition();
        }
        mat4x4 tmat;
        mat4x4_translate(tmat, toolPos.x, toolPos.y, toolPos.z);
        shader3D.Activate();
        shader3D.UpdateObjColor(toolColor);
        curMillOperation->RenderTool(tmat, identityMat);
    }

    shaderFlat.Activate();
    shaderFlat.UpdateObjColor(lightColor);
    glightObject->render();


    if (/*len > 2 &&*/ debug > 0)
    {
        mat4x4 test;
        mat4x4_dup(test, model);
        mat4x4_translate_in_place(test, 20, 20, 3);
        mat4x4_rotate_Z(test, test, 30.f * 3.14f / 180.f);
        if (debug >= MillPathSegments.size())
            debug = 1;
        MillSim::MillPathSegment* p = MillPathSegments.at(debug);
        p->render(1);
    }


    renderTime = (glfwGetTime() * 1000) - msec;

    //ShowStats();
}

int decim = 0;

void idle() {

    static int ancient = 0;
    static int last = 0;
    static int msec = 0;        
    static int fps = 0;
    last = msec;
    msec = (glfwGetTime() * 1000);
    if (spin) {
        rot += (msec-last)/4600.0f; 
        while (rot >= PI2)
			rot -= PI2;
    }

    if (last / 1000 != msec / 1000) {
        
        float calcFps = 1000.0f * fps / (msec - ancient);
        fpsStream.str("");
        fpsStream << "fps: " << calcFps << "    rendertime:" << renderTime << "    zpos:" << gDestMotion.z << std::ends;

        ancient = msec;
        fps = 0;  
    }
    if (simulate || singleStep)
    {
        SimNext();
        singleStep = false;
    }

    //display();

    ++fps;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
    if (action != GLFW_PRESS)
        return;
    switch (key) {
    case ' ': 
        spin = !spin; 
        break;
    case 'S':
        simulate = !simulate;
        break;
    case 'T':
        simulate = false;
        singleStep = true;
        break;
    case 'I':
        nsegs++;
        break;
    case'D':
        debug++;
        if (debug == 30) debug = 0;
        break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_UP:
        eyeHeight += 5;
        break;
    case GLFW_KEY_DOWN:
        eyeHeight -= 5;
        break;
    default:
        break;
    }
    //display();
}

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";


void InitOpengl()
{
    // gray background
    GL(glClearColor(0.7f, 0.7f, 0.4f, 1.0f));

    // Setup projection
    mat4x4 projmat;
    mat4x4_perspective(projmat, 0.7, 4.0 / 3.0, 1.0, 200.0);

    // use shaders
    //   standard diffuse shader
    shader3D.CompileShader((char*)VertShader3DNorm, (char*)FragShaderNorm);
    shader3D.UpdateEnvColor(lightPos, lightColor, ambientCol);
    shader3D.UpdateProjectionMat(projmat);

    //   invarted normal diffuse shader for inner mesh
    shaderInv3D.CompileShader((char*)VertShader3DInvNorm, (char*)FragShaderNorm);
    shaderInv3D.UpdateEnvColor(lightPos, lightColor, ambientCol);
    shaderInv3D.UpdateProjectionMat(projmat);

    //   null shader to calculate meshes only (simulation stage)
    shaderFlat.CompileShader((char*)VertShader3DNorm, (char*)FragShaderFlat);
    shaderFlat.UpdateProjectionMat(projmat);

    glMatrixMode(GL_MODELVIEW);
    
    // setup tools ans stock
    //MillSim::resolution = 0.1;
    gStockObject = new MillSim::StockObject(-40, -40, -20, 80, 80, 20);
    glightObject = new MillSim::StockObject(-0.5f, -0.5f, -0.5f, 1, 1, 1);
    glightObject->SetPosition(lightPos);
    endMillFlat01.GenerateDisplayLists();
    endMillFlat02.GenerateDisplayLists();
    endMillTaper02.GenerateDisplayLists();
    endMillBall03.GenerateDisplayLists();
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

    glfwMakeContextCurrent(glwind);
    glewInit();
    glfwSwapInterval(1);

    std::cout << glGetString(GL_VERSION) << std::endl;

    if (gcodeParser.Parse("cam_test1.txt"))
        std::cout << "GCode file loaded successfuly" << std::endl;

    InitSimulation();
    InitOpengl();
    while (!glfwWindowShouldClose(glwind))
    {
        idle();
        display();
        glfwSwapBuffers(glwind);
        glfwPollEvents();
    }

    glfwDestroyWindow(glwind);

    glfwTerminate();
    exit(EXIT_SUCCESS);


    return 0;
}

