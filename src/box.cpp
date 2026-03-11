
extern "C" {
    #include <libdragon.h>
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "box.hpp"

// TODO: override the draw class instead
Edge Box::update(b2Mat33& matrix) {
    b2Vec2 vertex1 = body->GetWorldPoint(polygonShape.m_vertices[1]);
    b2Vec2 vertex2 = body->GetWorldPoint(polygonShape.m_vertices[2]);
    b2Vec2 vertex3 = body->GetWorldPoint(polygonShape.m_vertices[0]);

    b2Vec3 v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
    b2Vec3 v2 = b2Mul(matrix, b2Vec3(vertex2.x, vertex2.y, 1.));
    b2Vec3 v3 = b2Mul(matrix, b2Vec3(vertex3.x, vertex3.y, 1.));

    b2Vec2 min = b2Vec2(0., 0.);
    b2Vec2 max = b2Vec2(
        constants::gameAreaWidth * constants::scale,
        constants::gameAreaHeight * (constants::scale/2.0f)
    );

    vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);
    vertex2 = b2Clamp(b2Vec2(v2.x, v2.y), min, max);
    vertex3 = b2Clamp(b2Vec2(v3.x, v3.y), min, max);

    // TODO: clamp to valid range or we get exceptions for large values when
    // converting to integer
    rdpq_triangle(&TRIFMT_FILL,
        &vertex1.x,
        &vertex2.x,
        &vertex3.x);

    vertex1 = body->GetWorldPoint(polygonShape.m_vertices[3]);

    v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
    vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);

    rdpq_triangle(&TRIFMT_FILL,
        &vertex2.x,
        &vertex3.x,
        &vertex1.x);

    return {
        vertex3,
        vertex1
    };
}
