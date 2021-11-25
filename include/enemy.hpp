#ifndef __ENEMY_H
#define __ENEMY_H

#include "box2d/box2d.h"

#include "box.hpp"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Enemy : public Box
{
    private:
        int shouldResetWith = 0;
    public:
        Enemy(b2World* world);
        void reset(int multiplier);
        void update(RdpDisplayList* rdl, b2Vec2 cameraPos, float scale);
};


#endif /* __ENEMY_H */
