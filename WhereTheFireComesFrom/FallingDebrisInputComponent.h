#ifndef MCINPUTCOMPONENT_H
#define MCINPUTCOMPONENT_H

#include <gamelib_actor.hpp>

namespace GameLib
{
	class FallingDebrisInputComponent : public InputComponentForDynamic
	{
		void update(Actor& actor) override;
	};
}

#endif // !MCINPUTCOMPONENT_H
