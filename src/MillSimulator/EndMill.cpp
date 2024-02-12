#include "EndMill.h"

EndMill::EndMill(float radius, int nslices)
{
	mRadius = radius;
	mNSlices = nslices;
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
