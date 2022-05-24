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
        b2PolygonShape polygonShape;
        b2FixtureDef fixtureDef;

    public:
        b2Body* body;
        Box() {};
        void update(b2Mat33& matrix);
};


#endif /* __BOX_H */
