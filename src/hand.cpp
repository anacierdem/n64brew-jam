
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "hand.hpp"

Hand::Hand(b2World* world) : Box(world) {
    body = world->CreateBody(&bodyDef);

    dynamicBox.SetAsBox(0.2f, 0.2f);

    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    b2Filter filter;
    filter.categoryBits = CollisionCategory::hand;
    filter.maskBits = CollisionCategory::enemy | CollisionCategory::environment | CollisionCategory::blade;
    fixtureDef.filter = filter;

    body->CreateFixture(&fixtureDef);
}

void Hand::update(RdpDisplayList* rdl, b2Mat33& matrix, bool held) {
    b2Fixture* fixture = body->GetFixtureList();
    if (held) {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 128)));
        fixtureDef.filter.maskBits = CollisionCategory::enemy | CollisionCategory::environment | CollisionCategory::blade;
        fixture->SetFilterData(fixtureDef.filter);
    } else {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 50)));
        fixtureDef.filter.maskBits = CollisionCategory::environment;
        fixture->SetFilterData(fixtureDef.filter);
    }
    Box::update(rdl, matrix);
}
