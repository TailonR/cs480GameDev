#include "MCActorComponent.h"
#include <gamelib_locator.hpp>
#include <gamelib_random.hpp>
#include <limits>

namespace GameLib {
	extern void debugDraw(Actor& a);
	extern void debugDrawSweptAABB(Actor& a);
	extern float SweptAABB(Actor& a, Actor& b, glm::vec3& normal);

	void MainCharacterComponent::update(Actor& actor, World& world)
	{
		if (actor.isStatic()) {
			staticInfo.t += actor.dt;
			float movement = std::sin(staticInfo.t) * staticInfo.movement;
			actor.position = staticInfo.position;
			if (staticInfo.horizontal) {
				actor.position.x += movement;
			} else {
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

	void MainCharacterComponent::beginPlay(Actor& actor)
	{
		if (actor.isStatic())
		{
			staticInfo.horizontal = true;
			staticInfo.movement = 5.0f*1.5f;
			staticInfo.position = actor.position;
		}
	}

	void MainCharacterComponent::handleCollisionStatic(Actor& a, Actor& b)
	{
		b.velocity.x += -1;
	}

	void MainCharacterComponent::handleCollisionDynamic(Actor& a, Actor& b)
	{
		a.velocity.x *= -1;
	}

	void MainCharacterComponent::handleCollisionWorld(Actor& actor, World& world)
	{
		// determine whether to move the player up, or to the left
		int ix1 = (int)(actor.position.x);
		int iy1 = (int)(actor.position.y);
		int ix2 = ix1 + 1; //(int)(a.position.x + a.size.x);
		int iy2 = iy1 + 1; //(int)(a.position.y + a.size.y);
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

		// if ((collision & BLOCKS_RIGHT) && !(collision & BLOCKS_LEFT))
		//    collision = BLOCKS_RIGHT;
		// if (!(collision & BLOCKS_RIGHT) && (collision & BLOCKS_LEFT))
		//    collision = BLOCKS_LEFT;
		// if ((collision & BLOCKS_ABOVE) && !(collision & BLOCKS_BELOW))
		//    collision = BLOCKS_ABOVE;
		// if (!(collision & BLOCKS_ABOVE) && (collision & BLOCKS_BELOW))
		//    collision = BLOCKS_BELOW;

		float x1 = (float)ix2 - actor.size.x;
		float x2 = (float)ix2; // ix1 + 1.0f;
		float y1 = (float)iy2 - actor.size.y;
		float y2 = (float)iy2; // iy1 + 1.0f;
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
				move = MOVE_UP;
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
				actor.velocity.y = -actor.velocity.y;
		}
		else {
			if (velx)
				actor.velocity.x = 0.0f;
			if (vely)
				actor.velocity.y = 0.0f;
		}
	}

	void MainCharacterComponent::beginOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Actor '%d' is now overlapping trigger actor '%d'", a.getId(), b.getId());
	}

	void MainCharacterComponent::endOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Actor '%d' is not overlapping trigger actor '%d'", a.getId(), b.getId());
	}

	void MainCharacterComponent::beginTriggerOverlap(Actor& a, Actor& b)
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
		Locator::getAudio()->playAudio(1, true);
	}

	void MainCharacterComponent::endTriggerOverlap(Actor& a, Actor& b)
	{
		HFLOGDEBUG("Trigger actor '%d' is not overlapped by actor '%d'", a.getId(), b.getId());
	}

}
