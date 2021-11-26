#include "game.hpp"

b2Vec2 gravity(0.0f, 1.0f);
b2World world(gravity);

// Start positions
// TODO: move to constants
b2Vec2 leftHandInitialPos = {2.0f, 2.0f};
b2Vec2 rightHandInitialPos = {constants::gameAreaWidth - 2.0f, 2.0f};
float leftHandInitialAngle = 0.;
float rightHandInitialAngle = 0.;

Hand leftHand(&world);
Hand rightHand(&world);

// Why can't use blade as a name?
Blade bladeE(&world);

Rope gameRope(19, leftHandInitialPos, rightHandInitialPos);

extern "C" {
    Game::Game(RdpDisplayList* rdlParam) : Box()
    {
        world.SetContactListener(this);
        if (get_tv_type() == TV_PAL) {
            timeStep = 1.0 / 50.0;
        }

        rdl = rdlParam;

        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position.Set(0.0f, constants::gameAreaHeight);
        body = world.CreateBody(&bodyDef);
        polygonShape.SetAsBox(50.0f, 0.1f);

        fixtureDef.shape = &polygonShape;
        fixtureDef.density = 0.0f;

        b2Filter filter;
        filter.categoryBits = CollisionCategory::environment;

        fixtureDef.filter = filter;
        body->CreateFixture(&fixtureDef);

        for (int i = 0; i < box_count; i++) {
            enemies[i] = new Enemy(&world);
        }

        reset();
    };

    void Game::reset() {
        leftHand.body->SetTransform(leftHandInitialPos, leftHandInitialAngle);
        leftHand.body->SetAwake(true);
        rightHand.body->SetTransform(rightHandInitialPos, rightHandInitialAngle);
        rightHand.body->SetAwake(true);
        leftHand.body->SetLinearVelocity(b2Vec2_zero);
        leftHand.body->SetAngularVelocity(0.0f);
        rightHand.body->SetLinearVelocity(b2Vec2_zero);
        rightHand.body->SetAngularVelocity(0.0f);

        bladeE.body->SetTransform(b2Vec2_zero, 0.0f);

        // Reset game
        isDead = true;
        score = 0;
        lives = 3;
        shouldReset = false;
        level = 0;

        int activeCount = 5 + level;
        for (int i = 0; i < box_count; i ++) {
            // Disable if not active
            enemies[i]->body->SetEnabled(i < activeCount);
            enemies[i]->die(level, 0);
        }

        gameRope.reset();
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
            Hand* hand = reinterpret_cast<Hand*> (fixtureA->GetUserData().pointer);
            if (hand == nullptr) hand = reinterpret_cast<Hand*> (fixtureB->GetUserData().pointer);
            if (hand == &leftHand) leftHand.takeDamage(rdl);
            if (hand == &rightHand) rightHand.takeDamage(rdl);

            startedShowingDamage = timer_ticks();

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

    // Full screen quad proved to be costly
    void Game::updateBG() {
        rdl_push(rdl,RdpSetOtherModes(SOM_CYCLE_FILL));

        int64_t animationTime = timer_ticks() - startedShowingDamage;

        // Full screen flash
        if (animationTime > 0 && animationTime < TICKS_FROM_MS(25)) {
            rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(30,10,10,255)));
        } else {
            rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(0,0,0,255)));
        }

        // Clear
        rdl_push(rdl,RdpFillRectangleI(0, 0, 640, 240));

        // Set other modes
        rdl_push(rdl, RdpSetOtherModes(
            SOM_CYCLE_1 |
            SOM_BLENDING |
            SOM_READ_ENABLE |
            SOM_AA_ENABLE |
            SOM_COLOR_ON_COVERAGE |
            SOM_COVERAGE_DEST_WRAP |
            // (P*A + M*B)
            (cast64(0x0) << 30) | (cast64(0x0) << 28) | (cast64(0x0) << 26) | (cast64(0x0) << 24) |
            (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) ) );
    }

    void Game::update() {
        // Do pre conditions
        b2Vec2 posL = leftHand.body->GetPosition();
        b2Vec2 posR = rightHand.body->GetPosition();

        b2Vec2 limits = b2Vec2(
            constants::gameAreaWidth,
            constants::gameAreaHeight
        );
        posL = b2Clamp(posL, b2Vec2_zero,limits);
        leftHand.body->SetTransform(posL, leftHand.body->GetAngle());

        posR = b2Clamp(posR, b2Vec2_zero, limits);
        rightHand.body->SetTransform(posR, rightHand.body->GetAngle());

        if (
            shouldReset ||
            posL.y < 0.0f || posL.y > constants::gameAreaHeight || posR.y < 0.0f || posR.y > constants::gameAreaHeight
        ) {
            reset();
        }

        if (isDead) {
            leftHand.body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            rightHand.body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            leftHand.body->SetTransform(leftHandInitialPos, leftHandInitialAngle);
            rightHand.body->SetTransform(rightHandInitialPos, rightHandInitialAngle);
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
            b2Vec3(0., (constants::scale/2.0f) * constants::to16_16, 0.),
            b2Vec3(-cameraPos.x * constants::to16_16, -cameraPos.y * constants::to16_16, 1.)
        );

        // Draw ground
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(120, 100, 100, 255)));
        Box::update(rdl, mainM);

        // Update and re-spawn enemies if necessary
        int activeCount = 5 + level;
        for (int i = 0; i < box_count; i++) {
            if (i >= activeCount) {
                break;
            }

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

        bladeE.body->SetTransform(gameRope.update(pos1, pos2), bladeE.body->GetAngle());
        bladeE.body->SetAngularVelocity(1500.0f * timeStep);

        b2Vec2 distanceVector = (pos2 - pos1);
        float distanceOverflow = distanceVector.Length() - constants::allowedDistance;

        if (distanceOverflow > 0.0f && holdingLeft && holdingRight) {
            reset();
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
        gameRope.draw(rdl, mainM, (holdingLeft && holdingRight) ? (tension + 1.0) : 0.0 );
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
                graphics_draw_text(disp, 320 -28*4, 120, "Hold both triggers to START");
            } else {
                graphics_draw_text(disp, 320 -20*4, 120, "Hold L & R to START");
            }

            graphics_set_color(0xFFFFFFFF, 0x00000000);
            sprintf(sbuf, "HISCORE: %d", highScore);
            graphics_draw_text(disp, 320 - strlen(sbuf)*4, 144, sbuf);
        }


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

        graphics_set_color(0xFFFFFFFF, 0x00000000);
        sprintf(sbuf, "SCORE: %d LEVEL: %d T: %0.1f", score, level,  TIMER_MICROS_LL(timer_ticks() - lastUpdate) / 1000.0f);
        graphics_draw_text(disp, 60, 20, sbuf);


        // Frame limiter
        // while(TIMER_MICROS_LL(timer_ticks() - lastUpdate) < (timeStep * 1000.0f * 990.0f));

        // graphics_set_color(0xFFFFFFFF, 0x00000000);
        // sprintf(sbuf, "F: %0.1f", TIMER_MICROS_LL(timer_ticks() - lastUpdate) / 1000.0f);
        // graphics_draw_text(disp, 60, 32, sbuf);
        lastUpdate = timer_ticks();
    }

    Game* new_Game(RdpDisplayList* rdl)
    {
        return new Game(rdl);
    }

    void delete_Game(Game* self)
    {
        delete self;
    }

    void update_BG(Game* self)
    {
        return self->updateBG();
    }

    void update_Game(Game* self)
    {
        return self->update();
    }

    void update_UI(Game* self, display_context_t disp)
    {
        return self->updateUI(disp);
    }
}
