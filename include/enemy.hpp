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
        int64_t startedShowingScore = 0;
    public:
        Enemy(b2World* world);
        void die(int multiplier, int score);
        void update(RdpDisplayList* rdl, b2Mat33& matrix);

        b2Vec2 scorePosition;
        bool showingScore = false;
        int score;
};


#endif /* __ENEMY_H */
