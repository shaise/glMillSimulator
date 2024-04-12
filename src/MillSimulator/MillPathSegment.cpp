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

    float resolution = 1;

    MillPathSegment::MillPathSegment(EndMill *endmill, MillMotion* from, MillMotion* to)
    {
        mDiff.x = to->x - from->x;
        mDiff.y = to->y - from->y;
        mDiff.z = to->z - from->z;
        mXYDistance = sqrtf(mDiff.x * mDiff.x+ mDiff.y * mDiff.y);
        mZDistance = fabs(mDiff.z);
        mXYZDistance = sqrtf(mXYDistance * mXYDistance + mDiff.z * mDiff.z);
        mXYAngle = atan2f(mDiff.y, mDiff.x) * 180.0f / PI;
        mEndmill = endmill;
        mDisplayListId = 0;
        mStartPos.FromMillMotion(from);
        mStartAngRad = mStepAngRad = 0;
        if (IsArcMotion(to))
        {
            mMotionType = MTCurved;
            mRadius = sqrtf(to->j * to->j + to->i * to->i);
            mStepAngRad = asinf(resolution / mRadius);
            mCenter.FromMillMotion(from);
            mCenter.x += to->i;
            mCenter.y += to->j;
            if (mStepAngRad > MAX_SEG_DEG)
                mStepAngRad = MAX_SEG_DEG;
            else if (mStepAngRad < NIN_SEG_DEG)
                mStepAngRad = NIN_SEG_DEG;
            mStartAngRad = atan2f(mCenter.x - from->x, from->y - mCenter.y);
            float endAng = atan2f(mCenter.x - to->x, to->j - mCenter.y);
            float sweepAng = mStartAngRad - endAng;
            if (sweepAng < EPSILON)
                sweepAng += PI * 2;
            numRenderSteps = (int)(sweepAng / mStepAngRad) + 1;
            mStepAngRad = sweepAng / numRenderSteps;
            mDisplayListId = mEndmill->GenerateArcSegmentDL(mRadius, mStepAngRad+0.02, 0);
            mStartAngDeg = mStartAngRad * 180.0f / PI;
            mStepAngDeg = mStepAngRad * 180.0f / PI;
        }
        else
        {
            numRenderSteps = (int)(mXYZDistance / resolution);
            if (numRenderSteps == 0)
                numRenderSteps = 1;
            mStepDistance = mXYDistance / numRenderSteps;
            mStepLength = mDiff;
            mStepLength.Div(numRenderSteps);

            if (IsVerticalMotion(from, to)) {
                mMotionType = MTVertical;
            }
            else {
                mMotionType = MTHorizontal;
            }
        }
    }

    MillPathSegment::~MillPathSegment()
    {
        if (mDisplayListId > 0)
            glDeleteLists(mDisplayListId, 1);
    }

    void MillPathSegment::render(bool isReversed)
    {
        render(numRenderSteps, isReversed);
    }

    void MillPathSegment::render(int step, bool isReversed) 
    {
        mStepNumber = step;
        glPushMatrix();
        if (mMotionType == MTCurved)
        {
            glTranslatef(mCenter.x, mCenter.y, mCenter.z);
            for (int i = 0; i < step; i++)
            {
                glPushMatrix();
                int n = isReversed ? step - i - 1 : i;
                glRotatef(mStartAngDeg - n * mStepAngDeg, 0, 0, 1);
                glCallList(mDisplayListId);
                glPopMatrix();
            }
        }
        else
        {
            if (mMotionType == MTVertical) {
                if (mStepLength.z > 0)
                    glTranslatef(mStartPos.x, mStartPos.y, mStartPos.z);
                else
                    glTranslatef(mStartPos.x, mStartPos.y, mStartPos.z + mStepNumber * mStepLength.z);
                glCallList(mEndmill->mToolDisplayId);
            }
            else
            {
                float renderDist = step * mStepDistance;
                glTranslatef(mStartPos.x, mStartPos.y, mStartPos.z);
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

    Vector3* MillPathSegment::GetHeadPosition()
    {
        if (mMotionType == MTCurved)
        {
            float angRad = mStartAngRad - mStepNumber * mStepAngRad;
            mHeadPos.Set(-mRadius * sinf(angRad), mRadius * cosf(angRad), 0);
            mHeadPos.Add(&mCenter);
        }
        else
        {
            mHeadPos = mStepLength;
            mHeadPos.Mul(mStepNumber);
            mHeadPos.Add(&mStartPos);
        }
        return &mHeadPos;
    }

}