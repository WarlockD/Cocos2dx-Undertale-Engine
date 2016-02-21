#pragma once
#include "cocos2d.h"
#include "LuaSprite.h"
#include "LuaFont.h"

namespace Undertale {
	enum class CharaDirection : unsigned char { UP = 0, LEFT, DOWN, RIGHT, NOTMOVING = 0x10 };

	class CharaOverworld : public cocos2d::Sprite {
	protected:
		MovementAction* _movmentAction;
		cocos2d::Action*  _facingAnimation[4];
		cocos2d::SpriteFrame*  _facingNormal[4];
	//	UndertaleSpriteFrames _facing[4]; // north, south, east, west
		CharaDirection _currentlyFacing;
		CharaDirection _currentlyMoving;
		bool init() override;
		CC_SYNTHESIZE_READONLY(CharaDirection, _currentFacing, Facing)
	public:
		CharaOverworld();
		virtual ~CharaOverworld();
		static CharaOverworld* create();
		void setFacing(CharaDirection value);
		void moveDirection(CharaDirection value);
		void stopMovement();
		inline bool isMoving() const { return _currentlyMoving != CharaDirection::NOTMOVING; }
	};
};