#pragma once
#include <vector>
#include "binaryReader.h"
#include "UndertaleResources.h"

std::vector<uint32_t> getOffsetEntries(BinaryReader& r);
std::vector<uint32_t> getOffsetEntries(BinaryReader& r, uint32_t start);

// internal structures
#pragma pack(push,1)
struct GM_SpriteHeader {
	uint32_t width;
	uint32_t height;
	uint32_t flags;
	uint32_t width0;
	uint32_t height0;
	uint32_t another;
	uint32_t extra[7];
};
struct GM_RawSpriteFrame {
	short x;
	short y;
	short width;
	short height;
	short offsetX;
	short offsetY;
	short cropWidth;
	short cropHeight;
	short originalWidth;
	short originalHeight;
	short texture_id;
};
struct GM_FontGlyph {
	short ch;
	short x;
	short y;
	short width;
	short height;
	short shift;
	short offset;
	// after this is a count and the kerning
};
struct RawViews {
	int visible;
	int view_x;
	int view_y;
	int view_width;
	int view_height;
	int port_x;
	int port_y;
	int port_width;
	int port_height;
	int border_x;
	int border_y;
	int speed_x;
	int speed_y;
	int index;
};
struct RawRoomBackground {
	int visible;
	int foreground;
	int index;
	int x;
	int y;
	int tiled_x;
	int tiled_y;
	int speed_x;
	int speedy_y;
	int stretch;
};
struct RawInstance {
	int x;
	int y;
	int index;
	int id;
	int codeOffset;
	float scale_x;
	float scale_y;
	int colour;
	float rotation;
};
struct RawRoomTile {
	int x;
	int y;
	int index;
	int offset_x;
	int offset_y;
	int width;
	int height;
	int depth;
	int id;
	float scale_x;
	float scale_y;
	int mixed;
};
struct RawRoom {
	int nameOffset;
	int captionOffset;
	int width;
	int height;
	int speed;
	int persistent;
	int colour;
	int show_colour;
	int codeOffset;
	int flags;
	int backgroundsOffset;
	int viewsOffset;
	int instancesOffset;
	int tilesOffset;
	// wierd physic stuff not sure off
	int physics_world;
	int physics_up;
	int physics_left;
	int physics_right;
	int physicis_bottom;
	float what_x;
	float what_y;
	float something;
};
struct RawObject {
	int nameOffset;
	int spriteIndex;
	int visible;
	int solid;
	int depth;
	int persistent;
	int parentIndex;
	int mask;
	int physics_object;
	int physics_sensor;
	int physics_shape;
	float physics_density;
	float physics_restitution;
	int physics_group;
	float physics_linearDamping;
	float physics_angulardamping;
	int physics_vertices_count;
	float physics_friction;
	int physics_awake;
	int physics_kiematic;
	// shape vertices, float x, float y
	// events, 8 inits , if not -1 or 0 then subtype first then data
};
struct RawBackground {
	int nameOffset;
	int bool_transparent;
	int bool_smooth;
	int bool_preload;
	int offset_data;
};
#pragma pack(pop)
template<typename T> struct GM_ReaderHelper {
	typedef T RawDataType;
	virtual void setFromReader(BinaryReader& r, uint32_t offset) {
		r.push(offset);
		setFromReader(r);
		r.pop();
	}
	virtual void setFromReader(BinaryReader& r) {
		RawDataType raw;
		r.read(raw);
		setFromRaw(raw,r);
	}

	// in case there are multipul lists,
	template<typename F> static void setArrayFromOffset(BinaryReader& r, std::vector<F>& data) {
		auto entries = getOffsetEntries(r); 
		r.push();
		setFromEntries(entries, r,data);
		r.pop();
	}
	template<typename F> static void setArrayFromOffset(BinaryReader& r, std::vector<F>& data, std::streamoff offset) {
		r.push();
		auto entries = getOffsetEntries(r, offset);
		setFromEntries(entries, r,data);
		r.pop();
	}
	// in case there are multipul lists,
	template<typename F> static void setPtrArrayFromOffset(BinaryReader& r, std::vector<F*>& data) {
		auto entries = getOffsetEntries(r);
		r.push();
		setPtrFromEntries(entries, r, data);
		r.pop();
	}
	template<typename F> static void setPtrArrayFromOffset(BinaryReader& r, std::vector<F*>& data, std::streamoff offset) {
		r.push();
		auto entries = getOffsetEntries(r, offset);
		setPtrFromEntries(entries, r, data);
		r.pop();
	}
	virtual void setFromRaw(const RawDataType& raw, BinaryReader& r) {
		CC_UNUSED(r);
		setFromRaw(raw);
	}
	virtual void setFromRaw(const RawDataType& raw) = 0;
private:
	template<typename F> static void setFromEntries(const std::vector<uint32_t>& entries, BinaryReader& r, std::vector<F>& data) {
		data.resize(entries.size());
		RawDataType temp;
		std::vector<RawDataType> ret(entries.size());
		for (size_t i = 0; i < entries.size(); i++) {
			r.seek(entries[i]);
			r.read(temp);
			GM_ReaderHelper<T>* toSet = dynamic_cast<GM_ReaderHelper<T>*>(&data[i]);
			toSet->setFromRaw(temp, r);
			
		}
	}
	template<typename F> static void setPtrFromEntries(const std::vector<uint32_t>& entries, BinaryReader& r, std::vector<F*>& data) {
		data.resize(entries.size());
		RawDataType temp;
		std::vector<RawDataType> ret(entries.size());
		for (size_t i = 0; i < entries.size(); i++) {
			r.seek(entries[i]);
			r.read(temp);
			F* obj = new F;
			GM_ReaderHelper<T>* toSet = dynamic_cast<GM_ReaderHelper<T>*>(obj);
			toSet->setFromRaw(temp, r);
			data[i] = obj;
		}
	}
};

class GM_SpriteFrame : public GM_ReaderHelper<GM_RawSpriteFrame> {
public:
	void setFromRaw(const RawDataType& raw) override {
		rect.setRect(raw.x, raw.y, raw.width, raw.height);
		offset.setPoint(raw.offsetX, raw.offsetY);
		crop.setSize(raw.cropWidth, raw.cropHeight);
		original.setSize(raw.originalWidth, raw.originalHeight);
		texture_id = raw.texture_id;
	}

	cocos2d::Rect rect;
	cocos2d::Point offset;
	cocos2d::Size crop;
	cocos2d::Size original;
	uint16_t texture_id;
	
};
class Background : public GM_ReaderHelper<RawBackground> {
public:
	void setFromRaw(const RawDataType& raw) override {
		throw std::exception("Do not use");
	}
	void setFromRaw(const RawDataType& raw, BinaryReader& r) override {
		name = r.readStringAtOffset(raw.nameOffset);
		transparent = raw.bool_transparent != 0 ? true : false;
		smooth = raw.bool_smooth != 0 ? true : false;
		preload = raw.bool_preload != 0 ? true : false;
		info.setFromReader(r, raw.offset_data);
	}
	istring name;
	bool transparent;
	bool smooth;
	bool preload;
	GM_SpriteFrame info;
};


class RoomBackgrounds : public GM_ReaderHelper<RawRoomBackground> {
public:
	void setFromRaw(const RawDataType& raw) override {
		visible = raw.visible != 0 ? true : false;
		foreground = raw.foreground != 0 ? true : false;
		index = raw.index;
		pos.set(raw.x, raw.y);
		tiled_x = raw.tiled_x != 0 ? true : false;
		tiled_y = raw.tiled_y != 0 ? true : false;
		speed.set(raw.speed_x, raw.speedy_y);
		stretch = raw.stretch != 0 ? true : false;
	}

	bool visible;
	bool foreground;
	int index;
	cocos2d::Point pos; //( int,int)
	bool tiled_x;
	bool tiled_y;
	cocos2d::Point speed;
	bool stretch;
	
};
class RoomTile : public GM_ReaderHelper<RawRoomTile> {
public:
	void setFromRaw(const RawDataType& raw) override {
		rect.setRect(raw.x, raw.y, raw.width, raw.height);
		index = raw.index;
		offset.set(raw.offset_x, raw.offset_y);
		depth = raw.depth;
		id = raw.id;
		scale.set(raw.scale_x, raw.scale_y);
		blend = raw.mixed & 0x00FFFFFFFF;
		ocupancy = raw.mixed >> 24;
	}
	cocos2d::Rect rect;
	cocos2d::Point offset;
	int index;
	int depth;
	int id;
	cocos2d::Point scale;
	int blend;
	int ocupancy;
	
};
class RoomViews : public GM_ReaderHelper<RawViews> {
public:
	void setFromRaw(const RawDataType& raw) {
		view.setRect(raw.view_x, raw.view_y, raw.view_width, raw.view_height);
		port.setRect(raw.port_x, raw.port_y, raw.port_width, raw.port_height);
		border.set(raw.border_x, raw.border_y);
		speed.set(raw.speed_x, raw.speed_y);
		index = raw.index;
	}
	bool visible;
	cocos2d::Rect view;
	cocos2d::Rect port;
	cocos2d::Point border;
	cocos2d::Point speed;
	int index; 
	
};

class RoomInstance : public GM_ReaderHelper<RawInstance> {
public:
	void setFromRaw(const RawDataType& raw) override {
		pos.set(raw.x, raw.y);
		index = raw.index;
		id = raw.id;
		codeOffset = raw.codeOffset;
		scale.set(raw.scale_x, raw.scale_y);
		colour.set((raw.colour & 0xFF), (raw.colour >> 8) & 0xFF, (raw.colour >> 16) & 0xFF, (raw.colour >> 24) & 0xFF);
		rotation = raw.rotation;
	}
	cocos2d::Point pos;
	int index;
	int id;
	uint32_t codeOffset;
	cocos2d::Point scale;
	cocos2d::Color4B colour;
	float rotation;

	

};
class RoomData : public GM_ReaderHelper<RawRoom> {
public:
	istring roomName;
	istring caption;
	cocos2d::Size size;
	int speed;
	bool persistent;
	cocos2d::Color4B colour;
	bool showColour;
	istring codeFile;
	bool enableViews;  // 1 int
	bool viewClearScreen; // 2
	bool clearDisplayBuffer;  // 4
	std::vector<RoomBackgrounds> backrounds;
	std::vector<RoomInstance> objects;
	std::vector<RoomViews> views;
	std::vector<RoomTile> tiles;
	void setFromRaw(const RawDataType& raw) override {
		throw std::exception("Do not use");
	}
	void setFromRaw(const RawDataType& raw, BinaryReader& r) override {
		roomName = r.readStringAtOffset(raw.nameOffset);
		caption = r.readStringAtOffset(raw.captionOffset);
		size.setSize(raw.width, raw.height);
		speed = raw.speed;
		persistent = raw.persistent != 0 ? true : false;
		colour.set(raw.colour & 0xFF, (raw.colour >> 8) & 0xFF, (raw.colour >> 16) & 0xFF, (raw.colour >> 24) & 0xFF);
		showColour = raw.show_colour != 0 ? true : false;
		if (raw.codeOffset != -1)
			codeFile = r.readStringAtOffset(raw.codeOffset - 8); // Name, size, data
		else
			codeFile = istring();
		RoomBackgrounds::setArrayFromOffset(r, backrounds, raw.backgroundsOffset);
		RoomInstance::setArrayFromOffset(r, objects, raw.instancesOffset);
		RoomViews::setArrayFromOffset(r, views, raw.viewsOffset);
		RoomTile::setArrayFromOffset(r, tiles, raw.tilesOffset);
		// we have to fix the x,y positions as the cordinate system is diffrent
		// doing it here as I was going CRAZY in the Room creator
		for (auto& bgn : backrounds) bgn.pos.y = std::floorf(size.height - bgn.pos.y);
		for (auto& bgn : objects) bgn.pos.y = std::floorf(size.height - bgn.pos.y);
		for (auto& bgn : views) bgn.view.origin.y = std::floorf(size.height - bgn.view.origin.y);
		for (auto& bgn : tiles) bgn.rect.origin.y = std::floorf(size.height - bgn.rect.origin.y);
	}

};