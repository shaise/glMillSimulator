#include "MillPathSegment.h"
#include <GL/glut.h>
#include "SimShapes.h"

#define PI 3.1415926
#define N_MILL_SLICES 8
#define SET_TRIPLE(var, idx, x, y, z) {var[idx] = x; var[idx+1] = y; var[idx+2] = z;}
#define SET_TRIPLE_OFFS(var, idx, offs, x, y, z) {var[idx] = x + offs; var[idx+1] = y + offs; var[idx+2] = z + offs;}

namespace MillSim {
    MillPathSegment::MillPathSegment(EndMill *endmill, MillMotion* from, MillMotion* to)
    {
        float diffx = to->x - from->x;
        float diffy = to->y - from->y;
        mXYDistance = sqrtf(diffx * diffx + diffy * diffy);
        mXYAngle = atan2f(diffy, diffx) * 180.0f / PI;
        mEndmill = endmill;
        mDisplayListId = 0;
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

    MillPathSegment::~MillPathSegment()
    {
        if (mDisplayListId > 0)
            glDeleteLists(mDisplayListId, 1);
    }

    void MillPathSegment::render() {
        glPushMatrix();
        glTranslatef(mTarget.x, mTarget.y, mTarget.z);
        if (mMotionType == MTVertical)
            glCallList(mEndmill->mToolDisplayId);
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