#ifndef __ROPE_H
#define __ROPE_H

#include "box2d/box2d.h"
#include "box2d/b2_rope.h"


extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Rope
{
    private:
        b2RopeDef ropeDef;
        b2Rope rope;
        b2RopeTuning ropeTuning;

        int32 velocityIterations = 6;
        int32 positionIterations = 2;
        float timeStep = 1.0f / 60.0f;

    public:
        Rope(int count, b2Vec2 pos1, b2Vec2 pos2);
        b2Vec2 update(b2Vec2 pos1, b2Vec2 pos2);
        void draw(RdpDisplayList* rdl, float tension);
        void reset();
};


#endif /* __ROPE_H */
