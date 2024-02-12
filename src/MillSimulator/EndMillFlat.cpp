#include "EndMillFlat.h"
#include "SimShapes.h"

EndMillFlat::EndMillFlat(float radius, int nslices) : EndMill(radius, nslices)
{
	int idx = 0;
	SET_DUAL(_profVerts, idx, radius, MILL_HEIGHT);
	SET_DUAL(_profVerts, idx, radius, 0);
	SET_DUAL(_profVerts, idx, 0, 0);
	mNPoints = 3;
	mProfPoints = _profVerts;
	MirrorPointBuffer();
}
