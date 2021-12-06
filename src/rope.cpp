
#include "box2d/box2d.h"
#include "box2d/b2_rope.h"

#include "rope.hpp"
#include "game.hpp"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

// TODO: move things into a PlayerContraption class
Rope::Rope(int count, b2Vec2 pos1, b2Vec2 pos2) {
    ropeDef.count = count;

    if (get_tv_type() != TV_PAL) {
        // I mistakenly used 1/60 for PAL and adjusted everything based on that,
        // correct for that when it is actually at a higher frame rate :)
        timeStep = (1.0 / 60.0) * 0.83333f;
    }

    b2Vec2 vertices[ropeDef.count];
    float masses[ropeDef.count];

    b2Vec2 diff = (pos2 - pos1);
    diff *= 1. / (float)ropeDef.count;

    for (int i = 0; i < ropeDef.count; i++)
    {
        b2Vec2 add = diff;
        add *= (float)i;
        if (i == 0 || i == (ropeDef.count - 1)) {
            masses[i] = 0.0f;
        } else if (i == (ropeDef.count/2)) {
            masses[i] = 5.0f;
        } else {
            masses[i] = 0.5f;
        }
        vertices[i] = add;
    }

    ropeDef.gravity = b2Vec2(0.0f, 10.0f);
    ropeDef.vertices = vertices;
    ropeDef.masses = masses;
    ropeDef.position = pos1;

    ropeTuning.bendStiffness= 0.0001f;

    ropeDef.tuning = ropeTuning;

    rope.Create(ropeDef);
}
b2Vec2 Rope::update(b2Vec2 pos1, b2Vec2 pos2) {
    rope.Step(timeStep, velocityIterations, pos1, pos2);
    return rope.m_ps[ropeDef.count / 2];
}

void Rope::draw(RdpDisplayList* rdl, b2Mat33& matrix, float tension) {
    if (tension == 0.0f) {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(255, 255, 255, 80)));
    } else {
        rdl_push(rdl,RdpSetPrimColor(RDP_COLOR32(
            255 -static_cast<int>(tension * (255 - 160)),
            255 -static_cast<int>(tension * (255 - 78)),
            255 -static_cast<int>(tension * (255 - 221))
        , 80 -static_cast<int>(tension * (80 - 250)))));
    }
    for (int i = 0; i < ropeDef.count - 1; i++)
    {
        b2Vec2 vertex1 = rope.m_ps[i];
        b2Vec2 vertex3 = rope.m_ps[i+1];

        b2Vec2 offset = (vertex3 - vertex1).Skew();
        offset.Normalize();
        offset *= 0.02;

        b2Vec2 vertex2 = rope.m_ps[i] + offset;

        b2Vec3 v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
        b2Vec3 v2 = b2Mul(matrix, b2Vec3(vertex2.x, vertex2.y, 1.));
        b2Vec3 v3 = b2Mul(matrix, b2Vec3(vertex3.x, vertex3.y, 1.));

        render_tri_strip(rdl,
            v1.x, v1.y,
            v2.x, v2.y,
            v3.x, v3.y
        );

        vertex1 = rope.m_ps[i+1] + offset;
        v1 = b2Mul(matrix, b2Vec3(vertex1.x, vertex1.y, 1.));
        render_tri_strip_next(rdl, v1.x, v1.y);
    }
}

void Rope::reset() {
    rope.Reset(ropeDef.position);
}