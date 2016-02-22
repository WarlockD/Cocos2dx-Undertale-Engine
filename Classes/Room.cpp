#include "Room.h"

USING_NS_CC;

namespace Undertale {

	Room::Room() : _allObjectsVisible(false), _chara(nullptr) , _collisionTestMode(true), _currentObjectTouched(nullptr) {}
	Room::~Room() {
		stopAllActions();
		//CC_SAFE_RELEASE_NULL(_chara);
	}
	void Room::setBackgroundVisible(uint32_t index, bool value)
	{
		if (index < _backgrounds.size()) {
			RoomBackground * b = _backgrounds.at(index);
			CCLOG("Setting '%s'(%i/%i) = %s", b->getName().c_str(), index, _backgrounds.size(), value ? "true" : "false");
			b->setVisible(value);
		}
	}
	bool Room::getBackgroundVisible(uint32_t index) const {
		if (index < _backgrounds.size()) {
			return _backgrounds.at(index)->_visible;
		}
		return false;
	}
	Room * Room::create(uint32_t index)
	{
		Room* c = new Room;
		if (c && c->init(index)) {
			c->autorelease();
			return c;
		}
		CC_SAFE_DELETE(c);
		return nullptr;
	}

	Room * Room::create(istring name)
	{
		Room* c = new Room;
		if (c && c->init(name)) {
			c->autorelease();
			return c;
		}
		CC_SAFE_DELETE(c);
		return nullptr;
	}

	void Room::update(float dt)
	{
		if (_chara && _chara->isMoving()) {
			bool touchedSomething = false;
			for (const auto& b : _objects) {
				if (b->getContentSize().equals(Size::ZERO)) continue;
				const Rect& targetRect = _chara->getBoundingBox();
				const Rect& wallRect = b->getBoundingBox();
				if (wallRect.intersectsRect(targetRect)) {
					touchedSomething = true;
					if (_currentObjectTouched == b) continue;
					_currentObjectTouched = b;
					//_chara->moveDirection(CharaDirection::NOTMOVING);
					CCLOG("Collision between char and %s (%f,%f)", b->getUndertaleObject()->getFullName().c_str(), wallRect.origin.x, wallRect.origin.y);
					if (_collisionTestMode) _chara->collided(b->getUndertaleObject());
					b->setColor(Color3B::GREEN);
				}
			}
			if (!touchedSomething) {
				if (_currentObjectTouched) _currentObjectTouched->setColor(Color3B::BLACK);
				_currentObjectTouched = nullptr;
			}
		}
	}
	bool Room::loadRoom(istring name) {
		return internalLoadRoom(name);
	}
	bool Room::loadRoom(uint32_t index)
	{
		return internalLoadRoom(index);
	}
	bool Room::loadNextRoom() {
		return internalLoadRoom(_roomIndex + 1);
	}
	bool Room::loadPreviousRoom() {
		return internalLoadRoom(_roomIndex - 1);
	}
	void Room::setAllObjectsVisible(bool value)
	{
		if (_allObjectsVisible != value) {
			for (auto& obj : _objects) {
				if (obj->getContentSize().equals(Size::ZERO)) continue;
				if (value) obj->setVisible(value);
				else obj->setVisible(obj->getUndertaleObject()->getVisibleAtStart());
			}
			_allObjectsVisible = value;
		}

	}


	void Room::updateChara()
	{
		if (_chara == nullptr) {
			_chara = Undertale::CharaOverworld::create();
			addChild(_chara);
			RoomObject* obj = getObject("obj_mainchara");
			if (obj == nullptr) return;
			_chara->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
			_chara->setPosition(obj->getStartingPosition());
			_chara->setZOrder(obj->getUndertaleObject()->getDepth());
			obj->setVisible(false);
			removeChild(obj);
			Size visibleSize = Director::getInstance()->getVisibleSize();
			Vec2 origin = Director::getInstance()->getVisibleOrigin();
			Vec2 deadCenter(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
			_chara->enableKeyboard();
			//deadCenter += 
			setPosition(this->convertToWorldSpace(_chara->getPosition()));

			runAction(Follow::create(_chara));

		}
	}

}