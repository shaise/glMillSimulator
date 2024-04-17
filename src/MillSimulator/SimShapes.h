#ifndef __sim_shapes_h__
#define __sim_shapes_h__
#include <GLFW/glfw3.h>

#define SET_DUAL(var, idx, y, z) {var[idx++] = y; var[idx++] = z; }
#define SET_TRIPLE(var, idx, x, y, z) {var[idx++] = x; var[idx++] = y; var[idx++] = z; }
#define SET_TRIPLE_OFFS(var, idx, offs, x, y, z) {var[idx++] = x + offs; var[idx++] = y + offs; var[idx++] = z + offs; }

void RotateProfile(float* profPoints, int nPoints, float distance, float deltaHeight, int nSlices, bool isHalfTurn);
void ExtrudeProfileRad(float* profPoints, int nPoints, float radius, float angleRad, float deltaHeight, bool capStart, bool capEnd);
void ExtrudeProfileRadial(float* profPoints, int nPoints, float radius, float angleRad, float deltaHeight, bool capStart, bool capEnd);
void ExtrudeProfileLinear(float* profPoints, int nPoints, float fromX, float toX, float fromZ, float toZ, bool capStart, bool capEnd);

#endif


