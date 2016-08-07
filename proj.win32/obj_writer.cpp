#include "obj_writer.h"

USING_NS_CC;


obj_writer::obj_writer() : UObject(), _fontAtlas(nullptr) , _face(0), _emotion(0) {}
obj_writer::~obj_writer() {
	
	if (_fontAtlas)
	{
		Node::removeAllChildrenWithCleanup(true);
	//	CC_SAFE_RELEASE_NULL(_reusedLetter);
		CC_SAFE_RELEASE_NULL(_fontAtlas);
		CC_SAFE_RELEASE_NULL(_textLayer);
	}
}

bool obj_writer::init()  {
	if (UObject::init(object_index)) {
		_textLayer = Node::create();
		addChild(_textLayer,1000);
		_textLayer->retain();
		_textLayer->setVisible(true);
		return true;
	}
	return false;
}

void obj_writer::setUndertaleFont(size_t font_index) {
	setFontAtlas(Undertale::getSingleton()->LookupFontAtlas(font_index));
}
void obj_writer::setFontAtlas(FontAtlas* atlas) {
	if (atlas != _fontAtlas) {
		CC_SAFE_RELEASE_NULL(_fontAtlas);
		_fontAtlas = atlas;
		CC_SAFE_RETAIN(_fontAtlas);
	}
}
#define TOKEN UndertaleLib::UndertaleText::Token

union FaceState {
	struct {
		char face;
		char emotion;
		char flag;
		char delay;
	};
	void* to_void;
	int to_int;
};

void obj_writer::changeFace(size_t face) {
	if (_face != face) {
		_face = face;
		EventCustom event("face_change");
		event.setUserData((void*)face);
		_eventDispatcher->dispatchEvent(&event);
	}
}
void obj_writer::changeEmotion(size_t emotion) {
	if (_emotion != emotion) {
		_emotion = emotion;
		EventCustom event("emotion_change");
		event.setUserData((void*)emotion);
		_eventDispatcher->dispatchEvent(&event);
	}
}
void obj_writer::updateLetters(bool visable) {
	_textLayer->removeAllChildren();
	_currentCachePosition = 0;
	Color3B color(_config.mycolor >> 16, _config.mycolor >> 8, _config.mycolor);
	Vec2 startWriting(0.0f, getContentSize().height);
	Vec2 writing = startWriting;
	FaceState state;
	state.face = _face;
	state.emotion = _emotion;
	state.delay = 1;
	state.flag = 0;
	for (auto& t : _text) {
		switch (t.token()) {
		case TOKEN::Color:
		{
			int icolor = t.value();
			color = Color3B(icolor >> 16, icolor >> 8, icolor);
		}
		break;
		case TOKEN::Face:
			state.face = t.value();
			break;
		case TOKEN::Emotion:
			state.emotion = t.value();
			break;
		case TOKEN::Delay:
			if (t.value() != 0) state.delay = t.value();
			break;
		case TOKEN::NewLine:
			writing.x = startWriting.x;
			writing.y -= _config.spacing;
			break;
		case TOKEN::Letter:
			// letter, type it and make a sound
		{
			char16_t ch = t.value();
			if (writing.y > _config.writingxend) {
				writing.x = startWriting.x;
				writing.y -= _config.spacing;
			}
			else {
				if (_config.typer == 18) {
					if (ch == 'l' || ch == 'i') writing.x += 2;
					if (ch == 'I') writing.x += 2;
					if (ch == '!') writing.x += 2;
					if (ch == '.') writing.x += 2;
					if (ch == 'S') writing.x++;
					if (ch == '?') writing.x += 2;
					if (ch == 'D') writing.x++;
					if (ch == 'A') writing.x++;
					if (ch == '\'') writing.x++;
				}
			}
			auto sprite = getletter(ch);
			if (sprite) {
				state.flag = ch;
				sprite->setPosition(writing);
				sprite->setVisible(visable);
				sprite->setTag(state.to_int);
				_textLayer->addChild(sprite);
				state.delay = 1; // reset frame delay
			}
			{
				int kern = 0;
				if (_config.myfont == 8) { // only two cases that have kernings?
					if (ch == 'w') kern += 2;
					if (ch == 'm') kern += 2;
					if (ch == 'i') kern -= 2;
					if (ch == 'l') kern -= 2;
					if (ch == 's') kern--;
					if (ch == 'j') kern--;
				}
				if (_config.myfont == 9) { // only two cases that have kernings?
					if (ch == 'D') kern++;
					if (ch == 'Q') kern += 3;
					if (ch == 'M') kern++;
					if (ch == 'L') kern--;
					if (ch == 'K') kern--;
					if (ch == 'C') kern++;
					if (ch == '.') kern -= 3;
					if (ch == '!') kern -= 3;
					if (ch == 'O' || ch == 'W') kern += 2;
					if (ch == 'I') kern -= 6;
					if (ch == 'T') kern--;
					if (ch == 'P') kern -= 2;
					if (ch == 'R') kern -= 2;
					if (ch == 'A') kern++;
					if (ch == 'H') kern++;
					if (ch == 'B') kern++;
					if (ch == 'G') kern++;
					if (ch == 'F') kern--;
					if (ch == '?') kern -= 3;
					if (ch == '\'') kern -= 6;
					if (ch == 'J') kern--;
				}
				writing.x += kern;
				//postFixWriting(ch);
			}


			writing.x += _config.spacing;
			break;
		}
		}
	}
}
/**
* LabelLetter used to update the quad in texture atlas without SpriteBatchNode.
*/
class LabelLetter : public Sprite
{
public:
	LabelLetter() : Sprite() {}
	void setLetterDef(Texture2D* texture, const FontLetterDefinition& letterDef) {
		Rect uvRect;
		uvRect.size.height = letterDef.height;
		uvRect.size.width = letterDef.width;
		uvRect.origin.x = letterDef.U;
		uvRect.origin.y = letterDef.V;

		_offsetPosition = Vec2(letterDef.offsetX, letterDef.offsetY);
		setTexture(texture);
		setTextureRect(uvRect);
		_dirty = true;
	}
	static LabelLetter* create(Texture2D* texture, const FontLetterDefinition& letterDef)
	{
		auto letter = new (std::nothrow) LabelLetter();
		if (letter && letter->init())
		{
			letter->Sprite::setVisible(false);
			letter->setLetterDef(texture, letterDef);
			letter->autorelease();
			return letter;
		}
		CC_SAFE_DELETE(letter);
		return nullptr;
	}
};

Sprite* obj_writer::getletter(char16_t ch)  {
	FontLetterDefinition letterDef;
	if (_fontAtlas && _fontAtlas->getLetterDefinitionForChar(ch, letterDef)) {
		auto textureID = letterDef.textureID;
		LabelLetter* sprite = nullptr;
		if (_currentCachePosition < _letterCache.size()) {
			sprite = (LabelLetter*)_letterCache.at(_currentCachePosition++);
			sprite->setLetterDef(_fontAtlas->getTexture(textureID), letterDef);
		}
		else {
			sprite = LabelLetter::create( _fontAtlas->getTexture(textureID), letterDef);
			sprite->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);// top left
			_letterCache.pushBack(sprite);
			_currentCachePosition++;
		}
		return sprite;
	}
	return nullptr;
}


void obj_writer::setString(const std::string& text,bool instant) {
	_text.setText(text);
	_instant = instant;
	if (_fontAtlas) {
		setContentSize(Size(_config.writingxend, 2 * _config.vspacing));
	}
	updateLetters(instant);
	_typeingPosition = instant ? getChildrenCount() : 0;
}
void obj_writer::clear() {
	if (_children.size()) {
		for (auto& child : _children) child->setVisible(false);
	}
}
void obj_writer::start() {
	if (!_instant) {
		_typeingPosition = 0;
		_frameDelay = 0;
		scheduleUpdate();
		clear();
	}
}
void obj_writer::stop() {
	if (!_instant) {
		unscheduleUpdate();
	}
}
void obj_writer::reset() {
	updateLetters(_instant);
}
void obj_writer::setType(const TEXTTYPE& type) {
	
	//setPosition(type.writingx, _room->getContentSize().height- type.writingy);
	if (!_fontAtlas|| _config.myfont != type.myfont) {
		setFontAtlas(Undertale::getSingleton()->LookupFontAtlas(type.myfont));
	}
	_config = type;
	reset();
}


void obj_writer::update(float dt) {
	
	if (!_instant) {
		auto& children = _textLayer->getChildren();
		if (_typeingPosition < children.size()) {
			if (_frameDelay == 0) {
				Node* sprite = sprite = children.at(_typeingPosition++);
				sprite->setVisible(true);
				FaceState state;
				
				state.to_int = sprite->getTag();
				CCLOG("Displaying: '%c', F:%i, E:%i, D:%i", state.flag, state.face, state.emotion, state.delay);
				_frameDelay = state.delay;
				changeFace(state.face);
				changeEmotion(state.emotion);
			}
			else _frameDelay--;
		}
		
	}

	if (getChildrenCount() > 0) {
		for (auto& n : getChildren()) {
			// handle shake
		}
	}
	UObject::update(dt);
}