#include "FallingDebrisActorComponent.h"
#include <gamelib_locator.hpp>
#include <gamelib_random.hpp>
#include <limits>
#include <algorithm>

namespace GameLib {
	extern void debugDraw(Actor& a);
	extern void debugDrawSweptAABB(Actor& a);
	extern float SweptAABB(Actor& a, Actor& b, glm::vec3& normal);

	void removeActorWhenOnTheGround(Actor& actor, World& world)
	{
		/*int actorPlace = 0;
		for (int ii = 0; ii < world.dynamicActors.size(); ii++)
		{
			if (actor.getId() == world.dynamicActors[ii]->getId())
			{
				actorPlace = ii;
				break;
			}
		}

		std::swap(world.dynamicActors[actorPlace], world.dynamicActors[world.dynamicActors.size() - 1]);
		world.dynamicActors.pop_back();*/
	}

	void FallingDebrisActorComponent::update(Actor& actor, World& world)
	{
		if (actor.isStatic()) {
			staticInfo.t += actor.dt;
			float movement = std::sin(staticInfo.t) * staticInfo.movement;
			actor.position = staticInfo.position;
			if (staticInfo.horizontal) {
				actor.position.x += movement;
			}
			else {
				actor.position.y += movement;
			}
		}

		if (actor.isTrigger()) {
			if (triggerInfo.t > 0.0f) {
				actor.position.y = triggerInfo.position.y + std::sin(50.0f * triggerInfo.t) * 0.25f;
				triggerInfo.t -= actor.dt;
				if (triggerInfo.t < 0.0f) {
					actor.position = triggerInfo.position;
					actor.anim.baseId = actor.spriteId();
				}
			}
		}
	}

	void FallingDebrisActorComponent::beginPlay(Actor& actor)
	{
		if (actor.isStatic())
		{
			staticInfo.horizontal = (random.rd() & 1) == 1;
			staticInfo.movement = random.positive() * 5.0f + 2.0f;
			staticInfo.position = actor.position;
		}
	}

	void FallingDebrisActorComponent::handleCollisionStatic(Actor& a, Actor& b)
	{
		b.velocity.x *= -1;
	}


	void FallingDebrisActorComponent::handleCollisionDynamic(Actor& a, Actor& b)
	{
		return;
		// backup a's position
		glm::vec3 curPosition = a.position;
		glm::vec3 curVelocity = a.velocity;
		a.velocity = a.position - a.lastPosition;
		a.position = a.lastPosition;
		glm::vec3 normal;
		float collisionTime = SweptAABB(a, b, normal);
		if (collisionTime >= 1.0f) {
			a.position = curPosition;
			a.velocity = curVelocity;
			return;
		}
		a.position += a.velocity * collisionTime;
		float timeLeft = 1.0f - collisionTime;
		bool deflecting{ true };
		if (deflecting) {
			a.velocity.x *= timeLeft;
			a.velocity.y *= timeLeft;
			if (std::abs(normal.x) > 0.0001f)
				a.velocity.x = -a.velocity.x;
			if (std::abs(normal.y) > 0.0001f)
				a.velocity.y = -a.velocity.y;
		}
		else {
			glm::vec3 tangent = { normal.y, normal.x, 0.0f };
			float cos_theta = glm::dot(a.velocity, tangent) * timeLeft;
			a.velocity = cos_theta * tangent;
		}
		a.position += a.velocity;
	}


	void FallingDebrisActorComponent::handleCollisionWorld(Actor& actor, World& world)
	{
		// determine whether to move the player up, or to the left
		int ix1 = (int)(actor.position.x);
		int iy1 = (int)(actor.position.y);
		int ix2 = ix1 + 1;
		int iy2 = iy1 + 1;
		float fx1 = actor.position.x - ix1;
		float fy1 = actor.position.y - iy1;
		float fx2 = actor.position.x + actor.size.x - ix2;
		float fy2 = actor.position.y + actor.size.y - iy2;
		constexpr float tw = 1.0f;
		constexpr float th = 1.0f;
		float tx1 = std::floor(actor.position.x);
		float tx2 = tx1 + tw;
		float ty1 = std::floor(actor.position.y);
		float ty2 = tx2 + th;

		bool leftHalf = fx1 < 0.5f;
		bool rightHalf = fx1 > 0.5f;
		bool topHalf = fy1 < 0.5f;
		bool bottomHalf = fy1 > 0.5f;

		bool tl = world.getTile(ix1, iy1).solid();
		bool tr = world.getTile(ix2, iy1).solid();
		bool bl = world.getTile(ix1, iy2).solid();
		bool br = world.getTile(ix2, iy2).solid();
		int collision = 0;
		// ####### We handle all cases of blocks with a switch statement
		// #TL#TR#
		// #######
		// #BL#BR#
		// #######
		collision |= tl ? 1 : 0;
		collision |= tr ? 2 : 0;
		collision |= bl ? 4 : 0;
		collision |= br ? 8 : 0;

		constexpr int BLOCKS_RIGHT = 2 | 8;
		constexpr int BLOCKS_LEFT = 1 | 4;
		constexpr int BLOCKS_ABOVE = 1 | 2;
		constexpr int BLOCKS_BELOW = 4 | 8;

		float x1 = (float)ix2 - actor.size.x;
		float x2 = (float)ix2;
		float y1 = (float)iy2 - actor.size.y;
		float y2 = (float)iy2;
		int move = 0;
		constexpr int MOVE_LT = 1;
		constexpr int MOVE_RT = 2;
		constexpr int MOVE_DN = 4;
		constexpr int MOVE_UP = 8;
		switch (collision) {
		case 0: // nothing, so do nothing
			break;
		case 1: // top left single block
			if (fx1 > fy1)
				move = MOVE_RT;
			else
				move = MOVE_DN;
			break;
		case 2: // top right single block
			if (fy1 < 0.99f)
				move = MOVE_LT;
			else
				move = MOVE_DN;
			break;
		case BLOCKS_ABOVE: // top wall
			move = MOVE_DN;
			break;
		case 4: // bottom left single block
			if (fx1 > 0.99f)
				move = MOVE_RT;
			else
				move = MOVE_RT;
			break;
		case BLOCKS_LEFT: // left wall
			move = MOVE_RT;
			break;
		case 6: // bottom left single block, top right single block
				// .#
				// #.
			if (leftHalf)
				move |= MOVE_LT;
			if (topHalf)
				move |= MOVE_UP;
			if (rightHalf)
				move |= MOVE_RT;
			if (bottomHalf)
				move |= MOVE_DN;
			break;
		case 7: // top left wall
			move = MOVE_RT | MOVE_DN;
			break;
		case 8: // bottom right single block
			if (fx1 < fy1)
				move = MOVE_LT;
			else
				move = MOVE_UP;
			break;
		case 9: // top left single block, bottom right single block
				// #.
				// .#
			if (leftHalf)
				move |= MOVE_LT;
			if (bottomHalf)
				move |= MOVE_DN;
			if (rightHalf)
				move |= MOVE_RT;
			if (topHalf)
				move |= MOVE_UP;
			break;
		case BLOCKS_RIGHT: // right wall
			move = MOVE_LT;
			break;
		case 11: // top right wall
			move = MOVE_LT | MOVE_DN;
			break;
		case BLOCKS_BELOW: // bottom wall
			move = MOVE_UP;
			break;
		case 13: // bottom left wall
			move = MOVE_RT | MOVE_UP;
			break;
		case 14: // bottom right wall
			move = MOVE_LT | MOVE_UP;
			break;
		case 15: // all walls (just do nothing)
			break;
		}

		int velx = 0;
		int vely = 0;
		if (move & MOVE_LT) {
			actor.position.x = x1;
			velx = 1;
		}
		else if (move & MOVE_RT) {
			actor.position.x = x2;
			velx = 1;
		}
		if (move & MOVE_UP) {
			actor.position.y = y1;
			vely = 1;
		}
		else if (move & MOVE_DN) {
			actor.position.y = y2;
			vely = 1;
		}
		constexpr int elastic = 1;
		if (elastic) {
			if (velx)
				actor.velocity.x = -actor.velocity.x;
			if (vely)
			{
				actor.velocity.y = -actor.velocity.y*0.7;
				//removeActorWhenOnTheGround(actor, world);
			}
		}
		else {
			if (velx)
				actor.velocity.x = 0.0f;
			if (vely)
				actor.velocity.y = 0.0f;
		}
	}

	void FallingDebrisActorComponent::beginOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Actor '%d' is now overlapping trigger actor '%d'", a.getId(), b.getId());
	}

	void FallingDebrisActorComponent::endOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Actor '%d' is not overlapping trigger actor '%d'", a.getId(), b.getId());
	}

	void FallingDebrisActorComponent::showText(GameLib::Font& font, Actor& trigger, std::vector<ActorPtr> worldActors)
	{
		font.load("URWClassico-Italic.ttf", 16);
		float cx = Locator::getWorld()->worldSizeX * 0.5f;
		float cy = Locator::getWorld()->worldSizeY * 0.5f;

		float gcx = Locator::getGraphics()->getCenterX();
		float gcy = Locator::getGraphics()->getCenterY();
		switch (trigger.getId())
		{
		case 6:
			font.draw(gcx, gcy+10, "Tatiana: I saw Jake got into the laundry room and he looked suspicious", GameLib::Blue, GameLib::Font::SHADOWED);
			break;
		case 7:
			font.draw(gcx, gcy+10, "Mike: Jake was with me the whole time but Tim has been locked in his apartment all week", GameLib::Blue, GameLib::Font::SHADOWED);
			break;
		case 8:
			font.draw(gcx, gcy+10, "Jake: I did see Tatiana walking around the building like she was surveiling it", GameLib::Blue, GameLib::Font::SHADOWED);
			break;
		case 9:
			font.draw(gcx, gcy+10, "I did it: Mark Maron", GameLib::Blue, GameLib::Font::SHADOWED);
			break;
		case 10:
			worldActors[worldActors.size()-1]->didWeWin = true;
			break;
		default:
			break;
		}
	}

	void FallingDebrisActorComponent::beginTriggerOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Trigger actor '%d' is now overlapped by actor '%d'", a.getId(), b.getId());
		if (triggerInfo.t <= 0.0f) {
			// if we are not already animating
			a.anim.baseId = a.spriteId() + 50;
		}
		triggerInfo.position = a.position;
		triggerInfo.t = 2.0f;
		b.position.x = 1 + random.positive() * (Locator::getWorld()->worldSizeX - 2);
		b.position.y = 1 + random.positive() * (Locator::getWorld()->worldSizeY - 2);
		GameLib::Font newFont{ Locator::getContext() };
		showText(newFont, a, Locator::getWorld()->dynamicActors);
	}

	void FallingDebrisActorComponent::endTriggerOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Trigger actor '%d' is not overlapped by actor '%d'", a.getId(), b.getId());
	}
}
