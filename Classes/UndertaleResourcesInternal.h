#pragma once
#include <vector>
#include "binaryReader.h"
#include "UndertaleResources.h"

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
#pragma pack(pop)


class GM_SpriteFrame {

public:
	cocos2d::Rect rect;
	cocos2d::Point offset;
	cocos2d::Size crop;
	cocos2d::Size original;
	uint16_t texture_id;
	void setFromRaw(const GM_RawSpriteFrame& raw) {
		rect.setRect(raw.x, raw.y, raw.width, raw.height);
		offset.setPoint(raw.offsetX, raw.offsetY);
		crop.setSize(raw.cropWidth, raw.cropHeight);
		original.setSize(raw.originalWidth, raw.originalHeight);
		texture_id = raw.texture_id;
	}
	void setFromReader(BinaryReader& r, uint32_t offset) {
		r.push(offset);
		setFromReader(r);
		r.pop();
	}
	void setFromReader(BinaryReader& r) {
		GM_RawSpriteFrame raw;
		r.read(raw);
		setFromRaw(raw);
	}
};

std::vector<uint32_t> getOffsetEntries(BinaryReader& r);
std::vector<uint32_t> getOffsetEntries(BinaryReader& r, uint32_t start);
