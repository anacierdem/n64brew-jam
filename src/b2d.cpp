#include "b2d.hpp"

#include "box2d/box2d.h"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

extern "C" {
    b2Vec2 gravity(0.0f, -10.0f);
    b2World world(gravity);

    Game::Game(RdpDisplayList* rdlParam)
    {
        rdl = rdlParam;

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

    int Game::update() {
        world.Step(timeStep, velocityIterations, positionIterations);
        b2Vec2 position = body->GetPosition();
        float angle = body->GetAngle();
        debugf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);

        float to16_16 = 65536.f;

        float x = position.x;
        float y = (4. - position.y) * 60.;
        render_tri(rdl,
            (50. + x) * to16_16,    (50. + y) * to16_16,
            (250. + x) * to16_16,   (50. + y) * to16_16,
            (150. + x) * to16_16,   (0. + y) * to16_16
        );

        return 0;
    }

    Game* new_Game(RdpDisplayList* rdl)
    {
        return new Game(rdl);
    }

    void delete_Game(Game* self)
    {
        delete self;
    }

    int update_Game(Game* self)
    {
        return self->update();
    }

}
