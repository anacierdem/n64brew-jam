#include "b2d.hpp"

#include "box2d/box2d.h"

float to16_16 = 65536.f;
#include "box.cpp"

b2Vec2 gravity(0.0f, 10.0f);
b2World world(gravity);
Box box(&world);

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"

    Game::Game(RdpDisplayList* rdlParam)
    {
        rdl = rdlParam;

        groundBodyDef.position.Set(0.0f, 16.0f);
        groundBody = world.CreateBody(&groundBodyDef);
        groundBox.SetAsBox(50.0f, 10.0f);
        groundBody->CreateFixture(&groundBox, 0.0f);
    };

    int Game::update() {
        world.Step(timeStep, velocityIterations, positionIterations);

        box.update(rdl);
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
