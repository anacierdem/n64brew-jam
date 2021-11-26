
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "box.hpp"

// TODO: override the draw class instead
void Box::update(RdpDisplayList* rdl, b2Mat33& matrix) {
    b2Vec2 vertex1 = body->GetWorldPoint(polygonShape.m_vertices[0]);
    b2Vec2 vertex2 = body->GetWorldPoint(polygonShape.m_vertices[1]);
    b2Vec2 vertex3 = body->GetWorldPoint(polygonShape.m_vertices[3]);

    b2Vec3 v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
    b2Vec3 v2 = b2Mul(matrix, b2Vec3(vertex2.x, vertex2.y, 1.));
    b2Vec3 v3 = b2Mul(matrix, b2Vec3(vertex3.x, vertex3.y, 1.));

    b2Vec2 min = b2Vec2(0., 0.);
    b2Vec2 max = b2Vec2(
        constants::gameAreaWidth * constants::scale * constants::to16_16,
        constants::gameAreaHeight * (constants::scale/2.0f) * constants::to16_16
    );

    vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);
    vertex2 = b2Clamp(b2Vec2(v2.x, v2.y), min, max);
    vertex3 = b2Clamp(b2Vec2(v3.x, v3.y), min, max);

    // TODO: clamp to valid range or we get exceptions for large values when
    // converting to integer
    render_tri_strip(rdl,
        vertex1.x, vertex1.y,
        vertex2.x, vertex2.y,
        vertex3.x, vertex3.y
    );

    vertex1 = body->GetWorldPoint(polygonShape.m_vertices[2]);

    v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
    vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);

    render_tri_strip_next(rdl, vertex1.x, vertex1.y);
}
