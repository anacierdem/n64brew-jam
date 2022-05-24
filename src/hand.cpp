
extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

#include "box2d/box2d.h"

#include "game.hpp"
#include "hand.hpp"

Hand::Hand(b2World* world) : Box() {
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);

    float size = 0.3f;
    b2Vec2 vertices[3] = {{ 0.0f * size, -1.0f * size},{-0.866f * size, 0.5f * size}, {0.866f * size, 0.5f * size}};
    polygonShape.Set(vertices, 3);

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

    // wav64_open(&hitTaken[0], "hit1.wav64");
    // wav64_open(&hitTaken[1], "hit2.wav64");
    // wav64_open(&hitTaken[2], "hit3.wav64");
}

void Hand::update(b2Mat33& matrix, bool held) {
    b2Fixture* fixture = body->GetFixtureList();
    int64_t animationTime = timer_ticks() - startedShowingDamage;

    if (animationTime < 0 || animationTime > TICKS_FROM_MS(800)) {
        if (held) {
            rdpq_set_fill_color(RGBA32(255, 255, 255, 128));
            fixtureDef.filter.maskBits = CollisionCategory::enemy | CollisionCategory::environment | CollisionCategory::blade;
            fixture->SetFilterData(fixtureDef.filter);
        } else {
            rdpq_set_fill_color(RGBA32(255, 255, 255, 50));
            fixtureDef.filter.maskBits = CollisionCategory::environment;
            fixture->SetFilterData(fixtureDef.filter);
        }
    } else if (animationTime < TICKS_FROM_MS(200)) {
        rdpq_set_fill_color(RGBA32(255, 0, 0, 255));
    } else if (animationTime < TICKS_FROM_MS(400)){
        rdpq_set_fill_color(RGBA32(255, 255, 255, 255));
    } else if (animationTime < TICKS_FROM_MS(600)){
        rdpq_set_fill_color(RGBA32(255, 0, 0, 255));
    } else {
        rdpq_set_fill_color(RGBA32(255, 255, 255, 50));
    }

    // TODO: use common logic
    b2Vec2 vertex1 = body->GetWorldPoint(polygonShape.m_vertices[0]);
    b2Vec2 vertex2 = body->GetWorldPoint(polygonShape.m_vertices[1]);
    b2Vec2 vertex3 = body->GetWorldPoint(polygonShape.m_vertices[2]);

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

    render_tri_strip(
        vertex1.x, vertex1.y,
        vertex2.x, vertex2.y,
        vertex3.x, vertex3.y
    );
}

bool Hand::takeDamage() {
    int64_t gracePeriod = timer_ticks() - startedShowingDamage;
    if (gracePeriod < 0 || gracePeriod > TICKS_FROM_MS(constants::gracePeriodMs)) {
        startedShowingDamage = timer_ticks();

        float normalized = rand() / std::nextafter(float(RAND_MAX), FLT_MAX);
        int id = int(normalized * 3);

        mixer_ch_play(constants::hitTakenChannel, &hitTaken[id].wave);
        return true;
    };
    return false;
}
