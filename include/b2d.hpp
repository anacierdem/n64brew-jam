#ifndef __B2D_H
#define __B2D_H

#ifdef __cplusplus

#include "box2d/box2d.h"
#include "rdl.h"

class Game
{
    private:
        b2BodyDef groundBodyDef;
        b2Body* groundBody;
        b2PolygonShape groundBox;
        b2BodyDef bodyDef;
        b2Body* body;
        b2PolygonShape dynamicBox;
        b2FixtureDef fixtureDef;
        int32 velocityIterations = 6;
        int32 positionIterations = 2;
        float timeStep = 1.0f / 60.0f;

        RdpDisplayList* rdl;

    public:
        Game(RdpDisplayList* rdlParam);
        int update();
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
EXPORT_C void delete_Game(Game*);
EXPORT_C int update_Game(Game*);

#endif /* __B2D_H */

