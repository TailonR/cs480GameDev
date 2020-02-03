#ifndef COLLISION_H
#define COLLISION_H
namespace GameLib {
    class Collision {
    public:
        static bool collisionDetector(const Actor& one, const Actor& theOther);
    };

}

#endif // !COLLISION_H

