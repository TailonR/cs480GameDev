#include "FallingDebrisInputComponent.h"
#include "FallingDebrisPhysicsComponent.h"
#include <gamelib_locator.hpp>

namespace GameLib
{
	void FallingDebrisInputComponent::update(Actor& actor) {
		if (actor.position.y >= 0 && actor.position.y < Locator::getWorld()->worldSizeY-10 ) {
			actor.velocity.y += 0.001f;
			actor.velocity.x = 0;
		}
		else {
			actor.velocity.y -= 0.001f;
			actor.velocity.x = 0;
		}
	}
}
