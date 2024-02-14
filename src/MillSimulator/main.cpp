
//
// main.cpp
//
// simulating cnc mill operation using Sequenced Convex Subtraction (SCS) by Nigel Stewart et al.
//

#include "MillPathSegment.h"
#include "StockObject.h"
#include "MillOperation.h"
#include "SimShapes.h"
#include "EndMillFlat.h"
#include "EndMillTaper.h"
#include "EndMillBall.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <GL/glut.h>

bool spin = true;
bool simulate = false;
float rot = 0.0f;
std::ostringstream fpsStream;

MillMotion gZeroPos = { 0, 0, 0 };
MillMotion gMotionStep = gZeroPos;
MillMotion gCurPos = gZeroPos;
MillMotion gDestPos = gZeroPos;
int gcurstep = 0;
int gnsteps = 0;
int gPathStep = 0;
bool gIsInStock;
int gToolId = -1;
int curMillOpIx = 0;
#define STOCK_HEIGHT 0.2f

std::vector<MillSim::MillPathSegment*> MillPathSegments;

MillMotion flatMillMotions[] = {
    {0.4f, 0.4f, 1},
    {0.4f, 0.4f, 0.1f},
    {0.4f, 0.4f, 1},
    {-0.4f, -0.4f, 1},
    {-0.4f, -0.4f, 0.1f},
    {-0.4f, -0.4f, 1},

    {1.5f, 1.5f, 1},
    { 1.5f, 1.5f, 0.15f},
    {1.5f, -1.5f, 0.15f},
    {-1.5f, -1.5f, 0.15f},
    {-1.5f, 1.5f, 0.15f},
    {1.5f, 1.5f, 0.15f},

    {1.5f, 1.5f, 0.1f},
    {1.5f, -1.5f, 0.1f},
    {-1.5f, -1.5f, 0.1f},
    {-1.5f, 1.5f, 0.1f},
    {1.5f, 1.5f, 0.1f},

    {1.5f, 1.5f, 0.05f},
    {1.5f, -1.5f, 0.05f},
    {-1.5f, -1.5f, 0.05f},
    {-1.5f, 1.5f, 0.05f},
    {1.5f, 1.5f, 0.05f},

    {1.5f, 1.5f, 0},
    {1.5f, -1.5f, 0},
    {-1.5f, -1.5f, 0},
    {-1.5f, 1.5f, 0},
    {1.5f, 1.5f, 0},

    {1.5f, 1.5f, 1},

    {0.8f, 0.8f, 1},
    {0.8f, 0.8f, 0.15f},
    {0.8f, -0.8f, 0.15f},
    {0.61f, -0.8f, 0.15f},
    {0.61f, 0.8f, 0.15f},
    {0.42f, 0.8f, 0.15f},
    {0.42f, -0.8f, 0.15f},
    {0.23f, -0.8f, 0.15f},
    {0.23f, 0.8f, 0.15f},
    {0.04f, 0.8f, 0.15f},
    {0.04f, -0.8f, 0.15f},
    {-0.15f, -0.8f, 0.15f},
    {-0.15f, 0.8f, 0.15f},
    {-0.34f, 0.8f, 0.15f},
    {-0.34f, -0.8f, 0.15f},
    {-0.53f, -0.8f, 0.15f},
    {-0.53f, 0.8f, 0.15f},
    {-0.72f, 0.8f, 0.15f},
    {-0.72f, -0.8f, 0.15f},
    {-0.8f,  -0.8f, 0.15f},
    {-0.8f,  0.8f, 0.15f},
    { 0.8f,  0.8f, 0.15f},
    { 0.8f,  -0.8f, 0.15f},
    {-0.8f,  -0.8f, 0.15f},

    {-0.8f,  -0.8f, 1},
};

MillMotion taperMillMotions[] = {
    {1.42f, 1.42f, 1},
    {1.42f, 1.42f, 0.15f},
    {1.42f, -1.42f, 0.15f},
    {-1.42f, -1.42f, 0.15f},
    {-1.42f, 1.42f, 0.15f},
    {1.42f, 1.42f, 0.15f},
    {1.42f, 1.42f, 1},
    {0, 0, 1},
};

MillMotion ballMillMotions[] = {
    {1.2f, 1.2f, 1},
    {1.2f, 1.2f, 0.15f},
    {1.2f, -1.2f, 0.15f},
    {-1.2f, -1.2f, 0.15f},
    {-1.2f, 1.2f, 0.15f},
    {1.2f, 1.2f, 0.15f},
    {1.2f, 1.2f, 1},
    {0, 0, 1},
};

#define NUM_FLAT_MOTIONS (sizeof(flatMillMotions) / sizeof(MillMotion))
#define NUM_TAPER_MOTIONS (sizeof(taperMillMotions) / sizeof(MillMotion))
#define NUM_BALL_MOTIONS (sizeof(ballMillMotions) / sizeof(MillMotion))

EndMillFlat endMillFlat01(0.1f, 16);
EndMillTaper endMillTaper02(0.1f, 16, 90, 0.02f);
EndMillBall endMillBall03(0.1f, 16, 4, 0.02f);

MillOperation millOperations[] = {
    {&endMillFlat01, {0, 0, 1}, flatMillMotions, NUM_FLAT_MOTIONS },
    {&endMillTaper02, {0, 0, 1}, taperMillMotions, NUM_TAPER_MOTIONS },
    {&endMillBall03, {0, 0, 1}, ballMillMotions, NUM_BALL_MOTIONS },
    {nullptr}
};

MillOperation* curMillOperation;
MillSim::StockObject* gStockObject;

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

void setMill(bool isClear) {
    if (isClear)
    {
        clearMillPathSegments();
        curMillOpIx = 0;
        curMillOperation = &millOperations[curMillOpIx];
    }

    gMotionStep = gZeroPos;
    gCurPos = gZeroPos;
    gDestPos = curMillOperation->startPos;
    gcurstep = 0;
    gnsteps = 0;
    gPathStep = 0;
}

void GenetateTool(MillOperation* op)
{
    if (gToolId >= 0)
        glDeleteLists(gToolId, 1);
    gToolId = glGenLists(1);
    glNewList(gToolId, GL_COMPILE);
    op->RenderTool();
    glEndList();
}

void SimNext()
{
    static int simDecim = 0;

    simDecim++;
    if (simDecim < 1)
        return;

    simDecim = 0;

    if (gcurstep >= gnsteps)
    {
        if (gPathStep >= curMillOperation->nPathSections)
        {
            if (curMillOperation->endmill == nullptr)
                return;
            curMillOpIx++;
            curMillOperation = &millOperations[curMillOpIx];
            if (curMillOperation->endmill == nullptr)
                return;
            setMill(false);
            return;
        }

        if (gPathStep == 0)
            GenetateTool(curMillOperation);

        gCurPos = gDestPos;
        MillMotion* curMotion = &curMillOperation->path[gPathStep];
        float diffx = curMotion->x - gCurPos.x;
        float diffy = curMotion->y - gCurPos.y;
        float diffz = curMotion->z - gCurPos.z;
        float dist = sqrtf(diffx * diffx + diffy * diffy + diffz * diffz);
        gnsteps = (int)(dist / 0.1);
        if (gnsteps == 0)
            gnsteps = 1;
        gcurstep = 1;
        gMotionStep.x = diffx / gnsteps;
        gMotionStep.y = diffy / gnsteps;
        gMotionStep.z = diffz / gnsteps;
        gDestPos.x = gCurPos.x + gMotionStep.x;
        gDestPos.y = gCurPos.y + gMotionStep.y;
        gDestPos.z = gCurPos.z + gMotionStep.z;
        gIsInStock = !((curMotion->z > STOCK_HEIGHT && gCurPos.z > STOCK_HEIGHT) || ((gDestPos.z > gCurPos.z) && IsVerticalMotion(&gCurPos , &gDestPos)));
        if (gIsInStock)
            MillPathSegments.push_back(new MillSim::MillPathSegment(curMillOperation->endmill, &gCurPos, &gDestPos));
        gPathStep++;
    }
    else
    {
        gcurstep++;
        gDestPos.x = gCurPos.x + gMotionStep.x * gcurstep;
        gDestPos.y = gCurPos.y + gMotionStep.y * gcurstep;
        gDestPos.z = gCurPos.z + gMotionStep.z * gcurstep;
        if (gIsInStock)
        {
            MillSim::MillPathSegment* p = MillPathSegments.back();
            MillPathSegments.pop_back();
            delete(p);
            MillPathSegments.push_back(new MillSim::MillPathSegment(curMillOperation->endmill, &gCurPos, &gDestPos));
        }
    }
}


void ShowStats() {
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-1.0f, -1.0f);
    glDisable(GL_LIGHTING);
    std::string s = fpsStream.str();
    for (unsigned int i=0; i<s.size(); ++i) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s[i]);
    }
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

int renderTime = 0, renderModel = 0;

float vMillProfile[] = { 0.1f, 1, 0.1f, 0.1f, 0, 0, -0.1f, 0.1f, -0.1f, 1 };

void GlsimStart()
{
    glEnable(GL_CULL_FACE);
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

void display()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 7.5, 3.0,  /* eye  */
              0.0, 0.0, 0.0,  /* taeget  */
              0.0, 0.0, 1.0); /* up vector */
    glRotatef(rot, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    unsigned int msec = glutGet(GLUT_ELAPSED_TIME);
    
    GlsimStart();
    gStockObject->render();

    int len = (int)MillPathSegments.size();

    GlsimToolStep2();

    for (int i = 0; i < len; i++)
    {
        MillSim::MillPathSegment* p = MillPathSegments.at(i);
        GlsimToolStep1();
        p->render();
        GlsimToolStep2();
        p->render();
    }

    for (int i = len - 1; i >= 0 ; i--)
    {
        MillSim::MillPathSegment* p = MillPathSegments.at(i);
        GlsimToolStep1();
        p->render();
        GlsimToolStep2();
        p->render();
    }

    GlsimClipBack();
    gStockObject->render();
    GlsimRenderStock();
    gStockObject->render();
    GlsimRenderTools();
    for (int i = 0; i < len; i++)
        MillPathSegments.at(i)->render();

    GlsimEnd();
    
    //gStockPrimitive->render();
    glEnable(GL_CULL_FACE);

    if (gToolId >= 0)
    {
        glEnable(GL_CULL_FACE);
        glPushMatrix();
        /*int n = primitives.size() - 1;
        if (n > 0)
            primitives[n]->render();*/
        glTranslatef(gDestPos.x, gDestPos.y, gDestPos.z);
        glCallList(gToolId);
        glPopMatrix();
        glDisable(GL_CULL_FACE);
    }

    renderTime = glutGet(GLUT_ELAPSED_TIME) - msec;

    ShowStats();

    glutSwapBuffers();
}

void idle() {

    static int ancient = 0;
    static int last = 0;
    static int msec = 0;        
    static int fps = 0;
    last = msec;
    msec = glutGet(GLUT_ELAPSED_TIME);
    if (spin) {
        rot += (msec-last)/40.0f; 
        while (rot >= 360.0f)
			rot -= 360.0f;
    }

    if (last / 1000 != msec / 1000) {
        
        float calcFps = 1000.0f * fps / (msec - ancient);
        fpsStream.str("");
        fpsStream << "fps: " << calcFps << "    rendertime:" << renderTime << "    zpos:" << gDestPos.z << std::ends;

        ancient = msec;
        fps = 0;  
    }
    if (simulate)
        SimNext();

    display();

    ++fps;
}

void key(unsigned char k, int, int) {
    switch (k) {
    case ' ': 
        spin = !spin; 
        break;
    case 's':
        simulate = !simulate;
    default:
        break;
    }
    display();
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

void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(1000 / 60, timer, 0);
    idle();
}

void init()
{
   // gray background
    glClearColor(0.9f, 0.9f, 0.5f, 1.0f);

    // Enable two OpenGL lights
    GLfloat light_diffuse[] = { 1.0f,  1.0f,  1.0f,  1.0f };  // white diffuse light
    GLfloat light_diffuse2[] = { 0.6f,  0.6f,  0.9f,  1.0f };  // purple diffuse light
    GLfloat light_position0[] = {-2.0f, -2.0f,  -2.0f,  0.0f};  // Infinite light location
    GLfloat light_position1[] = { 2.0f,  2.0f,  2.0f,  0.0f};  // Infinite light location

    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glEnable(GL_LIGHT0);  
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    // Use depth buffering for hidden surface elimination
    glEnable(GL_DEPTH_TEST);

    // Setup the view of the CSG shape
    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0, 4.0/3.0, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    
    // setup tools ans stock
    gStockObject = new MillSim::StockObject(-2, -2, 0.0001f, 3.5, 3.5, 0.2f);
    endMillFlat01.GenerateDisplayLists();
    endMillTaper02.GenerateDisplayLists();
    endMillBall03.GenerateDisplayLists();

    setMill(true);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutCreateWindow("Mill Simulation");

    glutDisplayFunc(display);
    glutKeyboardFunc(key);

    //glutIdleFunc(idle);
    init();
    timer(0);
    glutMainLoop();

    return 0;
}

