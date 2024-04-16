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

    int numVerts = GetNumExtrudedProfileVerts(NUM_PROFILE_POINTS, 2);
    int numIndices = GetNumExtrudedProfileIndices(NUM_PROFILE_POINTS, 2);

    float* vbuffer = (float*)malloc(numVerts * 6 * sizeof(float));
    GLushort* ibuffer = (GLushort*)malloc(numIndices * sizeof(float));

    ExtrudeProfileLinear(vbuffer, ibuffer, mProfile, NUM_PROFILE_POINTS, x, x + l, 0, 0, true, true);
    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 24, vbuffer);
    glNormalPointer(GL_FLOAT, 24, vbuffer + 3);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, ibuffer);
    free(vbuffer);
    free(ibuffer);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
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
