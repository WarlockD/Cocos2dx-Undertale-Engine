#include "UndertaleResources.h"
#include "UndertaleResourcesInternal.h"
#include "Room.h"

USING_NS_CC;

std::vector<Background> bgnData;
std::vector<RoomData> roomData; // hacks here
std::unordered_map<istring, uint32_t> roomDataLookup;

void UndertaleResources::readAllBackgrounds() {
	const Chunk& bgnChunk = _chunks["BGND"];
	Background::setArrayFromOffset(r, bgnData, bgnChunk.begin());
}

void UndertaleResources::readAllRooms() {
	const Chunk& roomChunk = _chunks["ROOM"];
	RoomData::setArrayFromOffset(r, roomData, roomChunk.begin());
	// room order apperntly matters so room data lookup is seperate
	for (uint32_t i = 0; i < roomData.size(); i++) roomDataLookup[roomData[i].roomName] = i;
}
#define RANCHOR Vec2::ANCHOR_BOTTOM_LEFT
//#define ANCHOR Vec2::ANCHOR_BOTTOM_LEFT
#define ANCHOR  Vec2::ANCHOR_TOP_LEFT
namespace Undertale {

		//CC_SYNTHESIZE_READONLY(bool, _hasSprite, HasSprite);
	//	CC_SYNTHESIZE_READONLY(bool, _, HasSprite);
	//	CC_SYNTHESIZE_READONLY(istring, _objectName, ObjectName);
		//CC_SYNTHESIZE_READONLY(RoomObject*, _objectParent, ObjectParent);
		RoomObject::RoomObject() :_object(nullptr){}
		RoomObject::~RoomObject() {}

		bool RoomObject::init(const UndertaleObject* object, cocos2d::Vec2 StartingPosition, float StartingRotation, cocos2d::Vec2 StartingScale, cocos2d::Color4B StartingColour) {
			if (!object) return false;
			if (object->getSpriteIndex() >= 0) {
				UndertaleResources* res = UndertaleResources::getInstance();
				SpriteFrame* frame = res->getSpriteFrame(object->getSpriteIndex(), 0);
				if (!Sprite::initWithSpriteFrame(frame)) return false;
			}
			else if(!Sprite::init()) return false;
			_object = object;
			_startingPosition = StartingPosition;
			_startingRotation = StartingRotation;
			_startingScale = StartingScale;
			_startingColour = StartingColour;
			setColor(Color3B(StartingColour.r, StartingColour.g, StartingColour.b));
			setOpacity(_startingColour.a);
			setRotation(StartingRotation);
			setScaleX(_startingScale.x);
			setScaleY(_startingScale.y);
			return true;
		}


	RoomObject* RoomObject::create(const UndertaleObject* object, cocos2d::Vec2 StartingPosition, float StartingRotation, cocos2d::Vec2 StartingScale, cocos2d::Color4B StartingColour) {
		RoomObject* o = new RoomObject;
		if (o && o->init(object, StartingPosition, StartingRotation, StartingScale, StartingColour)) {
			o->autorelease();
			return o;
		}
		CC_SAFE_DELETE(o);
		return nullptr;
	}
	void RoomObject::setAllStartingValues()
	{

		setColor(Color3B(_startingColour.r, _startingColour.g, _startingColour.b));
		setOpacity(_startingColour.a);
		setRotation(_startingRotation);
		setScaleX(_startingScale.x);
		setScaleY(_startingScale.y);
		setPosition(_startingPosition);
		setVisible(_object->getVisibleAtStart());
		setZOrder(_object->getDepth());
	}



	void Room::clearRoom() { // we do it manualy here as its the fastest way to do so
		stopAllActions(); // IMMPORTANT
		removeChild(_chara, true);
		_chara = nullptr;
		removeAllChildrenWithCleanup(true);
		_objects.clear();
		_backgrounds.clear();
		_objectLookup.clear();
		_roomName = istring();
		return;
		std::vector<size_t> indexes_to_delete;
		indexes_to_delete.reserve(_children.size());
		for (size_t i = 0; i < _children.size(); i++) {
			const auto& child = _children.at(i);
			int tag = child->getTag();
			if (tag != (int)RoomTags::BACKGROUND && tag != (int)RoomTags::OBJECT && tag != (int)RoomTags::TILE) continue;
			// save the index to be deleted
			indexes_to_delete.push_back(i);
			// IMPORTANT:
			//  -1st do onExit
			//  -2nd cleanup
			if (_running)
			{
				child->onExitTransitionDidStart();
				child->onExit();
			}
			// set parent nil at the end
			child->cleanup();
			child->setParent(nullptr);
		}
		for(uint32_t i : indexes_to_delete) 
		removeChildByTag((int)RoomTags::BACKGROUND);

		_children.clear();
		Ref* test;
		
	//	removeChildByTag((int)RoomTags::TILE, true);
		removeAllChildrenWithCleanup(true);
		if (_objects.size() > 0) {
			test = _objects.at(0);
			//	removeChildByTag((int)RoomTags::OBJECT, true);
			_objects.clear();
		}
		if (_backgrounds.size() > 0) {
			test = _backgrounds.at(0);
			//	removeChildByTag((int)RoomTags::BACKGROUND, true);
			_backgrounds.clear();
		}
	
		
	}
	bool Room::internalLoadRoom(istring name) {
		auto it = roomDataLookup.find(name);
		if (it == roomDataLookup.end()) return false;
		return internalLoadRoom(it->second);
	}
	bool Room::internalLoadRoom(uint32_t index) {
		if (index >= roomData.size()) return false;
		setVisible(false);
		clearRoom();

		auto& room = roomData[index];// "room_ruins3"];
		_roomIndex = index;
		_roomName = room.roomName;
		UndertaleResources* res = UndertaleResources::getInstance();
		setContentSize(room.size);
		Node* tile_nodes = Node::create();
		//Node* background = Node::create();
		Node* foreground = Node::create();
		for (const RoomBackgrounds& bgn : room.backrounds) {
			if (bgn.index > -1) {
				const Background& background_tile = bgnData[bgn.index];
				Texture2D* texture = res->getTexture(background_tile.info.texture_id);
				//texture->setTexParameters(Textu)
				Rect spriteRect = background_tile.info.rect;
				spriteRect.origin += background_tile.info.offset;
				spriteRect.size = background_tile.info.rect.size;
				RoomBackground* sprite = new RoomBackground;
				sprite->autorelease();
				sprite->initWithTexture(texture, spriteRect);
				
				sprite->setTag((int)RoomTags::BACKGROUND);
				sprite->setName(background_tile.name);
				sprite->_startingPos = bgn.pos;
				sprite->_visibleOnRoomStart = bgn.visible;
				sprite->_strech = bgn.stretch;
				sprite->_speed = bgn.speed;
				
				if (bgn.foreground) foreground->addChild(sprite); else tile_nodes->addChild(sprite);
				sprite->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
				sprite->setPosition(bgn.pos);
			//	addChild(sprite, bgn.foreground ? 1000 : -1000);
				_backgrounds.pushBack(sprite);
				//assert(!bgn.foreground);
				assert(!bgn.stretch);
			}
		}


	
		for (const auto& tile : room.tiles) {
			const Background& tile_set = bgnData[tile.index];
			Texture2D* texture = res->getTexture(tile_set.info.texture_id);

			Rect spriteRect = tile_set.info.rect;
			spriteRect.origin += tile.offset;
			spriteRect.size = tile.rect.size;
			Sprite* sprite = Sprite::createWithTexture(texture, spriteRect);
			sprite->setAnchorPoint(ANCHOR);
			//sprite->setAnchorPoint(Vec2(0, 0));
			tile_nodes->addChild(sprite);

			sprite->setPosition(tile.rect.origin);
			sprite->setScale(tile.scale.x, tile.scale.y);
			sprite->setTag((int)RoomTags::TILE);
		}
		addChild(tile_nodes);
		if (foreground->getChildrenCount() > 0) addChild(foreground, 50000);



		for (const RoomInstance& room_obj : room.objects) {

			auto obj = res->lookupObject(room_obj.index);
			RoomObject* robject = RoomObject::create(obj, room_obj.pos, room_obj.rotation, room_obj.scale, room_obj.colour);
			_objects.pushBack(robject);
			_objectLookup[robject->getObjectName()] = robject;
			if (obj->getSpriteIndex() >= 0) {
				addChild(robject, 0);
				robject->setAnchorPoint(ANCHOR);
				robject->setAllStartingValues();
				robject->setTag((int)RoomTags::OBJECT);
				if (_allObjectsVisible) robject->setVisible(true);
				else robject->setVisible(robject->getUndertaleObject()->getVisibleAtStart());
			}
		}

		setVisible(true);
		updateChara();
		return true;
	}
	bool Room::init(istring name)
	{
		if (!Node::init()) return false;
		return internalLoadRoom(name);
	}
	bool Room::init(uint32_t index)
	{
		if (!Node::init()) return false;
		return internalLoadRoom(index);
	}
}