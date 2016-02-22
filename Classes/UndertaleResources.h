#pragma once

#include "cocos2d.h"
#include "LuaEngine.h"
#include "binaryReader.h"
#include "LuaSprite.h"

#include <fstream>

// We are copying the label class.  Because undertale has very flexable labels we have to reimplment it ourseves to get that
// fine, fine letter control

class UndertaleObject  {
	CC_SYNTHESIZE_READONLY(istring, _objectName, ObjectName);
	CC_SYNTHESIZE_READONLY(int, _spriteIndex, SpriteIndex);
	CC_SYNTHESIZE_READONLY(bool, _visibleAtStart, VisibleAtStart);
	CC_SYNTHESIZE_READONLY(bool, _solid, Solid);
	CC_SYNTHESIZE_READONLY(int, _depth, Depth);
	CC_SYNTHESIZE_READONLY(UndertaleObject*, _parent, Parent); // Don't need to release this as there should always be one ref in the object list
	CC_SYNTHESIZE_READONLY(int, _mask, Mask);
	UndertaleObject();
public:
	virtual ~UndertaleObject();
	std::string getFullName() const;
	bool hasSprite() const { return _spriteIndex >= 0; } // if this is a sprite.  Uselly not if its a parrent
	bool isObject(istring name) const;
	LuaSprite* createSprite() const;
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
	std::vector<istring>  _spriteFrameIndex;

	std::unordered_map<istring, cocos2d::Vector<cocos2d::Font*>> _fontLookup;

	// BUGBUGBUG  Be sure to delete all these once we are done with them at the end of the program
	std::vector<UndertaleObject*> _objectIndex;
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
	inline const UndertaleObject* lookupObject(size_t index) const { return _objectIndex.at(index); }
	inline const   UndertaleObject* lookupObject(istring name) const {
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
	istring getSpriteIndexToName(uint32_t index) const { return _spriteFrameIndex[index]; }
	const cocos2d::Vector<cocos2d::SpriteFrame*>& getSpriteFrames(istring name) const  { return _spriteFrameLookup.at(name); }

	cocos2d::SpriteFrame* getSpriteFrame(size_t index, size_t frame) const { return _spriteFrameLookup.at(getSpriteIndexToName(index)).at(frame); }

	const cocos2d::Vector<cocos2d::SpriteFrame*>& const getSpriteFrames(size_t index)  const { return _spriteFrameLookup.at(getSpriteIndexToName(index)); }
	static UndertaleResources* getInstance();
};


