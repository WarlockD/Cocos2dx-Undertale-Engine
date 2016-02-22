#pragma once
#include "LuaFont.h"
#include "UndertaleResources.h"
#include "Chara.h"
namespace Undertale {
	enum class RoomTags { // these are also the depth till I can figure that out in the resources
		BACKGROUND = 5,
		TILE ,
		OBJECT,
		CHARA
	};
	class Room;
	// This class contains the room and managings loading and switching rooms
	// and moving the little charater around
	class RoomBackground : public cocos2d::Sprite {
		CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _startingPos, StartingPosition);
		CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _speed, Speed);
		CC_SYNTHESIZE_READONLY(bool, _strech, Strech);
		CC_SYNTHESIZE_READONLY(bool, _visibleOnRoomStart, VisibleOnRoomStart);
	private: // below makes sure that it can only be made in room
		RoomBackground() : _startingPos(cocos2d::Vec2::ZERO), _speed(cocos2d::Vec2::ZERO), _strech(false), _visibleOnRoomStart(0) {}
		
	public:


		CC_DISALLOW_COPY_AND_ASSIGN(RoomBackground);
		friend class Room;
	};
	class RoomObject : public LuaSprite {
	protected:
		CC_SYNTHESIZE_READONLY(const UndertaleObject*, _object, UndertaleObject);
		CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _startingPosition, StartingPosition);
		CC_SYNTHESIZE_READONLY(float, _startingRotation, StartingRotation);
		CC_SYNTHESIZE_READONLY(cocos2d::Vec2, _startingScale, StartingScale);
		CC_SYNTHESIZE_READONLY(cocos2d::Color4B, _startingColour, StartingColour);
		RoomObject();
		bool init(const UndertaleObject* object, cocos2d::Vec2 StartingPosition, float StartingRotation , cocos2d::Vec2 StartingScale, cocos2d::Color4B StartingColour);
	public:
		virtual ~RoomObject();
		void setAllStartingValues();
		bool isObject(istring name) { return _object != nullptr ? _object->isObject(name) : false; }
		istring getObjectName() const  { return _object->getObjectName(); }
		static RoomObject* create(const UndertaleObject* object, cocos2d::Vec2 StartingPosition = cocos2d::Vec2::ZERO, float StartingRotation = 0.0f, cocos2d::Vec2 StartingScale = cocos2d::Vec2::ZERO , cocos2d::Color4B StartingColour = cocos2d::Color4B(0,0,0,0xFF));
		const cocos2d::Point& getWorldPos() { return getParent()->convertToWorldSpace(getPosition()); }
		const cocos2d::Rect& getWorldRect() { return cocos2d::Rect(getWorldPos(), _contentSize); }
		friend class Room;
		CC_DISALLOW_COPY_AND_ASSIGN(RoomObject);
	};
	class Room : public cocos2d::Node {
	protected:
		// basic stupid room
		cocos2d::Vector<RoomBackground*> _backgrounds;
		cocos2d::Vector<RoomObject*> _objects;
		RoomObject* _currentObjectTouched;
		std::unordered_map<istring, RoomObject*> _objectLookup; // just a quick way to find an object in the room
		virtual bool init(istring name);
		virtual bool init(uint32_t index);
		void clearRoom();
		bool internalLoadRoom(istring name);
		bool internalLoadRoom(uint32_t index);
	protected:
		CC_SYNTHESIZE_READONLY(istring, _roomName, RoomName);
		CC_SYNTHESIZE_READONLY(uint32_t, _roomIndex, RoomIndex);
		CC_SYNTHESIZE_READONLY(bool, _allObjectsVisible, AllObjectsVisible);
		CC_SYNTHESIZE(bool, _collisionTestMode, CollisionTestMode);
		Undertale::CharaOverworld* _chara;
		void updateChara(); // updates his/her position on the map if there is a new map
	public:
		RoomObject* getObject(uint32_t index)  { return _objects.at(index); }
		RoomObject* getObject(istring name)  { 
			auto it = _objectLookup.find("obj_mainchara");
			if (it == _objectLookup.end()) return nullptr;
			else return it->second;
		}
		const RoomObject* getObject(uint32_t index) const { return getObject(index); }
		const RoomObject* getObject(istring name) const { return getObject(name); }
		Room();
		virtual ~Room();
		void setBackgroundVisible(uint32_t index, bool value);
		bool getBackgroundVisible(uint32_t index) const;
		static Room* create(istring name); // resource name
		static Room * Room::create(uint32_t index);
		void update(float dt) override;
		bool loadRoom(istring name);
		bool loadRoom(uint32_t index);
		bool loadNextRoom();
		bool loadPreviousRoom();
		//void setCharaPosition(cocos2d::Vec2 pos); // use this as we do a moveTo Instant so any Follows work
		void setAllObjectsVisible(bool value); // forces all objets to be visiable at true, false hides only the objects that have to be
		void setChara(Undertale::CharaOverworld* chara);
		friend class RoomBackground;
	};





}