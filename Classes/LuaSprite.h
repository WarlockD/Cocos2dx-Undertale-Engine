#pragma once
#include "cocos2d.h"
#include "LuaEngine.h"
#include <math.h>

inline float lengthdir_x(float len, float dir) {
	return std::cosf(CC_DEGREES_TO_RADIANS(dir)) * len;
}
inline float lengthdir_y(float len, float dir) {
	return -std::sinf(CC_DEGREES_TO_RADIANS(dir)) * len;
}
inline cocos2d::Vec2 CreateMovementVector(float direction, float speed) {
	float x = std::cosf(CC_DEGREES_TO_RADIANS(direction)) * speed;
	float y = std::sinf(CC_DEGREES_TO_RADIANS(direction)) * speed;
	return cocos2d::Vec2(x, y);
}
class LuaSprite : public cocos2d::Sprite {
protected:
	cocos2d::Vector<cocos2d::SpriteFrame*>* _frames;
	uint32_t _image_index = 0;
	size_t _frameCount = 0;
	cocos2d::Vec2 _movementVector;
	float _image_speed;
	float _current_image_time;
	float _speed;
	float _direction;
	int _waitFrames;
	inline void setMovementVector() {
		float x = std::cosf(CC_DEGREES_TO_RADIANS(_direction)) * _speed;
		float y = std::sinf(CC_DEGREES_TO_RADIANS(_direction)) * _speed;
		_movementVector = cocos2d::Vec2(x, y);
	}
public:
	LuaSprite();
	~LuaSprite();
	static LuaSprite* create(cocos2d::Vector<cocos2d::SpriteFrame*>* frame);
	virtual bool init(cocos2d::Vector<cocos2d::SpriteFrame*>* frame);

	inline void setImageIndex(uint32_t index) { if(index < _frameCount) setSpriteFrame(_frames->at(_image_index = index)); }
	inline uint32_t getImageIndex() const { return _image_index;  }
	inline void setZeroMovement() { _speed = _direction = 0; _movementVector = cocos2d::Vec2(); }
	inline void setSpeed(float value) { _speed = value; setMovementVector(); }
	inline void setDirection(float value) { _direction = value; setMovementVector(); }
	inline float getSpeed() const { return _speed; }
	inline float getDirection() const { return _direction; }
	inline void setImageSpeed(float value) { if (_frameCount > 1) { _image_speed = value; _current_image_time = 0.0f; } }
	inline float getImageSpeed() const { return _image_speed; }

protected:
	void update(float dt) override; 
	// functions for using this sprite like a bullet
	inline virtual void setupBullet(cocos2d::Vec2 pos) { setVisible(true); setPosition(pos); unscheduleUpdate(); } // set's the position of the sprite and makes it invisiable
	inline virtual void fireBullet() { scheduleUpdate();  setVisible(true); }
	inline virtual bool stepBullet(float dt) { return false;  } // this step is run at the start of update, on true return, update dosn't run this frame.
	inline virtual void stopBullet() { setVisible(true);  unscheduleUpdate(); }
};


namespace LuaEngineMetaTableNames {
	template<> constexpr const char* metaTableName<LuaSprite>() { return "LuaSpriteMT"; }
}