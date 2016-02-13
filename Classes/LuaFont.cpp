#include "LuaFont.h"
#include "LuaEngine.h"
#include "UndertaleResourcesInternal.h"
#include "AudioEngine.h"
#include "SimpleAudioEngine.h"
#include "2d/CCFontAtlasCache.h"

USING_NS_CC;

using namespace experimental;
class LuaFontConfiguration {
public:
	istring font_name;
	istring font_description;
	float font_size;
	bool bold;
	bool italic;
	uint16_t first_char;
	uint16_t last_char;
	bool antiAlias;
	int charSet;
	GM_RawSpriteFrame frameOffset;
	float scaleW;
	float scaleH;
	std::unordered_map<uint32_t, uint16_t> _kerningDictionary;
	std::unordered_map<uint32_t, GM_FontGlyph> _fontDictonary;
	static std::unordered_map<istring, LuaFontConfiguration*> _fontConfigs;
	//const GM_FontGlyph& lookupFont(uint16_t ch) const {
	//	auto it = _fontDictonary.find(ch);
	//	if (it == _fontDictonary.cend()) return nullptr;
	//	else return it->second;
	//}
	void AddGlyph(const GM_FontGlyph& glyph) {
		_fontDictonary.emplace(std::make_pair(glyph.ch, glyph));
	}
	uint16_t lookupKerning(uint16_t first, uint16_t second) const {
		auto it = _kerningDictionary.find(first | (second << 16));
		if (it == _kerningDictionary.cend()) return 0;
		else return it->second;
	}
	void addKerning(uint16_t first, uint16_t second, uint16_t amount) {
		_kerningDictionary[(uint32_t)(first | (second << 16))] = amount;
	}
	bool init(istring fontName) {


		return true;
	}
	static LuaFontConfiguration* getFontConfig(istring font_name) {
		auto it = _fontConfigs.find(font_name);
		if (it != _fontConfigs.end()) return it->second;
		return nullptr;
	}
};

 std::unordered_map<istring, LuaFontConfiguration*> LuaFontConfiguration::_fontConfigs;

LuaFont::LuaFont()
{
}

LuaFont * LuaFont::create(istring fontName)
{
	LuaFontConfiguration* fontConfig = LuaFontConfiguration::getFontConfig(fontName);
	if (!fontConfig) return nullptr;
	LuaFont* font = new	LuaFont();
	font->_config = fontConfig;
	return font;
}

FontAtlas * LuaFont::createFontAtlas()
{
	FontAtlas *tempAtlas = nullptr;
	do {
		// check that everything is fine with the BMFontCofniguration
		if (!_config) break;
		tempAtlas = new (std::nothrow) FontAtlas(*this);
		if (!tempAtlas) break;
		// common height
		int originalFontSize = _config->font_size;

		tempAtlas->setLineHeight(originalFontSize);
		int textureID = 0;
		for (auto it : _config->_fontDictonary) {
			const GM_FontGlyph& g = it.second;
			char c = g.ch;
			FontLetterDefinition tempDefinition;
			memset(&tempDefinition, 0, sizeof(tempDefinition));
			Rect tempRect(g.x, g.y, g.width, g.height);
			tempRect = CC_RECT_PIXELS_TO_POINTS(tempRect);

			tempDefinition.offsetX = 0;// g.offset;
			tempDefinition.offsetY = 0;// fontDef.yOffset;

			tempDefinition.U = tempRect.origin.x + _config->frameOffset.x;
			tempDefinition.V = tempRect.origin.y + _config->frameOffset.y;

			tempDefinition.width = tempRect.size.width;
			tempDefinition.height = tempRect.size.height;
			//carloX: only one texture supported FOR NOW
			tempDefinition.textureID = 0;

			tempDefinition.validDefinition = true;
			tempDefinition.xAdvance =  g.shift; // I THINK its shift, hopefuly
			tempAtlas->addLetterDefinition(g.ch, tempDefinition);
		}
		Texture2D *tempTexture = UndertaleResources::getInstance()->getTexture(_config->frameOffset.texture_id);
		tempAtlas->addTexture(tempTexture, 0);
	} while (false);
	return tempAtlas;
}

int * LuaFont::getHorizontalKerningForTextUTF16(const std::u16string & text, int & outNumLetters) const
{
	int* sizes = nullptr;
	do {
		outNumLetters = static_cast<int>(text.length());
		if (!outNumLetters) break;
		sizes = new (std::nothrow) int[outNumLetters];
		if (!sizes)  break;
		for (int c = 0; c < outNumLetters; ++c)
		{
			if (c < (outNumLetters - 1))
				sizes[c] = _config->lookupKerning(text[c], text[c + 1]);  
			else
				sizes[c] = 0;
		}
	} while (false);
	return sizes;
}

// Write the info block
#pragma pack(push)
#pragma pack(1)
struct infoBlock
{
	int            blockSize;
	unsigned short fontSize;
	char           reserved : 4;
	char           bold : 1;
	char           italic : 1;
	char           unicode : 1;
	char           smooth : 1;
	unsigned char  charSet;
	unsigned short stretchH;
	char           aa;
	unsigned char  paddingUp;
	unsigned char  paddingRight;
	unsigned char  paddingDown;
	unsigned char  paddingLeft;
	unsigned char  spacingHoriz;
	unsigned char  spacingVert;
	unsigned char  outline;
	char           fontName[1];
} info;
#pragma pack(pop)
#pragma pack(push)
#pragma pack(1)
struct commonBlock
{
	int blockSize;
	unsigned short lineHeight;
	unsigned short base;
	unsigned short scaleW;
	unsigned short scaleH;
	unsigned short pages;
	unsigned char  packed : 1;
	unsigned char  reserved : 7;
	unsigned char  alphaChnl;
	unsigned char  redChnl;
	unsigned char  greenChnl;
	unsigned char  blueChnl;
} common;
#pragma pack(pop)
#pragma pack(push)
#pragma pack(1)
struct charBlock
{
	DWORD id;
	WORD x;
	WORD y;
	WORD width;
	WORD height;
	short xoffset;
	short yoffset;
	short xadvance;
	char  page;
	char  channel;
} charInfo;
#pragma pack(pop)
void UndertaleResources::readAllFonts() {
	_fontLookup.clear();
	const Chunk& fontChunk = _chunks["FONT"];
	GM_RawSpriteFrame rawFrame;
	infoBlock info;
	commonBlock common;
	charBlock block;
	r.seek(fontChunk.begin());
	auto fontOFfsets = getOffsetEntries(r);
	GM_SpriteFrame fontFrame;
	std::string common_font_texture_name;
	Texture2D* texture =  _textures.at(6);
	
	for (uint32_t offset : fontOFfsets) {
		r.seek(offset);
		std::string font_name = r.readStringAtOffset(r.readInt());
		std::string font_description = r.readStringAtOffset(r.readInt());
		int fontSize = r.readInt();
		bool bold = r.readBool();
		bool italic = r.readBool();
		int flag = r.readInt();
		int first_char = flag & 0xFFFF;
		int char_set = (flag >> 16) & 0xFF;
		int antiAlias = (flag >> 24) & 0xFF;
		int last_char = r.readInt();
		r.readAtOffset(r.readInt(), rawFrame);
		float scaleW = r.readSingle();
		float scaleH = r.readSingle();
		// BMFont binary format
		std::fstream font_writer;
		
		font_writer.open(font_name + ".fnt", std::fstream::out | std::fstream::binary | std::ifstream::trunc);
		font_writer.seekp(0);
		font_writer.write("BMF", 3); // write the file name and version
		font_writer.put(3); // version 3

		std::memset(&info, 0, sizeof(info));
		std::memset(&common, 0, sizeof(common));

#if 0
		LuaFontConfiguration* fontConfig = new LuaFontConfiguration();
		fontConfig->font_name = r.readStringAtOffset(r.readInt());
		fontConfig->font_description = r.readStringAtOffset(r.readInt());
		
		fontConfig->font_size = r.readInt();
		fontConfig->bold = r.readBool();
		fontConfig->italic = r.readBool();
		int flag = r.readInt();
		fontConfig->first_char = flag & 0xFFFF;
		fontConfig->charSet = (flag >> 16) & 0xFF;
		fontConfig->antiAlias = (flag >> 24) & 0xFF;
		fontConfig->last_char = r.readInt();
		r.readAtOffset(r.readInt(), fontConfig->frameOffset);
		fontConfig->scaleW = r.readSingle();
		fontConfig->scaleH = r.readSingle();
		

#else
		// header
		info.blockSize = sizeof(info) + font_name.length() - 4;
		info.fontSize = fontSize;
		info.bold = bold ? 1 : 0;
		info.italic = italic ? 1 : 0;
		info.unicode = 0;
		info.smooth = 0; // put aa here?
		info.charSet = char_set;
		info.stretchH = 100; // Humm check scaleH
		info.aa = 0;
		info.spacingHoriz = 1; // cocos only cares about teh spacing here
		info.spacingVert = 1;
		font_writer.put(1); // header block id
		font_writer.write((const char*)&info, sizeof(info) - 1);
		font_writer.write(font_name.c_str(), font_name.length() + 1);
		
		// common block
		common.blockSize = sizeof(common) - 4;
		common.lineHeight = fontSize;
		common.base = fontSize-2; // maybe?
		common.scaleW = 1024; // rawFrame.width; main texture sizie is this
		common.scaleH = 1024; // rawFrame.height;
		common.pages = 1;
		common.packed = 0;
		common.alphaChnl = 1;//Set to 1 if the monochrome characters have been packed into each of the texture channels.In this case alphaChnl describes what is stored in each channel.
		common.redChnl = 0;//Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyph and the outline, 3 if its set to zero, and 4 if its set to one.
		common.greenChnl = 0;//Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyph and the outline, 3 if its set to zero, and 4 if its set to one.
		common.blueChnl = 0;//Set to 0 if the channel holds the glyph data, 1 if it holds the outline, 2 if it holds the glyph and the outline, 3 if its set to zero, and 4 if its set to one.
		font_writer.put(2); // common block id
		font_writer.write((const char*)&common, sizeof(common));

		// Write the page block
		font_writer.put(3);
		// Undertale uses texture 6 as the page block, otherwise we would have to know this ahead of checking all the glyph data
		const std::string& texture_filename = _textureFilenames[6];
		int size = texture_filename.length() + 1;
		font_writer.write((const char*)&size, sizeof(size)); // only one page_textureFilenames
		font_writer.write(texture_filename.c_str(), texture_filename.length() + 1);
#endif
		auto glyphEntries = getOffsetEntries(r);
		// Write the page block
		font_writer.put(4); // put in size of the blocks
		size = sizeof(charInfo) * glyphEntries.size(); // (4+2+2+2+2+2+2+2+1+1)
		font_writer.write((const char*)&size, sizeof(size));// count of glyphs
		

		for (uint32_t goffset : glyphEntries) {
			r.seek(goffset);
			GM_FontGlyph glyph;
			r.read(glyph);
			block.id = glyph.ch;
			block.x = glyph.x +rawFrame.x;
			block.y = glyph.y +rawFrame.y;
			block.width = glyph.width;
			block.height = glyph.height;
			block.xoffset = glyph.offset;
			block.yoffset = 0;
			block.xadvance = glyph.shift;
			block.page = 0;// 1;// glyph.offset;
			block.channel = 8; // 	The texture channel where the character image is found (1 = blue, 2 = green, 4 = red, 8 = alpha, 15 = all channels).
			font_writer.write((const char*)&block, sizeof(block));// count of glyphs
		}
		font_writer.close();
		// none of the undertale fonts have kerrnings
		//LuaFontConfiguration::_fontConfigs[fontConfig->font_name] = fontConfig;
	}
}
// sigh, can't get away from tokenizing this whole thing, unless we want to do it
// live in the update function.  While that IS an option, its also slow as fuck and


class LuaLabelSprite : public Sprite {
	static const  float _updateTime;
	bool _hide;
	Vec2 _shakeSavePos;
	float _currentTime;
	CC_SYNTHESIZE(float, _shake, Shake);
	LuaLabelSprite() : _shake(0.0f), _hide(false), _shakeSavePos(0, 0), _currentTime(0), Sprite() {}
public:
	static LuaLabelSprite* create() {
		LuaLabelSprite* sprite = new LuaLabelSprite();
		if (sprite && sprite->init()) {
			sprite->autorelease();
			sprite->hideLetter();
			return sprite;
		}
		CC_SAFE_DELETE(sprite);
		return nullptr;
	}
	// Just need to override this as 
	void showLetter() { if (_hide) { _shakeSavePos = getPosition();  _hide = false; setScale(1.0f);  _currentTime = _updateTime; scheduleUpdate(); } }
	void hideLetter() { if (!_hide) { _hide = true; setScale(0.0f); unscheduleUpdate(); } }

	void update(float dt) override {
		if (!_hide && _shake > FLT_EPSILON) {
			if ((_currentTime -= dt) < 0.0f) {
				_currentTime = _updateTime;
				Vec2 pos = _shakeSavePos;
				pos.x += cocos2d::random(0.0f, _shake) - (_shake / 2);
				pos.y += cocos2d::random(0.0f, _shake) - (_shake / 2);
				Sprite::setPosition(pos);
			}
		}
	}
};
const  float LuaLabelSprite::_updateTime = 1.0 / 30.0f;

void LuaLabel::setTypingSound(istring soundFile)
{
	auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
	if (_typesound != 0) { _typesound = 0;  audio->stopEffect(_typesound); }
	audio->preloadEffect(soundFile.c_str());
	_typingSound = soundFile;
}

void LuaLabel::setString(const std::string & text, bool startTyping)
{
	if (_originalString != text) {
		_originalString = text;
		_utf16Text.clear();
		StringUtils::UTF8ToUTF16(text, _utf16Text);
		_atlas->prepareLetterDefinitions(_utf16Text);
		_utf16Text.clear();
		_needsUpdate = true;
	}
	updateContent(true);
	if (startTyping) restartTyping();
}
void LuaLabel::updateContent(bool hideLetters)
{
	if (!_needsUpdate || _originalString.empty()) return;
	const auto& text = _originalString;
	// Very annoying.  We have to pars
	//	global.msg[0] = "\\E2* If you truly wish to&  leave the RUINS.../* I will not stop you./\\E2* However^1, when you&  leave.../\\E1* Please do not come&  back./\\E2* I hope you understand./%%"
	Texture2D* letterTexture = _atlas->getTexture(0);
	if (text.size() > _letterCache.size()) {
		for (int i = _letterCache.size(); i < text.size(); i++) {
			LuaLabelSprite* letter = LuaLabelSprite::create();
			Rect uvRect(730, 4, 16, 22); // filler
			letter->setTextureRect(uvRect);
			letter->setTexture(letterTexture);
			_letterCache.pushBack(letter);
			addChild(letter, 1);
		}
	}
	Sprite* s;
	Color3B color = Color3B::WHITE;

	HaltType halt = HaltType::Typing;
	Point pos;
	int l = 0;
	FontLetterDefinition letterDef;
	Size contentSize;
	auto getNextLetter = [this, &l, letterTexture, &letterDef](char16_t c) {
		LuaLabelSprite* sprite = this->_letterCache.at(l++);
		if (!_atlas->getLetterDefinitionForChar(c, letterDef)) throw std::exception("ugh");
		//auto textureID = letterDef.textureID;
		Rect uvRect;
		uvRect.size.height = letterDef.height;
		uvRect.size.width = letterDef.width;
		uvRect.origin.x = letterDef.U;
		uvRect.origin.y = letterDef.V;

		sprite->setTextureRect(uvRect);
		sprite->setTexture(letterTexture);
		return sprite;
	};
	std::vector<LabelToken> line;
	for (int i = 0; i < text.length(); i++) {
		char16_t c, n;
		c = text[i];
		switch (c) {
		case '^':
		{
			n = text[++i];
			uint32_t speed = n - '0';
			line.emplace_back(speed);
			continue;
		}
		case '&': // inline return
			c = '\n';
			break;
		case '\\': // color!
			n = text[++i];
			switch (n) {
			case 'R':color = Color3B::RED; break;
			case 'W':color = Color3B::WHITE; break;
			case 'Y':color = Color3B::YELLOW; break;
			case 'X':color = Color3B::BLACK; break;
			case 'B':color = Color3B::BLUE; break;
			case 'C': break;// choise not implmented
			case 'E': {
				n = text[++i];
				int face = n - '0';
				line.emplace_back((void*)(face), &_facemotion_event);
				continue;
			}
			case 'F':
			{
				n = text[++i];
				int face = n - '0';
				line.emplace_back((void*)(face), &_facemovement_event);
				continue;
			}
			case 'T': ++i; break; // TextType, need to figure that out
			}
			continue;
		case '/': // Halt commands
			n = text[i + 1]; // wierd
			halt = HaltType::WaitingOnKeyPress;
			if (n == '%') { i++; halt = HaltType::DoneTyping; }
			else if (n == '^') { i++; halt = HaltType::DoneTyping; } // this means self destory, not implmented
			line.emplace_back(halt);
			pos = Vec2::ZERO;
			color = Color3B::WHITE; // color resets on the line apperntly
			_letters.push_back(std::move(line));
			line = std::vector<LabelToken>();
			continue;
		}
		if (c == '\n') {
			if (contentSize.width < pos.x) contentSize.width = pos.x;
			pos.y -= _atlas->getLineHeight();
			if (contentSize.height < pos.y) contentSize.width = pos.y;
			pos.x = 0;
			continue;
		}
		if (c == '%') break;
		LuaLabelSprite* sprite = getNextLetter(c);
		sprite->setColor(color);
		sprite->setShake(_shake);
		if (hideLetters) sprite->hideLetter(); else sprite->showLetter();
		auto px = pos.x + letterDef.width / 2;
		auto py = pos.y - letterDef.height / 2;
		sprite->setPosition(px, py);
		line.emplace_back(c, sprite);
		_utf16Text.push_back(c);
		pos.x += letterDef.xAdvance;
		halt = HaltType::Typing;
	}
	if (line.size() > 0) {
		line.emplace_back(HaltType::DoneTyping); // we make sure we have a halt at the end
		_letters.push_back(std::move(line));

	}
	_needsUpdate = false;
}


LuaLabel::LuaLabel() :_atlas(nullptr), _typesound(0), _keyboardListener(nullptr), _currentTextPos(0), _currentLine(0), _textSpeed(0), _currentTime(0), _shakeTimer(0),
_facemotion_event("facemotionEvent"), _facemovement_event("facemovementEvent"), _halt(HaltType::DoneTyping), _needsUpdate(false) ,_shake(0.0f) {}
LuaLabel::~LuaLabel() {
	_letters.clear();
	_letterCache.clear();
	getEventDispatcher()->removeEventListener(_keyboardListener);
}
LuaLabel * LuaLabel::create(istring font, uint32_t speed, float shake)
{
	LuaLabel* label = new LuaLabel();
	if (label && label->init()) {
		label->_atlas = FontAtlasCache::getFontAtlasFNT(font.c_str());
		CC_SAFE_RETAIN(label->_atlas);
		label->_textSpeed = speed;
		label->_shake = shake;
		label->_halt = HaltType::DoneTyping;
		auto keyboardListener = EventListenerKeyboard::create();
		keyboardListener->onKeyPressed = [label](EventKeyboard::KeyCode key, Event* event) {
			if (label->isWaitingOnKeyPress() && key == EventKeyboard::KeyCode::KEY_RETURN || key == EventKeyboard::KeyCode::KEY_ENTER) label->nextLine();
		};
		label->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, label);
		return label;
	}
	CC_SAFE_DELETE(label);
	return nullptr;
}
bool LuaLabel::init(istring font, cocos2d::Color3B color, float x, float y, float end_x, uint32_t shake, istring sound, uint32_t hspacing, uint32_t vspacing)
{
	CC_SAFE_RELEASE_NULL(_atlas);
	_atlas = FontAtlasCache::getFontAtlasFNT(font.c_str());
	setPosition(x, y);
	_end_x = end_x;
	_shake = shake;
	setTypingSound(sound);
	_hspacing = hspacing;
	_vspacing = vspacing;
	_keyboardListener = EventListenerKeyboard::create();
	_keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode key, Event* event) {
		if (isWaitingOnKeyPress() && key == EventKeyboard::KeyCode::KEY_RETURN || key == EventKeyboard::KeyCode::KEY_ENTER) nextLine();
	};
	getEventDispatcher()->addEventListenerWithSceneGraphPriority(_keyboardListener, this);

	// events

	return true;
}
void LuaLabel::clear() {
	setHalt(HaltType::DoneTyping);
	_originalString.clear();
	_letters.clear();
}
void LuaLabel::restartTyping()
{
	if (_letters.size() == 0) return;
	updateContent(true);
	for (auto& a : _letterCache) a->hideLetter();
	_currentTextPos = 0;
	_currentTime = _textSpeed;
	_currentLine = 0;
	setHalt(HaltType::Typing);
}

void LuaLabel::nextLine()
{
	if ( _halt == HaltType::WaitingOnKeyPress) {
		if (_currentLine < _letters.size()) {
			auto& line = _letters[_currentLine++];
			for (auto& a : line) if (a.type() == TokenType::Sprite) a.getSprite()->hideLetter();
			_currentTextPos = 0;
			setHalt(HaltType::Typing);
		}
		else setHalt(HaltType::DoneTyping);
	}
}
void LuaLabel::setTypingSpeed(uint32_t speed) { _textSpeed = speed;  updateContent(true); }
void LuaLabel::setHalt(HaltType halt) {
	updateContent(true);
	switch (halt) {
	case HaltType::Paused:
	case HaltType::DoneTyping:
	case HaltType::WaitingOnKeyPress:
		this->unscheduleUpdate();
		break;
	case HaltType::Typing:
		setCurrentTime(_textSpeed); // reset the time
		this->scheduleUpdate();
		break;
	}
	_halt = halt;
}
void LuaLabel::update(float dt)
{
	if (_halt == HaltType::Typing) {
		if (_currentLine < _letters.size()) {
			
			auto& line = _letters[_currentLine];
		//	if (_currentTextPos >= line.size()) { // because we got a bad string

		//	}
			if ((_currentTime -= dt) < 0) {
				auto& token = line[_currentTextPos++];
				switch (token.type()) {
				case TokenType::Event:
					token.dispatchEvent(this);
					setCurrentTime(_textSpeed);
					break;
				case TokenType::Halt:
					setHalt(token.getHalt());
					break;
				case TokenType::Speed:
					setCurrentTime(token.getSpeed());
					break;
				case TokenType::Sprite:
				{
					LuaLabelSprite* sprite = token.getSprite();
					sprite->showLetter();
					if (!_typingSound.isEmpty()) {
						auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
						if (_typesound != 0) AudioEngine::stop(_typesound);
						audio->playEffect(_typingSound.c_str(), false);
					}
					setCurrentTime(_textSpeed);
				}
				break;
				default:
					throw std::runtime_error("Not supported yet");
				}
			}
		}
	}
	Node::update(dt);
}


