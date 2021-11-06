#include "game.hpp"

#include "box2d/box2d.h"

#include "box.hpp"
#include "rope.hpp"

b2Vec2 gravity(0.0f, 1.0f);
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
        if (get_tv_type() == TV_PAL) {
            timeStep = 1.0 / 50.0;
        }

        rdl = rdlParam;

        box1Transform.Set(b2Vec2(2.0f, 1.0f), 1.0f);
        box2Transform.Set(b2Vec2(6.0f, 1.0f), -1.2f);

        groundBodyDef.position.Set(0.0f, 6.0f);
        groundBody = world.CreateBody(&groundBodyDef);
        groundBox.SetAsBox(50.0f, 0.1f);
        groundBody->CreateFixture(&groundBox, 0.0f);

        rope = new Rope(10, box1Transform.p, box2Transform.p);

        for (int i = 0; i < box_count; i ++) {
            boxes[i] = new Box(&world);
        }

        reset();
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

        for (int i = 0; i < box_count; i ++) {
            float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 8.);
            float ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 6.);
            boxes[i]->body->SetTransform(b2Vec2(rx, ry), rx);
        }

        rope->reset();
    }

    int Game::update(int controllers, controller_data keys) {
        if((controllers & CONTROLLER_1_INSERTED)) {
            if( keys.c[0].Z )
            {
                box1.body->SetLinearVelocity(b2Vec2((float)keys.c[0].x / 20.0f, -(float)keys.c[0].y / 20.0f));
            }

            if( keys.c[0].A )
            {
                this->reset();
            }

            // cameraPos.Set(keys.c[0].x, -keys.c[0].y, 1.);
        }

        if((controllers & CONTROLLER_2_INSERTED)) {
            if( keys.c[1].Z )
            {
                box2.body->SetLinearVelocity(b2Vec2((float)keys.c[1].x / 20.0f, -(float)keys.c[1].y / 20.0f));
            }

            if( keys.c[1].A )
            {
                this->reset();
            }
        }

        world.Step(timeStep, velocityIterations, positionIterations);

        // float scale = b2Abs((box1.body->GetPosition() - box2.body->GetPosition()).x) * 160.;
        float scale = 80.;

        b2Vec2 cPos(cameraPos.x / scale, cameraPos.y / (scale/2.));

        b2Mat33 mainM(
            b2Vec3(scale * constants::to16_16, 0., 0.),
            b2Vec3(0., (scale/2) * constants::to16_16, 0.),
            b2Vec3(-cameraPos.x * constants::to16_16, -cameraPos.y * constants::to16_16, 1.)
        );

        b2Vec2 vertex1 = groundBody->GetWorldPoint(groundBox.m_vertices[0]);
        b2Vec2 vertex2 = groundBody->GetWorldPoint(groundBox.m_vertices[1]);
        b2Vec2 vertex3 = groundBody->GetWorldPoint(groundBox.m_vertices[3]);

        b2Vec3 v1 = b2Mul(mainM, b2Vec3(vertex1.x, vertex1.y, 1.));
        b2Vec3 v2 = b2Mul(mainM, b2Vec3(vertex2.x, vertex2.y, 1.));
        b2Vec3 v3 = b2Mul(mainM, b2Vec3(vertex3.x, vertex3.y, 1.));

        b2Vec2 min = b2Vec2(0., 0.);
        b2Vec2 max = b2Vec2(8. * scale * constants::to16_16, 6. * (scale/2) * constants::to16_16);

        vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);
        vertex2 = b2Clamp(b2Vec2(v2.x, v2.y), min, max);
        vertex3 = b2Clamp(b2Vec2(v3.x, v3.y), min, max);

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(120, 100, 100, 255)));
        render_tri_strip(rdl,
            vertex1.x, vertex1.y,
            vertex2.x, vertex2.y,
            vertex3.x, vertex3.y
        );

        vertex1 = groundBody->GetWorldPoint(groundBox.m_vertices[2]);

        v1 = b2Mul(mainM, b2Vec3(vertex1.x, vertex1.y, 1.));
        vertex1 = b2Clamp(b2Vec2(v1.x, v1.y), min, max);

        render_tri_strip_next(rdl, vertex1.x, vertex1.y);

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 128)));

        box1.update(rdl, cPos, scale);
        box2.update(rdl, cPos, scale);

        for (int i = 0; i < box_count; i ++) {
            boxes[i]->update(rdl, cPos, scale);
            if (boxes[i]->body->GetPosition().y > 5.) {
                float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 8.);
                boxes[i]->body->SetTransform(b2Vec2(rx, -1.), rx);
                boxes[i]->body->SetLinearVelocity(b2Vec2(0.,0.));
            }
        }

        b2Vec2 pos1 = box1.body->GetPosition();
        b2Vec2 pos2 = box2.body->GetPosition();
        rope->update(rdl,pos1, pos2);
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
