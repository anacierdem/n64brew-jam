
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "enemy.hpp"

Enemy::Enemy(b2World* world) : Box(world){
    dynamicBox.SetAsBox(0.2f, 0.2f);

    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    b2Filter filter;
    filter.categoryBits = CollisionCategory::enemy;
    filter.maskBits = CollisionCategory::enemy | CollisionCategory::hand;

    fixtureDef.filter = filter;

    body->CreateFixture(&fixtureDef);
}
