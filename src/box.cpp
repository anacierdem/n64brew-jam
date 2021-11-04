
#include "box2d/box2d.h"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Box
{
    private:
        b2BodyDef bodyDef;
        b2PolygonShape dynamicBox;
        b2FixtureDef fixtureDef;

    public:
        b2Body* body;
        Box(b2World* world) {
            bodyDef.type = b2_dynamicBody;
            // bodyDef.position = position;
            // bodyDef.angle = angle;
            body = world->CreateBody(&bodyDef);
            dynamicBox.SetAsBox(0.5f, 0.5f);

            fixtureDef.shape = &dynamicBox;
            fixtureDef.density = 1.0f;
            fixtureDef.friction = 0.3f;
            fixtureDef.restitution = 0.5f;
            body->CreateFixture(&fixtureDef);
        }
        void update(RdpDisplayList* rdl, b2Vec2 cameraPos, float scale) {
            b2Vec2 vertex1 = body->GetWorldPoint(dynamicBox.m_vertices[0]);
            b2Vec2 vertex2 = body->GetWorldPoint(dynamicBox.m_vertices[1]);
            b2Vec2 vertex3 = body->GetWorldPoint(dynamicBox.m_vertices[3]);

            vertex1 -= cameraPos;
            vertex2 -= cameraPos;
            vertex3 -= cameraPos;

            // TODO: build a simple transform for this
            render_tri_strip(rdl,
                vertex1.x * scale * to16_16,   (vertex1.y) * (scale/2.) * to16_16,
                vertex2.x * scale * to16_16,   (vertex2.y) * (scale/2.) * to16_16,
                vertex3.x * scale * to16_16,   (vertex3.y) * (scale/2.) * to16_16
            );

            vertex1 = body->GetWorldPoint(dynamicBox.m_vertices[2]);
            vertex1 -= cameraPos;

            render_tri_strip_next(rdl, vertex1.x * scale * to16_16,   (vertex1.y) * (scale/2.) * to16_16);
        }
};
