#ifndef __end_mill_ball_h__
#define __end_mill_ball_h__

#include "EndMill.h"

class EndMillBall :
    public EndMill
{
public:
    EndMillBall(float radius, int nslices, int nSections, float flatRadius);
    virtual ~EndMillBall();

};

#endif
