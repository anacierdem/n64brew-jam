#include "enemy.hpp"

Enemy::Enemy(b2World* world) : Box(){
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    body = world->CreateBody(&bodyDef);

    polygonShape.SetAsBox(0.2f, 0.2f);

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

void Enemy::die(int level, int s) {
    score = s;
    showingScore = s > 0;

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
        float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX / constants::gameAreaWidth);
        body->SetTransform(b2Vec2(rx, -constants::swawnSafeRadius), rx);
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

    if (timer_ticks() > startedShowingScore + TICKS_FROM_MS(300)) {
        showingScore = false;
    }

    Box::update(rdl, matrix);
}