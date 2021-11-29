#ifndef __B2D_H
#define __B2D_H

#ifdef __cplusplus

#include <limits>
#include <algorithm>
#include <stdlib.h>

#include "box2d/box2d.h"

#include "hand.hpp"
#include "rope.hpp"
#include "blade.hpp"
#include "enemy.hpp"
#include "box.hpp"

extern "C" {
    #include <libdragon.h>
    #include "rdl.h"
    #include "geometry.h"
}

class Enemy;

namespace constants {
    constexpr float to16_16 = 65536.f;
    constexpr float gameAreaWidth = 8.f;
    constexpr float gameAreaHeight = 6.f;
    constexpr float swawnSafeRadius = 0.45f; // Must be larget than enemy size
    constexpr float scale = 80.0f;
    constexpr float allowedDistance = 5.0f;
    constexpr int gracePeriodMs = 600;
    constexpr float healthRate = 0.02;
    constexpr int startCount = 8;

    // Gameplay - original value commented
    constexpr float speedPerLevel = 0.3f; // 0.3f
    constexpr int maxLevel = 30; // 30
    constexpr int startLevel = 0; // 0
    constexpr int maxCount = 20; // 20
    constexpr int maxLives = 3; // 3
    constexpr int startIncreasingSpeedAtLevel = constants::maxCount - constants::startCount; // constants::maxCount - constants::startCount

    // Audio channels
    constexpr int hitTakenChannel = 0;
    constexpr int jumpChannel = 0;
    constexpr int pickupChannel = 1;
    constexpr int healthChannel = 2;
    constexpr int gameoverChannel = 2;
    constexpr int musicChannel1 = 3;
    constexpr int musicChannel2 = 5;
}

enum CollisionCategory: uint16
{
    enemy = 0x0001,
    environment = 0x0002,
    hand = 0x0004,
    blade = 0x0008,
};

class Game : public b2ContactListener, Box
{
    private:
        void BeginContact(b2Contact* contact);

        RdpDisplayList* rdl;

        // Simulation params
        int32 velocityIterations = 6;
        int32 positionIterations = 2;
        float timeStep = 1.0f / 60.0f;

        b2Vec3 cameraPos = {0., 0., 1.};

        // TODO: move to stack - how to keep the original pointer
        // created when the array is static?
        static const int box_count = constants::maxCount;
        Enemy* enemies[box_count];

        // Controller data
        int controllers = 0;
        struct controller_data keys;
        struct controller_data keysDown;
        bool holdingLeft = false;
        bool holdingRight = false;

        // Gameplay
        int highScore = 0;
        int score = 0;
        int lives = 3;
        bool isDead = true;
        bool isReset = true;
        int level = 0;

        // Animation
        int64_t startedShowingDamage = std::numeric_limits<int64_t>::min();

        // Audio
        wav64_t collectHealth, gameover, pickup, jump, ambient[2];
        int64_t nextCallback = 0;
        int loop1Callback();
        int loop2Callback();

#ifndef NDEBUG
        int64_t lastUpdate = std::numeric_limits<int64_t>::min();
#endif

        // We increase difficulty after this many seconds are elapsed
        // lvl 15 is reached in 10 + 15 + 15 + 20 + 11*30 < 6 minutes
        int64_t levelSwitch[13] = {
            10, 15, 15, 20, 25, // lvl 5 in ~1.5 min
            30, 30, 45, 45, 60, // lvl 10 in ~5min
            60, 60, 75, // lvl 20 in ~11min
            // then 5 lvls in 6min
        };
        int currentIndex = 0;
        int64_t lastLevelIncreaseAt = std::numeric_limits<int64_t>::min();
    public:
        Game(RdpDisplayList* rdlParam);
        void updateBG();
        void update();
        void updateUI(display_context_t disp);
        void start();
        void reset();
        void gameOver();
        int addScore(int points);
};

#else

typedef struct Game Game;

#endif

#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif

EXPORT_C Game* new_Game(RdpDisplayList*);
// TODO: implement cleanup routines
EXPORT_C void delete_Game(Game*);

EXPORT_C void update_BG(Game*);
EXPORT_C void update_Game(Game*);
EXPORT_C void update_UI(Game*, display_context_t disp);

#endif /* __B2D_H */

