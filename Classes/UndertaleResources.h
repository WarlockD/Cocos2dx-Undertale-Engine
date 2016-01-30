#pragma once

#include "cocos2d.h"

class UndertaleResources {
protected:
	std::string _data_win_path;
	cocos2d::Vector<cocos2d::Texture2D*> _textures;
	std::unordered_map<std::string, cocos2d::Vector<cocos2d::SpriteFrame*>> _frameMap;
	bool loadTextures(cocos2d::ValueVector& list);
	bool loadSpriteFrames(cocos2d::ValueVector& spriteDict, cocos2d::Vector<cocos2d::SpriteFrame*>& frames);
	virtual bool init();
	// This bit is in CabFile.cpp
	bool findUndertaleData();


public:
	static UndertaleResources* getInstance();
	inline cocos2d::Vector<cocos2d::SpriteFrame*>& getFrames(const std::string& sprite) { return _frameMap[sprite]; }
};