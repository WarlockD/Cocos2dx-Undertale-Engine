#pragma once

#include "cocos2d.h"
#include "LuaEngine.h"
#include "binaryReader.h"
#include <fstream>

// We are copying the label class.  Because undertale has very flexable labels we have to reimplment it ourseves to get that
// fine, fine letter control

class UndertaleObject : public cocos2d::Ref {
	CC_SYNTHESIZE_READONLY(istring, _name, Name);
	CC_SYNTHESIZE_READONLY(int, _spriteIndex, SpriteIndex);
	CC_SYNTHESIZE_READONLY(bool, _visible, Visible);
	CC_SYNTHESIZE_READONLY(bool, _solid, Solid);
	CC_SYNTHESIZE_READONLY(int, _depth, Depth);
	CC_SYNTHESIZE_READONLY(int, _parent, Parent); // its -1 if there is no parent
	CC_SYNTHESIZE_READONLY(int, _mask, Mask);
	//UndertaleObject();
public:
	inline bool hasParent() const { return _parent == -1; }
	friend class UndertaleResources;
	friend class ObjectHelper;
};

typedef cocos2d::Vector<cocos2d::SpriteFrame*>* UndertaleSpriteFrames;
class UndertaleResources {
protected:
	static cocos2d::Vector<cocos2d::Texture2D*> emptyList;
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
	std::vector<istring> _spriteFrameIndex;

	std::unordered_map<istring, cocos2d::Vector<cocos2d::Font*>> _fontLookup;

	cocos2d::Vector<UndertaleObject*> _objectIndex;
	std::unordered_map<istring, size_t> _objectLookup;
	



	void readAllTextures();
	void readAllChunks();
	void readAllSprites();
	void readAllFonts();
	void readAllObjects();
	void readAllBackgrounds();
	void readAllRooms();
protected:

	std::string _data_win_path;
	
	BinaryFileReader r;

	bool loadTextures(cocos2d::ValueVector& list);
	bool loadSpriteFrames(cocos2d::ValueVector& spriteDict, cocos2d::Vector<cocos2d::SpriteFrame*>& frames);
	virtual bool init();
	// This bit is in CabFile.cpp
	bool findUndertaleData();
	cocos2d::Vector<cocos2d::Texture2D*> _textures;
	std::vector<std::string> _textureFilenames;

public:
	cocos2d::Node* test_thing;
	inline cocos2d::Texture2D* getTexture(size_t i) const { return (i < _textures.size()) ? _textures.at(i) : nullptr;}
	inline UndertaleObject* lookupObject(size_t index) { return _objectIndex.at(index); }
	inline  UndertaleObject* lookupObject(istring name) {
		auto it = _objectLookup.find(name);
		if (it != _objectLookup.end()) return lookupObject(it->second);
		return nullptr;
	}

	cocos2d::SpriteFrame* getSpriteFrame(istring name, size_t frame)const {
		auto it = _spriteFrameLookup.find(name);
		if (it == _spriteFrameLookup.cend()) return nullptr;
		else return it->second.at(frame);
	}
	cocos2d::Sprite* createSprite(istring name, size_t frame=0) const {
		return cocos2d::Sprite::createWithSpriteFrame(getSpriteFrame(name, frame));
	}
	cocos2d::Sprite* createSprite(uint32_t index, size_t frame = 0) const {
		return cocos2d::Sprite::createWithSpriteFrame(getSpriteFrame(_spriteFrameIndex.at(index), frame));
	}
	const cocos2d::Vector<cocos2d::SpriteFrame*>& getSpriteFrames(istring name) { return _spriteFrameLookup[name]; }
	cocos2d::SpriteFrame* getSpriteFrame(size_t index, size_t frame)const { return getSpriteFrame(_spriteFrameIndex.at(index), frame); }
	const cocos2d::Vector<cocos2d::SpriteFrame*>& getSpriteFrames(size_t index)  { return getSpriteFrames(_spriteFrameIndex.at(index)); }
	static UndertaleResources* getInstance();
	//inline cocos2d::Vector<cocos2d::SpriteFrame*>& getFrames(const std::string& sprite) { return _frameMap[sprite]; }
};


