#include "obj_writer.h"

USING_NS_CC;


obj_writer::obj_writer() : UObject(), _fontAtlas(nullptr) {}
obj_writer::~obj_writer(){}
obj_writer* obj_writer::create() {
	obj_writer* obj = new obj_writer;
	if (obj && obj->init(obj_writer::object_index)) {
		obj->autorelease();
		return obj;
	}
	CC_SAFE_DELETE(obj);
	return nullptr;
}


Sprite* obj_writer::getletter(char16_t ch) const {
	FontLetterDefinition letterDef;
	if (_fontAtlas->getLetterDefinitionForChar(ch, letterDef)) {
		auto textureID = letterDef.textureID;
		Rect uvRect;
		uvRect.size.height = letterDef.height;
		uvRect.size.width = letterDef.width;
		uvRect.origin.x = letterDef.U;
		uvRect.origin.y = letterDef.V;
		auto sprite = Sprite::createWithTexture(_fontAtlas->getTexture(textureID), uvRect);
		sprite->setAnchorPoint(Vec2(0, 1)); // top left
		sprite->setColor(_currentColor);
		return sprite;
	}
	return nullptr;
}
void obj_writer::newLine() {
	_writing.x = 0.0f;
	_writing.y += _config.spacing;
	_lineno++;
}
void obj_writer::preFixWriting(char16_t ch) {
	if (_config.typer == 18) {
		if (ch == 'l' || ch == 'i') _writing.x += 2;
		if (ch == 'I') _writing.x += 2;
		if (ch == '!') _writing.x += 2;
		if (ch == '.') _writing.x += 2;
		if (ch == 'S') _writing.x++;
		if (ch == '?') _writing.x += 2;
		if (ch == 'D') _writing.x++;
		if (ch == 'A') _writing.x++;
		if (ch == '\'') _writing.x++;
	}
}

void obj_writer::postFixWriting(char16_t ch) {
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
	_writing.x += kern;
}

void obj_writer::setString(const std::string& text) {
	_text.setText(text);
	if (_fontAtlas == nullptr) setType(_config);
	
	reset();
}
void obj_writer::start() {
	scheduleUpdate();
	_frameDelay = 0;
}
void obj_writer::stop() {
	unscheduleUpdate();
	_frameDelay = -1;
}
void obj_writer::reset() {
	removeAllChildrenWithCleanup(true);
	setContentSize(Size(_config.writingxend, _text.lineno() * _config.vspacing));
	_lineno = 1;
	_currentColor = Color3B(_config.mycolor >> 24, _config.mycolor >> 16, _config.mycolor);
	_frameDelay = -1;
	_current = _text.begin();
	_writing = Vec2::ZERO;
}
void obj_writer::setType(const TEXTTYPE& type) {
	
	//setPosition(type.writingx, _room->getContentSize().height- type.writingy);
	if (_fontAtlas == nullptr || _config.myfont != type.myfont) {
		_fontAtlas = Undertale::getSingleton()->LookupFontAtlas(type.myfont);
	}
	_config = type;
	reset();
}
#define TOKEN UndertaleLib::UndertaleText::Token



void obj_writer::update(float dt) {
	if (_frameDelay == -1) return;
	
	if (_frameDelay == 0) {
		_frameDelay = 1;
		if (_current != _text.end()) {
			bool finished = false;
			do {
				switch (_current->token()) {
				case TOKEN::Color:
				{
					int color = _current->value();
					_currentColor = Color3B(color >> 24, color >> 16, color);
				}
				break;
				case TOKEN::Halt:
				{
					EventCustom event("obj_writer_halt");
					event.setUserData((void*)_current->value());
					_eventDispatcher->dispatchEvent(&event);
					stop();
					finished = true;
				}
				break;
				case TOKEN::Delay:
					if (_current->value() != 0) _frameDelay = _current->value();
					break;
				case TOKEN::NewLine:
					newLine();
					break;
				case TOKEN::Letter:
					// letter, type it and make a sound
				{
					auto sprite = getletter(_current->value());
					if (_writing.y > _config.writingxend) newLine();
					char16_t ch = _current->value();
					preFixWriting(ch);
					sprite->setPosition(_writing);
					postFixWriting(ch);
					_writing.x += _config.spacing;
					addChild(sprite);
					finished = true;
					break;
				}

				}
				_current++;
			} while (!finished || _current != _text.end());

		}
	}
	else _frameDelay--;;

	if (getChildrenCount() > 0) {
		for (auto& n : getChildren()) {
			// handle shake
		}
	}
}