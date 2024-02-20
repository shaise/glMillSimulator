#ifndef __end_mill_h__
#define __end_mill_h__

#define PROFILE_BUFFER_POINTS(npoints) ((npoints) * 2 - 1)
#define PROFILE_BUFFER_SIZE(npoints) (PROFILE_BUFFER_POINTS(npoints) * 2)
#define MILL_HEIGHT 1
#define PI 3.14159265

class EndMill
{
public:
	float* mProfPoints = nullptr;
	float mRadius;
	int mNPoints = 0;
	int mNSlices;
	unsigned int mPathDisplayId;
	unsigned int mHToolDisplayId;
	unsigned int mToolDisplayId;

public:
	EndMill(float radius, int nslices);
	virtual ~EndMill();
	void GenerateDisplayLists();
	unsigned int GenerateArcSegmentDL(float radius, float angleRad, float zShift);

protected:
	void MirrorPointBuffer();
};


#endif
