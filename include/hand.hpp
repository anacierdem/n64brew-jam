#ifndef __HAND_H
#define __HAND_H

#include <limits>

#include "box2d/box2d.h"

#include "box.hpp"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Hand : public Box
{
    private:
        int64_t startedShowingDamage = std::numeric_limits<int64_t>::min();
        // Audio
        wav64_t hitTaken[3];
    public:
        Hand(b2World* world);
        void update(RdpDisplayList* rdl, b2Mat33& matrix, bool held);
        bool takeDamage(RdpDisplayList* rdl);
};


#endif /* __HAND_H */
