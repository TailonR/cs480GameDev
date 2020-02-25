#ifndef MCACTORCOMPONENT_H
#define MCACTORCOMPONENT_H

#include <gamelib_actor_component.hpp>
#include <gamelib_story_screen.hpp>
#include "Game.hpp"

namespace GameLib
{
	class FallingDebrisActorComponent : public ActorComponent
	{
		public:
		virtual ~FallingDebrisActorComponent() {}

		void update(Actor&, World&) override;
		void beginPlay(Actor&) override;
		void handleCollisionStatic(Actor&, Actor&) override;
		void handleCollisionDynamic(Actor&, Actor&) override;
		void handleCollisionWorld(Actor&, World&) override;
		void beginOverlap(Actor&, Actor&) override;
		void endOverlap(Actor&, Actor&) override;
		void beginTriggerOverlap(Actor&, Actor&) override;
		void endTriggerOverlap(Actor&, Actor&) override;
		friend void removeActorWhenOnTheGround(Actor&, World&);
		void showText(GameLib::Font&, Actor&, std::vector<ActorPtr>);
		private:
			struct STATICINFO 
			{
				bool horizontal{false};
				float movement{2.0f};
				glm::vec3 position;
				float t{0.0f};
			} staticInfo;

			struct TRIGGERINFO {
				glm::vec3 position;
				float t{0.0f};
			} triggerInfo;

			//bool didWeWin = false;
	};
}

#endif //MCACTORCOMPONENT_H
