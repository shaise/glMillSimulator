#include "StockObject.h"
#include "SimShapes.h"
#include <GLFW/glfw3.h>
#include <malloc.h>

#define NUM_PROFILE_POINTS 4

MillSim::StockObject::StockObject(float x, float y, float z, float l, float w, float h)
{
    int idx = 0;
    SET_DUAL(mProfile, idx, y + w, z + h);
    SET_DUAL(mProfile, idx, y + w, z);
    SET_DUAL(mProfile, idx, y, z);
    SET_DUAL(mProfile, idx, y, z + h);


    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);
    ExtrudeProfileLinear(mProfile, NUM_PROFILE_POINTS, x, x + l, 0, 0, true, true);
    glEndList();

    mDisplayListId = id;
}

MillSim::StockObject::~StockObject()
{
    glDeleteLists(mDisplayListId, 1);
}

void MillSim::StockObject::render()
{
    glCallList(mDisplayListId);
}
