#ifndef __BLADE_H
#define __BLADE_H

#include "box2d/box2d.h"

#include "box.hpp"

extern "C" {
    #include <libdragon.h>
    #include "geometry.h"
}

class Blade : public Box
{
    public:
        Blade(b2World* world);
        void update(b2Mat33& matrix, bool holding);
};


#endif /* __BLADE_H */
