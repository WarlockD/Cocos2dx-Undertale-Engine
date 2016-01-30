#pragma once

#include "cocos2d.h"
#include "LuaEngine.h"
#include "binaryReader.h"
#include <fstream>

class UndertaleResources {
protected:

	class Chunk {
		istring _name; // there are ALOT of text here here, so we want fast equals, searches and save memory
		size_t _begin;
		size_t _end;
		size_t _size;
	public:
		Chunk() : _name(), _begin(0), _end(0), _size(0) {}
		Chunk(size_t begin, size_t limit) : _name(), _begin(begin), _end(limit), _size(limit - begin) {}
		Chunk(const char* name, size_t begin, size_t size) : _name(name), _begin(begin), _end(begin + size), _size(size) {}
		Chunk(istring name, size_t begin, size_t size) : _name(name), _begin(begin), _end(begin + size), _size(size) {}
		inline size_t begin() const { return _begin; }
		inline size_t end() const { return _end; }
		inline size_t size() const { return _size; }
		inline istring name() const { return _name; }
	};
	
	std::unordered_map<istring, Chunk> _chunks;
	std::unordered_map<istring, cocos2d::Vector<cocos2d::SpriteFrame*>> _spriteFrameLookup;
	std::unordered_map<istring, std::vector<uint8_t>> _spriteMaskLookup;

	


	void readAllTextures();
	void readAllChunks();
	void readAllSprites();
protected:
	std::string _data_win_path;
	
	BinaryFileReader r;

	bool loadTextures(cocos2d::ValueVector& list);
	bool loadSpriteFrames(cocos2d::ValueVector& spriteDict, cocos2d::Vector<cocos2d::SpriteFrame*>& frames);
	virtual bool init();
	// This bit is in CabFile.cpp
	bool findUndertaleData();
	cocos2d::Vector<cocos2d::Texture2D*> _textures;

public:
	inline cocos2d::Texture2D* getTexture(uint32_t i) const { return i < _textures.size() ? _textures.at(i) : nullptr;}
	cocos2d::SpriteFrame* getSpriteFrame(istring name, int frame = 0)const {
		auto it = _spriteFrameLookup.find(name);
		if (it == _spriteFrameLookup.cend()) return nullptr;
		else return it->second.at(0);
	}
	cocos2d::Sprite* createSprite(istring name) const {
		return cocos2d::Sprite::createWithSpriteFrame(getSpriteFrame(name));
	}
	
	cocos2d::Vector<cocos2d::SpriteFrame*>& getSpriteFrames(istring name) {
		auto it = _spriteFrameLookup.find(name);
		return it->second;
	}

	static UndertaleResources* getInstance();
	//inline cocos2d::Vector<cocos2d::SpriteFrame*>& getFrames(const std::string& sprite) { return _frameMap[sprite]; }
};