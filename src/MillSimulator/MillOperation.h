#ifndef __mill_operation_h__
#define __mill_operation_h__
#include <math.h>
#include "EndMill.h"

enum eEndMillType
{
	eEndmillFlat,
	eEndmillV,
	eEndmillBall,
	eEndmillFillet
};

struct MillMotion
{
	float x, y, z;
	float i, j, k;
};
constexpr auto EPSILON = 0.00001f;
#define EQ_FLOAT(x,y) (fabs((x) - (y)) < EPSILON)

bool IsVerticalMotion(MillMotion* m1, MillMotion* m2);

class MillOperation
{
public:
	EndMill *endmill;
	MillMotion startPos;
	MillMotion* path;
	int nPathSections;

public:
	void RenderTool();
};

#endif