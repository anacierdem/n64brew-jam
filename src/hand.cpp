
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "hand.hpp"

Hand::Hand(b2World* world) : Box(world) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);

    polygonShape.SetAsBox(0.2f, 0.2f);

    fixtureDef.shape = &polygonShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    b2Filter filter;
    filter.categoryBits = CollisionCategory::hand;
    filter.maskBits = CollisionCategory::enemy | CollisionCategory::environment | CollisionCategory::blade;
    fixtureDef.filter = filter;

    b2FixtureUserData userData;
    userData.pointer = reinterpret_cast<uintptr_t>(this);
    fixtureDef.userData = userData;

    body->CreateFixture(&fixtureDef);
}

void Hand::update(RdpDisplayList* rdl, b2Mat33& matrix, bool held) {
    b2Fixture* fixture = body->GetFixtureList();
    int64_t animationTime = timer_ticks() - startedShowingDamage;

    if (animationTime < 0 || animationTime > TICKS_FROM_MS(800)) {
        if (held) {
            rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 128)));
            fixtureDef.filter.maskBits = CollisionCategory::enemy | CollisionCategory::environment | CollisionCategory::blade;
            fixture->SetFilterData(fixtureDef.filter);
        } else {
            rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 50)));
            fixtureDef.filter.maskBits = CollisionCategory::environment;
            fixture->SetFilterData(fixtureDef.filter);
        }
    } else if (animationTime < TICKS_FROM_MS(200)) {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 0, 0, 255)));
    } else if (animationTime < TICKS_FROM_MS(400)){
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 255)));
    } else if (animationTime < TICKS_FROM_MS(600)){
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 0, 0, 255)));
    } else {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 255)));
    }
    Box::update(rdl, matrix);
}

void Hand::takeDamage(RdpDisplayList* rdl) {
    startedShowingDamage = timer_ticks();
}
