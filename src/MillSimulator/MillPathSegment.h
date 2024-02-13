#ifndef __flat_mill_primitive_h__
#define __flat_mill_primitive_h__


#include "MillOperation.h"
#include "EndMill.h"

namespace MillSim {

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

    protected:
        void GenerateCylinder(MillMotion* from, MillMotion* to);
        void GeneratePath(MillMotion* from, MillMotion* to);

    protected:
         unsigned int mDisplayListId;
         float mXYDistance;
         float mXYAngle;

    };
}

#endif
