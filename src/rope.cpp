
#include "box2d/box2d.h"
#include "box2d/b2_rope.h"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

// TODO: override draw?
// class Draw: b2Draw
// { }

class Rope
{
    private:
        b2RopeDef ropeDef;
        b2Rope rope;
        b2RopeTuning ropeTuning;

        int32 velocityIterations = 6;
        int32 positionIterations = 2;
        float timeStep = 1.0f / 60.0f;

    public:
        Rope(int count, b2Vec2 pos) {
            ropeDef.count = count;

            b2Vec2 vertices[ropeDef.count];
            float masses[ropeDef.count];
            for (int i = 0; i < ropeDef.count; i++)
            {
                masses[i] = i == 0 ? 0.0f : 0.5f;
                vertices[i] = b2Vec2((float)i / 8.0f, (float)i / 4.0f);
            }

            ropeDef.gravity = b2Vec2(0.0f, 10.0f);
            ropeDef.vertices = vertices;
            ropeDef.masses = masses;
            ropeDef.position = pos;

            ropeTuning.bendStiffness= 0.0001f;
            // ropeTuning.bendDamping = 0.1f;
            // ropeTuning.damping = 0.1f;

            rope.Create(ropeDef);
            rope.SetTuning(ropeTuning);
        }
        void update(RdpDisplayList* rdl) {
            b2Vec2 offset(0.02, 0.0);
            rope.Step(timeStep, velocityIterations, ropeDef.position);

            for (int i = 0; i < ropeDef.count - 1; i++)
            {
                b2Vec2 vertex1 = rope.m_ps[i];
                b2Vec2 vertex2 = rope.m_ps[i] + offset;
                b2Vec2 vertex3 = rope.m_ps[i+1];

                render_tri_strip(rdl,
                    vertex1.x * 80. * to16_16,   (vertex1.y) * 40. * to16_16,
                    vertex2.x * 80. * to16_16,   (vertex2.y) * 40. * to16_16,
                    vertex3.x * 80. * to16_16,   (vertex3.y) * 40. * to16_16
                );

                vertex1 = rope.m_ps[i+1] + offset;
                render_tri_strip_next(rdl, vertex1.x * 80. * to16_16,   (vertex1.y) * 40. * to16_16);
            }
        }

        void reset() {
            rope.Reset(ropeDef.position);
        }
};
