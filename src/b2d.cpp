#include "b2d.hpp"

#include "box2d/box2d.h"

float to16_16 = 65536.f;
#include "box.cpp"

b2Vec2 gravity(0.0f, 10.0f);
b2World world(gravity);

b2Transform box1Transform;
b2Transform box2Transform;

Box box1(&world);
Box box2(&world);

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
    }

    int Game::update(controller_data keys) {
        for( int i = 0; i < 4; i++ )
        {
            if( keys.c[i].A )
            {
                this->reset();
            }

            // if( keys.c[i].B )
            // {
            //     this->reset();
            // }

            // if( keys.c[i].left )
            // {
            //     box1Transform.Set(box1Transform.p,  box1Transform.q.GetAngle() + 0.01f);
            //     this->reset();
            // }
            // if( keys.c[i].right )
            // {
            //     box1Transform.Set(box1Transform.p,  box1Transform.q.GetAngle() - 0.01f);
            //     this->reset();
            // }
        }

        world.Step(timeStep, velocityIterations, positionIterations);

        box1.update(rdl);
        box2.update(rdl);
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

    int update_Game(Game* self, controller_data keys)
    {
        return self->update(keys);
    }
}
