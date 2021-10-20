#ifndef __B2D_H
#define __B2D_H

#ifdef __cplusplus

#include "box2d/box2d.h"

class Physics
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

    public:
        Physics();
        int update(float delta);
};

#else

typedef struct Physics Physics;

#endif

#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif

// EXPORT_C Physics
EXPORT_C Physics* new_Physics(void);
EXPORT_C void delete_Physics(Physics*);
EXPORT_C int update_Physics(Physics*, float delta);

#endif /* __B2D_H */

