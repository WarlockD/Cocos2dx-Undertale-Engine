#pragma once
#include "cocos2d.h"
#include "UndertaleLib.h"

class USprite : public cocos2d::Sprite {
protected:
	cocos2d::Vector<cocos2d::SpriteFrame*> _frames;
	UndertaleLib::Sprite _sprite;
	size_t _image_index;
	float _speed;
	cocos2d::Animate* _animateAction;
	cocos2d::Action* _animationAction;
	void startAnimation();
	void stopAnimation();
public:
	virtual bool init() override;
	virtual bool init(const UndertaleLib::Sprite& sprite);

	USprite() : _image_index(0), _speed(0.0f), _animationAction(nullptr), cocos2d::Sprite() {}
	virtual ~USprite();
	const UndertaleLib::Sprite& getUndertaleSprite() const { return _sprite; }
	void setUndertaleSprite(const UndertaleLib::Sprite& sprite);
	void setUndertaleSprite(size_t sprite_index);

	static USprite* create(const UndertaleLib::Sprite& sprite);
	static USprite* create(size_t sprite_index);
	static USprite* create(const std::string& name);
	CREATE_FUNC(USprite);
	
	virtual void setImageIndex(size_t index);
	size_t getImageIndex() const { return _image_index; }

	void setImageSpeed(float speed);
	float getImageSpeed() const { return _speed; }
};
class URoom;
class UObject : public USprite {
	friend class URoom;
public:
	static constexpr int object_index = -1; // = 1570
	static constexpr char* object_name = "";
protected:
	static constexpr float update_frame = (1.0f / 30.0f); // updates
	URoom* _room;
	bool _isSolid;
	cocos2d::Vec2 _movementVector;
	float _direction, _speed;
	UndertaleLib::Object _object;
	std::set<size_t> _objectIndexEquals; // list of parrents this object is equal too
	std::unordered_set<std::string> _objectNameEquals;
	static UObject* create(const UndertaleLib::Object& object);
	bool init(const UndertaleLib::Object& object);
	bool init(size_t object_index);
	bool init(const std::string& object_name);
#ifdef _DEBUG
private:
	cocos2d::DrawNode* _boxOverObject;
	bool _drawBox;
	bool _drawIndexes;
	bool _drawNonVisisableSprite;
	bool _lastvisiableState;
	bool _selected;
public:
	
	void selectNode(bool value);
	bool getDrawBox() const { return _drawBox; }
	bool getDrawIndex()const { return _drawIndexes; }
	bool getDrawNonVisiableSprite()const { return _drawNonVisisableSprite; }
	void setDrawBox(bool drawBox);
	void setDrawIndex(bool drawIndexes);
	void setDrawNonVisiableSprite(bool drawNonVisiableSprite);
	void checkNode(cocos2d::Node* node);
#endif
public:
	virtual void addChild(Node* child) override { checkNode(child); Node::addChild(child); }
	virtual void addChild(Node* child, int localZOrder) override { checkNode(child); Node::addChild(child, localZOrder); }
	virtual void addChild(Node* child, int localZOrder, int tag) override { checkNode(child);; Node::addChild(child, localZOrder,tag); }
	virtual void addChild(Node* child, int localZOrder, const std::string &name) override { checkNode(child); Node::addChild(child, localZOrder,name); }
	bool is(size_t object_index) const { return _objectIndexEquals.find(object_index) != _objectIndexEquals.cend(); }
	bool is(const std::string& object_name) const { return _objectNameEquals.find(object_name) != _objectNameEquals.cend(); }
	bool colides(UObject* object);
	UObject() : USprite(), _direction(0.0f), _speed(0.0f), _movementVector(0.0f, 0.0f), _isSolid(false), _selected(false) {}
	void setDirection(float direction, float speed) {
		_movementVector = cocos2d::Vec2(std::cosf(direction * M_PI / 180) * speed, sin(direction * M_PI / 180) * speed);
		_direction = direction;
		_speed = speed;
	}
	void setDirection(float direction) { setDirection(direction, _speed); }
	void setSpeed(float speed) { setDirection(_direction, speed); }
	float getDirection() const { return _direction; }
	float getSpeed() const { return _speed; }
	virtual ~UObject() {}
	const UndertaleLib::Object& getUndertaleObject() const { return _object; }
	bool isSolid() const { return _isSolid; }
	void setSolid(bool value);
	UndertaleLib::Object getUndertaleParentObject() const;

	static UObject* create(size_t object_index);
	static UObject* create(const std::string& name);
	virtual void update(float dt) override;

};