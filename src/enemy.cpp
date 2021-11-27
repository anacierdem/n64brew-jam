#include "enemy.hpp"

Enemy::Enemy(b2World* world) : Box(){
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);

    // float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.3f);

    float size = 0.2f; //  * (0.8f + rx)

    float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.03f);
    float r2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.03f);
    float r3 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.03f);
    float r4 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / 0.03f);

    b2Vec2 vertices[4] = {
        { 1.0f * size + r1, 1.0f * size + r1},
        {-1.0f * size + r2, 1.0f * size  + r2},
        {-1.0f * size  + r3, -1.0f * size + r3},
        {1.0f * size + r4, -1.0f * size + r4}
    };
    polygonShape.Set(vertices, 4);
    // polygonShape.SetAsBox(0.2f * (0.8f + rx), 0.2f * (0.8f + rx));

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
}

void Enemy::die(int level, int s, bool gameOver) {
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
            scorePosition.y >
                (constants::gameAreaHeight - constants::swawnSafeRadius) ?
                ((constants::scale/2.0f) * constants::gameAreaHeight - 12) :
                (constants::scale/2.0f) * scorePosition.y
        );

        startedShowingScore = timer_ticks();
    }

    // Not possible to SetTransform in a contact callback, defer to next update
    shouldResetWith = level < constants::startIncreasingSpeedAtLevel ?
        1 :
        (level - constants::startIncreasingSpeedAtLevel + 2);
}

void Enemy::update(RdpDisplayList* rdl, b2Mat33& matrix) {
    if (shouldResetWith) {
        body->SetEnabled(true);
        float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / constants::gameAreaWidth);
        float rx2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        body->SetTransform(b2Vec2(rx, -constants::swawnSafeRadius -2.0f * rx2), rx);
        float multiplier = static_cast<float>(shouldResetWith - 1) * 0.2f;
        b2Vec2 newVelocity(
            multiplier * (rx2 - 0.5f),
            multiplier
        );
        // Cap speeds
        newVelocity.x = newVelocity.x > 10.0f ? 10.0f : newVelocity.x;
        newVelocity.y = newVelocity.y > 20.0f ? 20.0f : newVelocity.y;
        body->SetLinearVelocity(newVelocity);
        body->SetAngularVelocity((rx2 - 0.5f) * (3.0f * (1.0f + multiplier)));
        shouldResetWith = 0;

        type = rx2 < constants::healthRate ? health : regularEnemy;
    }

    if (timer_ticks() > startedShowingScore + TICKS_FROM_MS(constants::gracePeriodMs)) {
        showingScore = false;
    }

    if (type == health) {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(100, 255, 100, 100)));
    } else {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(120, 100, 100, 255)));
    }
    Box::update(rdl, matrix);
}

enemyDamageType Enemy::getDamageType() const {
    return type;
}