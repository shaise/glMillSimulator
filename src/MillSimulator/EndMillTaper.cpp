#include "EndMillTaper.h"
#include "SimShapes.h"
#include <math.h>

EndMillTaper::EndMillTaper(float radius, int nslices, float taperAngle, float flatRadius):
	EndMill(radius, nslices)
{
	float ta = (float)tanf(taperAngle * PI / 360);
	float l1 = flatRadius / ta;
	if (l1 < 0.0001)
		l1 = 0;
	float l = radius / ta - l1;
	int idx = 0;
	SET_DUAL(_profVerts, idx, radius, MILL_HEIGHT);
	SET_DUAL(_profVerts, idx, radius, l);
	SET_DUAL(_profVerts, idx, flatRadius, 0);
	mNPoints = 3;
	if (l1 > 0)
	{
		SET_DUAL(_profVerts, idx, 0, 0);
		mNPoints++;
	}
	mProfPoints = _profVerts;
	MirrorPointBuffer();
	//GenerateDisplayLists();
}
