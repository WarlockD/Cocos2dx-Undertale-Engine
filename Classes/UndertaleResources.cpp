#include "UndertaleResources.h"
#include "binaryReader.h"
USING_NS_CC;


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

	findUndertaleData();
	ChunkReader r(_data_win_path);
	//if (!findUndertaleData()) return false;
	std::string filename("undertale_sprites.plist");
	std::string fullPath = FileUtils::getInstance()->fullPathForFilename(filename);
	if (fullPath.size() == 0)
	{
		CCLOGERROR("cocos2d: UndertaleResources: can not find %s", filename.c_str());	// return if plist file doesn't exist
		return false;
	}
	ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(fullPath);
	if (dict.find("textures") == dict.end()) {
		CCLOGERROR("cocos2d: UndertaleResources: can not find the texture list");	// return if plist file doesn't exist
		return false;
	}
	if (dict.find("sprites") == dict.end()) {
		CCLOGERROR("cocos2d: UndertaleResources: can not find the sprite list");	// return if plist file doesn't exist
		return false;
	}
	if (!loadTextures(dict["textures"].asValueVector())) return false;
	ValueMap& sprites = dict["sprites"].asValueMap();
	SpriteFrameCache* cache = SpriteFrameCache::getInstance();
	for (auto iter = sprites.begin(); iter != sprites.end(); ++iter)
	{
		const std::string& spriteName = iter->first;
		cocos2d::Vector<cocos2d::SpriteFrame*> frames; // Hopefuly this std::moves
		if (!loadSpriteFrames(iter->second.asValueVector(), frames)) return false;
		if (!loadSpriteFrames(iter->second.asValueVector(), frames)) {
			CCLOGERROR("cocos2d: UndertaleResources: can not load sprite %s", spriteName);	// return if plist file doesn't exist
			return false;
		}
		_frameMap[spriteName] = frames;
		for (int i = 0; i < frames.size(); i++) {
			std::string frameName = spriteName + std::to_string(i);
			cache->addSpriteFrame(frames.at(i), frameName);
		}
	}
	return true;
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
