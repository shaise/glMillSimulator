#ifndef __glutils_h__
#define __glutils_h__
#include <GL/glew.h>
#include "linmath.h"

#define PI 3.14159265f
#define PI2 (PI*2)

constexpr auto EPSILON = 0.00001f;
#define EQ_FLOAT(x,y) (fabs((x) - (y)) < EPSILON)

void GLClearError();
bool GLLogError();
#define GL(x) { GLClearError(); x; if (!GLLogError()) __debugbreak(); }
extern mat4x4 identityMat;
#endif // !__glutils_h__

