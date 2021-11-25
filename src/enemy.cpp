
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
    userData.pointer = reinterpret_cast<uintptr_t>(this);
    fixtureDef.userData = userData;

    body->CreateFixture(&fixtureDef);
}

void Enemy::reset(int level) {
    // Not possible to SetTransform in a contact callback, defer to next update
    shouldResetWith = level < constants::startIncreasingSpeedAtLevel ?
        1 :
        (level - constants::startIncreasingSpeedAtLevel + 2);
}

void Enemy::update(RdpDisplayList* rdl, b2Vec2 cameraPos, float scale) {
    if (shouldResetWith) {
        float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / constants::gameAreaWidth);
        body->SetTransform(b2Vec2(rx, -constants::swawnSafeRadius), rx);
        b2Vec2 oldVelocity = body->GetLinearVelocity();
        float multiplier = static_cast<float>(shouldResetWith - 1);
        b2Vec2 newVelocity(
            multiplier * ((constants::gameAreaWidth / 2.0f) - rx) / constants::gameAreaWidth,
            multiplier
        );
        // Cap speeds
        newVelocity.x = newVelocity.x > 10.0f ? 10.0f : newVelocity.x;
        newVelocity.y = newVelocity.y > 20.0f ? 20.0f : newVelocity.y;
        body->SetLinearVelocity(newVelocity);
        body->SetAngularVelocity(0.f);
        shouldResetWith = 0;
    }
    Box::update(rdl, cameraPos, scale);
}