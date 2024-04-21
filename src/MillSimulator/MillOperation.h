#ifndef __mill_operation_h__
#define __mill_operation_h__
//#include <math.h>
#include "EndMill.h"
#include "linmath.h"

enum eEndMillType
{
	eEndmillFlat,
	eEndmillV,
	eEndmillBall,
	eEndmillFillet
};

enum eCmdType
{
	eNop,
	eMoveLiner,
	eRotateCW,
	eRotateCCW,
	eChangeTool
};

struct MillMotion
{
	eCmdType cmd;
	int tool;
	float x, y, z;
	float i, j, k;
};
constexpr auto EPSILON = 0.00001f;
#define EQ_FLOAT(x,y) (fabs((x) - (y)) < EPSILON)

class MillOperation
{
public:
	EndMill *endmill;

public:
	void RenderTool(mat4x4 transformMat, mat4x4 normalMat);
};

class Vector3
{
public:
	float x, y, z;

public:
	Vector3()
	{
		x = 0; y = 0; z = 0;
	}

	void FromMillMotion(MillMotion *mm)
	{
		x = mm->x; y = mm->y; z = mm->z;
	}

	void Add(Vector3* other)
	{
		x += other->x;
		y += other->y;
		z += other->z;
	}

	void Sub(Vector3* other)
	{
		x -= other->x;
		y -= other->y;
		z -= other->z;
	}

	void Div(float fval)
	{
		x /= fval;
		y /= fval;
		z /= fval;
	}

	void Mul(float fval)
	{
		x *= fval;
		y *= fval;
		z *= fval;
	}

	void Set(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

};
#endif