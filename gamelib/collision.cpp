#include "pch.h"
#include "gamelib_actor.hpp"
#include "collision.h"
#include <ostream>
namespace GameLib {
    bool Collision::collisionDetector(const Actor& a, const Actor& b) {
        bool XaxisColl = a.position.x + a.size.x >= b.position.x && b.position.x + b.size.x >= b.position.x;
        bool YaxisColl = a.position.y + a.size.y >= b.position.y && b.position.y + b.size.y >= b.position.y;
         std::cout << "this is result of collisionDetector: " << (XaxisColl && YaxisColl) << std::endl;
        return XaxisColl && YaxisColl;
    }
}
