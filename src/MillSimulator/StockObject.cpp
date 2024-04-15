#include "StockObject.h"
#include <GLFW/glfw3.h>


MillSim::StockObject::StockObject(float x, float y, float z, float l, float w, float h)
{
    float l2 = l / 2 + x;
    float w2 = w / 2 + y;
    float h2 = h / 2 + z;

    GLuint id = glGenLists(1);
    glNewList(id, GL_COMPILE);
    glPushMatrix();
    glTranslatef(l2, w2, h2);
    glScalef(l, w, h);
    
    //glutSolidCube(1);
    glPopMatrix();
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
