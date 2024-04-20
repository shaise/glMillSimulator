#include "EndMillFlat.h"
#include "SimShapes.h"

EndMillFlat::EndMillFlat(float diameter, int nslices) : EndMill(diameter, nslices)
{
	int idx = 0;
	SET_DUAL(_profVerts, idx, mRadius, MILL_HEIGHT);
	SET_DUAL(_profVerts, idx, mRadius, 0);
	SET_DUAL(_profVerts, idx, 0, 0);
	mNPoints = 3;
	mProfPoints = _profVerts;
	MirrorPointBuffer();
	//GenerateDisplayLists();
}
