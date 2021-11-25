#ifndef __BOX_H
#define __BOX_H

#include "box2d/box2d.h"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Box
{
    protected:
        b2BodyDef bodyDef;
        b2PolygonShape dynamicBox;
        b2FixtureDef fixtureDef;

    public:
        b2Body* body;
        Box(b2World* world);
        void update(RdpDisplayList* rdl, b2Vec2 cameraPos);
};


#endif /* __BOX_H */
