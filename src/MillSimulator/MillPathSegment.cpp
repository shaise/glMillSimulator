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
        if (IsVerticalMotion(from, to))
            GenerateCylinder(from, to);
        else
            GeneratePath(from, to);
    }

    MillPathSegment::~MillPathSegment()
    {
        glDeleteLists(mDisplayListId, 1);
    }

    void MillPathSegment::render() {
        glCallList(mDisplayListId);
    }

    void MillPathSegment::GenerateCylinder(MillMotion* from, MillMotion* to)
    {
        GLuint id = glGenLists(1);
        glNewList(id, GL_COMPILE);
        glPushMatrix();
        glTranslatef(to->x, to->y, to->z);
        RotateProfile(mEndmill->mProfPoints, mEndmill->mNPoints, 0, 0, mEndmill->mNSlices, false);
        //SolidCylinder(diam / 2, 4, 16, 1);
        glPopMatrix();
        glEndList();
        mDisplayListId = id;
    }

    void MillPathSegment::GeneratePath(MillMotion* from, MillMotion* to)
    {
        int nFullPoints = PROFILE_BUFFER_POINTS(mEndmill->mNPoints);
        GLuint id = glGenLists(1);
        glNewList(id, GL_COMPILE);
        glPushMatrix();
        glTranslatef(from->x, from->y, from->z);
        glRotatef(mXYAngle, 0, 0, 1);
        ExtrudeProfile(mEndmill->mProfPoints, nFullPoints, mXYDistance, 0);
        TesselateProfile(mEndmill->mProfPoints, nFullPoints, 0, 0);
        RotateProfile(mEndmill->mProfPoints, mEndmill->mNPoints, mXYDistance, 0, mEndmill->mNSlices / 2, true);
        glPopMatrix();
        glEndList();

        mDisplayListId = id;
    }


}