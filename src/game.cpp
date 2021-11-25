#include <algorithm>

#include "game.hpp"

#include "box2d/box2d.h"

#include "box.hpp"
#include "hand.hpp"
#include "rope.hpp"

b2Vec2 gravity(0.0f, 1.0f);
b2World world(gravity);

b2Transform box1Transform;
b2Transform box2Transform;

Hand leftHand(&world);
Hand rightHand(&world);

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
        box2Transform.Set(b2Vec2(constants::gameAreaWidth - 2.0f, 1.0f), -1.2f);

        groundBodyDef.position.Set(0.0f, constants::gameAreaHeight);
        groundBody = world.CreateBody(&groundBodyDef);
        groundBox.SetAsBox(50.0f, 0.1f);

        groundFixtureDef.shape = &groundBox;
        groundFixtureDef.density = 0.0f;

        b2Filter filter;
        filter.categoryBits = CollisionCategory::environment;

        groundFixtureDef.filter = filter;
        groundBody->CreateFixture(&groundFixtureDef);

        rope = new Rope(20, box1Transform.p, box2Transform.p);

        for (int i = 0; i < box_count; i ++) {
            boxes[i] = new Box(&world);
        }

        world.SetContactListener(this);
        reset();
    };

    void Game::reset() {
        leftHand.body->SetTransform(box1Transform.p, box1Transform.q.GetAngle());
        leftHand.body->SetAwake(true);
        rightHand.body->SetTransform(box2Transform.p, box2Transform.q.GetAngle());
        rightHand.body->SetAwake(true);
        leftHand.body->SetLinearVelocity(b2Vec2(0., 0.));
        leftHand.body->SetAngularVelocity(0.);
        rightHand.body->SetLinearVelocity(b2Vec2(0., 0.));
        rightHand.body->SetAngularVelocity(0.);

        for (int i = 0; i < box_count; i ++) {
            float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / constants::gameAreaWidth);
            float ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / constants::gameAreaHeight);
            boxes[i]->body->SetTransform(b2Vec2(rx, ry), rx);
        }

        rope->reset();

        isDead = true;
        score = 0;
        lives = 3;
        shouldReset = false;
    }

    void Game::BeginContact(b2Contact* contact)
    {
        if (isDead) return;
        b2Body* bodyA = contact->GetFixtureA()->GetBody();
        b2Body* bodyB = contact->GetFixtureB()->GetBody();

        if (bodyA == leftHand.body || bodyA == rightHand.body || bodyB == rightHand.body || bodyB == rightHand.body) {
            score++;
            highScore = std::max(score, highScore);

            lives--;

            if (lives <= 0) {
                // Not possible to SetTransform in a contact callback, defer to next update
                shouldReset = true;
            }
        }
    }

    int Game::update() {
        // Do pre conditions
        b2Vec2 posL = leftHand.body->GetPosition();
        b2Vec2 posR = rightHand.body->GetPosition();
        if (
            shouldReset ||
            posL.x < 0.0f || posL.x > constants::gameAreaWidth || posR.x < 0.0f || posR.x > constants::gameAreaWidth ||
            posL.y < 0.0f || posL.y > constants::gameAreaHeight || posR.y < 0.0f || posR.y > constants::gameAreaHeight
        ) {
            reset();
            return 0;
        }

        if (isDead) {
            leftHand.body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            rightHand.body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            leftHand.body->SetTransform(box1Transform.p, box1Transform.q.GetAngle());
            rightHand.body->SetTransform(box2Transform.p, box2Transform.q.GetAngle());
        }

        // Step simulation!
        world.Step(timeStep, velocityIterations, positionIterations);

        // Handle input
        controller_scan();
        int controllers = get_controllers_present();
        struct controller_data keys = get_keys_pressed();
        struct controller_data keys_down = get_keys_down();

        if((controllers & CONTROLLER_1_INSERTED)) {
            if( keys_down.c[0].start && isDead)
            {
                isDead = false;
            }

            if( keys.c[0].Z && !isDead)
            {
                leftHand.body->SetLinearVelocity(b2Vec2((float)keys.c[0].x / 20.0f, -(float)keys.c[0].y / 20.0f));
            }
        }

        if((controllers & CONTROLLER_2_INSERTED)) {
            if( keys_down.c[1].start && isDead)
            {
                isDead = false;
            }

            if( keys.c[1].Z  && !isDead)
            {
                rightHand.body->SetLinearVelocity(b2Vec2((float)keys.c[1].x / 20.0f, -(float)keys.c[1].y / 20.0f));
            }
        }

        // Drawing parameters
        float scale = 80.;
        b2Vec2 cPos(cameraPos.x / scale, cameraPos.y / (scale/2.));

        b2Mat33 mainM(
            b2Vec3(scale * constants::to16_16, 0., 0.),
            b2Vec3(0., (scale/2) * constants::to16_16, 0.),
            b2Vec3(-cameraPos.x * constants::to16_16, -cameraPos.y * constants::to16_16, 1.)
        );

        // Draw ground
        b2Vec2 vertex1 = groundBody->GetWorldPoint(groundBox.m_vertices[0]);
        b2Vec2 vertex2 = groundBody->GetWorldPoint(groundBox.m_vertices[1]);
        b2Vec2 vertex3 = groundBody->GetWorldPoint(groundBox.m_vertices[3]);

        b2Vec3 v1 = b2Mul(mainM, b2Vec3(vertex1.x, vertex1.y, 1.));
        b2Vec3 v2 = b2Mul(mainM, b2Vec3(vertex2.x, vertex2.y, 1.));
        b2Vec3 v3 = b2Mul(mainM, b2Vec3(vertex3.x, vertex3.y, 1.));

        b2Vec2 min = b2Vec2(0., 0.);
        b2Vec2 max = b2Vec2(
            constants::gameAreaWidth * scale * constants::to16_16,
            constants::gameAreaHeight * (scale/2) * constants::to16_16
        );

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

        // Draw hands
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 128)));

        leftHand.update(rdl, cPos, scale);
        rightHand.update(rdl, cPos, scale);

        // Re-spawn boxes
        for (int i = 0; i < box_count; i ++) {
            boxes[i]->update(rdl, cPos, scale);
            if (boxes[i]->body->GetPosition().y > 6.) {
                float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 8.);
                boxes[i]->body->SetTransform(b2Vec2(rx, -1.), rx);
                boxes[i]->body->SetLinearVelocity(b2Vec2(0.,0.));
            }
        }

        // Handle rope
        b2Vec2 pos1 = leftHand.body->GetPosition();
        b2Vec2 pos2 = rightHand.body->GetPosition();
        rope->update(rdl, pos1, pos2);

        b2Vec2 distanceVector = (pos2 - pos1);
        float distanceOverflow = distanceVector.Length() - 5.0f;
        if (distanceOverflow > 0.0f) {
            leftHand.body->ApplyForceToCenter(distanceVector, true);
            rightHand.body->ApplyForceToCenter(-distanceVector, true);
        }

        return 0;
    }

    void Game::updateUI(display_context_t disp) {
        char sbuf[1024];
        graphics_set_color(0xFFFFFFFF, 0x00000000);
        if (isDead) {
            // Display menu
            graphics_draw_text(disp, 320 -10*8, 120, "PRESS START");

            sprintf(sbuf, "HIGHSCORE: %d", highScore);
            graphics_draw_text(disp, 320 - strlen(sbuf)*8, 130, "HIGH SCORE:");
        }

        sprintf(sbuf, "SCORE: %d", score);
        graphics_draw_text(disp, 40, 20, sbuf);

        sprintf(sbuf, "LIVES: %d", lives);
        graphics_draw_text(disp, 600 - strlen(sbuf)*8, 20, sbuf);
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

    void update_UI(Game* self, display_context_t disp)
    {
        return self->updateUI(disp);
    }
}
