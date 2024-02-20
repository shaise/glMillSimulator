#include "EndMill.h"
#include <GL/glut.h>
#include "SimShapes.h"

EndMill::EndMill(float radius, int nslices)
{
	mRadius = radius;
	mNSlices = nslices;
	mToolDisplayId = mHToolDisplayId = mPathDisplayId = 0;
}

EndMill::~EndMill()
{
	if (mToolDisplayId > 0)
		glDeleteLists(mToolDisplayId, 1);
	if (mHToolDisplayId > 0)
		glDeleteLists(mHToolDisplayId, 1);
	if (mPathDisplayId > 0)
		glDeleteLists(mPathDisplayId, 1);
}

void EndMill::GenerateDisplayLists()
{
	// full tool
	mToolDisplayId = glGenLists(1);
	glNewList(mToolDisplayId, GL_COMPILE);
	RotateProfile(mProfPoints, mNPoints, 0, 0, mNSlices, false);
	glEndList();

	// half tool
	mHToolDisplayId = glGenLists(1);
	glNewList(mHToolDisplayId, GL_COMPILE);
	RotateProfile(mProfPoints, mNPoints, 0, 0, mNSlices / 2, true);
	glEndList();

	// unit path
	int nFullPoints = PROFILE_BUFFER_POINTS(mNPoints);
	mPathDisplayId = glGenLists(1);
	glNewList(mPathDisplayId, GL_COMPILE);
	ExtrudeProfilePar(mProfPoints, nFullPoints, 1, 0);
	TesselateProfile(mProfPoints, nFullPoints, 0, 0);
	glEndList();
}

unsigned int EndMill::GenerateArcSegmentDL(float radius, float angleRad, float zShift)
{
	int nFullPoints = PROFILE_BUFFER_POINTS(mNPoints);
	unsigned int dispId = glGenLists(1);
	glNewList(dispId, GL_COMPILE);
	ExtrudeProfileRad(mProfPoints, PROFILE_BUFFER_POINTS(mNPoints), radius, angleRad, zShift, true, true);
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
