#include "b2d.hpp"

#include <libdragon.h>
#include "box2d/box2d.h"

extern "C" {
    b2Vec2 gravity(0.0f, -10.0f);
    b2World world(gravity);

    Physics::Physics()
    {
        groundBodyDef.position.Set(0.0f, -10.0f);
        groundBody = world.CreateBody(&groundBodyDef);
        groundBox.SetAsBox(50.0f, 10.0f);
        groundBody->CreateFixture(&groundBox, 0.0f);

        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(0.0f, 4.0f);
        body = world.CreateBody(&bodyDef);
        dynamicBox.SetAsBox(1.0f, 1.0f);

        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;
        body->CreateFixture(&fixtureDef);
    };

    int Physics::update(float delta) {
        world.Step(delta, velocityIterations, positionIterations);
        b2Vec2 position = body->GetPosition();
        float angle = body->GetAngle();
        debugf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
        return 0;
    }

    // Physics testPhysics;
}

EXPORT_C Physics* new_Physics(void)
{
    return new Physics();
}

EXPORT_C void delete_Physics(Physics* self)
{
    delete self;
}

EXPORT_C int update_Physics(Physics* self, float delta)
{
    return self->update(delta);
}
