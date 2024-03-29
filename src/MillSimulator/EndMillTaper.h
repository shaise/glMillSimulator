#ifndef __end_mill_taper_h__
#define __end_mill_taper_h__

#include "EndMill.h"

#define TAPER_MILL_PROFILE_VERTS 4
class EndMillTaper :
    public EndMill
{
public:
    EndMillTaper(float radius, int nslices, float TaperAngle, float flatRadius);

private:
    float _profVerts[PROFILE_BUFFER_SIZE(TAPER_MILL_PROFILE_VERTS)];
};

#endif
