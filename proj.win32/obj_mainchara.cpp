#include "obj_mainchara.h"
#include "UndertaleResourceNode.h"

USING_NS_CC;
static const float movement_Speed = 20.0f;
static const float animateSpeed = 10.0f;
obj_mainchara* obj_mainchara::create() {
	obj_mainchara* obj = new obj_mainchara;
	if (obj && obj->init("obj_mainchara")) {
		obj->setSolid(true);

		//sprite->setScale(4.0f);
		auto eventListener = EventListenerKeyboard::create();
		eventListener->onKeyPressed = [obj](EventKeyboard::KeyCode keyCode, Event* event) {
			switch (keyCode) {
			case EventKeyboard::KeyCode::KEY_I:
				obj->setRotation(obj->getRotation() + 90.0);
				break;
			case EventKeyboard::KeyCode::KEY_A:
				obj->setDirection(0.0f, -2.0f);
				obj->setUndertaleSprite(1046); // left
				obj->setImageSpeed(animateSpeed);
				break;
			case EventKeyboard::KeyCode::KEY_D:
				obj->setDirection(0.0f, 2.0f);
				obj->setUndertaleSprite(1045); // right
				obj->setImageSpeed(animateSpeed);
				break;
			case EventKeyboard::KeyCode::KEY_W:
				obj->setDirection(90.0f, 2.0f);
				obj->setUndertaleSprite(1044); // up
				obj->setImageSpeed(animateSpeed);
				break;
			case EventKeyboard::KeyCode::KEY_S:
				obj->setDirection(90.0f, -2.0f);
				obj->setUndertaleSprite(1043); // down
				obj->setImageSpeed(animateSpeed);
				break;
			}
		};
		eventListener->onKeyReleased = [obj](EventKeyboard::KeyCode keyCode, Event* event) {
			switch (keyCode) {
			case EventKeyboard::KeyCode::KEY_A:
			case EventKeyboard::KeyCode::KEY_D:
			case EventKeyboard::KeyCode::KEY_W:
			case EventKeyboard::KeyCode::KEY_S:
				obj->setSpeed(0.0f);
				obj->setImageSpeed(0.0f);
				obj->setImageIndex(0);
				break;
			}

		};
		obj->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, obj);


		return obj;
	}
	CC_SAFE_DELETE(obj);
	return obj;
}
void obj_mainchara::update(float dt) {
	// we want to check for colisions 
	UObject::update(dt);
	auto o = _room->containsObject([this](auto o) { return o->is(820) && o->colides(this); });
	if (o != nullptr) {
		setPosition(getPosition() - _movementVector);
		setImageSpeed(0.0f);
		setImageIndex(0);
	}
}