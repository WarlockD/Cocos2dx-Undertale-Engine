#include "Chara.h"
#include "UndertaleResources.h"

USING_NS_CC;

namespace Undertale{
	CharaOverworld::CharaOverworld() : _keyStates{ false , false ,  false ,  false } , _currentAnimation(nullptr) {}
	CharaOverworld::~CharaOverworld()
	{
		disableKeyboard();
		stopAllActions();
		for (int i = 0; i < 4; i++) {
			CC_SAFE_RELEASE_NULL(_facingAnimation[i]);
		}
		_movmentAction->release();
		_keyboardListener->release();
	}
	CharaOverworld* CharaOverworld::create() {
		CharaOverworld* c = new CharaOverworld;
		if (c && c->init()) {
			c->autorelease();
			return c;
		}
		CC_SAFE_DELETE(c);
		return nullptr;
	}
	void CharaOverworld::setFacing(CharaDirection value)
	{
		assert(value == CharaDirection::NOTMOVING);
		if (value != _currentFacing) {
			if (isMoving()) {
				if (isMoving()) stopAnimation();
				setSpriteFrame(_facingNormal[(unsigned char)value]);
				_currentlyMoving = CharaDirection::NOTMOVING;
			}
			_currentFacing = value;
		}
	}
	void CharaOverworld::moveDirection(CharaDirection value)
	{
		if (value != _currentlyMoving) {
			if (isMoving()) stopAnimation();
			
			if (value == CharaDirection::NOTMOVING) { // if we arn't moving anymore make sure the facing is lined up
				_currentFacing = _currentlyMoving;
				setSpriteFrame(_facingNormal[(unsigned char)_currentFacing]);
				_movmentAction->setSpeed(0);
			}
			else {
				startAnimation(value);
				switch (value) {
				case CharaDirection::UP: _movmentAction->setDirection(90); break;
				case CharaDirection::RIGHT:_movmentAction->setDirection(0); break;
				case CharaDirection::DOWN:_movmentAction->setDirection(270); break;
				case CharaDirection::LEFT:_movmentAction->setDirection(180); break;

				}
				_movmentAction->setSpeed(2);
			}
			_currentlyMoving = value;
		}
	}
	void CharaOverworld::updateKeys()
	{ // ... It migth be better if I had 4 bools to test for this, 30 times a second? humm
		stopMovement();
		for (int i = 0; i < 4; i++) {
			if (_keyStates[i]) {
				moveDirection((CharaDirection)i); 
				return;
			}
		}
		
	}
	void CharaOverworld::stopMovement() { moveDirection(CharaDirection::NOTMOVING); }

	void CharaOverworld::collided(const UndertaleObject * obj)
	{
		if (obj->isObject("obj_solidparent")) {
			stopMovement();
			runAction(MoveTo::create(0.0, getPosition() + _movmentAction->backwardVector(1)));
		}
	//	CCLOG("Collision with object %s", obj->getObjectName().c_str());
	}
	void CharaOverworld::startAnimation(CharaDirection d) {
		if (_currentAnimation != nullptr) stopAnimation();
		_currentAnimation = Animate::create(_facingAnimation[(int)d]);
		runAction(_currentAnimation);
	}
	void CharaOverworld::stopAnimation() {
		if (_currentAnimation != nullptr) {
			stopAction(_currentAnimation); _currentAnimation = nullptr;
		}
	}
	bool CharaOverworld::init()
	{
		UndertaleResources* res = UndertaleResources::getInstance();
		_facingNormal[(int)CharaDirection::DOWN] = res->getSpriteFrame("spr_maincharad", 0);
		_facingNormal[(int)CharaDirection::RIGHT] = res->getSpriteFrame("spr_maincharar", 0);
		_facingNormal[(int)CharaDirection::UP] = res->getSpriteFrame("spr_maincharau", 0);
		_facingNormal[(int)CharaDirection::LEFT] = res->getSpriteFrame("spr_maincharal", 0);
	

		if (!Sprite::initWithSpriteFrame(_facingNormal[0])) return false;
		_movmentAction = MovementAction::create(0, 0);
		_movmentAction->setTag(2); // the movment action
		//setContentSize(_charaSprite->getContentSize());
		
		runAction(_movmentAction);

	//	scheduleUpdate();
		_currentlyMoving = CharaDirection::NOTMOVING;
		_currentlyFacing = CharaDirection::DOWN;
		_facingAnimation[(int)CharaDirection::DOWN] = Animation::createWithSpriteFrames(*res->getSpriteFrames("spr_maincharad"), 0.25, -1);
		_facingAnimation[(int)CharaDirection::RIGHT] = Animation::createWithSpriteFrames(*res->getSpriteFrames("spr_maincharar"), 0.25, -1);
		_facingAnimation[(int)CharaDirection::UP] = Animation::createWithSpriteFrames(*res->getSpriteFrames("spr_maincharau"), 0.25, -1);
		_facingAnimation[(int)CharaDirection::LEFT] = Animation::createWithSpriteFrames(*res->getSpriteFrames("spr_maincharal"), 0.25, -1);
		for (int i = 0; i < 4; i++) _facingAnimation[i]->retain();

		//moveMe->setScale(2.0f);
	
		_keyboardListener = EventListenerKeyboard::create();
		_keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode key, Event* event) {
			switch (key) {
			case EventKeyboard::KeyCode::KEY_W:
			case EventKeyboard::KeyCode::KEY_UP_ARROW:
				_keyStates[(int)CharaDirection::UP] = false;
				updateKeys();
				event->stopPropagation();
				break;
			case EventKeyboard::KeyCode::KEY_S:
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
				_keyStates[(int)CharaDirection::DOWN] = false;
				updateKeys();
				event->stopPropagation();
				break;
			case EventKeyboard::KeyCode::KEY_A:
			case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
				_keyStates[(int)CharaDirection::LEFT] = false;
				updateKeys();
				event->stopPropagation();
				break;
			case EventKeyboard::KeyCode::KEY_D:
			case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
				_keyStates[(int)CharaDirection::RIGHT] = false;
				updateKeys();
				event->stopPropagation();
				break;
			}
		};
		_keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode key, Event* event) {
			switch (key) {
			case EventKeyboard::KeyCode::KEY_S:
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
				_keyStates[(int)CharaDirection::DOWN] = true;
				updateKeys();
				event->stopPropagation();
				break;
			case EventKeyboard::KeyCode::KEY_D:
			case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
				_keyStates[(int)CharaDirection::RIGHT] = true;
				updateKeys();
				event->stopPropagation();
				break;
			case EventKeyboard::KeyCode::KEY_W:
			case EventKeyboard::KeyCode::KEY_UP_ARROW:
				_keyStates[(int)CharaDirection::UP] = true;
				updateKeys();
				event->stopPropagation();
				break;
			case EventKeyboard::KeyCode::KEY_A:
			case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
				_keyStates[(int)CharaDirection::LEFT] = true;
				updateKeys();
				event->stopPropagation();
				break;
			}
		
		};

		_keyboardListener->retain();
	
		return true;
	}


}

