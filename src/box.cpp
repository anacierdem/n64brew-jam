
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "box.hpp"

Box::Box(b2World* world) {
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);
}

void Box::update(RdpDisplayList* rdl, b2Vec2 cameraPos, float scale) {
    b2Vec2 vertex1 = body->GetWorldPoint(dynamicBox.m_vertices[0]);
    b2Vec2 vertex2 = body->GetWorldPoint(dynamicBox.m_vertices[1]);
    b2Vec2 vertex3 = body->GetWorldPoint(dynamicBox.m_vertices[3]);

    vertex1 -= cameraPos;
    vertex2 -= cameraPos;
    vertex3 -= cameraPos;

    // TODO: build a simple transform for this
    // TODO: clamp to valid range or we get exceptions for large values
    render_tri_strip(rdl,
        vertex1.x * scale * constants::to16_16, (vertex1.y) * (scale/2.) * constants::to16_16,
        vertex2.x * scale * constants::to16_16, (vertex2.y) * (scale/2.) * constants::to16_16,
        vertex3.x * scale * constants::to16_16, (vertex3.y) * (scale/2.) * constants::to16_16
    );

    vertex1 = body->GetWorldPoint(dynamicBox.m_vertices[2]);
    vertex1 -= cameraPos;

    render_tri_strip_next(rdl, vertex1.x * scale * constants::to16_16, (vertex1.y) * (scale/2.) * constants::to16_16);
}
