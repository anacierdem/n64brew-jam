
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
    filter.maskBits = CollisionCategory::enemy | CollisionCategory::hand | CollisionCategory::blade;

    fixtureDef.filter = filter;

    b2FixtureUserData userData;
    userData.pointer = reinterpret_cast<uintptr_t> (this);
    fixtureDef.userData = userData;

    body->CreateFixture(&fixtureDef);
}

void Enemy::reset() {
    // Not possible to SetTransform in a contact callback, defer to next update
    shouldReset = true;
}

void Enemy::update(RdpDisplayList* rdl, b2Vec2 cameraPos, float scale) {
    if (shouldReset) {
        float rx = static_cast<float> (rand()) / static_cast<float> (RAND_MAX / constants::gameAreaWidth);
        body->SetTransform(b2Vec2(rx, -1.), rx);
        body->SetLinearVelocity(b2Vec2(0.f,0.f));
        body->SetAngularVelocity(0.f);
        shouldReset = false;
    }
    Box::update(rdl, cameraPos, scale);
}