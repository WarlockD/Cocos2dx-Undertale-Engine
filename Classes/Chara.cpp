#include "Chara.h"
#include "UndertaleResources.h"

USING_NS_CC;

namespace Undertale{
	CharaOverworld::CharaOverworld()  {}
	CharaOverworld::~CharaOverworld()
	{
		for (int i = 0; i < 4; i++) {
			CC_SAFE_RELEASE_NULL(_facingAnimation[i]);
			CC_SAFE_RELEASE_NULL(_facingNormal[i]);
		}
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
		if (value != _currentFacing) {
			if (isMoving()) {
				stopActionByTag(1);
				setSpriteFrame(_facingNormal[(unsigned char)value]);
				_currentlyMoving = _currentFacing;
			}
			_currentFacing = value;
		}
	}
	void CharaOverworld::moveDirection(CharaDirection value)
	{
		if (value != _currentlyMoving) {
			stopActionByTag(1);
			if (value == CharaDirection::NOTMOVING) { // if we arn't moving anymore make sure the facing is lined up
				_currentFacing = _currentlyMoving;
				setSpriteFrame(_facingNormal[(unsigned char)_currentFacing]);
				_movmentAction->setSpeed(0);
			}
			else {
				runAction(_facingAnimation[(unsigned char)value]);
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
	void CharaOverworld::stopMovement() { moveDirection(CharaDirection::NOTMOVING); }

	bool CharaOverworld::init()
	{
		UndertaleResources* res = UndertaleResources::getInstance();
		_facingNormal[2] = res->getSpriteFrame("spr_maincharad", 0);
		_facingNormal[3] = res->getSpriteFrame("spr_maincharar", 0);
		_facingNormal[0] = res->getSpriteFrame("spr_maincharau", 0);
		_facingNormal[1] = res->getSpriteFrame("spr_maincharal", 0);
		if (!Sprite::initWithSpriteFrame(_facingNormal[0])) return false;
		_movmentAction = MovementAction::create(0, 0);
		_movmentAction->setTag(2); // the movment action
		//setContentSize(_charaSprite->getContentSize());
		_facingAnimation[2] = Animate::create(Animation::createWithSpriteFrames(res->getSpriteFrames("spr_maincharad"), 0.25, -1));
		_facingAnimation[3] = Animate::create(Animation::createWithSpriteFrames(res->getSpriteFrames("spr_maincharar"), 0.25, -1));
		_facingAnimation[0] = Animate::create(Animation::createWithSpriteFrames(res->getSpriteFrames("spr_maincharau"), 0.25, -1));
		_facingAnimation[1] = Animate::create(Animation::createWithSpriteFrames(res->getSpriteFrames("spr_maincharal"), 0.25, -1));
		runAction(_movmentAction);

		_currentlyMoving = CharaDirection::NOTMOVING;
		_currentlyFacing = CharaDirection::DOWN;

		for (int i = 0; i < 4; i++) {
			_facingAnimation[i]->retain();
			_facingAnimation[i]->setTag(1); // tag one is the moving animation
			_facingNormal[i]->retain();
		}
		//moveMe->setScale(2.0f);
		auto keyboardListener = EventListenerKeyboard::create();
		keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode key, Event* event) {
			if (!isMoving()) return;
			switch (key) {
			case EventKeyboard::KeyCode::KEY_W:
			case EventKeyboard::KeyCode::KEY_UP_ARROW:
				//	moveMe->setPositionY(moveMe->getPositionY() - 10);
				break;
			case EventKeyboard::KeyCode::KEY_S:
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
				//	moveMe->setPositionY(moveMe->getPositionY() + 10);
				break;
			case EventKeyboard::KeyCode::KEY_A:
			case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
				//	moveMe->setPositionX(moveMe->getPositionX() - 10);
				break;
			case EventKeyboard::KeyCode::KEY_D:
			case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
				//	moveMe->setPositionX(moveMe->getPositionX() + 10);
				break;
			}
			stopMovement();
		};
		keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode key, Event* event) {
			switch (key) {
			case EventKeyboard::KeyCode::KEY_S:
			case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
				moveDirection(CharaDirection::DOWN);
				break;
			case EventKeyboard::KeyCode::KEY_D:
			case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
				moveDirection(CharaDirection::RIGHT);
				break;
			case EventKeyboard::KeyCode::KEY_W:
			case EventKeyboard::KeyCode::KEY_UP_ARROW:
				moveDirection(CharaDirection::UP);
				break;
			case EventKeyboard::KeyCode::KEY_A:
			case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
				moveDirection(CharaDirection::LEFT);
				break;
			}
		};
		getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, this);
		//if (!Sprite::initWithSpriteFrame(res->getSpriteFrame("spr_maincharad", 0))) return false;
		//_facing[0] = res->getSpriteFrames("spr_maincharad");
		//_facing[1] = res->getSpriteFrames("spr_maincharar");
		//_facing[2] = res->getSpriteFrames("spr_maincharau");
		//_facing[3] = res->getSpriteFrames("spr_maincharal");
		//_currentFacing = CharaDirection::SOUTH;
		return true;
	}


}

