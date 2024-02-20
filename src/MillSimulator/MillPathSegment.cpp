#include "MillPathSegment.h"
#include <GL/glut.h>
#include "SimShapes.h"

#define PI 3.1415926
#define N_MILL_SLICES 8
#define SET_TRIPLE(var, idx, x, y, z) {var[idx] = x; var[idx+1] = y; var[idx+2] = z;}
#define SET_TRIPLE_OFFS(var, idx, offs, x, y, z) {var[idx] = x + offs; var[idx+1] = y + offs; var[idx+2] = z + offs;}
#define MAX_SEG_DEG (PI / 4.0f)   // 45 deg
#define NIN_SEG_DEG (PI / 90.0f)  // 2 deg

bool IsVerticalMotion(MillMotion* m1, MillMotion* m2)
{
    return (m1->z != m2->z && EQ_FLOAT(m1->x, m2->x) && EQ_FLOAT(m1->y, m2->y));
}

bool IsArcMotion(MillMotion* m)
{
    return fabs(m->i > EPSILON) || fabs(m->j) > EPSILON;
}


namespace MillSim {

    float resolution = 0.1;

    MillPathSegment::MillPathSegment(EndMill *endmill, MillMotion* from, MillMotion* to)
    {
        float diffx = to->x - from->x;
        float diffy = to->y - from->y;
        float diffz = to->z - from->z;
        mXYDistance = sqrtf(diffx * diffx + diffy * diffy);
        mZDistance = fabs(diffz);
        mXYZDistance = sqrtf(mXYDistance * mXYDistance + diffz * diffz);
        mXYAngle = atan2f(diffy, diffx) * 180.0f / PI;
        mEndmill = endmill;
        mDisplayListId = 0;
        if (IsArcMotion(to))
        {
            mMotionType = MTCurved;
            mTarget = *from;
            float radius = sqrtf(to->j * to->j + to->i * to->i);
            float mStepAng = asinf(resolution / radius);
            if (mStepAng > MAX_SEG_DEG)
                mStepAng = MAX_SEG_DEG;
            else if (mStepAng < NIN_SEG_DEG)
                mStepAng = NIN_SEG_DEG;
            mStartAng = atan2f(-to->i, -to->j);
            float endAng = atan2f(diffx - to->i, diffy - to->j);
            float sweepAng = mStartAng - endAng;
            if (sweepAng < EPSILON)
                sweepAng += PI * 2;
            numRenderSteps = (int)(sweepAng / mStepAng) + 1;
            mStepAng = sweepAng / numRenderSteps;
            mDisplayListId = mEndmill->GenerateArcSegmentDL(radius, mStepAng, 0);
            numSimSteps = numRenderSteps;
        }
        else
        {
            numSimSteps = (int)(mXYZDistance / resolution);
            if (numSimSteps == 0)
                numSimSteps = 1;

            if (IsVerticalMotion(from, to)) {
                mMotionType = MTVertical;
                mTarget = *to;
                numRenderSteps = 1;
            }
            else {
                mMotionType = MTHorizontal;
                mTarget = *from;
                numRenderSteps = 1;
            }
        }
    }

    MillPathSegment::~MillPathSegment()
    {
        if (mDisplayListId > 0)
            glDeleteLists(mDisplayListId, 1);
    }

    void MillPathSegment::render() 
    {
        render(numRenderSteps - 1);
    }

    void MillPathSegment::render(int step) 
    {
        glPushMatrix();
        glTranslatef(mTarget.x, mTarget.y, mTarget.z);
        if (mMotionType == MTVertical)
            glCallList(mEndmill->mToolDisplayId);
        else if (mMotionType == MTCurved)
        {
            glTranslatef(mTarget.i, mTarget.j, 0);
            glRotatef(mStartAng - step * mStepAng, 0, 0, 1);
            glCallList(mDisplayListId);
        }
        else
        {
            glRotatef(mXYAngle, 0, 0, 1);
            glPushMatrix();
            glScalef(mXYDistance, 1, 1);
            glCallList(mEndmill->mPathDisplayId);
            glPopMatrix();
            glTranslatef(mXYDistance, 0, 0);
            glCallList(mEndmill->mHToolDisplayId);
        }
        //glCallList(mDisplayListId);
        glPopMatrix();
    }

}