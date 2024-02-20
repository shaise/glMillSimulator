#include "MillOperation.h"
#include "SimShapes.h"
#include <cmath>


void MillOperation::RenderTool()
{
	RotateProfile(endmill->mProfPoints, endmill->mNPoints, 0, 0, endmill->mNSlices, false);
}
