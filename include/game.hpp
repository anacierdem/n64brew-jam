#ifndef __B2D_H
#define __B2D_H

#ifdef __cplusplus

#include <limits>

#include "box2d/box2d.h"
#include "rdl.h"

#include "rope.hpp"
#include "enemy.hpp"
#include "box.hpp"

namespace constants {
    constexpr float to16_16 = 65536.f;
    constexpr float gameAreaWidth = 8.f;
    constexpr float gameAreaHeight = 6.f;
    constexpr int startIncreasingSpeedAtLevel = 20;
    constexpr float swawnSafeRadius = 0.45f; // Must be larget than enemy size
    constexpr float scale = 80.0f;
    constexpr float allowedDistance = 5.0f;
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

        // TODO: move to stack
        static const int box_count = 10;
        Enemy* enemies[box_count];

        bool shouldReset = false;

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
        int level = 0;

        // Animation
        int64_t startedShowingDamage = std::numeric_limits<int64_t>::min();

    public:
        Game(RdpDisplayList* rdlParam);
        int update();
        void updateUI(display_context_t disp);
        void reset();
        void addScore(int points);
};

#else

typedef struct Game Game;

#endif

#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif


#include <libdragon.h>
EXPORT_C Game* new_Game(RdpDisplayList*);
EXPORT_C void delete_Game(Game*);
EXPORT_C int update_Game(Game*);
EXPORT_C void update_UI(Game*, display_context_t disp);

#endif /* __B2D_H */

