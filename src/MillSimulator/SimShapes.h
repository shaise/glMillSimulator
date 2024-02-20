#ifndef __sim_shapes_h__
#define __sim_shapes_h__
#include <GL/glut.h>

#define SET_DUAL(var, idx, y, z) {var[idx++] = y; var[idx++] = z; }
#define SET_TRIPLE(var, idx, x, y, z) {var[idx++] = x; var[idx++] = y; var[idx++] = z; }
#define SET_TRIPLE_OFFS(var, idx, offs, x, y, z) {var[idx++] = x + offs; var[idx++] = y + offs; var[idx++] = z + offs; }

void SolidCylinder(GLdouble radius, GLdouble height, GLint slices, GLint stacks);
void ExtrudeProfilePar(float* profPoints, int nPoints, float distance, float deltaHeight);
void TesselateProfile(float* profPoints, int nPoints, float distance, float deltaHeight);
void RotateProfile(float* profPoints, int nPoints, float distance, float deltaHeight, int nSlices, bool isHalfTurn);
void ExtrudeProfileRad(float* profPoints, int nPoints, float radius, float angleRad, float deltaHeight, bool capStart, bool capEnd);



#endif


