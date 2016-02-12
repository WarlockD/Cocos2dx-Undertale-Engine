#include "LuaFont.h"
#include "LuaEngine.h"
#include "UndertaleResourcesInternal.h"
#include "AudioEngine.h"

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

class LuaLabelSprite : public Sprite {
	static const  float _updateTime;
	
	bool _hide;
	Vec2 _shakeSavePos;
	float _currentTime;
	CC_SYNTHESIZE(float, _shake, Shake);
	CC_SYNTHESIZE(char16_t, _ch, Char);
	CC_SYNTHESIZE(float, _textSpeed, TextSpeed);
	CC_SYNTHESIZE(int, _halt, Halt);
	LuaLabelSprite() :_ch(0), _shake(0.0f), _halt(0), _hide(true), _shakeSavePos(0, 0), _currentTime(0), Sprite() {}
public:
	static LuaLabelSprite* create() {
		LuaLabelSprite* sprite = new LuaLabelSprite();
		if (sprite && sprite->init()) {
			sprite->autorelease();
			return sprite;
		}
		CC_SAFE_DELETE(sprite);
		return nullptr;
	}
	// Just need to override this as 
	void setPosition(float x, float y) override {
		if(_hide)_shakeSavePos = Vec2(x, y);
		
		Sprite::setPosition(x, y);
	}
	void showLetter() { if (_hide) { _shakeSavePos = getPosition();  _hide = false; setScale(1.0f);  _currentTime = _updateTime; scheduleUpdate(); } }
	void hideLetter() { if (!_hide) { _hide = true; setScale(0.0f); unscheduleUpdate(); } }

	void update(float dt) override {
		if (!_hide) {
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
	if(_typesound > -1) AudioEngine::stop(_typesound);
	AudioEngine::preload(soundFile.c_str());
	_typingSound = soundFile;
}

void LuaLabel::setString(const std::string & text)
{
	_originalString = text;
	_utf16Text.clear();
	StringUtils::UTF8ToUTF16(text,_utf16Text);
	_atlas->prepareLetterDefinitions(_utf16Text);
	_utf16Text.clear();
	// Very annoying.  We have to pars
//	global.msg[0] = "\\E2* If you truly wish to&  leave the RUINS.../* I will not stop you./\\E2* However^1, when you&  leave.../\\E1* Please do not come&  back./\\E2* I hope you understand./%%"
	Texture2D* letterTexture = _atlas->getTexture(0);
	if (text.size() > _letterCache.size()) {
		for (int i = _letterCache.size(); i < text.size(); i++) {
			LuaLabelSprite* letter = LuaLabelSprite::create();
			addChild(letter);
			_letterCache.pushBack(letter);
		}
	}
	float speed = _textSpeed;
	Sprite* s;
	Color3B color = Color3B::WHITE;

	int halt = 0;
	Point pos;
	int l = 0;
	FontLetterDefinition letterDef;
	auto getNextLetter = [this, &l, letterTexture,&letterDef](char16_t c) {
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
	Vector<LuaLabelSprite*> line;
	for (int i = 0; i < text.length();i++) {
		char16_t c, n;
		c = text[i];
		switch (c) {
		case '^':
			speed = (('0'-text[++i]) * 10) / 30.0f;// speed is 30 frames a second...hopefuly
			continue;
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
			case 'E': break; // facemotion not implmented;
			case 'F': break; // face Chioe not implmented
			case 'T': ++i; break; // TextType, need to figure that out
			}
			continue;
		case '/': // Halt commands
			n = text[i + 1]; // wierd
			halt = 1;
			if (n == '%') { i++; halt = 2; }
			else if (n == '^') { i++; halt = 4; }
			c = ' '; // HACK here, but it fixes
			break;
		}
		if (c == '\n') {
			pos.y -= _atlas->getLineHeight();
			pos.x = 0;
			continue;
		}
		LuaLabelSprite* sprite = getNextLetter(c);
		sprite->setHalt(halt);
		sprite->setTextSpeed(speed);
		sprite->setColor(color);
		sprite->setShake(_shake);
		sprite->setChar(c);
		sprite->hideLetter();
		auto px = pos.x + letterDef.width / 2;
		auto py = pos.y - letterDef.height / 2;
		sprite->setPosition(px, py);
		
		line.pushBack(sprite);
		_utf16Text.push_back(c);
		if (halt != 0) {
			pos = Vec2::ZERO;
			_letters.push_back(std::move(line));
			line = Vector<LuaLabelSprite*>();
		}
		pos.x += letterDef.xAdvance;
		speed = _textSpeed;
		halt = 0;
		if (c == '%') break;
	}
	if(line.size() > 0) _letters.push_back(std::move(line));
	_currentTextPos = 0;
	_currentTime = _textSpeed;
	_currentLine = 0;
	_halt = 0;
	_needsUpdate = true;
	updateContent(false);
	this->scheduleUpdate();
}
void LuaLabel::updateContent(bool hideLetters)
{
	_needsUpdate = false;
}

void LuaLabel::nextLine()
{
	auto& line = _letters[_currentLine++];
	for (int i = 0; i < line.size(); i++) line.at(i)->hideLetter();
	_currentLine++;
	_currentTextPos = 0;
	_halt = 0;
	_currentTime = _textSpeed;//->getTextSpeed(); // get the new text speed if it exisits
}

LuaLabel * LuaLabel::create(istring font, float speed)
{
	LuaLabel* label = new LuaLabel();
	if (label && label->init()) {
		label->_atlas = FontAtlasCache::getFontAtlasFNT(font.c_str());
		label->_textSpeed = speed;
		label->_shake = 0.0f;
		label->_shakeTimer = 1.0f / 30.0f;
		label->_halt = 0;
		auto keyboardListener = EventListenerKeyboard::create();
		keyboardListener->onKeyPressed = [label](EventKeyboard::KeyCode key, Event* event) {
			if (label->_halt == 1 && key == EventKeyboard::KeyCode::KEY_RETURN || key == EventKeyboard::KeyCode::KEY_ENTER) label->nextLine();
		};
		label->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyboardListener, label);
		return label;
	}
	CC_SAFE_DELETE(label);
	return nullptr;
}
void LuaLabel::setTypingSpeed(float speed) { _textSpeed = speed; setString(_originalString); }
void LuaLabel::update(float dt)
{
	if (_halt == 1) return;
	if ( _currentLine < _letters.size()) {
		auto& line = _letters[_currentLine];
		if (_currentTextPos >= line.size())  return;
		if ((_currentTime -= dt) < 0) {
			auto sprite = line.at(_currentTextPos);
			_halt = sprite->getHalt();
			_currentTime = sprite->getTextSpeed(); // get the new text speed if it exisits
		//	assert(_halt == 1);
			if (_halt != 0) return;
			sprite->showLetter();// (1.0f); // cheap way to turn on visiual
			if (!_typingSound.isEmpty()) {
				if (_typesound > -1) AudioEngine::stop(_typesound);
				_typesound = AudioEngine::play2d(_typingSound.c_str());
			}
			_currentTextPos++;
		}
	}
	Node::update(dt);
}

