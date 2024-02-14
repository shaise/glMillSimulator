#ifndef __flat_mill_primitive_h__
#define __flat_mill_primitive_h__


#include "MillOperation.h"
#include "EndMill.h"

namespace MillSim {

    enum MotionType
    {
        MTVertical = 0,
        MTHorizontal,
        MTCurved
    };

    class MillPathSegment
    {
    public:        
        /// <summary>
        /// Create a flat mill primitive
        /// </summary>
        /// <param name="diam">Mill diameter</param>
        /// <param name="from">Start point</param>
        /// <param name="to">End point</param>
        MillPathSegment(EndMill *endmill, MillMotion *from, MillMotion *to);
        virtual ~MillPathSegment();


        /// Calls the display list.
        virtual void render();

    public:
        EndMill* mEndmill = nullptr;
        int numRenderSteps;

    protected:

    protected:
         unsigned int mDisplayListId;
         float mXYDistance;
         float mXYAngle;
         MillMotion mTarget;
         MotionType mMotionType;
    };
}

#endif
