#include "StockObject.h"
#include "Shader.h"
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


    ExtrudeProfileLinear(mProfile, NUM_PROFILE_POINTS, x, x + l, 0, 0, true, true, &mShape);
 
    mat4x4_identity(modelMat);
}

MillSim::StockObject::~StockObject()
{
    mShape.FreeResources();
}

void MillSim::StockObject::render()
{
    //glCallList(mDisplayListId);
    //UpdateObjColor(color);
    mShape.Render(modelMat, modelMat); // model is not rotated hence both are identity matrix
}

void MillSim::StockObject::SetPosition(vec3 position)
{
    mat4x4_translate(modelMat, position[0], position[1], position[2]);
}
