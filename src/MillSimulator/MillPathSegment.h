#ifndef __mill_path_segment_h__
#define __mill_path_segment_h__


#include "MillOperation.h"
#include "EndMill.h"

namespace MillSim {

    enum MotionType
    {
        MTVertical = 0,
        MTHorizontal,
        MTCurved
    };

    extern float resolution;


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
        virtual void render(int step);

    public:
        EndMill* mEndmill = nullptr;
        int numRenderSteps;
        int numSimSteps;

    protected:

    protected:
         unsigned int mDisplayListId;
         float mXYDistance;
         float mXYZDistance;
         float mZDistance;
         float mXYAngle;
         float mStartAng;
         float mStepAng;
         MillMotion mTarget;
         MotionType mMotionType;
    };
}

#endif
