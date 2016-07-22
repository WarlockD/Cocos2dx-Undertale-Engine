#include "obj_mainchara.h"

USING_NS_CC;
static const float movement_Speed = 20.0f;
static const float animateSpeed = 10.0f;
obj_mainchara* obj_mainchara::create() {
	obj_mainchara* obj = new obj_mainchara;
	if (obj) {
		obj->setUndertaleObject("obj_mainchara");
		if (!obj->_body) {
			obj->_body = PhysicsBody::createBox(obj->getContentSize());
			obj->_body->setDynamic(true);
			obj->_body->setGravityEnable(false);
			obj->_body->setLinearDamping(1.0f);
			obj->_body->setAngularDamping(1.0f);
			obj->setPhysicsBody(obj->_body);
		}

		//sprite->setScale(4.0f);
		auto eventListener = EventListenerKeyboard::create();
		eventListener->onKeyPressed = [obj](EventKeyboard::KeyCode keyCode, Event* event) {

			Vec2 loc = event->getCurrentTarget()->getPosition();
			switch (keyCode) {
			case EventKeyboard::KeyCode::KEY_A:
				obj->_body->setVelocity(Vec2(-movement_Speed, 0.0));
				obj->_sprite->setUndertaleSprite(1046); // left
				obj->_sprite->setImageSpeed(animateSpeed);
				break;
			case EventKeyboard::KeyCode::KEY_D:
				obj->_body->setVelocity(Vec2(movement_Speed,0.0));
				obj->_sprite->setUndertaleSprite(1045); // right
				obj->_sprite->setImageSpeed(animateSpeed);
				break;
			case EventKeyboard::KeyCode::KEY_W:
				obj->_body->setVelocity(Vec2(0.0f, movement_Speed));
				obj->_sprite->setUndertaleSprite(1044); // up
				obj->_sprite->setImageSpeed(animateSpeed);
				break;
			case EventKeyboard::KeyCode::KEY_S:
				obj->_body->setVelocity(Vec2(0.0f, -movement_Speed));
				obj->_sprite->setUndertaleSprite(1043); // down
				obj->_sprite->setImageSpeed(animateSpeed);
				break;
			}
		};

		obj->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, obj);


		return obj;
	}
	CC_SAFE_DELETE(obj);
	return obj;
}