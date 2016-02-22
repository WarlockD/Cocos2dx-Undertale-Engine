#pragma once
#include "cocos2d.h"
#include "LuaEngine.h"
#include <math.h>

typedef const cocos2d::Vector<cocos2d::SpriteFrame*>* LuaSpriteFrames;

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
class TimedAction : public cocos2d::Action {
protected:
	bool _runForever;
	float _currentDuration;;
	CC_SYNTHESIZE_READONLY(int, _duration, Duration);
public:
	TimedAction() : _runForever(true), _currentDuration(0.0f), _duration(0.0f) {}
	void runForever() {
		_runForever = true;
		_duration = 0.0f;
	}
	void setDuration(float duration) {
		if (duration != _duration) {
			_duration = duration;
			_runForever = std::isinf(duration) || duration == 0.0f;
		}
	}
	bool init(float duration) {
		setDuration(duration);
		return true;
	}
	bool isDone() const override { return !_runForever && _currentDuration < 0.0f; }
	void step(float t) override {
		if (!_runForever) { 	// if(!std::isinf(_duration)) // preformance iffy per frame
			if (_currentDuration < 0.0f) return;
			_currentDuration -= t;
		}
	}
	CC_DISALLOW_COPY_AND_ASSIGN(TimedAction);
};
// moves the node, never stops moving unless you change the speed
class MovementAction : public TimedAction {
	cocos2d::Vec2 _movementVector;
	void updateVector() {
		float x = std::roundf(std::cosf(CC_DEGREES_TO_RADIANS(_direction)) * _speed);
		float y = std::roundf(std::sinf(CC_DEGREES_TO_RADIANS(_direction)) * _speed);
		_movementVector = cocos2d::Vec2(x, y);
	}
	CC_SYNTHESIZE_READONLY(int, _speed, Speed);
	CC_SYNTHESIZE_READONLY(int, _direction, Direction);
public:
	inline void setSpeed(int value) { if (value != _speed) { _speed = value;  updateVector(); } }
	inline void setDirection(int value) { if (value != _direction) { _direction = value;  updateVector(); } }
	MovementAction() : _speed(0), _direction(0), _movementVector(cocos2d::Vec2::ZERO) {}
	virtual bool init(int speed, int direction, float duration) {
		TimedAction::init(duration);
		_speed = speed;
		_direction = direction;
		updateVector();
		return true;
	}
	virtual bool init(int speed, int direction) {
		runForever();
		_speed = speed;
		_direction = direction;
		updateVector();
		return true;
	}
	cocos2d::Vec2 forwardVector(uint32_t frames) { // number of frames moved forward in the current vector
		return _movementVector * frames;
	}
	cocos2d::Vec2 backwardVector(uint32_t frames) { // frames backwards, used when we hit something and need to go back a frame
		return _movementVector * -frames;
	}
	void moveBackward(uint32_t frames) { // used to move backward if we hit something, target should be valid
		assert(_target);
		if (_target) _target->setPosition(_target->getPosition() + backwardVector(frames));
	}
	static MovementAction* create(int speed, int direction, float duration) {
		MovementAction* a = new MovementAction;
		if (a && a->init(speed, direction, duration)) {
			a->autorelease();
			return a;
		}
		CC_SAFE_DELETE(a);
		return nullptr;
	}
	static MovementAction* create(int speed, int direction) {
		MovementAction* a = new MovementAction;
		if (a && a->init(speed, direction)) {
			a->autorelease();
			return a;
		}
		CC_SAFE_DELETE(a);
		return nullptr;
	}
	MovementAction* clone() const override {
		auto a = new (std::nothrow) MovementAction();
		a->init(_speed, _direction, _duration);
		a->autorelease();
		return a;
	}
	MovementAction* reverse() const override {
		auto a = new (std::nothrow) MovementAction();
		a->init(_speed, -_direction,_duration);
		a->autorelease();
		return a;
	}

	void step(float t) override {
		TimedAction::step(t);
		if (_target && _speed) {
			cocos2d::Vec2 current = _target->getPosition();
			current += _movementVector;
			_target->setPosition(std::ceilf(current.x),std::ceilf(current.y));
		}
	}
	CC_DISALLOW_COPY_AND_ASSIGN(MovementAction);
};
// Animates a sprite with a an array of frames
class AnimateAction : public TimedAction {
	typedef const cocos2d::Vector<cocos2d::SpriteFrame*>* SpriteFrameVector;
	SpriteFrameVector _frames;
	uint32_t _currentFrame;
	float _currentFrameTime;
	bool _animationOn;
	CC_SYNTHESIZE_READONLY(uint32_t, _startingFrame, StartingFrame);
	CC_SYNTHESIZE_READONLY(uint32_t, _endingFrame, EndingFrame);
	CC_SYNTHESIZE_READONLY(float, _imageSpeed, ImageSpeed);
public:
	AnimateAction() : _frames(0), _currentFrame(0), _animationOn(false), _startingFrame(0), _endingFrame(0), _imageSpeed(0.0f), _currentFrameTime(0.0f) {}
	inline void setImageSpeed(float imageSpeed) {
		if (_frames->size() != 1) {
			_imageSpeed = imageSpeed;
			_animationOn = !std::isinf(imageSpeed) && imageSpeed != 0.0f;
		}
	}
	inline void setStartingFrame(uint32_t frame) { if ((_frames->size() - 1) <= frame) _startingFrame = frame; }
	inline void setEndingFrame(uint32_t frame) { if ((_frames->size() - 1) <= frame) _endingFrame = frame; }

	bool init(SpriteFrameVector frames, float imageSpeed) {
		if (frames == nullptr || frames->size() == 0) return false;
		runForever();
		setImageSpeed(imageSpeed);
		_frames = frames;
		_currentFrame = 0;
		_currentFrameTime = 0.0f;
		_startingFrame = 0;
		_endingFrame = frames->size() - 1;
		return true;
	}

	static AnimateAction* create(SpriteFrameVector frames, float imageSpeed) {
		AnimateAction* a = new AnimateAction;
		if (a && a->init(frames, imageSpeed)) {
			a->autorelease();
			return a;
		}
		CC_SAFE_DELETE(a);
		return nullptr;
	}
	AnimateAction* clone() const override {
		auto a = new (std::nothrow) AnimateAction();
		a->init(_frames, _imageSpeed);
		a->_startingFrame = _startingFrame;
		a->_endingFrame = _endingFrame;
		a->autorelease();
		return a;
	}
	AnimateAction* reverse() const override {
		auto a = clone();
		a->_imageSpeed = -a->_imageSpeed;
		return a;
	}
	void step(float t) override {
		if (_target && _animationOn) {
			if (_currentFrameTime > _imageSpeed) _currentFrameTime += t;
			else {
				_currentFrameTime = 0.0f;
				cocos2d::Sprite* sprite = dynamic_cast<cocos2d::Sprite*>(_target);
				assert(sprite);
				if (_imageSpeed > 0) _currentFrame++; else _currentFrame--;
				if (_currentFrame > _endingFrame) _currentFrame = _startingFrame;
				else if (_currentFrame < _startingFrame) _currentFrame = _endingFrame;
				sprite->setSpriteFrame(_frames->at(_currentFrame));
			}
		}
	}
	CC_DISALLOW_COPY_AND_ASSIGN(AnimateAction);
};

class LuaSprite : public cocos2d::Sprite {
protected:
	cocos2d::Vector<cocos2d::SpriteFrame*> _frames;
	istring _spriteName;
	uint32_t _image_index = 0;
	size_t _frameCount = 0;
	cocos2d::Vec2 _movementVector;
	int _image_speed;
	float _current_image_time;
	int _speed;
	int _direction;
	int _waitFrames;
	inline void setMovementVector() {
		if (_speed == 0) _movementVector = cocos2d::Vec2::ZERO;
		else {
			float x = std::cosf(CC_DEGREES_TO_RADIANS(_direction)) * _speed;
			float y = std::sinf(CC_DEGREES_TO_RADIANS(_direction)) * _speed;
			_movementVector = cocos2d::Vec2(x, y);
		}
		if(_image_speed == 0 && _speed == 0) unscheduleUpdate();  else scheduleUpdate();
	}
	virtual bool init(istring spriteName);
	virtual bool init(uint32_t index);
	void resetImageTime() { _current_image_time = _image_speed * (1.0f / 30.0f); }
public:
	LuaSprite();
	~LuaSprite();
	static LuaSprite* create(istring spriteName);
	static LuaSprite* create(uint32_t index);
	void setSpriteName(istring name);
	istring getSpriteName() const { return _spriteName;  }
	inline void setImageIndex(uint32_t index) { if(index < _frameCount) setSpriteFrame(_frames.at(_image_index = index)); }
	inline uint32_t getImageIndex() const { return _image_index;  }
	inline void setZeroMovement() { _speed = _direction = 0; setMovementVector(); }
	inline void setSpeed(int value) { _speed = value; setMovementVector(); }
	inline void setDirection(int value) { _direction = value; setMovementVector(); }
	inline int getSpeed() const { return _speed; }
	inline int getDirection() const { return _direction; }
	inline void setImageSpeed(int value) { if (_frameCount > 1) { _image_speed = value;  setMovementVector(); resetImageTime(); } }
	inline int getImageSpeed() const { return _image_speed; }

protected:
	void update(float dt) override; 
	// functions for using this sprite like a bullet
	inline virtual void setupBullet(cocos2d::Vec2 pos) { setVisible(true); setPosition(pos); unscheduleUpdate(); } // set's the position of the sprite and makes it invisiable
	inline virtual void fireBullet() { scheduleUpdate();  setVisible(true); }
	inline virtual bool stepBullet(float dt) { return false;  } // this step is run at the start of update, on true return, update dosn't run this frame.
	inline virtual void stopBullet() { setVisible(true);  unscheduleUpdate(); }
	friend class Room;
};


namespace LuaEngineMetaTableNames {
	template<> constexpr const char* metaTableName<LuaSprite>() { return "LuaSpriteMT"; }
}