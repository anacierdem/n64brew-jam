
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "blade.hpp"

Blade::Blade(b2World* world) : Box(world) {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);

    polygonShape.SetAsBox(0.4f, 0.4f);

    fixtureDef.shape = &polygonShape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    fixtureDef.restitution = 0.5f;

    b2Filter filter;
    filter.categoryBits = CollisionCategory::blade;
    filter.maskBits = CollisionCategory::enemy | CollisionCategory::hand;
    fixtureDef.filter = filter;

    body->CreateFixture(&fixtureDef);
}

void Blade::update(RdpDisplayList* rdl, b2Mat33& matrix) {
    rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(80, 80, 80, 255)));
    Box::update(rdl, matrix);
}