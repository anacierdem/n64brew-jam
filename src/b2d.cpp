#include <libdragon.h>

#include "box2d/box2d.h"

b2Vec2 gravity(0.0f, -10.0f);
b2World world(gravity);


float timeStep = 1.0f / 60.0f;

int32 velocityIterations = 6;
int32 positionIterations = 2;

extern "C" {
    void test_func(void) {
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(0.0f, -10.0f);

        b2Body* groundBody = world.CreateBody(&groundBodyDef);

        b2PolygonShape groundBox;
        groundBox.SetAsBox(50.0f, 10.0f);


        groundBody->CreateFixture(&groundBox, 0.0f);



        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position.Set(0.0f, 4.0f);
        b2Body* body = world.CreateBody(&bodyDef);


        b2PolygonShape dynamicBox;
        dynamicBox.SetAsBox(1.0f, 1.0f);


        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;


        body->CreateFixture(&fixtureDef);


        world.Step(timeStep, velocityIterations, positionIterations);
        b2Vec2 position = body->GetPosition();
        float angle = body->GetAngle();
        debugf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
    }
}
