#include "MillOperation.h"
#include "SimShapes.h"

bool IsVerticalMotion(MillMotion* m1, MillMotion* m2)
{
	return (m1->z != m2->z && EQ_FLOAT(m1->x, m2->x) && EQ_FLOAT(m1->y, m2->y));
}

void MillOperation::RenderTool()
{
	RotateProfile(endmill->mProfPoints, endmill->mNPoints, 0, 0, endmill->mNSlices, false);
}
