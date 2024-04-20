#include "EndMill.h"
#include <GLFW/glfw3.h>
#include "SimShapes.h"

EndMill::EndMill(float radius, int nslices)
{
	mRadius = radius;
	mNSlices = nslices;
	//mToolDisplayId = mHToolDisplayId = mPathDisplayId = 0;
}

EndMill::~EndMill()
{
	mToolShape.FreeResources();
	mHToolShape.FreeResources();
	mPathShape.FreeResources();
}

void EndMill::GenerateDisplayLists()
{
	// full tool
	RotateProfile(mProfPoints, mNPoints, 0, 0, mNSlices, false, &mToolShape);

	// half tool
	RotateProfile(mProfPoints, mNPoints, 0, 0, mNSlices / 2, true, &mHToolShape);

	// unit path
	int nFullPoints = PROFILE_BUFFER_POINTS(mNPoints);
	ExtrudeProfileLinear(mProfPoints, nFullPoints, 0, 1, 0, 0, true, false, &mPathShape);
}

unsigned int EndMill::GenerateArcSegmentDL(float radius, float angleRad, float zShift, Shape *retShape)
{
	int nFullPoints = PROFILE_BUFFER_POINTS(mNPoints);
	unsigned int dispId = glGenLists(1);
	glNewList(dispId, GL_COMPILE);
	ExtrudeProfileRadial(mProfPoints, PROFILE_BUFFER_POINTS(mNPoints), radius, angleRad, zShift, true, true, retShape);
	glEndList();
	return dispId;
}

void EndMill::MirrorPointBuffer()
{
	int endpoint = PROFILE_BUFFER_POINTS(mNPoints) - 1;
	for (int i = 0, j = endpoint * 2; i < (mNPoints - 1) * 2; i += 2, j -= 2)
	{
		mProfPoints[j] = -mProfPoints[i];
		mProfPoints[j + 1] = mProfPoints[i + 1];
	}
}
