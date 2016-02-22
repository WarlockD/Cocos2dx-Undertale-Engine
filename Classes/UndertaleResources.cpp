
#include "UndertaleResourcesInternal.h"

USING_NS_CC;


std::vector<uint32_t> getOffsetEntries(BinaryReader& r) {
	std::vector<uint32_t> entries;
	uint32_t count = r.read<uint32_t>();
	while (count > 0) {
		uint32_t offset = r.read<uint32_t>();
		entries.emplace_back(offset);
		count--;
	}
	return entries;

}
std::vector<uint32_t> getOffsetEntries(BinaryReader& r, uint32_t start) {
	r.push(start);
	std::vector<uint32_t> vec(std::move(getOffsetEntries(r)));
	r.pop();
	return vec;
}

void GetFrames(ValueMap& dictionary, const std::string& sprite_name, Texture2D *texture, Vector<SpriteFrame*>& frames) {
	ValueMap& framesDict = dictionary["newframes"].asValueMap();
	int format = 0;

	Size textureSize;

	// get the format
	if (dictionary.find("metadata") != dictionary.end())
	{
		ValueMap& metadataDict = dictionary["metadata"].asValueMap();
		format = metadataDict["format"].asInt();

		if (metadataDict.find("size") != metadataDict.end())
		{
			textureSize = SizeFromString(metadataDict["size"].asString());
		}
	}

	// check the format
	CCASSERT(format >= 0 && format <= 3, "format is not supported for SpriteFrameCache addSpriteFramesWithDictionary:textureFilename:");

	auto textureFileName = Director::getInstance()->getTextureCache()->getTextureFilePath(texture);
	Image* image = nullptr;
	ValueVector& sprite = framesDict[sprite_name].asValueVector();
	int count = 0;
	for (auto iter = sprite.begin(); iter != sprite.end(); ++iter)
	{
		ValueMap& frameDict = iter->asValueMap();
		SpriteFrame* spriteFrame = nullptr;

		if (format == 0)
		{

		}
		else if (format == 1 || format == 2)
		{
			Rect frame = RectFromString(frameDict["frame"].asString());
			bool rotated = false;

			// rotation
			if (format == 2) rotated = frameDict["rotated"].asBool();

			Vec2 offset = PointFromString(frameDict["offset"].asString());
			Size sourceSize = SizeFromString(frameDict["sourceSize"].asString());

			// create frame
			spriteFrame = SpriteFrame::createWithTexture(texture,
				frame,
				rotated,
				offset,
				sourceSize
				);
		}
		if (spriteFrame != nullptr) frames.pushBack(spriteFrame);
	}
	CC_SAFE_DELETE(image);
}


UndertaleResources* s_undertailResources = nullptr;
bool UndertaleResources::loadTextures(ValueVector& list)
{
	_textures.clear();
	for (auto iter = list.begin(); iter != list.end(); ++iter)
	{
		std::string filename = iter->asString();
		std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filename);
		if (fullPath.size() == 0)
		{
			CCLOG("cocos2d: UndertaleResources::loadTextures can not find %s", filename.c_str());	// return if plist file doesn't exist
			return false;
		}
		Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(fullPath.c_str());
		if (!texture) {
			CCLOG("cocos2d: UndertaleResources::loadTextures can not load %s", filename.c_str());	// return if plist file doesn't exist
			return false;
		}
		_textures.pushBack(texture);
	}
	return true;
}
bool UndertaleResources::loadSpriteFrames(ValueVector & list, cocos2d::Vector<cocos2d::SpriteFrame*>& frames)
{

	for (auto iter = list.begin(); iter != list.end(); ++iter)
	{
		ValueMap& frameDict = iter->asValueMap();
		float x = frameDict["x"].asFloat();
		float y = frameDict["y"].asFloat();
		float w = frameDict["width"].asFloat();
		float h = frameDict["height"].asFloat();
		float ox = frameDict["offsetX"].asFloat();
		float oy = frameDict["offsetY"].asFloat();
		int ow = frameDict["originalWidth"].asInt();
		int oh = frameDict["originalHeight"].asInt();
		int textureIndex = frameDict["textureIndex"].asInt();
		// check ow/oh
		if (!ow || !oh)
		{
			CCLOGERROR("cocos2d: WARNING: originalWidth/Height not found on the SpriteFrame. AnchorPoint won't work as expected. Regenerate the .plist");
		}
		// abs ow/oh
		ow = abs(ow);
		oh = abs(oh);
		// create frame
		Texture2D* texture = _textures.at(textureIndex);
		SpriteFrame* spriteFrame = SpriteFrame::createWithTexture(texture,
			Rect(x, y, w, h),
			false,
			Vec2(ox, oy),
			Size((float)ow, (float)oh)
			);
		if (!spriteFrame) {
			CCLOGERROR("UndertaleResources::loadSpriteFrames: spriteFrame could not load");
			return false;
		}

		frames.pushBack(spriteFrame);
	}
	return true;
}
bool UndertaleResources::init()
{
	if (!findUndertaleData() || !r.open(_data_win_path)) {
		CCLOGERROR("cocos2d: UndertaleResources::init() can not open %s", _data_win_path.c_str());	// return if plist file doesn't exist
		return false;
	}
	readAllChunks();
	readAllTextures();
	readAllSprites();
	readAllFonts();
	readAllObjects();
	readAllBackgrounds();
	readAllRooms();
	return true;
}



//Humm, in reality how elegant IS this.  If I do it manualy I can link the spriteframes
// to this object but humm.
class ObjectBuilder : public UndertaleObject, public GM_ReaderHelper<RawObject> {
	void setFromRaw(const RawDataType& raw) override {
		throw std::exception("Do not use");
	}
	void setFromRaw(const RawDataType& raw, BinaryReader& r) override {
		_objectName = r.readStringAtOffset(raw.nameOffset);
		_spriteIndex = raw.spriteIndex;
		_depth = raw.depth;
		_visibleAtStart = raw.visible != 0 ? true : false;
		_solid = raw.solid != 0 ? true : false;
		_parent = (UndertaleObject*)raw.parentIndex; // fix latter
		_mask = raw.mask;
	}
};

UndertaleObject::UndertaleObject(): _spriteIndex(-1),_visibleAtStart(true),_solid(false),_depth(-100),_parent(nullptr),_mask(-1) {}
UndertaleObject::~UndertaleObject() {}
std::string UndertaleObject::getFullName() const
{
	std::string full;
	if (_parent) full += _parent->getFullName();
	if (full.size() > 0) full += "::";
	full += _objectName.c_str();
	return full;
}
bool UndertaleObject::isObject(istring name) const {
	if (name == _objectName) return true;
	else if (_parent != nullptr) return _parent->isObject(name);
	else return false;
}
LuaSprite * UndertaleObject::createSprite() const
{
	UndertaleResources* res = UndertaleResources::getInstance();
	if (_spriteIndex >= 0) return LuaSprite::create(_spriteIndex);
	else return nullptr;
}

void UndertaleResources::readAllObjects() {
	const Chunk& objChunk = _chunks["OBJT"];
	std::vector<ObjectBuilder*> tempObjects;
	ObjectBuilder::setPtrArrayFromOffset(r, tempObjects, objChunk.begin());
	for (size_t i = 0; i < tempObjects.size(); i++) {
		UndertaleObject* o = tempObjects.at(i);
		int objIndex = (int)o->_parent; // fix parrent
		if (objIndex >= 0) o->_parent = tempObjects.at(objIndex);
		else o->_parent = nullptr;
		_objectLookup[o->getObjectName()] = i;
		_objectIndex.push_back(o);
	}
}

void UndertaleResources::readAllTextures()
{
		_textures.clear();
		const Chunk& txrtChunk = _chunks["TXTR"];
		r.seek(txrtChunk.begin());
		auto textureOffsets = getOffsetEntries(r);
		std::vector<char> fileBuffer;
		fileBuffer.resize(100000);
		for (uint32_t i = 0; i < textureOffsets.size(); i++) {
			uint32_t offset = textureOffsets[i];
			uint32_t next_offset = (i + 1) < textureOffsets.size() ? textureOffsets[i + 1] : txrtChunk.end();
			uint32_t size = next_offset = offset;
			r.seek(offset);
			int dummy = r.readInt(); // always a 1
			uint32_t new_offset = r.readInt();
			r.seek(new_offset);
		//	std::string path = cocos2d::FileUtils::getInstance()->getWritablePath();
		//	path += "UndertaleTexture_" + std::to_string(textureFiles.size()) + ".png";
			fileBuffer.resize(size);
			r.read(fileBuffer.data(), size);
			Image* image = new Image;
			image->autorelease();
			image->initWithImageData((uint8_t*)fileBuffer.data(), size);
			std::string image_key = "UndertaleTexture_" + std::to_string(i) + ".png";
			Texture2D* texture = TextureCache::getInstance()->addImage(image, image_key);
			texture->setAliasTexParameters(); // fix it
			_textures.pushBack(texture);
			_textureFilenames.push_back(image_key);
			// still need to save it though
			std::fstream image_writer;
			
			image_writer.open(image_key, std::fstream::out | std::fstream::binary);
			image_writer.write(fileBuffer.data(), size);
			image_writer.close();
		}
}
void UndertaleResources::readAllChunks()
{
	r.seek(0); // got to start
	std::streamsize full_size = r.length();
	while (r.tell() < full_size) {
		char chunkNameBuffer[5]; r.read(chunkNameBuffer, 4); chunkNameBuffer[4] = 0;
		istring chunkName = chunkNameBuffer;
		uint32_t chunkSize = r.read<uint32_t>();
		uint32_t chunkStart = r.tell();
		_chunks.emplace(std::make_pair(chunkName, Chunk(chunkName, chunkStart, chunkSize)));
		if (chunkName == "FORM") full_size = chunkSize; // special case
		else r.seek(chunkStart + chunkSize);
	}
	r.seek(0); // got to start
}


void UndertaleResources::readAllSprites()
{
	GM_SpriteHeader header;
	_spriteFrameLookup.clear();
	const Chunk& spriteChunk = _chunks["SPRT"];
	GM_RawSpriteFrame rawFrame;
	std::vector<GM_SpriteFrame> framesData;
	_spriteFrameIndex.clear();
	r.seek(spriteChunk.begin());
	auto spriteOffsets = getOffsetEntries(r);
	_spriteFrameIndex.reserve(spriteOffsets.size());
	for (uint32_t offset : spriteOffsets) {
		r.seek(offset);
		istring name = r.readStringAtOffset(r.readInt());
		r.read(header);
		r.push();
#define VECTOR_BUG 
#ifdef VECTOR_BUG
		cocos2d::Vector<SpriteFrame*>  frames;
#else
		_spriteFrameIndex.push_back(name);
		_spriteFrameLookup[name] = Vector<SpriteFrame*>();
		cocos2d::Vector<SpriteFrame*> & frames = _spriteFrameLookup[name];
#endif
		
		GM_SpriteFrame::setArrayFromOffset(r, framesData);
		for (const auto& frame : framesData) {
			if (frame.texture_id > _textures.size()) {
				CCLOG("Texture id invalid for sprite %sd", name.c_str());
				continue;
			}
			Texture2D* texture = _textures.at(frame.texture_id);
			SpriteFrame* spr_frame = SpriteFrame::createWithTexture(texture, frame.rect, false, frame.offset, frame.original);
			frames.pushBack(spr_frame);
		}
		r.pop();
#ifdef VECTOR_BUG
		if (frames.size() > 0) {
			_spriteFrameIndex.push_back(name);
			_spriteFrameLookup.emplace(std::make_pair(name, std::move(frames)));
		}
#endif
		// read mask stuff
		int haveMask = r.readInt();
		if (haveMask) { // have mask?
			uint32_t stride = (header.width % 8) != 0 ? header.width + 1 : header.width;
		//	std::vector<uint8_t>* mask = new std::vector<uint8_t>();
		//	mask->resize(stride * header.height);
		//	r.read(mask->data(), mask->size());
		//	_spriteMaskLookup.emplace(std::make_pair(name, mask));
		}
	}
}
UndertaleResources * UndertaleResources::getInstance()
{
	if (s_undertailResources == nullptr)
	{
		s_undertailResources = new UndertaleResources();
		if (!s_undertailResources->init())
		{
			delete s_undertailResources;
			s_undertailResources = nullptr;
			CCLOG("ERROR: Could not init UndertaleResources");
		}
	}
	return s_undertailResources;
}



