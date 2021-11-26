#ifndef __HAND_H
#define __HAND_H

#include "box2d/box2d.h"

#include "box.hpp"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Hand : public Box
{
    public:
        Hand(b2World* world);
        void update(RdpDisplayList* rdl, b2Mat33& matrix, bool held);
};


#endif /* __HAND_H */
