#include "UObject.h"
#include "UndertaleResourceNode.h"

USING_NS_CC;
#ifdef _DEBUG
extern UndertaleLabel* s_debugText;

#endif



USprite::~USprite() {
	CC_SAFE_RELEASE_NULL(_animationAction);
}
bool USprite::init(const UndertaleLib::Sprite& sprite) {
	if (Sprite::init() && sprite.valid()) {
		auto res = Undertale::getSingleton();
		if (_animationAction != nullptr) {
			stopAction(_animationAction); _animationAction = nullptr;
		}
		_frames.clear();
		_sprite = sprite;
		setContentSize(Size(sprite.width(), sprite.height()));
		//	Vec2 anchor_point((float)sprite.origin_x() / (float)sprite.width(), (float)sprite.origin_y() / (float)sprite.height());
		//	Vec2 anchor_point(0.0f, 1.0f);
		///	anchor_point.y = 1.0 - anchor_point.y;
		//	this->setAnchorPoint(anchor_point);
		for (auto& f : sprite.frames()) if (f.valid()) _frames.pushBack(res->createSpriteFrame(f));
		_image_index = 0;
		setSpriteFrame(_frames.at(0));
		setName(sprite.name().c_str());
		setTag(sprite.index());
		// we make a point that is the 0,0 point
		DrawNode* node = DrawNode::create();
		node->drawLine(Vec2(-2, 0), Vec2(2, 0), Color4F::GREEN);
		node->drawLine(Vec2(0, -2), Vec2(0, 2), Color4F::GREEN);
		node->setRotation(45);
		node->setGlobalZOrder(1000);
		addChild(node, 1000);
		return true;
	}
	return false;
}
void USprite::setUndertaleSprite(size_t sprite_index) {
	assert(init(Undertale::getFile()->LookupSprite(sprite_index)));
}


USprite* USprite::create(const UndertaleLib::Sprite& sprite) {
	if (sprite.valid()) {
		USprite* usprite = new USprite;
		if (usprite && usprite->init(sprite)) {
			usprite->autorelease();
			return usprite;
		}
		CC_SAFE_DELETE(usprite);
	}
	return nullptr;
}

USprite* USprite::create(size_t sprite_index) {
	return create(Undertale::getFile()->LookupSprite(sprite_index));
}
USprite* USprite::create(const std::string& name) { return create(Undertale::getFile()->LookupByName<UndertaleLib::Sprite>(name.c_str())); }
bool USprite::init() {
	return Sprite::init();
}


void USprite::setImageIndex(size_t index) {
	index %= _frames.size();
	if (_image_index != index) setSpriteFrame(_frames.at(_image_index));
}
void USprite::startAnimation() {
	if (_animationAction != nullptr)
		_animateAction->getAnimation()->setDelayPerUnit(_speed * (1.0f / 30.0f));
	else {
		_animateAction = Animate::create(Animation::createWithSpriteFrames(_frames, _speed * (1.0f / 30.0f)));
		_animationAction = RepeatForever::create(_animateAction);
		runAction(_animationAction);
	}
}
void USprite::stopAnimation() {
	if (_animationAction != nullptr) {
		stopAction(_animationAction);
		_animationAction = nullptr;
		_animateAction = nullptr;
	}
}
void USprite::setImageSpeed(float speed) {
	_speed = speed;
	if (_speed == 0.0f) stopAnimation();
	else startAnimation();
}
void URoom::setUndertaleRoom(size_t room_index) {
	setUndertaleRoom(Undertale::getFile()->LookupRoom(room_index));
}

UndertaleLib::Object UObject::getUndertaleParentObject() const {
	if (_object.parent_index() >= 0)
		return Undertale::getFile()->LookupObject(_object.parent_index());
	else
		return UndertaleLib::Object();
}

bool UObject::colides(UObject* object) {
	if (this != object) { // sanity check
		Rect thisRect = getBoundingBox();
		Rect otherRect = object->getBoundingBox();
		assert(is(820));
		return thisRect.intersectsRect(otherRect);
	}
	return false;
}
void UObject::setSolid(bool value) {
	if (value != _isSolid) {
		_isSolid = value;
		// draw the hard box around this room

	}


	
}


void UObject::update(float dt) {
	if (_speed != 0.0f) setPosition(getPosition() + _movementVector);
	USprite::update(dt);
}

#ifdef _DEBUG
void UObject::setDrawBox(bool drawBox) {
	if (drawBox != _drawBox) {
		_drawBox = drawBox;
		if (_drawBox) {
			auto drawNode = DrawNode::create(1);
			drawNode->drawRect(Vec2(0, 0), getContentSize(), Color4F::WHITE);
			drawNode->drawPoint(getContentSize() / 2, 1.0f, Color4F::WHITE);
			//drawNode->setPosition(getContentSize() / 2);
			drawNode->setName("drawBox");
			addChild(drawNode, 1000);
		}
		else removeChildByName("drawBox");
	}
}
void UObject::setDrawIndex(bool drawIndexes) {
	if (drawIndexes != _drawIndexes) {
		_drawIndexes = drawIndexes;
		if (_drawIndexes) {
			std::stringstream ss;
			ss.precision(2);
			ss << '(' << std::fixed << getPosition().x << ',' <<  getPosition().y << ')' << std::endl;
			bool comma = false;
			for (auto n : _objectIndexEquals) {
				if (comma) ss << ','; else comma = true;
				ss << n; 
			}

			auto label = UndertaleLabel::create(2);
			label->setAnchorPoint(Vec2(0.0f, 0.0f));
			label->setScale(0.5f);
			label->setString(ss.str());
			//label->setAnchorPoint(Vec2(0.0, -1.0f));
			Vec2 pos = getContentSize()/2; // center
			//label->setPosition(pos);
			label->setName("drawIndexes");
			addChild(label, 1000);
		}
		else removeChildByName("drawIndexes");
	}
}
void UObject::setDrawNonVisiableSprite(bool drawNonVisiableSprite) {
	if (drawNonVisiableSprite != _drawNonVisisableSprite) {
		_drawNonVisisableSprite = drawNonVisiableSprite;
		if (_drawNonVisisableSprite) {
			_lastvisiableState = isVisible();
			setVisible(true);
		}
	}
	else {
		setVisible(_lastvisiableState);
	}
}
#endif

void UObject::selectNode(bool value) {
	if (value != _selected){
		_selected = value;
		if (_selected) {
			std::stringstream ss;
			ss.precision(2);
			ss << _object.to_string() << std::endl;
			;
			for (auto parent = Undertale::getFile()->LookupObject(_object.parent_index());
				parent.valid();
				parent = Undertale::getFile()->LookupObject(parent.parent_index())) {
				ss << "-" << parent.to_string() << std::endl;
			}
			ss << '(' << std::fixed << getPosition().x << ',' << getPosition().y << ')' << std::endl;
			ss << '[';
			s_debugText->setString(ss.str());
			auto drawNode = DrawNode::create(1);
			drawNode->drawRect(Vec2(0, 0), getContentSize(), Color4F::RED);
			drawNode->drawPoint(getContentSize() / 2, 1.0f, Color4F::RED);
			//drawNode->setPosition(getContentSize() / 2);
			drawNode->setName("selectedBox");
			addChild(drawNode, 1000);

		}
		else removeChildByName("selectedBox");
	}
	
}
void UObject::checkNode(cocos2d::Node* node) {
	UObject* obj = dynamic_cast<UObject*>(node);
	if (obj) obj->_room = _room;
}
bool UObject::init(const UndertaleLib::Object& object) {
	if (object.valid() && Node::init()) {
		_object = object;
#ifdef _DEBUG
		_drawBox = false;
		_drawIndexes = false;
		_drawNonVisisableSprite = false;
		_lastvisiableState = false;
#endif

		setLocalZOrder(object.depth());
		setName(object.name().c_str());
		setTag(object.index());
		setSolid(object.solid());

		if (object.sprite_index() > -1) {
			setUndertaleSprite(object.sprite_index());
		}
		_objectNameEquals.insert(object.name().c_str());
		_objectIndexEquals.insert(object.index());
		for (auto parent = Undertale::getFile()->LookupObject(object.parent_index());
			parent.valid();
			parent = Undertale::getFile()->LookupObject(parent.parent_index())
			) {
			_objectNameEquals.insert(parent.name().c_str());
			_objectIndexEquals.insert(parent.index());
		}

		scheduleUpdate();
#ifdef _DEBUG
		auto moseListener = EventListenerMouse::create();


		moseListener->onMouseMove = [this](EventMouse* event) {
			auto target = static_cast<UObject*>(event->getCurrentTarget());
			if (!target)return;
			Point locationInNode = target->convertToNodeSpace(event->getLocationInView());
			Size s = target->getContentSize();
			Rect rect = Rect(0, 0, s.width, s.height);

			//Check the click area
			if (rect.containsPoint(locationInNode))
				target->selectNode(true);
			else
				target->selectNode(false);
		};

		this->_eventDispatcher->addEventListenerWithSceneGraphPriority(moseListener, this);
#endif
		return true;
	}
	return false;
}
bool UObject::init(size_t object_index) { return init(Undertale::getFile()->LookupObject(object_index)); }
bool UObject::init(const std::string& name) { return init(Undertale::getFile()->LookupByName<UndertaleLib::Object>(name.c_str())); }


UObject* UObject::create(const UndertaleLib::Object& object) {
	UObject* obj = new UObject;
	if (obj && obj->init(object)) {
		obj->autorelease();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}

UObject* UObject::create(size_t object_index) {
	return create(Undertale::getFile()->LookupObject(object_index));
}
UObject* UObject::create(const std::string& name) {
	return create(Undertale::getFile()->LookupByName<UndertaleLib::Object>(name.c_str()));
}