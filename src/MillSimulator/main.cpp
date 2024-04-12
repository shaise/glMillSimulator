
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
bool singleStep = false;
int debug = 0;
bool isRotate = true;
float rot = 0.0f;
std::ostringstream fpsStream;

MillMotion gZeroPos = { 0, 0,  10 };
MillMotion gCurPos;
MillMotion gDestPos;
int gcurstep = 0;
int gnsteps = 0;
int gPathStep = 0;
bool gIsInStock;
int gToolId = -1;
int curMillOpIx = 0;
#define STOCK_HEIGHT 2.0f

std::vector<MillSim::MillPathSegment*> MillPathSegments;

MillMotion flatMillMotions[] = {
    //{4, 4, 10},
    //{4, 4, 0},
    //{4, 4, 10},
    {-4, -4, 10},
    //{-4, -4, 0},
    {-4, -4, 2},
    {4, 4, 0, 4, 4, 0},
    {4, 4, 10},

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

EndMillFlat endMillFlat01(1, 16);
EndMillTaper endMillTaper02(1, 16, 90, 0.2f);
EndMillBall endMillBall03(1, 16, 4, 0.2f);

MillOperation millOperations[] = {
    {&endMillBall03, {0, 0, 10}, ballMillMotions, NUM_BALL_MOTIONS },
    {&endMillFlat01, {0, 0, 10}, flatMillMotions, NUM_FLAT_MOTIONS },
    {&endMillTaper02, {0, 0, 10}, taperMillMotions, NUM_TAPER_MOTIONS },
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
        gDestPos = curMillOperation->path[gPathStep];
        MillSim::MillPathSegment* segment = new MillSim::MillPathSegment(curMillOperation->endmill, &gCurPos, &gDestPos);
        gnsteps = segment->numSimSteps;
        gcurstep = 0;
        MillPathSegments.push_back(segment);
        /*
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
        gIsInStock = !((curMotion->z > STOCK_HEIGHT && gCurPos.z > STOCK_HEIGHT) || ((gDestPos.z > gCurPos.z) && MillSim::IsVerticalMotion(&gCurPos , &gDestPos)));
        if (gIsInStock)
            MillPathSegments.push_back(new MillSim::MillPathSegment(curMillOperation->endmill, &gCurPos, &gDestPos));
        */
        gPathStep++;
    }
    gcurstep++;
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
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 45, 30,  /* eye  */
              0.0, 0.0, 0.0,  /* taeget  */
              0.0, 0.0, 1.0); /* up vector */
    if (isRotate)
        glRotatef(rot, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    unsigned int msec = glutGet(GLUT_ELAPSED_TIME);
    
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
    GlsimRenderStock();
    gStockObject->render();
    GlsimRenderTools();
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

    if (gToolId >= 0)
    {
        Vector3 toolPos;
        toolPos.FromMillMotion(&gDestPos);
        if (len > 0)
        {
            MillSim::MillPathSegment* p = MillPathSegments.at(len - 1);
            toolPos = *p->GetHeadPosition();
        }
        glPushMatrix();
        glTranslatef(toolPos.x, toolPos.y, toolPos.z);
        glCallList(gToolId);
        glPopMatrix();
    }

    if (len > 3 && debug > 0)
    {
        MillSim::MillPathSegment* p = MillPathSegments.at(3);
        p->render(p->numSimSteps);
    }


    renderTime = glutGet(GLUT_ELAPSED_TIME) - msec;

    ShowStats();

    glutSwapBuffers();
}

int decim = 0;

void idle() {

    static int ancient = 0;
    static int last = 0;
    static int msec = 0;        
    static int fps = 0;
    last = msec;
    msec = glutGet(GLUT_ELAPSED_TIME);
    if (spin) {
        rot += (msec-last)/80.0f; 
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
    if (simulate || singleStep)
    {
        SimNext();
        singleStep = false;
    }

    //display();

    ++fps;
}

void key(unsigned char k, int, int) {
    switch (k) {
    case ' ': 
        spin = !spin; 
        break;
    case 's':
        simulate = !simulate;
        break;
    case 't':
        simulate = false;
        singleStep = true;
        break;
    case 'i':
        nsegs++;
        break;
    case'd':
        debug++;
        if (debug == 30) debug = 0;
        break;
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
    GLfloat light_position0[] = {-20, -20,  -20,  0};  // Infinite light location
    GLfloat light_position1[] = { 20,  20,  20,  0};  // Infinite light location

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
    gluPerspective(40.0, 4.0/3.0, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
    
    // setup tools ans stock
    //MillSim::resolution = 0.1;
    gStockObject = new MillSim::StockObject(-20, -20, 0.001f, 40, 40, 2);
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

