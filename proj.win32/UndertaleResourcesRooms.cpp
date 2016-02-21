#include "UndertaleResources.h"
#include "UndertaleResourcesInternal.h"

USING_NS_CC;

std::vector<Background> bgnData;

void UndertaleResources::readAllBackgrounds() {
	const Chunk& bgnChunk = _chunks["BGND"];
	Background::setArrayFromOffset(r, bgnData, bgnChunk.begin());
}

void UndertaleResources::readAllRooms() {
	const Chunk& roomChunk = _chunks["ROOM"];
	std::vector<RoomData> roomData;
	RoomData::setArrayFromOffset(r, roomData, roomChunk.begin());

	auto& test = roomData[8];// "room_ruins3"];
	//TMXTilesetInfo
	cocos2d::TMXTiledMap *tileMap;
	cocos2d::TMXLayer *background;
//	TMXLayerInfo* layer = new TMXLayerInfo();
//	TMXMapInfo* tinfo = new TMXMapInfo();
	Node* temp = Node::create();
	temp->setContentSize(test.size);
	for (const auto& tile : test.tiles) {
		const Background& tile_set = bgnData[tile.index];
		Texture2D* texture = getTexture(tile_set.info.texture_id);
		Texture2D::TexParams vm;
		
		//texture->setTexParameters(Textu)
		Rect spriteRect = tile_set.info.rect;
		spriteRect.origin += tile.offset;
		spriteRect.size = tile.rect.size;
		Sprite* sprite = Sprite::createWithTexture(texture, spriteRect);
		//sprite->setAnchorPoint(Vec2(0, 0));
		temp->addChild(sprite,-10);
		
		sprite->setPosition(tile.rect.origin.x,-tile.rect.origin.y);
		sprite->setScale(tile.scale.x, tile.scale.y);
	}
	for (const auto& room_obj : test.objects) {
		const auto obj = _objectIndex.at(room_obj.index);
		Node* sprite = nullptr;
		if (obj->getSpriteIndex() >= 0) {
			sprite = this->createSprite(obj->getSpriteIndex());
		}
		else {
			auto label = Label::create(obj->getName().c_str(), "Arial", 9);
			label->setColor(Color3B::WHITE);
			sprite = label;
		}
		sprite->setRotation(room_obj.rotation);
		sprite->setScale(room_obj.scale.x,room_obj.scale.y);
		temp->addChild(sprite,1);
		sprite->setPosition(room_obj.pos.x, -room_obj.pos.y);
	}
	test_thing = temp;

}