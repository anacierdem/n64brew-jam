#include "enemy.hpp"

Enemy::Enemy(b2World* world) : Box(){
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);

#ifdef SQUARE_ENEMIES
    // float rx = 0.3f - static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    float size = 0.18f; //  * (0.8f + rx)

    float r1 = 0.03f * static_cast<float>(rand()) / RAND_MAX;
    float r2 = 0.03f * static_cast<float>(rand()) / RAND_MAX;
    float r3 = 0.03f * static_cast<float>(rand()) / RAND_MAX;
    float r4 = 0.03f * static_cast<float>(rand()) / RAND_MAX;

    b2Vec2 vertices[4] = {
        { 1.0f * (size + r1), 1.0f * (size + r1)},
        {-1.0f * (size + r2), 1.0f * (size  + r2)},
        {-1.0f * (size  + r3), -1.0f * (size + r3)},
        {1.0f * (size + r4), -1.0f * (size + r4)}
    };
    polygonShape.Set(vertices, 4);
    // polygonShape.SetAsBox(0.2f * (0.8f + rx), 0.2f * (0.8f + rx));
#else
    float size = 0.25f;

    float r1 = 0.05f * static_cast<float>(rand()) / RAND_MAX;
    float r2 = 0.05f * static_cast<float>(rand()) / RAND_MAX;
    float r3 = 0.05f * static_cast<float>(rand()) / RAND_MAX;
    float r4 = 0.05f * static_cast<float>(rand()) / RAND_MAX;

    b2Vec2 vertices[5] = {
        {1.00000f* (size + r1), 0.00000f},
        {0.30902f* (size + r2), 0.95106f* (size + r2)},
        {-0.80902f* (size + r3), 0.58779f* (size + r3)},
        {-0.80902f* (size + r4), -0.58779f* (size + r4)},
        {0.30902f* (size), -0.95106f* (size)},
    };
    polygonShape.Set(vertices, 5);
#endif

    fixtureDef.shape = &polygonShape;
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

    // Make sure out of sreen
    body->SetTransform(b2Vec2(-constants::spawnSafeRadius, -constants::spawnSafeRadius), 0.0f);
    body->SetEnabled(false);
}

// Kill the enemy and enqueue for a delayed reset
void Enemy::die(int level, int s, bool gameOver, float maxDelay) {
    score = s;
    showingScore = s > 0;

    if (gameOver) {
        fixtureDef.filter.maskBits = CollisionCategory::enemy | CollisionCategory::hand | CollisionCategory::blade | CollisionCategory::environment;
    } else {
        fixtureDef.filter.maskBits = CollisionCategory::enemy | CollisionCategory::hand | CollisionCategory::blade;
    }
    b2Fixture* fixture = body->GetFixtureList();
    fixture->SetFilterData(fixtureDef.filter);

    if (showingScore) {
        scorePosition = body->GetPosition();
        scorePosition = b2Vec2(
            constants::scale * scorePosition.x,
            (constants::scale / 2.0f) * scorePosition.y
        );

        startedShowingScore = timer_ticks();
    }

    // Not possible to SetTransform in a contact callback, defer to next update
    shouldResetWith = level < constants::startIncreasingSpeedAtLevel ?
        1 :
        (level - constants::startIncreasingSpeedAtLevel + 1);

    // Wait before respawning
    delay = timer_ticks() + (TICKS_FROM_MS(maxDelay * 1000.0f * (static_cast<float>(rand()) / RAND_MAX)));
}

void Enemy::update(RdpDisplayList* rdl, b2Mat33& matrix) {
    if (shouldResetWith) {
        // Disable once dead but wait before respan
        body->SetEnabled(false);

        if (timer_ticks() > delay) {
            body->SetEnabled(true);
            float rx = constants::gameAreaWidth * static_cast<float>(rand()) / RAND_MAX;
            float rx2 = static_cast<float>(rand()) / RAND_MAX;
            body->SetTransform(b2Vec2(rx, -constants::spawnSafeRadius - (constants::gameAreaHeight / 6.0f) * rx2), rx);
            float multiplier = static_cast<float>(shouldResetWith - 1) * constants::speedPerLevel;
            b2Vec2 newVelocity(
                multiplier * (rx2 - 0.5f),
                multiplier
            );
            body->SetLinearVelocity(newVelocity);
            body->SetAngularVelocity((rx2 - 0.5f) * (3.0f * (1.0f + multiplier)));
            shouldResetWith = 0;

            type = rx2 < constants::healthRate ? health : regularEnemy;
        }
    }


    if (timer_ticks() > startedShowingScore + TICKS_FROM_MS(constants::gracePeriodMs)) {
        showingScore = false;
    }

    if (type == health) {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(100, 255, 100, 100)));
    } else {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(150, 68, 201, 180)));
    }
    if (body->IsEnabled()) {
        Box::update(rdl, matrix);

#ifndef SQUARE_ENEMIES
        b2Vec2 min = b2Vec2(0., 0.);
        b2Vec2 max = b2Vec2(
            constants::gameAreaWidth * constants::scale * constants::to16_16,
            constants::gameAreaHeight * (constants::scale/2.0f) * constants::to16_16
        );

        b2Vec2 vertex1 = body->GetWorldPoint(polygonShape.m_vertices[4]);

        b2Vec3 v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
        vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);

        render_tri_strip_next(rdl, vertex1.x, vertex1.y);
#endif
    }
}

enemyDamageType Enemy::getDamageType() const {
    return type;
}