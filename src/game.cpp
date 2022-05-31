#include "game.hpp"

b2World world(b2Vec2(0.0f, constants::gravity));

// Start positions
// TODO: move to constants
b2Vec2 leftHandInitialPos = {2.0f, 2.0f};
b2Vec2 rightHandInitialPos = {constants::gameAreaWidth - 2.0f, 2.0f};

Hand leftHand(&world);
Hand rightHand(&world);

// Why can't use blade as a name?
Blade bladeE(&world);

Rope gameRope(19, leftHandInitialPos, rightHandInitialPos);

extern "C" {
    int loop_cb1_Game(Game* self)
    {
        // return self->loopCallback(0);
    }

    int loop_cb2_Game(Game* self)
    {
        // return self->loopCallback(1);
    }

    Game::Game() : Box()
    {
        world.SetContactListener(this);
        if (get_tv_type() == TV_PAL) {
            timeStep = 1.0 / 50.0;
        }

        b2BodyDef bodyDef;
        bodyDef.type = b2_staticBody;
        bodyDef.position.Set(0.0f, constants::gameAreaHeight);
        body = world.CreateBody(&bodyDef);
        polygonShape.SetAsBox(50.0f, 0.15f);

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

        // wav64_open(&noise, "noise.wav64");
        // mixer_ch_set_vol(constants::noiseChannel, 0.0f, 0.0f);
        // wav64_set_loop(&noise, true);
        // mixer_ch_play(constants::noiseChannel, &noise.wave);

        // wav64_open(&collectHealth, "health.wav64");
        // wav64_open(&gameover, "gameover.wav64");
        // wav64_open(&pickup, "pickup.wav64");
        // wav64_open(&jump, "jump.wav64");
        // mixer_ch_set_vol(constants::gameoverChannel, 0.7f, 0.7f);
        // mixer_ch_set_vol(constants::pickupChannel, 0.3f, 0.3f);

        // wav64_open(&(ambient[0]), "ambient1.wav64");
        // mixer_ch_set_vol(constants::musicChannel1, 0.8f, 0.6f);

        // wav64_open(&(ambient[1]), "ambient2.wav64");
        // mixer_ch_set_vol(constants::musicChannel2, 0.6f, 0.8f);

        // mixer_add_event(0, (int (*)(void*))loop_cb1_Game, this);
        // mixer_add_event(0, (int (*)(void*))loop_cb2_Game, this);
    };

    // A quick hack for an unnerving feel
    int Game::loopCallback(int id) {
        float r = 6000.0f * static_cast<float>(rand()) / RAND_MAX;
        int newFreq = 44100 - 3000 + static_cast<int>(r);
        int newLength = static_cast<int>((static_cast<float>(ambient[id].wave.len) / static_cast<float>(newFreq)) * 44100.0f);
        // mixer_ch_play(id*2, &(ambient[id]).wave);
        // mixer_ch_set_freq(id*2, newFreq);
        return static_cast<int>(newLength);
    }

    void Game::reset() {
        float r = static_cast<float>(rand()) / RAND_MAX;
        leftHand.body->SetTransform(leftHandInitialPos, r);
        leftHand.body->SetAwake(true);
        rightHand.body->SetTransform(rightHandInitialPos, -r);
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
        if (level >= constants::startIncreasingGravityAtLevel) {
            world.SetGravity(b2Vec2(0.0f, constants::gravity + constants::gravityPerLevel * (level - constants::startIncreasingGravityAtLevel)));
        } else {
            world.SetGravity(b2Vec2(0.0f, constants::gravity));
        }
        isReset = true;
        isDead = true;

        int activeCount = constants::startCount + level;
        for (int i = 0; i < box_count; i ++) {
            // Disable if not active
            enemies[i]->body->SetEnabled(i < activeCount);
            // max delay of screen fall time for initial gravity
            float rt = constants::gameAreaWidth * static_cast<float>(rand()) / RAND_MAX;
            enemies[i]->die(level, 0, false, rt * std::sqrt((2.0f * constants::gameAreaHeight / constants::gravity)));
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
            // mixer_ch_play(constants::gameoverChannel, &gameover.wave);
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
            // mixer_ch_play(constants::jumpChannel, &jump.wave);
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
                hand->takeDamage();
                // Touching ground is always deadly
                gameOver();
            }

            if (bits & CollisionCategory::blade) {
                if (hand->takeDamage()) {
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
                    // mixer_ch_play(constants::healthChannel, &collectHealth.wave);
                    enemy->die(level, addScore(2), false, 0.0f);
                    lives += dmg;
                } else if (hand->takeDamage()) {
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
            // mixer_ch_play(constants::pickupChannel, &pickup.wave);
            // Add a random delay, this will skew the spawn timing a little but that's ok
            float rt = constants::gameAreaWidth * static_cast<float>(rand()) / RAND_MAX;
            enemy->die(level, addScore(scoreToAdd), isDead && !isReset, rt);
        }
    }

    // Full screen quad proved to be costly
    void Game::updateBG() {
        rdpq_set_other_modes(SOM_CYCLE_FILL);

        int64_t animationTime = timer_ticks() - startedShowingDamage;

        // Full screen flash & shake
        if ((animationTime > 0 && animationTime < TICKS_FROM_MS(25))) {
            rdpq_set_fill_color(RGBA32(30,10,10,255));
            cameraPos = b2Vec3(10.f, 5.f, 10.0f);
        } else if (animationTime > 0 && animationTime < TICKS_FROM_MS(50)) {
            rdpq_set_fill_color(RGBA32(30,10,10,255));
            cameraPos = b2Vec3(-10.f, 5.f, 10.0f);
        } else if (animationTime > 0 && animationTime < TICKS_FROM_MS(75)) {
            rdpq_set_fill_color(RGBA32(30,10,10,255));
            cameraPos = b2Vec3(10.f, 5.f, 10.0f);
        } else {
            if (isDead && !isReset) {
                rdpq_set_fill_color(RGBA32(30,10,10,255));
            } else {
                rdpq_set_fill_color(RGBA32(0,0,0,255));
            }

            cameraPos = b2Vec3(0.f, 0.f, 0.f);
        }

        // Clear
        rdpq_fill_rectangle(0, 0, 640, 240);

        // Set other modes
        rdpq_set_other_modes(
            SOM_CYCLE_1 |
            SOM_BLENDING |
            SOM_READ_ENABLE |
            SOM_AA_ENABLE |
            SOM_COLOR_ON_COVERAGE |
            SOM_COVERAGE_DEST_WRAP |
            // (P*A + M*B)
            (cast64(0x0) << 30) | (cast64(0x0) << 28) | (cast64(0x0) << 26) | (cast64(0x0) << 24) |
            (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) );
    }

    void Game::update() {
        render_tri_strip(
            0.0f, 0.0f,
            50.f * constants::to16_16, 0.0f,
            0.0f, 50.f * constants::to16_16
        );

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

                if (level >= constants::startIncreasingGravityAtLevel) {
                    world.SetGravity(b2Vec2(0.0f, constants::gravity + constants::gravityPerLevel * (level - constants::startIncreasingGravityAtLevel)));
                }
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
            leftHand.body->SetTransform(leftHandInitialPos, leftHand.body->GetAngle());
            rightHand.body->SetTransform(rightHandInitialPos, rightHand.body->GetAngle());
        }

        // Step simulation!
        world.Step(timeStep, velocityIterations, positionIterations);

        // Read input
        controller_scan();
        if (frameCount % 60 == 0) {
            controllers = get_controllers_present();
            frameCount = 0;
        }
        frameCount++;
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
            if( (((keys.c[0].L || keys.c[0].Z) && keysDown.c[0].R) ||
                ((keysDown.c[0].L || keysDown.c[0].Z) && keys.c[0].R)) && isDead && isReset)
            {
                start();
            }

            // Releasing both triggers while dead resets the game
            if((!(keys.c[0].L || keys.c[0].Z) && !keys.c[0].R) && isDead && !isReset)
            {
                reset();
            }

            holdingLeft = (keys.c[0].L || keys.c[0].Z);
            if( holdingLeft && !isDead)
            {
                if(keys.c[0].left || keys.c[0].right || keys.c[0].up || keys.c[0].down) {
                    leftHand.body->SetLinearVelocity(b2Vec2(
                        static_cast<float>(keys.c[0].left) * -2.0f + static_cast<float>(keys.c[0].right) * 2.0f,
                        static_cast<float>(keys.c[0].up)* -2.0f + static_cast<float>(keys.c[0].down) * 2.0f
                    ));
                } else {
                    leftHand.body->SetLinearVelocity(b2Vec2(
                        static_cast<float>(keys.c[0].x) / 20.0f,
                        -static_cast<float>(keys.c[0].y) / 20.0f
                    ));
                }
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

        leftHand.body->ApplyTorque(leftHand.body->GetLinearVelocity().x / 300.0f, true);
        rightHand.body->ApplyTorque(rightHand.body->GetLinearVelocity().x / 300.0f, true);

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

            enemies[i]->update(mainM);

            // Kill the enemy at a random location in approx next screen height such that we spawn them "apparently" at random
            float ry = constants::gameAreaWidth * static_cast<float>(rand()) / RAND_MAX;
            if ((enemies[i]->body->GetPosition().x < -constants::spawnSafeRadius) ||
                (enemies[i]->body->GetPosition().x > (constants::gameAreaWidth + constants::spawnSafeRadius)) ||
                (enemies[i]->body->GetPosition().y > ((constants::gameAreaHeight * (1.0f + ry)) + constants::spawnSafeRadius))) {
                enemies[i]->die(level, 0, isDead && !isReset, 0.0f);
                continue;
            }
        }

        // Handle rope
        b2Vec2 pos1 = leftHand.body->GetPosition();
        b2Vec2 pos2 = rightHand.body->GetPosition();

        bladeE.body->SetTransform(gameRope.update(pos1, pos2), bladeE.body->GetAngle());
        bladeE.body->SetAngularVelocity(1000.0f * timeStep);

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
        bladeE.update(mainM, !isDead && (holdingLeft && holdingRight));

        // Draw hands
        leftHand.update(mainM, holdingLeft && (!isDead || isReset));
        rightHand.update(mainM, holdingRight && (!isDead || isReset));

        // Draw rope
        float tension = distanceOverflow < -1.0f ? -1.0f : distanceOverflow;
        tension = tension > 0.0f ? 0.0f : tension;
        float positiveTension = (holdingLeft && holdingRight && !isDead) ? (tension + 1.0) : 0.0;
        // mixer_ch_set_vol(constants::noiseChannel, positiveTension * 0.4, positiveTension * 0.4);
        gameRope.draw(mainM, positiveTension );

        // Draw ground
        // Draw noise if it is letting through
        if (!isDead || isReset) {
            rdpq_set_other_modes(
                SOM_CYCLE_1 |
                SOM_BLENDING |
                SOM_READ_ENABLE |
                SOM_AA_ENABLE |
                SOM_COLOR_ON_COVERAGE |
                SOM_COVERAGE_DEST_WRAP |
                SOM_ALPHADITHER_ENABLE |
                SOM_ALPHA_COMPARE |
                // (P*A + M*B)
                (cast64(0x0) << 30) | (cast64(0x0) << 28) | (cast64(0x0) << 26) | (cast64(0x0) << 24) |
                (cast64(0x1) << 22) | (cast64(0x0) << 20) | (cast64(0x0) << 18) | (cast64(0x0) << 16) );
        }

        rdpq_set_fill_color(RGBA32(150, 68, 201, 180));
        Box::update(mainM);
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
        sprintf(sbuf, "SCORE: %d", score);
        graphics_draw_text(disp, 60, 20, sbuf);

        // Frame limiter
        // while(TIMER_MICROS_LL(timer_ticks() - lastUpdate) < (timeStep * 1000.0f * 990.0f));

#ifndef NDEBUG
        sprintf(sbuf, "LEVEL: %d", level);
        graphics_draw_text(disp, 60, 44, sbuf);

        sprintf(sbuf, "F: %0.1f", TIMER_MICROS_LL(timer_ticks() - lastUpdate) / 1000.0f);
        graphics_draw_text(disp, 60, 58, sbuf);
        lastUpdate = timer_ticks();
#endif
    }

    Game* new_Game()
    {
        return new Game();
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
