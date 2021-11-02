#include "b2d.hpp"

#include "box2d/box2d.h"

float to16_16 = 65536.f;
#include "box.cpp"
#include "rope.cpp"

b2Vec2 gravity(0.0f, 10.0f);
b2World world(gravity);

b2Transform box1Transform;
b2Transform box2Transform;

Box box1(&world);
Box box2(&world);
Rope rope1(5, b2Vec2(4.0f, 0.5f));
Rope rope2(10, b2Vec2(2.0f, 0.5f));

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"

    Game::Game(RdpDisplayList* rdlParam)
    {
        rdl = rdlParam;

        box1Transform.Set(b2Vec2(2.0f, 1.0f), 1.0f);
        box2Transform.Set(b2Vec2(6.0f, 1.0f), -1.2f);

        groundBodyDef.position.Set(0.0f, 15.9f);
        groundBody = world.CreateBody(&groundBodyDef);
        groundBox.SetAsBox(50.0f, 10.0f);
        groundBody->CreateFixture(&groundBox, 0.0f);
    };

    void Game::reset() {
        box1.body->SetTransform(box1Transform.p, box1Transform.q.GetAngle());
        box1.body->SetAwake(true);
        box2.body->SetTransform(box2Transform.p, box2Transform.q.GetAngle());
        box2.body->SetAwake(true);
        box1.body->SetLinearVelocity(b2Vec2(0., 0.));
        box1.body->SetAngularVelocity(0.);
        box2.body->SetLinearVelocity(b2Vec2(0., 0.));
        box2.body->SetAngularVelocity(0.);

        rope1.reset();
        rope2.reset();
    }

    int Game::update(int controllers, controller_data keys) {
        if((controllers & CONTROLLER_1_INSERTED)) {
            debugf("x: %d y: %d\n", keys.c[0].x, keys.c[0].y);
            box1.body->ApplyForceToCenter(b2Vec2((float)keys.c[0].x / 2.0f, -(float)keys.c[0].y / 2.0f) , true);
            if( keys.c[0].A )
            {
                this->reset();
            }
        }


        if((controllers & CONTROLLER_2_INSERTED)) {
            if( keys.c[1].A )
            {
                this->reset();
            }
        }


        world.Step(timeStep, velocityIterations, positionIterations);

        box1.update(rdl);
        box2.update(rdl);
        rope1.update(rdl);
        rope2.update(rdl);
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

    int update_Game(Game* self, int controllers, controller_data keys)
    {
        return self->update(controllers, keys);
    }
}
