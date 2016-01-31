#pragma once
#include "cocos2d.h"
#include "LuaEngine.h"

class LuaSprite : public cocos2d::Sprite {
protected:
	cocos2d::Vector<cocos2d::SpriteFrame*> _frames;
	uint32_t _currentFrame = 0;
	cocos2d::Vec2 _movementVector;
	float _speed;
	float _direction;
	inline void setMovementVector() {
		float x = std::cos(_direction) * _speed;
		float y = std::sin(_direction) * _speed;
		_movementVector = cocos2d::Vec2(x, y);
	}
public:
	LuaSprite();
	~LuaSprite();
	static LuaSprite* create(const cocos2d::Vector<cocos2d::SpriteFrame*>& frame, int start, int size);
	static LuaSprite* create(const cocos2d::Vector<cocos2d::SpriteFrame*>& frame);

	void initOptions();
	void addEvents();
	void setFrame(uint32_t frame) { setSpriteFrame(_frames.at(_currentFrame=frame)); }
	uint32_t getFrame() const { return _currentFrame;  }
	void setZeroMovement() { _speed = _direction = 0; _movementVector = cocos2d::Vec2(); }
	void setSpeed(float value) { _speed = value; setMovementVector(); }
	void setDirection(float value) { _direction = value; setMovementVector(); }
	float getSpeed() const { return _speed; }
	float getDirection() const { return _direction; }
protected:
	void update(float dt) override;

};

namespace LuaEngineMetaTableNames {
	template<> constexpr const char* metaTableName<LuaSprite>() { return "LuaSpriteMT"; }
}