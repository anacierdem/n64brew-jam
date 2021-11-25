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
    public:
        Enemy(b2World* world) ;
};


#endif /* __ENEMY_H */
