#include <algorithm>

#include "game.hpp"

#include "box2d/box2d.h"

#include "box.hpp"
#include "hand.hpp"
#include "rope.hpp"
#include "blade.hpp"

b2Vec2 gravity(0.0f, 1.0f);
b2World world(gravity);

// Start position
b2Transform leftHandInitialTransform;
b2Transform rightHandInitialTransform;

Hand leftHand(&world);
Hand rightHand(&world);

// Why can't use blade as a name?
Blade bladeE(&world);

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"

    Game::Game(RdpDisplayList* rdlParam) : Box(&world)
    {
        if (get_tv_type() == TV_PAL) {
            timeStep = 1.0 / 50.0;
        }

        rdl = rdlParam;

        // Start position
        leftHandInitialTransform.Set(b2Vec2(2.0f, 2.0f), 1.0f);
        rightHandInitialTransform.Set(b2Vec2(constants::gameAreaWidth - 2.0f, 2.0f), -1.2f);

        bodyDef.type = b2_staticBody;
        bodyDef.position.Set(0.0f, constants::gameAreaHeight);
        body = world.CreateBody(&bodyDef);
        dynamicBox.SetAsBox(50.0f, 0.1f);

        fixtureDef.shape = &dynamicBox;
        fixtureDef.density = 0.0f;

        b2Filter filter;
        filter.categoryBits = CollisionCategory::environment;

        fixtureDef.filter = filter;
        body->CreateFixture(&fixtureDef);

        rope = new Rope(19, leftHandInitialTransform.p, rightHandInitialTransform.p);

        for (int i = 0; i < box_count; i ++) {
            enemies[i] = new Enemy(&world);
        }

        world.SetContactListener(this);
        reset();
    };

    void Game::reset() {
        leftHand.body->SetTransform(leftHandInitialTransform.p, leftHandInitialTransform.q.GetAngle());
        leftHand.body->SetAwake(true);
        rightHand.body->SetTransform(rightHandInitialTransform.p, rightHandInitialTransform.q.GetAngle());
        rightHand.body->SetAwake(true);
        leftHand.body->SetLinearVelocity(b2Vec2(0., 0.));
        leftHand.body->SetAngularVelocity(0.);
        rightHand.body->SetLinearVelocity(b2Vec2(0., 0.));
        rightHand.body->SetAngularVelocity(0.);

        bladeE.body->SetTransform(b2Vec2(0.0, 0.0), 0.0);

        // Reset game
        isDead = true;
        score = 0;
        lives = 3;
        shouldReset = false;
        level = 0;

        int activeCount = 5 + level;
        for (int i = 0; i < box_count; i ++) {
            float rx = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / constants::gameAreaWidth);
            float ry = static_cast <float> (rand()) / static_cast <float> (RAND_MAX / constants::gameAreaHeight);
            if (i < activeCount) {
                enemies[i]->body->SetTransform(b2Vec2(rx, ry), rx);
                enemies[i]->body->SetEnabled(true);
            } else {
                enemies[i]->body->SetTransform(b2Vec2(rx, -constants::swawnSafeRadius), rx);
                enemies[i]->body->SetEnabled(false);
            }
        }

        rope->reset();
    }

    void Game::BeginContact(b2Contact* contact)
    {
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();
        b2Filter filterA = fixtureA->GetFilterData();
        b2Filter filterB = fixtureB->GetFilterData();

        // We don't care about enemy contacts
        if (
            filterA.categoryBits == CollisionCategory::enemy &&
            filterB.categoryBits == CollisionCategory::enemy) {
            return;
        }

        // If something touches hands, loose a life
        if (
            !isDead &&
            (
                filterA.categoryBits == CollisionCategory::hand ||
                filterB.categoryBits == CollisionCategory::hand
            )
        ) {

            // TODO: implement looseLife?
            lives--;

            if (lives <= 0) {
                // Not possible to SetTransform in a contact callback, defer to next update
                shouldReset = true;
            }
        }

        Enemy* enemy = nullptr;
        if (filterA.categoryBits == CollisionCategory::enemy && filterB.categoryBits == CollisionCategory::blade) {
            enemy = reinterpret_cast<Enemy*> (fixtureA->GetUserData().pointer);
            assert(enemy != nullptr);
        }

        if (filterB.categoryBits == CollisionCategory::enemy && filterA.categoryBits == CollisionCategory::blade) {
            enemy = reinterpret_cast<Enemy*> (fixtureB->GetUserData().pointer);
            assert(enemy != nullptr);
        }

        if (enemy != nullptr) {
            addScore(10);
            enemy->die(level, 10);
        }
    }

    void Game::addScore(int points)
    {
        if (isDead) return;
        score += points;
        highScore = std::max(score, highScore);
        level = score / 50;
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
            leftHand.body->SetTransform(leftHandInitialTransform.p, leftHandInitialTransform.q.GetAngle());
            rightHand.body->SetTransform(rightHandInitialTransform.p, rightHandInitialTransform.q.GetAngle());
        }

        // Step simulation!
        world.Step(timeStep, velocityIterations, positionIterations);

        // Read input
        controller_scan();
        controllers = get_controllers_present();
        keys = get_keys_pressed();
        keysDown = get_keys_down();

        if((controllers & CONTROLLER_1_INSERTED && controllers & CONTROLLER_2_INSERTED)) {
            // Dual controller mode
            if( ((keys.c[0].Z && keysDown.c[1].Z) || (keysDown.c[0].Z && keys.c[1].Z)) && isDead)
            {
                // TODO: implement start
                isDead = false;
            }

            holdingLeft = keys.c[0].Z;
            if( holdingLeft && !isDead)
            {
                leftHand.body->SetLinearVelocity(b2Vec2(
                    static_cast<float>(keys.c[0].x) / 20.0f,
                    -static_cast<float>(keys.c[0].y) / 20.0f
                ));
            }

            holdingRight = keys.c[1].Z;
            if( holdingRight && !isDead)
            {
                rightHand.body->SetLinearVelocity(b2Vec2(
                    static_cast<float>(keys.c[1].x) / 20.0f,
                    -static_cast<float>(keys.c[1].y) / 20.0f
                ));
            }
        } else if((controllers & CONTROLLER_1_INSERTED)) {
            // Single controller mode
            if( ((keys.c[0].L && keysDown.c[1].R) || (keysDown.c[0].L && keys.c[1].R)) && isDead)
            {
                // TODO: implement start
                isDead = false;
            }

            holdingLeft = keys.c[0].L;
            if( holdingLeft && !isDead)
            {
                leftHand.body->SetLinearVelocity(b2Vec2(
                    static_cast<float>(keys.c[0].left) * -2.0f + static_cast<float>(keys.c[0].right) * 2.0f,
                    static_cast<float>(keys.c[0].up)* -2.0f + static_cast<float>(keys.c[0].down) * 2.0f
                ));
            }

            holdingRight = keys.c[0].L;
            if( holdingRight && !isDead)
            {
                rightHand.body->SetLinearVelocity(b2Vec2(
                    static_cast<float>(keys.c[0].C_left) * -2.0f + static_cast<float>(keys.c[0].C_right) * 2.0f,
                    static_cast<float>(keys.c[0].C_up)* -2.0f + static_cast<float>(keys.c[0].C_down) * 2.0f
                ));
            }
        }

        // Main transformation matrix
        b2Mat33 mainM(
            b2Vec3(constants::scale * constants::to16_16, 0., 0.),
            b2Vec3(0., (constants::scale/2) * constants::to16_16, 0.),
            b2Vec3(-cameraPos.x * constants::to16_16, -cameraPos.y * constants::to16_16, 1.)
        );

        // Draw ground
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(120, 100, 100, 255)));
        Box::update(rdl, mainM);

        // Update and re-spawn enemies if necessary
        int activeCount = 5 + level;
        for (int i = 0; i < box_count; i ++) {
            if (i >= activeCount) {
                break;
            }

            enemies[i]->body->SetEnabled(true);
            enemies[i]->update(rdl, mainM);

            if (enemies[i]->body->GetPosition().x < -constants::swawnSafeRadius ||
                enemies[i]->body->GetPosition().x > constants::gameAreaWidth + constants::swawnSafeRadius) {
                enemies[i]->die(level, 0);
                continue;
            }

            if (enemies[i]->body->GetPosition().y > constants::gameAreaHeight + constants::swawnSafeRadius) {
                // Minor scoring condition
                addScore(1);
                enemies[i]->die(level, 1);
                continue;
            }
        }

        // Handle rope
        b2Vec2 pos1 = leftHand.body->GetPosition();
        b2Vec2 pos2 = rightHand.body->GetPosition();

        bladeE.body->SetTransform(rope->update(pos1, pos2), bladeE.body->GetAngle());
        bladeE.body->SetAngularVelocity(1000.0f * timeStep);

        b2Vec2 distanceVector = (pos2 - pos1);
        float distanceOverflow = distanceVector.Length() - constants::allowedDistance;

        if (distanceOverflow > 0.0f && holdingLeft && holdingRight) {
            isDead = true;
        }

        if (distanceOverflow > 0.0f && !(holdingLeft && holdingRight)) {
            leftHand.body->ApplyForceToCenter(distanceVector, true);
            rightHand.body->ApplyForceToCenter(-distanceVector, true);
        }

        // Draw everything except enemies below
        bladeE.update(rdl, mainM);

        // Draw hands
        leftHand.update(rdl, mainM, holdingLeft);
        rightHand.update(rdl, mainM, holdingRight);

        // Draw rope
        float tension = distanceOverflow < -1.0f ? -1.0f : distanceOverflow;
        tension = tension > 0.0f ? 0.0f : tension;
        rope->draw(rdl, mainM, (holdingLeft && holdingRight) ? (tension + 1.0) : 0.0 );
        return 0;
    }

    void Game::updateUI(display_context_t disp) {
        char sbuf[512];
        if (isDead) {
            graphics_set_color(0x888888FF, 0x00000000);

            if((controllers & CONTROLLER_1_INSERTED && controllers & CONTROLLER_2_INSERTED)) {
                graphics_draw_text(disp, 320 -24*4, 130, "with both controllers ;)");
            } else {
                graphics_draw_text(disp, 320 -40*4, 130, "use dual controllers for best experience");
            }

            // Display menu
            if ((timer_ticks() % TICKS_FROM_MS(600)) < TICKS_FROM_MS(300)) {
                graphics_set_color(0xFFFFFFFF, 0x00000000);
            } else {
                graphics_set_color(0x888888FF, 0x00000000);
            }

            if((controllers & CONTROLLER_1_INSERTED && controllers & CONTROLLER_2_INSERTED)) {
                graphics_draw_text(disp, 320 -28*4, 120, "Press both triggers to START");
            } else {
                graphics_draw_text(disp, 320 -20*4, 120, "Press L & R to START");
            }

            graphics_set_color(0xFFFFFFFF, 0x00000000);
            sprintf(sbuf, "HISCORE: %d", highScore);
            graphics_draw_text(disp, 320 - strlen(sbuf)*4, 144, sbuf);
        }

        graphics_set_color(0xFFFFFFFF, 0x00000000);
        sprintf(sbuf, "SCORE: %d LEVEL: %d", score, level);
        graphics_draw_text(disp, 60, 20, sbuf);

        sbuf[0] = '\0';

        for (int i = 0; i < lives; i++) {
            strcat(sbuf, "< ");
        }

        graphics_set_color(0xFF0000FF, 0x00000000);
        graphics_draw_text(disp, 580 - strlen(sbuf)*8, 20, sbuf);

        graphics_set_color(0x00FF00FF, 0x00000000);
        for (int i = 0; i < box_count; i ++) {
            if (enemies[i]->showingScore) {
                sprintf(sbuf, "+%d", enemies[i]->score);
                graphics_draw_text(disp,
                    enemies[i]->scorePosition.x,
                    enemies[i]->scorePosition.y
                , sbuf);
            }
        }
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
