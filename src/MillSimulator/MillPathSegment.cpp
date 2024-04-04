#include "MillPathSegment.h"
#include <GL/glut.h>
#include "SimShapes.h"

#define PI 3.1415926
#define N_MILL_SLICES 8
#define SET_TRIPLE(var, idx, x, y, z) {var[idx] = x; var[idx+1] = y; var[idx+2] = z;}
#define SET_TRIPLE_OFFS(var, idx, offs, x, y, z) {var[idx] = x + offs; var[idx+1] = y + offs; var[idx+2] = z + offs;}
#define MAX_SEG_DEG (PI / 4.0f)   // 45 deg
#define NIN_SEG_DEG (PI / 90.0f)  // 2 deg



namespace MillSim {

    bool IsVerticalMotion(MillMotion* m1, MillMotion* m2)
    {
        return (m1->z != m2->z && EQ_FLOAT(m1->x, m2->x) && EQ_FLOAT(m1->y, m2->y));
    }

    bool IsArcMotion(MillMotion* m)
    {
        return fabs(m->i > EPSILON) || fabs(m->j) > EPSILON;
    }

    float resolution = 0.1;

    MillPathSegment::MillPathSegment(EndMill *endmill, MillMotion* from, MillMotion* to)
    {
        mDiffX = to->x - from->x;
        mDiffY = to->y - from->y;
        mDiffZ = to->z - from->z;
        mXYDistance = sqrtf(mDiffX * mDiffX + mDiffY * mDiffY);
        mZDistance = fabs(mDiffZ);
        mXYZDistance = sqrtf(mXYDistance * mXYDistance + mDiffZ * mDiffZ);
        mXYAngle = atan2f(mDiffY, mDiffX) * 180.0f / PI;
        mEndmill = endmill;
        mDisplayListId = 0;
        mStartPos = *from;
        mStartAng = mStepAng = 0;
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
            float endAng = atan2f(mDiffX - to->i, mDiffY - to->j);
            float sweepAng = mStartAng - endAng;
            if (sweepAng < EPSILON)
                sweepAng += PI * 2;
            numRenderSteps = (int)(sweepAng / mStepAng) + 1;
            mStepAng = sweepAng / numRenderSteps;
            mDisplayListId = mEndmill->GenerateArcSegmentDL(radius, mStepAng, 0);
        }
        else
        {
            numRenderSteps = (int)(mXYDistance / resolution);
            if (numRenderSteps == 0)
                numRenderSteps = 1;
            mStepDistance = mXYDistance / numRenderSteps;
            mStepX = mDiffX / numRenderSteps;
            mStepY = mDiffY / numRenderSteps;
            mStepZ = mDiffZ / numRenderSteps;

            if (IsVerticalMotion(from, to)) {
                mMotionType = MTVertical;
                mTarget = *to;
            }
            else {
                mMotionType = MTHorizontal;
                mTarget = *from;
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
        render(numRenderSteps);
    }

    void MillPathSegment::render(int step) 
    {
        glPushMatrix();
        glTranslatef(mTarget.x, mTarget.y, mTarget.z);
        if (mMotionType == MTCurved)
        {
            glTranslatef(mTarget.i, mTarget.j, 0);
            glRotatef(mStartAng - (step - 1) * mStepAng, 0, 0, 1);
            glCallList(mDisplayListId);
        }
        else
        {
            headPos.x = mStartPos.x + mStepX * step;
            headPos.y = mStartPos.y + mStepY * step;
            headPos.z = mStartPos.z + mStepZ * step;
            if (mMotionType == MTVertical)
                glCallList(mEndmill->mToolDisplayId);
            else
            {
                float renderDist = step * mStepDistance;
                glRotatef(mXYAngle, 0, 0, 1);
                glPushMatrix();
                glScalef(renderDist, 1, 1);
                glCallList(mEndmill->mPathDisplayId);
                glPopMatrix();
                glTranslatef(renderDist, 0, 0);
                glCallList(mEndmill->mHToolDisplayId);
            }
        }
        //glCallList(mDisplayListId);
        glPopMatrix();
    }

}