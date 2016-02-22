#pragma once
#include "cocos2d.h"
#include "UndertaleResources.h"
#include "LuaSprite.h"
#include "LuaFont.h"
#include "Keyboard.h"

namespace Undertale {
	enum class CharaDirection : unsigned char { UP = 0, LEFT, DOWN, RIGHT, NOTMOVING = 0x10 };

	class CharaOverworld : public cocos2d::Sprite {
	protected:
		cocos2d::EventListenerKeyboard* _keyboardListener;
		MovementAction* _movmentAction;
		cocos2d::Animation*  _facingAnimation[4];
		cocos2d::Animate* _currentAnimation;
		cocos2d::SpriteFrame*  _facingNormal[4];
		bool _keyStates[4];
		CharaDirection _currentlyFacing;
		CharaDirection _currentlyMoving;
		bool init() override;
		CC_SYNTHESIZE_READONLY(CharaDirection, _currentFacing, Facing)
			void updateKeys();
		void startAnimation(CharaDirection d);
		void stopAnimation();
	public:
		CharaOverworld();
		cocos2d::Rect getWorldBox() const {
			cocos2d::Rect r = this->getBoundingBox();
			r.origin = this->getParent()->convertToWorldSpace(this->getPosition());
			return r;
		}
		virtual ~CharaOverworld();
		static CharaOverworld* create();
		void setFacing(CharaDirection value);
		void moveDirection(CharaDirection value);
		//void update(float dt) override;
		void enableKeyboard() { getEventDispatcher()->addEventListenerWithFixedPriority(_keyboardListener, 30); }
		void disableKeyboard() { getEventDispatcher()->removeEventListener(_keyboardListener); }
		void stopMovement();
		CharaDirection getMovingDirection() const { return _currentlyMoving; }
		inline bool isMoving() const { return _currentlyMoving != CharaDirection::NOTMOVING; }
		void collided(const UndertaleObject* obj);
	};
}