#include "UndertaleResourceNode.h"
#include "UndertaleLib.h"
#include "Global.h"
#include "obj_mainchara.h"

static UndertaleLib::UndertaleFile* _file = nullptr;
static Undertale* _singleton = nullptr;


Vec2 FixUndertalePosition(const Vec2& pos, const Size& size) {
	Vec2 npos(pos.x + size.width / 2, pos.y + size.height / 2);
	return npos;
}
Vec2 FixUndertalePosition(Node* node) {
	Vec2 npos = FixUndertalePosition(node->getPosition(), node->getContentSize());
	node->setPosition(npos);
	return npos;
}

bool Undertale::loadDataWin(const std::string& filename) {
	if (!_file) {
		_file = new UndertaleLib::UndertaleFile;
		if (_file->loadFromFilename(filename))return true;
		delete _file;
		_file = nullptr;
	}
	return false;
}

bool Undertale::loadDataWin(std::vector<uint8_t>&& data) {
	if (!_file) {
		_file = new UndertaleLib::UndertaleFile;
		if (_file->loadFromData(data)) return true;
		delete _file;
		_file = nullptr;
	}
	return false;
}
Undertale* Undertale::getSingleton() {
	if (_file) {
		if (!_singleton) _singleton = new Undertale;
	}
	return _singleton;
}
UndertaleLib::UndertaleFile* Undertale::getFile() {
	return _file;
}
const cocos2d::Value& Undertale::getGlobal(const std::string& name) const {
	return _globals.at(name);
}
cocos2d::Value& Undertale::getGlobal(const std::string& name)  {
	return _globals.at(name);
}
void Undertale::setGlobal(const std::string& name, cocos2d::Value value) {
	_globals[name] = value;
}
int Undertale::getFlag(int index) const {
	Value value = _flags.at(index);
	return value.asInt();
}
void Undertale::setFlag(int index, int value) {
	_flags[index] = value;
}

Texture2D* Undertale::LookupTexture(size_t index) {
	std::string key = "undertale_" + std::to_string(index) + ".png";
	auto texture = Director::getInstance()->getTextureCache()->addImage(LookupImage(index), key);

	Texture2D::TexParams tp = { GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_CLAMP };
	texture->setTexParameters(tp);
	return texture;
}
Image* Undertale::ImageFromBackground(const UndertaleLib::Background& b) {
	if (!b.valid()) return nullptr;
	auto& f = b.frame();
	Image* image = LookupImage(b.frame().texture_index);
	size_t bpb = image->getBitPerPixel() / 8;
	size_t istride = image->getWidth() * bpb;
	size_t cstride = bpb * f.width;
	uint8_t* crop = new uint8_t[f.width*f.height*bpb];
	const uint8_t* scanline = image->getData() + istride * f.y + f.x*bpb;
	uint8_t* cscanline = crop;
	for (size_t y = 0; y < f.height; y++) {
		std::memcpy(cscanline, scanline, sizeof(uint8_t) * cstride);
		scanline += istride;
		cscanline += cstride;
	}
	Image* cimage = new Image;
	if (cimage && cimage->initWithRawData(crop, f.width*f.height*bpb, f.width, f.height, image->getBitPerPixel())) {
		CC_SAFE_DELETE_ARRAY(crop);
		cimage->autorelease();
		//	cimage->saveToFile(b.name().string() + ".png");
		return cimage;
	}
	CC_SAFE_DELETE_ARRAY(crop);
	CC_SAFE_DELETE(cimage);
	return nullptr;
}
Image* Undertale::ImageFromBackground(size_t index) { return ImageFromBackground(_file->LookupBackground(index)); }
Image* Undertale::ImageFromBackground(const std::string& name) { return ImageFromBackground(_file->LookupByName<UndertaleLib::Background>(name.c_str())); }

Texture2D* Undertale::TextureFromBackground(const UndertaleLib::Background& b) {
	if (!b.valid()) return nullptr;
	return Director::getInstance()->getTextureCache()->addImage(ImageFromBackground(b), b.name().string());
}
Texture2D* Undertale::TextureFromBackground(size_t index) { return TextureFromBackground(_file->LookupBackground(index)); }
Texture2D* Undertale::TextureFromBackground(const std::string& name) { return TextureFromBackground(_file->LookupByName<UndertaleLib::Background>(name.c_str())); }





Image* Undertale::LookupImage(size_t index) {
	Image* image = _images.at(index);
	if (!image) {
		auto uimage = _file->LookupTexture(index);
		if (uimage.data() != nullptr) {
			image = new Image;
			if (image->initWithImageData(uimage.data(), uimage.len())) {
				image->autorelease();
				_images.insert(index, image);
			} else {
				image->release();
				image = nullptr;
			}
		}
	}
	return image;
}
class UndertaleFont : public Font {
	UndertaleLib::Font _configuration;
	FontAtlas* _atlas = nullptr;
	std::unordered_map<size_t, int> _kernings; // cheap kerning lookup
	UndertaleFont(UndertaleLib::Font configuration) : _configuration(configuration) {}
public:
	virtual ~UndertaleFont() { if (_atlas != nullptr) _atlas->release(); }
	static UndertaleFont* UndertaleFont::create(const UndertaleLib::Font& ufont) {
		if (!ufont.valid()) return nullptr;
		Texture2D* texture = Undertale::getSingleton()->LookupTexture(ufont.frame().texture_index);
		if (texture == nullptr) return nullptr;
		UndertaleFont* font = new UndertaleFont(ufont);
		font->_configuration = std::move(ufont);

		font->autorelease();
		return font;
	}
	virtual FontAtlas* createFontAtlas();
	virtual int* getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const override;
	int getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const;
	virtual int getFontMaxHeight() const override { return _configuration.size(); }
};


static Map<size_t, FontAtlas*> s_undertaleFontAtlases;
static Map<size_t, FontAtlas*> s_undertaleFontCache;




FontAtlas* UndertaleFont::createFontAtlas() {
	size_t numGlyphs = _configuration.glyphs().size();
	if (numGlyphs == 0) return nullptr;
	auto& frame = _configuration.frame();


	FontAtlas *tempAtlas = new (std::nothrow) FontAtlas(*this);
	if (tempAtlas == nullptr) return nullptr;
	tempAtlas->setLineHeight(_configuration.size());
	
	Rect frameRect(frame.x, frame.y, frame.width, frame.height);
	frameRect = CC_RECT_PIXELS_TO_POINTS(frameRect);
	for (auto& f : _configuration.glyphs()) {
		char c = f.ch;
		FontLetterDefinition def;
		Rect tempRect(f.x, f.y, f.width, f.height);
		tempRect = CC_RECT_PIXELS_TO_POINTS(tempRect);
	//	assert(f.ch != 'r' && f.ch != 'o');
		//def.offsetX = f.offset;
	    def.offsetX = f.offset;
		def.offsetY = 0;
		def.U = tempRect.origin.x + frameRect.origin.x;
		def.V = tempRect.origin.y + frameRect.origin.y;
		def.width = tempRect.size.width;
		def.height = tempRect.size.height;
		def.textureID = 0;
		def.validDefinition = true;
		def.xAdvance = f.shift;
		
		// add the new definition
		if (65535 < f.ch) {
			CCLOGWARN("Warning: 65535 < fontDef.charID (%u), ignored", f.ch);
		}
		else {
			tempAtlas->addLetterDefinition(f.ch, def);
		}
		if (f.kerning_count > 0) {
			for (size_t i = 0; i < f.kerning_count; i++) {
				UndertaleLib::Font::Kerning k = f.kernings[i];
				unsigned int key = (f.ch << 16) | (k.other & 0xffff);
				_kernings[key] =  k.amount;
			}
		}
		
	}
	
	Texture2D* tempTexture = Undertale::getSingleton()->LookupTexture(frame.texture_index);
	if (tempTexture == nullptr) {
		CC_SAFE_RELEASE(tempAtlas);
		return nullptr;
	}
	// add the texture
	tempAtlas->addTexture(tempTexture, 0);
	tempAtlas->autorelease();

	// done
	return tempAtlas;
}
int* UndertaleFont::getHorizontalKerningForTextUTF16(const std::u16string& text, int &outNumLetters) const {
	outNumLetters = static_cast<int>(text.length());

	if (!outNumLetters)
		return 0;

	int *sizes = new (std::nothrow) int[outNumLetters];
	if (!sizes) return 0;


	for (int c = 0; c < outNumLetters; ++c)
	{

		if (c < (outNumLetters - 1))
			sizes[c] = getHorizontalKerningForChars(text[c], text[c + 1]);
		else
			sizes[c] = 0;
	}

	return sizes;
}
int UndertaleFont::getHorizontalKerningForChars(unsigned short firstChar, unsigned short secondChar) const {
	int kern = 0;
	if (_configuration.index() == 8) { // only two cases that have kernings?
		if (firstChar == 'w') kern += 2;
		if (firstChar == 'm') kern += 2;
		if (firstChar == 'i') kern -= 2;
		if (firstChar == 'l') kern -= 2;
		if (firstChar == 's') kern--;
		if (firstChar == 'j') kern--;
		return kern;
	}
	if (_configuration.index() == 9) { // only two cases that have kernings?
		if (firstChar == 'D') kern++;
		if (firstChar == 'Q') kern += 3;
		if (firstChar == 'M') kern++;
		if (firstChar == 'L') kern--;
		if (firstChar == 'K') kern--;
		if (firstChar == 'C') kern++;
		if (firstChar == '.') kern -= 3;
		if (firstChar == '!') kern -= 3;
		if (firstChar == 'O' || firstChar == 'W') kern += 2;
		if (firstChar == 'I') kern -= 6;
		if (firstChar == 'T') kern--;
		if (firstChar == 'P') kern -= 2;
		if (firstChar == 'R') kern -= 2;
		if (firstChar == 'A') kern++;
		if (firstChar == 'H') kern++;
		if (firstChar == 'B') kern++;
		if (firstChar == 'G') kern++;
		if (firstChar == 'F') kern--;
		if (firstChar == '?') kern -= 3;
		if (firstChar == '\'') kern -= 6;
		if (firstChar == 'J') kern--;
		return kern;
	}
	if (!_kernings.empty()) {
		unsigned int key = (firstChar << 16) | (secondChar & 0xffff);
		kern= _kernings.at(key);
	}
	return kern;
}

UndertaleLabel::UndertaleLabel() : _shake(0.0f), _direction(0.0f), _speed(0.0f), Label() {  }

bool UndertaleLabel::init()  {
	if (Label::init()) {
		//setAnchorPoint(Vec2(0.0f, 1.0f));
		return true;
	}
	return false;
}
UndertaleLabel* UndertaleLabel::create(size_t font_index) {
	if (_file->LookupFont(font_index).valid()) {
		UndertaleLabel* label = new UndertaleLabel;
		if (label && label->init()) {
			label->setUndertaleFont(font_index);
			return label;
		}
		CC_SAFE_DELETE(label);
	}
	return nullptr;
}

void UndertaleLabel::setUndertaleFont(size_t font_index) {
	_currentLabelType = LabelType::CHARMAP;
	setFontAtlas(Undertale::getSingleton()->LookupFontAtlas(font_index));
	setLineSpacing(4);
}

void UndertaleLabel::updateColor()
{ // only one we can override, so might as well
	_quadOrigin.clear();
	if (_batchNodes.empty())
	{
		return;
	}

	Color4B color4(_displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity);

	// special opacity for premultiplied textures
	if (_isOpacityModifyRGB)
	{
		color4.r *= _displayedOpacity / 255.0f;
		color4.g *= _displayedOpacity / 255.0f;
		color4.b *= _displayedOpacity / 255.0f;
	}
	cocos2d::TextureAtlas* textureAtlas;
	V3F_C4B_T2F_Quad *quads;
	for (auto&& batchNode : _batchNodes)
	{
		textureAtlas = batchNode->getTextureAtlas();
		quads = textureAtlas->getQuads();
		auto count = textureAtlas->getTotalQuads();

		for (int index = 0; index < count; ++index)
		{
			
			color4 = _leterColors[index];
			quads[index].bl.colors = color4;
			quads[index].br.colors = color4;
			quads[index].tl.colors = color4;
			quads[index].tr.colors = color4;
			textureAtlas->updateQuad(&quads[index], index);
			_quadOrigin.push_back(quads[index]);
		}
	}

}
void UndertaleLabel::update(float delta)  {
	if ((int)_shake == 0) return;
	cocos2d::TextureAtlas* textureAtlas;
	V3F_C4B_T2F_Quad *quads;
	if (_shake > 38) {
		_speed = 2;
		_direction += 20;
	}
	if ((int)_shake == 42) {
		_speed = 4;
		_direction -= 19;
	}
	for (auto&& batchNode : _batchNodes)
	{
		textureAtlas = batchNode->getTextureAtlas();
		quads = textureAtlas->getQuads();
		auto count = textureAtlas->getTotalQuads();
		for (int index = 0; index < count; ++index) {
			const auto& org = _quadOrigin[index];
			Vec3 shake;
			if(_shake <=38)
				shake = Vec3(RandomHelper::random_real(0.0f, _shake) - (_shake / 2.0f), RandomHelper::random_real(0.0f, _shake) - (_shake / 2.0f), 0.0f);
			else {
				switch ((int)_shake) {
				case 39:
					_direction += 10;
					shake = CreateMovementVector(_direction, _speed);
					break;
				case 40:
					shake = CreateMovementVector(_direction, _speed);
					break;

				case 41:
					_direction += 10.0f * index;
					shake = CreateMovementVector(_direction, _speed);
					_direction += 10.0f * index;
					break;
				case 42:
					_direction += 20.0f * index;
					shake = CreateMovementVector(_direction, _speed);
					_direction += 20.0f * index;
					break;
				case 43:
					_direction += 30.0f * index;
					shake = CreateMovementVector(30 * index, 2);
					_direction += 30.0f * index;
					shake *= 0.7f;
					shake += Vec3(10, 0, 0);
					break;
				}
			}
			
			
			
			quads[index].bl.vertices = org.bl.vertices - shake;
			quads[index].br.vertices = org.br.vertices - shake;
			quads[index].tl.vertices = org.tl.vertices - shake;
			quads[index].tr.vertices = org.tr.vertices - shake;
		}
			
	}
}
void UndertaleLabel::setShake(float shake) {
	_shake = std::fabsf(shake);
	if (_shake == 0.0f) unscheduleUpdate();
	else scheduleUpdate();
}
void UndertaleLabel::setString(const std::string& text) {
	_parser.setText(text);
	_leterColors.clear();
	_quadOrigin.clear();
	Color4B color4 = Color4B::WHITE;

	for (auto& t : _parser) { // coun't SHOULD equal the _text size
		switch (t.token()) {
		case UndertaleLib::UndertaleText::Token::Color:
		{
			unsigned int v = t.value();
			color4 = Color4B((v ) & 0xFF, (v  >> 16) & 0xFF, (v>>24) & 0xFF,0xFF);
		}
		break;
		case UndertaleLib::UndertaleText::Token::Letter:
			_leterColors.push_back(color4);
			break;
		}
	}
	
	Label::setString(_parser.getCleanedText());
	cocos2d::TextureAtlas* textureAtlas;
	V3F_C4B_T2F_Quad *quads;
	for (auto&& batchNode : _batchNodes)
	{
		textureAtlas = batchNode->getTextureAtlas();
		quads = textureAtlas->getQuads();
		auto count = textureAtlas->getTotalQuads();
		for (int index = 0; index < count; ++index)
			_quadOrigin.push_back(quads[index]);
	}
	
}
FontAtlas* Undertale::LookupFontAtlas(const UndertaleLib::Font& f) {
	FontAtlas* atlas = nullptr;
	if (f.valid()) {
		atlas = _fontAtlases.at(f.index());
		if (!atlas) {
			auto font = UndertaleFont::create(f);
			if (font && (atlas = font->createFontAtlas()))
				_fontAtlases.insert(f.index(), atlas);
		}
	}
	return atlas;
}
FontAtlas* Undertale::LookupFontAtlas(size_t index) { return LookupFontAtlas(_file->LookupFont(index)); }
FontAtlas* Undertale::LookupFontAtlas(const std::string& name) { return LookupFontAtlas(_file->LookupByName<UndertaleLib::Font>(name.c_str())); }

cocos2d::SpriteFrame* Undertale::createSpriteFrame(const UndertaleLib::SpriteFrame& frame) const {
	if (frame.valid()) {
		auto texture = Undertale::getSingleton()->LookupTexture(frame.texture_index);
		SpriteFrame* cframe = SpriteFrame::createWithTexture(texture, Rect(frame.x, frame.y, frame.width, frame.height), false, Vec2(frame.offset_x, frame.offset_y), Size(frame.original_width, frame.original_height));
		return cframe;
	}
	return nullptr;
}


class BackgroundCache {
	Map<size_t, SpriteFrame*> _backgrounds;
	std::unordered_map<UndertaleLib::String, SpriteFrame*> _backgroundText;
public:
	SpriteFrame* operator[](size_t i) {
		SpriteFrame* ret = _backgrounds.at(i);
		if (ret == nullptr) {
			auto& b = _file->LookupBackground(i);
			ret = Undertale::getSingleton()->createSpriteFrame(b.frame());
			_backgrounds.insert(i, ret);
			_backgroundText[b.name()] =  ret;
		}
		return ret;
	}
};

static Sprite* CreateTile(BackgroundCache& cache,  const UndertaleLib::Room::Tile& t) {
	auto& b = _file->LookupBackground(t.background_index);
	auto& f = b.frame();
	auto texture = Undertale::getSingleton()->getSingleton()->LookupTexture(f.texture_index);
	auto frame = cache[t.background_index];

	Rect tileRect(Vec2(t.offset_x+f.x,t.offset_y+f.y), Size(t.width, t.height));
	auto sprite = Sprite::createWithTexture(texture, tileRect);

	//sprite->setAnchorPoint(Vec2(0.0f, 1.0f));
	sprite->setScale(t.scale_x, t.scale_y);
	assert(t.blend==-1);
	return sprite;
}
bool compare_nocase(const std::string& first, const std::string& second)
{
	unsigned int i = 0;
	while ((i<first.length()) && (i<second.length()))
	{
		if (tolower(first[i])<tolower(second[i])) return true;
		else if (tolower(first[i])>tolower(second[i])) return false;
		++i;
	}
	return (first.length() < second.length());
}
UObject* URoom::instance_exists(size_t object_index) const {
		auto it = std::find_if(_objects.cbegin(), _objects.cend(), [object_index](const UObject* o) { return o->is(object_index); });
		return it == _objects.cend() ? nullptr : (*it);
}


UObject* CreateInstance(size_t object_index);


UObject* URoom::instance_create(float x, float y, size_t object_index) {
	UObject* obj = CreateInstance(object_index);
	assert(obj->getUndertaleObject().index() != -1);
	CCLOG("\tCreating Instance: (%i)%s", obj->getUndertaleObject().index(), obj->getUndertaleObject().name().c_str());
	addUObject(obj);

	obj->setAnchorPoint(Vec2(0.0f, 1.0f));
	obj->setPosition(x, getContentSize().height - y);
	return obj;

}


void URoom::instance_destroy(size_t object_index) {
	auto it = _objects.begin();
	while (it != _objects.end()) {
		UObject* obj = (*it);
		if (obj->is(object_index)) {
			_objectLayer->removeChild(obj);
			it = _objects.erase(it);
		}
		else it++;
	}
}
#include "obj_dialoguer.h"

void URoom::setUndertaleRoom(const UndertaleLib::Room& room) {
	if (room.valid() && room.index() != _room.index()) {
		//setAnchorPoint(Vec2(0.0f, 1.0f));
		_objects.clear();
		removeAllChildrenWithCleanup(true);
		CCLOG("Loading Room: (%i)%s", room.index(), room.name().c_str());
		Size roomSize(room.width(), room.height());
		setContentSize(roomSize);
		_room = room;
#if _DEBUG
		// draw the hard box around this room
		{
			auto box = DrawNode::create();
			box->drawRect(Vec2(0.0f, 0.0f), this->getContentSize(), Color4F::WHITE);
			box->setPosition(0.0f, 0.0f);
			addChild(box, -100);
		}

#endif
		

		addChild(_tileLayer = Layer::create(),-1);
		_tileLayer->setAnchorPoint(Vec2(0.0f, 1.0f));
		BackgroundCache cache;
		for (auto& t : _room.tiles()) {
			auto sprite = CreateTile(cache,  t);
			Vec2 objpos(t.x, roomSize.height - t.y);
			sprite->setAnchorPoint(Vec2(0.0f, 1.0f));
			sprite->setPosition(t.x, roomSize.height - t.y);
			_tileLayer->addChild(sprite, t.depth);
			}

		addChild(_objectLayer = Layer::create(),0);
		for (auto& o : _room.objects()) {
			auto obj = instance_create(o.x, o.y, o.object_index);

			obj->setScale(o.scale_x, o.scale_y);
			assert(o.rotation == 0.0f);
			obj->setRotation(o.rotation);
			assert(o.color == -1);		
		}
	
		
		Layer* backgroundLayer=nullptr;// weak pointer
		Layer* forgroundLayer = nullptr; // weak pointer
		for (auto& b : _room.backgrounds()) {
			if (b.background_index >= 0) {
				auto frame = cache[b.background_index];
				auto sprite = Sprite::createWithSpriteFrame(frame);
				addChild(sprite, b.foreground ? 2:-3);
				auto d = DrawNode::create();
				d->drawDot(Vec2(0, 0), 4.0f, Color4F::WHITE);
				d->drawCircle(Vec2(0, 0), 4.0f, 90.0f, 10, true, Color4F::WHITE);
				addChild(d, 10);

				//sprite->setAnchorPoint(Vec2(0.0f, 1.0f));
				sprite->setPosition(b.x, roomSize.height - b.y);
			}
		
		}
	}
	
}
UObject* URoom::containsObject(std::function<bool(UObject*)> pred) const {
	for (auto o : _objects)
		if (pred(o)) return o;
	return nullptr;
}
void URoom::addUObject(UObject* object) {
	object->_room = this;
	_objects.push_back(object);
	_objectLayer->addChild(object);

}
void URoom::removeObject(UObject* object) {
	object->_room = nullptr;
	_objects.remove(object);
	_objectLayer->removeChild(object, true);
}
#ifdef _DEBUG
extern Label* s_debugText;

#endif
bool URoom::init() {
	if (LayerColor::init()) {
#if 0
		auto moseListener = EventListenerMouse::create();


		moseListener->onMouseMove = [this](EventMouse* event) {
			auto pos = event->getLocation();
			Point locationInRoom = this->convertToNodeSpace(pos);
			for (auto o : this->getObjects()) {
				if (o->getBoundingBox().containsPoint(locationInRoom)) {
					std::stringstream ss;
					ss.precision(2);
					ss << '(' << o->getTag() << ')' << o->getName() << std::endl;
					ss << '(' << std::fixed << getPosition().x << ',' << getPosition().y << ')' << std::endl;
					s_debugText->setString(ss.str());
					return;
				}
			}
			s_debugText->setString("nothing");
		};

		this->_eventDispatcher->addEventListenerWithSceneGraphPriority(moseListener, this);
#endif
		return true;
	}
	return false;

}
void URoom::nextRoom() {
	if (_room.valid()) setUndertaleRoom(_room.index() + 1);
}
void URoom::previousRoom() {
	if (_room.valid()) setUndertaleRoom(_room.index() - 1);
}
std::string URoom::getDebugName() const {
	if (_room.valid()) {
		std::stringstream ss;
		ss << _room.name().c_str() << '(' << _room.index() << ')';
		return ss.str();
	} return "notvalid(-1)";
}
URoom* URoom::create(const UndertaleLib::Room& room) {
	if (room.valid()) {
		URoom* uroom = new URoom;
		if (uroom && uroom->init()) {
			uroom->setUndertaleRoom(room);
			uroom->autorelease();
			return uroom;
		}
		CC_SAFE_DELETE(uroom);
	}
	return nullptr;
}
URoom* URoom::create(size_t room_index) {
	return create(_file->LookupRoom(room_index));
}