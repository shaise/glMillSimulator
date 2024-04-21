#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "MillPathSegment.h"
#include "SimShapes.h"
#include "linmath.h"
#include "GlUtils.h"

#define N_MILL_SLICES 8
#define SET_TRIPLE(var, idx, x, y, z) {var[idx] = x; var[idx+1] = y; var[idx+2] = z;}
#define SET_TRIPLE_OFFS(var, idx, offs, x, y, z) {var[idx] = x + offs; var[idx+1] = y + offs; var[idx+2] = z + offs;}
#define MAX_SEG_DEG (PI / 8.0f)   // 22.5 deg
#define NIN_SEG_DEG (PI / 90.0f)  // 2 deg
#define SWEEP_ARC_PAD 1.05f



namespace MillSim {

    bool IsVerticalMotion(MillMotion* m1, MillMotion* m2)
    {
        return (m1->z != m2->z && EQ_FLOAT(m1->x, m2->x) && EQ_FLOAT(m1->y, m2->y));
    }

    bool IsArcMotion(MillMotion* m)
    {
        if (m->cmd != eRotateCCW && m->cmd != eRotateCW)
            return false;
        return fabs(m->i > EPSILON) || fabs(m->j) > EPSILON;
    }

    float resolution = 1;

    MillPathSegment::MillPathSegment(EndMill *endmill, MillMotion* from, MillMotion* to)
        : mShearMat {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        }
    {
        
        mDiff.x = to->x - from->x;
        mDiff.y = to->y - from->y;
        mDiff.z = to->z - from->z;
        mXYDistance = sqrtf(mDiff.x * mDiff.x+ mDiff.y * mDiff.y);
        mZDistance = fabs(mDiff.z);
        mXYZDistance = sqrtf(mXYDistance * mXYDistance + mDiff.z * mDiff.z);
        mXYAngle = atan2f(mDiff.y, mDiff.x);
        mEndmill = endmill;
        mStartPos.FromMillMotion(from);
        mStartAngRad = mStepAngRad = 0;
        if (IsArcMotion(to))
        {
            mMotionType = MTCurved;
            mRadius = sqrtf(to->j * to->j + to->i * to->i);
            mSmallRad = mRadius <= mEndmill->mRadius;
            mStepAngRad = mSmallRad ? MAX_SEG_DEG : asinf(resolution / mRadius);
            mCenter.FromMillMotion(from);
            mCenter.x += to->i;
            mCenter.y += to->j;
            mArcDir = to->cmd == eRotateCCW ? -1 : 1;
            if (mStepAngRad > MAX_SEG_DEG)
                mStepAngRad = MAX_SEG_DEG;
            else if (mStepAngRad < NIN_SEG_DEG)
                mStepAngRad = NIN_SEG_DEG;
            mStartAngRad = atan2f(mCenter.x - from->x, from->y - mCenter.y);
            float endAng = atan2f(mCenter.x - to->x, to->y - mCenter.y);
            float sweepAng = (mStartAngRad - endAng) * mArcDir;
            if (sweepAng < EPSILON)
                sweepAng += PI * 2;
            numSimSteps = (int)(sweepAng / mStepAngRad) + 1;
            mStepAngRad = mArcDir * sweepAng / numSimSteps;
            if (mSmallRad)
                // when the radius is too small, we just use the tool itself to carve the stock
                mShape = mEndmill->mToolShape;
            else
            {
                mEndmill->GenerateArcSegmentDL(mRadius, mStepAngRad * SWEEP_ARC_PAD, mDiff.z / numSimSteps, &mShape);
                numSimSteps++;
            }
            
            isMultyPart = true;
        }
        else
        {
            numSimSteps = (int)(mXYZDistance / resolution);
            if (numSimSteps == 0)
                numSimSteps = 1;
            isMultyPart = false;
            mStepDistance = mXYDistance / numSimSteps;
            mStepLength = mDiff;
            mStepLength.Div(numSimSteps);

            if (IsVerticalMotion(from, to)) {
                mMotionType = MTVertical;
            }
            else {
                mMotionType = MTHorizontal;
                mShearMat[0][2] = mDiff.z / mXYDistance;
            }
        }
    }

    MillPathSegment::~MillPathSegment()
    {
        mShape.FreeResources();
    }


    void MillPathSegment::render(int step) 
    {
        mStepNumber = step;
        mat4x4 mat, mat2, rmat;
        mat4x4_identity(mat);
        mat4x4_identity(rmat);
        if (mMotionType == MTCurved)
        {
            mat4x4_translate_in_place(mat, mCenter.x, mCenter.y, mCenter.z + mDiff.z * (step - 1) / numSimSteps);
            mat4x4_rotate_Z(mat, mat, mStartAngRad - (step - 1) * mStepAngRad);
            mat4x4_rotate_Z(rmat, rmat, mStartAngRad - (step - 1) * mStepAngRad);
            if (mSmallRad || step == (numSimSteps - 1))
            {
                mat4x4_translate_in_place(mat, 0, mRadius, 0);
                mEndmill->mToolShape.Render(mat, rmat);
            }
            else
                mShape.Render(mat, rmat);
        }
        else
        {
            if (mMotionType == MTVertical) {
                if (mStepLength.z > 0)
                    mat4x4_translate_in_place(mat, mStartPos.x, mStartPos.y, mStartPos.z);
                else
                    mat4x4_translate_in_place(mat, mStartPos.x, mStartPos.y, mStartPos.z + mStepNumber * mStepLength.z);
                mEndmill->mToolShape.Render(mat, rmat);
            }
            else
            {
                float renderDist = step * mStepDistance;
                mat4x4_translate_in_place(mat, mStartPos.x, mStartPos.y, mStartPos.z);
                mat4x4_rotate_Z(mat, mat, mXYAngle);
                mat4x4_rotate_Z(rmat, rmat, mXYAngle);
                mat4x4_dup(mat2, mat);
                if (mDiff.z != 0.0)
                    mat4x4_mul(mat2, mat2, mShearMat);
                mat4x4_scale_aniso(mat2, mat2, renderDist, 1, 1);
                mEndmill->mPathShape.Render(mat2, rmat);
                mat4x4_translate_in_place(mat, renderDist, 0, mDiff.z);
                mEndmill->mHToolShape.Render(mat, rmat);

            }
        }
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