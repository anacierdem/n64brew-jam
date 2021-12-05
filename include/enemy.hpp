#ifndef __ENEMY_H
#define __ENEMY_H

#include <stdlib.h>
#include <cmath>

#include "box2d/box2d.h"

#include "box.hpp"
#include "game.hpp"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

enum enemyDamageType {
    health = 1,
    regularEnemy = -1,
};

class Enemy : public Box
{
    private:
        int shouldResetWith = 0;
        int64_t startedShowingScore = 0;
        int64_t delay = 0;

        enemyDamageType type = regularEnemy;

    public:
        Enemy(b2World* world);
        void die(int multiplier, int score, bool isDead, float maxDelay);
        void update(RdpDisplayList* rdl, b2Mat33& matrix);
        enemyDamageType getDamageType() const;

        // Scoring
        int score;
        b2Vec2 scorePosition;
        bool showingScore = false;
};


#endif /* __ENEMY_H */
