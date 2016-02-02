#pragma once
#include "cocos2d.h"
#include "LuaEngine.h"

class LuaSprite : public cocos2d::Sprite {
protected:
	cocos2d::Vector<cocos2d::SpriteFrame*>& _frames;
	uint32_t _image_index = 0;
	size_t _frameCount = 0;
	cocos2d::Vec2 _movementVector;
	float _image_speed;
	float _current_image_time;
	float _speed;
	float _direction;
	inline void setMovementVector() {
		float x = std::cos(_direction) * _speed;
		float y = std::sin(_direction) * _speed;
		_movementVector = cocos2d::Vec2(x, y);
	}
public:
	LuaSprite(Vector<cocos2d::SpriteFrame*>& frames);
	~LuaSprite();
	static LuaSprite* create(const cocos2d::Vector<cocos2d::SpriteFrame*>& frame);
	
	void initOptions();
	void addEvents();
	void setImageIndex(uint32_t index) { if(index < _frameCount) setSpriteFrame(_frames.at(_image_index = index)); }
	uint32_t getImageIndex() const { return _image_index;  }
	void setZeroMovement() { _speed = _direction = 0; _movementVector = cocos2d::Vec2(); }
	void setSpeed(float value) { _speed = value; setMovementVector(); }
	void setDirection(float value) { _direction = value; setMovementVector(); }
	float getSpeed() const { return _speed; }
	float getDirection() const { return _direction; }
	float setImageSpeed(float value) { if (_frameCount > 1) { _image_speed = value; _current_image_time = 0.0f; } }
	float getImageSpeed() const { return _image_speed; }
protected:
	void update(float dt) override;

};


namespace LuaEngineMetaTableNames {
	template<> constexpr const char* metaTableName<LuaSprite>() { return "LuaSpriteMT"; }
}