#ifndef __BOX_H
#define __BOX_H

#include "box2d/box2d.h"

extern "C" {
    #include <libdragon.h>
}

struct Edge {
    b2Vec2 v1;
    b2Vec2 v2;
};

class Box
{
    protected:
        b2PolygonShape polygonShape;
        b2FixtureDef fixtureDef;

    public:
        b2Body* body;
        Box() {};
        Edge update(b2Mat33& matrix);
};


#endif /* __BOX_H */
