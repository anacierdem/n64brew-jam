#include "game.hpp"

b2Vec2 gravity(0.0f, 1.0f);
b2World world(gravity);

// Start positions
// TODO: move to constants
b2Vec2 leftHandInitialPos = {2.0f, 2.0f};
b2Vec2 rightHandInitialPos = {constants::gameAreaWidth - 2.0f, 2.0f};
float leftHandInitialAngle = 0.1;
float rightHandInitialAngle = -0.1;

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

        wav64_open(&collectHealth, "health.wav64");
        wav64_open(&gameover, "gameover.wav64");
        wav64_open(&pickup, "pickup.wav64");
        wav64_open(&jump, "jump.wav64");
        mixer_ch_set_vol(constants::pickupChannel, 0.4f, 0.4f);

        wav64_open(&(ambient[0]), "ambient1.wav64");
        mixer_ch_play(constants::musicChannel1, &(ambient[0]).wave);
        mixer_ch_set_vol(constants::musicChannel1, 0.8f, 0.6f);

        wav64_open(&(ambient[1]), "ambient2.wav64");
        mixer_ch_play(constants::musicChannel2, &(ambient[1]).wave);
        mixer_ch_set_vol(constants::musicChannel2, 0.6f, 0.8f);

        mixer_add_event(ambient[0].wave.len, (int (*)(void*))&Game::loop1Callback, this);
        mixer_add_event(ambient[1].wave.len, (int (*)(void*))&Game::loop2Callback, this);
    };

    // A quick hack for an unnerving feel
    int Game::loop1Callback() {
        float r = 6000.0f * static_cast<float>(rand()) / RAND_MAX;
        int newFreq = 44100.0f - 3000.0f + r;
        int newLength = (static_cast<float>(ambient[0].wave.len) / 44100.0f) * newFreq;
        mixer_ch_play(constants::musicChannel1, &(ambient[0]).wave);
        mixer_ch_set_freq(constants::musicChannel1, newFreq);
        return static_cast<int>(newLength);
    }

    int Game::loop2Callback() {
        float r = 6000.0f * static_cast<float>(rand()) / RAND_MAX;
        int newFreq = 44100.0f - 3000.0f + r;
        int newLength = (static_cast<float>(ambient[1].wave.len) / 44100.0f) * newFreq;
        mixer_ch_play(constants::musicChannel2, &(ambient[1]).wave);
        mixer_ch_set_freq(constants::musicChannel2, newFreq);
        return static_cast<int>(newLength);
    }

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
        score = 0;
        lives = constants::maxLives;
        level = constants::startLevel;
        isReset = true;
        isDead = true;

        int activeCount = constants::startCount + level;
        for (int i = 0; i < box_count; i ++) {
            // Disable if not active
            enemies[i]->body->SetEnabled(i < activeCount);
            enemies[i]->die(level, 0, false);
        }

        gameRope.reset();
    }

    void Game::start() {
        lastLevelIncreaseAt = timer_ticks();
        currentIndex = 0;
        isDead = false;
        isReset = false;
    }

    int Game::addScore(int points)
    {
        if (isDead) return 0;
        int scored = points * (level + 1);
        score += scored;
        highScore = std::max(score, highScore);
        return scored;
    }

    void Game::gameOver() {
        if(!isDead) {
            mixer_ch_stop(constants::gameoverChannel);
            mixer_ch_play(constants::gameoverChannel, &gameover.wave);
            isDead = true;
        }
    }

    void Game::BeginContact(b2Contact* contact)
    {
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();
        b2Filter filterA = fixtureA->GetFilterData();
        b2Filter filterB = fixtureB->GetFilterData();

        uint16 bits = filterA.categoryBits | filterB.categoryBits;

        // Just for fun
        if (!isReset && isDead && (
            bits == (CollisionCategory::enemy) ||
            bits == (CollisionCategory::enemy | CollisionCategory::environment) ||
            bits == (CollisionCategory::hand | CollisionCategory::enemy) ||
            bits == (CollisionCategory::hand | CollisionCategory::environment)
        )) {
            mixer_ch_stop(constants::jumpChannel);
            mixer_ch_play(constants::jumpChannel, &jump.wave);
        }

        // We don't care about enemy-enemy contacts
        if (bits == CollisionCategory::enemy) {
            return;
        }

        // TODO: implement looseLife?
        if (
            !isDead &&
            bits & CollisionCategory::hand &&
            (bits & (CollisionCategory::enemy | CollisionCategory::blade | CollisionCategory::environment))
        ){
            Hand* hand = nullptr;
            Enemy* enemy = nullptr;

            if (filterA.categoryBits == CollisionCategory::hand)
                hand = reinterpret_cast<Hand*> (fixtureA->GetUserData().pointer);
            if (filterB.categoryBits == CollisionCategory::hand)
                hand = reinterpret_cast<Hand*> (fixtureB->GetUserData().pointer);
            assert(hand != nullptr);

            if (bits & CollisionCategory::environment) {
                hand->takeDamage(rdl);
                // Touching ground is always deadly
                gameOver();
            }

            if (bits & CollisionCategory::blade) {
                if (hand->takeDamage(rdl)) {
                    startedShowingDamage = timer_ticks();
                    lives -= 1;
                }
            }

            if (bits & CollisionCategory::enemy) {
                if (filterA.categoryBits == CollisionCategory::enemy)
                    enemy = reinterpret_cast<Enemy*> (fixtureA->GetUserData().pointer);
                if (filterB.categoryBits == CollisionCategory::enemy)
                    enemy = reinterpret_cast<Enemy*> (fixtureB->GetUserData().pointer);
                assert(enemy != nullptr);

                enemyDamageType dmg = enemy->getDamageType();
                if (dmg == health) {
                    mixer_ch_stop(constants::healthChannel);
                    mixer_ch_play(constants::healthChannel, &collectHealth.wave);
                    enemy->die(level, addScore(2), false);
                    lives += dmg;
                } else if (hand->takeDamage(rdl)) {
                    startedShowingDamage = timer_ticks();
                    lives += dmg;
                }
            }

            if (lives >= 3) {
                lives = 3;
            }

            if (lives <= 0) {
                lives = 0;
                gameOver();
            }
        }

        if (!isDead && bits == (CollisionCategory::enemy | CollisionCategory::blade)) {
            Enemy* enemy = reinterpret_cast<Enemy*> (fixtureA->GetUserData().pointer);
            if (enemy == nullptr) enemy = reinterpret_cast<Enemy*> (fixtureB->GetUserData().pointer);
            assert(enemy != nullptr);
            int scoreToAdd = enemy->getDamageType() != health ? 1 : 0;
            mixer_ch_stop(constants::pickupChannel);
            mixer_ch_play(constants::pickupChannel, &pickup.wave);
            enemy->die(level, addScore(scoreToAdd), isDead && !isReset);
        }
    }

    // Full screen quad proved to be costly
    void Game::updateBG() {
        rdl_push(rdl,RdpSetOtherModes(SOM_CYCLE_FILL));

        int64_t animationTime = timer_ticks() - startedShowingDamage;

        // Full screen flash & shake
        if ((animationTime > 0 && animationTime < TICKS_FROM_MS(25))) {
            rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(30,10,10,255)));
            cameraPos = b2Vec3(10.f, 5.f, 10.0f);
        } else if (animationTime > 0 && animationTime < TICKS_FROM_MS(50)) {
            rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(30,10,10,255)));
            cameraPos = b2Vec3(-10.f, 5.f, 10.0f);
        } else if (animationTime > 0 && animationTime < TICKS_FROM_MS(75)) {
            rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(30,10,10,255)));
            cameraPos = b2Vec3(10.f, 5.f, 10.0f);
        } else {
            if (isDead && !isReset) {
                rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(30,10,10,255)));
            } else {
                rdl_push(rdl,RdpSetFillColor(RDP_COLOR32(0,0,0,255)));
            }

            cameraPos = b2Vec3(0.f, 0.f, 0.f);
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
        // Calculate level (difficulty)
        if (!isDead) {
            if ((timer_ticks() - lastLevelIncreaseAt) > (static_cast<int64_t>(levelSwitch[currentIndex]) * static_cast<int64_t>(TICKS_PER_SECOND))) {
                debugf("increase level %d, %lld\n", currentIndex, levelSwitch[currentIndex]);
                lastLevelIncreaseAt = timer_ticks();

                currentIndex++;
                int count = sizeof(levelSwitch)/sizeof(levelSwitch[0]);
                if (currentIndex >= count) {
                    currentIndex = count - 1;
                }

                level++;

                // Cap level
                if (level > constants::maxLevel) level = constants::maxLevel;
            }
        }

        // Do pre conditions
        b2Vec2 posL = leftHand.body->GetPosition();
        b2Vec2 posR = rightHand.body->GetPosition();

        b2Vec2 limits = b2Vec2(
            constants::gameAreaWidth,
            constants::gameAreaHeight
        );
        posL = b2Clamp(posL, b2Vec2_zero,limits);
        leftHand.body->SetTransform(posL, leftHand.body->GetAngle());
        leftHand.body->SetAngularVelocity(leftHand.body->GetAngularVelocity() * 0.99f);

        posR = b2Clamp(posR, b2Vec2_zero, limits);
        rightHand.body->SetTransform(posR, rightHand.body->GetAngle());
        rightHand.body->SetAngularVelocity(rightHand.body->GetAngularVelocity() * 0.99f);

        if (
            posL.x < 0.0f || posL.x > constants::gameAreaWidth || posR.x < 0.0f || posR.x > constants::gameAreaWidth ||
            posL.y < 0.0f || posL.y > constants::gameAreaHeight || posR.y < 0.0f || posR.y > constants::gameAreaHeight
        ) {
            reset();
        }

        if (isReset) {
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
            if( ((keys.c[0].Z && keysDown.c[1].Z) || (keysDown.c[0].Z && keys.c[1].Z)) && isDead && isReset)
            {
                start();
            }

            // Releasing both triggers while dead resets the game
            if((!keys.c[0].Z && !keys.c[1].Z) && isDead && !isReset)
            {
                reset();
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
            if( ((keys.c[0].L && keysDown.c[0].R) || (keysDown.c[0].L && keys.c[0].R)) && isDead && isReset)
            {
                start();
            }

            // Releasing both triggers while dead resets the game
            if((!keys.c[0].L && !keys.c[0].R) && isDead && !isReset)
            {
                reset();
            }

            holdingLeft = keys.c[0].L;
            if( holdingLeft && !isDead)
            {
                leftHand.body->SetLinearVelocity(b2Vec2(
                    static_cast<float>(keys.c[0].left) * -2.0f + static_cast<float>(keys.c[0].right) * 2.0f,
                    static_cast<float>(keys.c[0].up)* -2.0f + static_cast<float>(keys.c[0].down) * 2.0f
                ));
            }

            holdingRight = keys.c[0].R;
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

        // Update and re-spawn enemies if necessary
        int activeCount = constants::startCount + level;
        for (int i = 0; i < box_count; i++) {
            if (i >= activeCount) {
                break;
            }

            enemies[i]->update(rdl, mainM);

            if ((enemies[i]->body->GetPosition().x < -constants::swawnSafeRadius) ||
                (enemies[i]->body->GetPosition().x > (constants::gameAreaWidth + constants::swawnSafeRadius)) ||
                (enemies[i]->body->GetPosition().y > (constants::gameAreaHeight + constants::swawnSafeRadius))) {
                enemies[i]->die(level, 0, isDead && !isReset);
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

        // Rope snaps if too far apart
        if (distanceOverflow > 0.0f && holdingLeft && holdingRight) {
            gameOver();
        }

        // They attract each other if not holding both
        if (distanceOverflow > 0.0f && (!(holdingLeft && holdingRight) || isDead)) {
            leftHand.body->ApplyForceToCenter(distanceVector, true);
            rightHand.body->ApplyForceToCenter(-distanceVector, true);
        }

        // Draw everything except enemies below
        bladeE.update(rdl, mainM, !isDead);

        // Draw hands
        leftHand.update(rdl, mainM, holdingLeft && (!isDead || isReset));
        rightHand.update(rdl, mainM, holdingRight && (!isDead || isReset));

        // Draw rope
        float tension = distanceOverflow < -1.0f ? -1.0f : distanceOverflow;
        tension = tension > 0.0f ? 0.0f : tension;
        gameRope.draw(rdl, mainM, (holdingLeft && holdingRight) ? (tension + 1.0) : 0.0 );

        // Draw ground
        // Draw noise if it is letting through
        if (!isDead || isReset) {
            rdl_push(rdl, RdpSetOtherModes(
                SOM_CYCLE_1 |
                SOM_BLENDING |
                SOM_READ_ENABLE |
                SOM_AA_ENABLE |
                SOM_COLOR_ON_COVERAGE |
                SOM_COVERAGE_DEST_WRAP |
                SOM_ENABLE_DITHER_ALPHA |
                SOM_ENABLE_ALPHA_COMPARE |
                // (P*A + M*B)
                (cast64(0x0) << 30) | (cast64(0x0) << 28) | (cast64(0x0) << 26) | (cast64(0x0) << 24) |
                (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) ) );
        }

        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(150, 68, 201, 180)));
        Box::update(rdl, mainM);
    }

    void Game::updateUI(display_context_t disp) {
        char sbuf[512];
        if (isReset) {
            graphics_set_color(0x888888FF, 0x00000000);

            if((controllers & CONTROLLER_1_INSERTED && controllers & CONTROLLER_2_INSERTED)) {
                graphics_draw_text(disp, 320 -24*4, 140, "with both controllers ;)");
            } else {
                graphics_draw_text(disp, 320 -40*4, 140, "use dual controllers for best experience");
            }

            // Display menu
            if ((timer_ticks() % TICKS_FROM_MS(600)) < TICKS_FROM_MS(300)) {
                graphics_set_color(0xFFFFFFFF, 0x00000000);
            } else {
                graphics_set_color(0x888888FF, 0x00000000);
            }

            if((controllers & CONTROLLER_1_INSERTED && controllers & CONTROLLER_2_INSERTED)) {
                graphics_draw_text(disp, 320 -28*4, 130, "Hold both triggers to START");
            } else {
                graphics_draw_text(disp, 320 -20*4, 130, "Hold L & R to START");
            }

            graphics_set_color(0xFFFFFFFF, 0x00000000);
            sprintf(sbuf, "HISCORE: %d", highScore);
            graphics_draw_text(disp, 320 - strlen(sbuf)*4, 154, sbuf);
        }

        if (isDead && !isReset) {
            graphics_set_color(0xFF0000FF, 0x00000000);
            graphics_draw_text(disp, 320 -9*4, 120, "GAME OVER");
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
                    enemies[i]->scorePosition.x - strlen(sbuf)*4,
                    enemies[i]->scorePosition.y - 4
                , sbuf);
            }
        }

        graphics_set_color(0xFFFFFFFF, 0x00000000);
        sprintf(sbuf, "SCORE: %d LEVEL: %d", score, level);
        graphics_draw_text(disp, 60, 20, sbuf);

        // Frame limiter
        // while(TIMER_MICROS_LL(timer_ticks() - lastUpdate) < (timeStep * 1000.0f * 990.0f));

#ifndef NDEBUG
        sprintf(sbuf, "F: %0.1f", TIMER_MICROS_LL(timer_ticks() - lastUpdate) / 1000.0f);
        graphics_draw_text(disp, 60, 44, sbuf);
        lastUpdate = timer_ticks();
#endif
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
