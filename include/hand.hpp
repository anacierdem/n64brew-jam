#ifndef __HAND_H
#define __HAND_H

#include "box2d/box2d.h"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Hand
{
    private:
        b2BodyDef bodyDef;
        b2PolygonShape dynamicBox;
        b2FixtureDef fixtureDef;

    public:
        b2Body* body;
        Hand(b2World* world);
        void update(RdpDisplayList* rdl, b2Vec2 cameraPos, float scale);
};


#endif /* __HAND_H */
