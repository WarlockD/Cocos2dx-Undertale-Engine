#pragma once
#include "cocos2d.h"

inline cocos2d::Vec3 CreateMovementVector(float dir, float speed) {
	const float rad = 0.01745329252f;
	float x = std::cosf(dir* rad) * speed;
	float y = std::sinf(dir* rad) * speed;
	return cocos2d::Vec3(x, y, 0.0f);
}
inline cocos2d::Vec3 CreateMovementVector(float dir, const cocos2d::Vec3& speed) {
	const float rad = 0.01745329252f;
	float x = std::cosf(dir* rad) * speed.x;
	float y = std::sinf(dir* rad) * speed.y;
	return cocos2d::Vec3(x, y,0.0f);
}