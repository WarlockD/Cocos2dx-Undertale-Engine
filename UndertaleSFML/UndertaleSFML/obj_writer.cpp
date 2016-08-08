#include "obj_writer.h"
obj_writer::obj_writer() : UndertaleLabel(), _color(sf::Color::White), _isTyping(false) {
	setConfig(TEXTTYPE());
}

void obj_writer::do_typing() {

}
void obj_writer::setText(const std::string& text)  {
	_text.setText(text);
	clear();
}
#define TOKEN UndertaleLib::UndertaleText::Token
void obj_writer::setConfig(const TEXTTYPE& config) {
	_config = config;
	_font = UFont::LoadUndertaleFont(_config.myfont);
	_color = sf::Color(_config.mycolor, _config.mycolor >> 8, _config.mycolor >> 16);
	clear();
}
void obj_writer::update(float dt) {
	if (!_isTyping) return;
	if (_clock.getElapsedTime().asMilliseconds() > _nextLetterDelay) {
		_nextLetterDelay = 100;
		_clock.restart();
		while (_pos != _text.end()) {
			const auto& t = *_pos++;
			switch (t.token()) {
			case TOKEN::Color:
			{
				int icolor = t.value();
				_currentColor = sf::Color(icolor, icolor >> 8, icolor>>16);
			}
			break;
			case TOKEN::Face:
				setFace(t.value());
				break;
			case TOKEN::Emotion:
				setEmotion(t.value());
				break;
			case TOKEN::Delay:
				if (t.value() != 0) _nextLetterDelay *= t.value();
				break;
			case TOKEN::NewLine:
				_nextLetterPosition.y += _font->getFontSize() + 2;
				_nextLetterPosition.x = 0.0f;
				_bounds.height += _font->getFontSize() + 2;
				break;
			case TOKEN::Letter:
				// letter, type it and make a sound
			{
				char16_t ch = t.value();
				if (_nextLetterPosition.y > _config.writingxend) {
					_nextLetterPosition.x = 0.0;
					_nextLetterPosition.y -= _config.spacing;
				}
				else {
					if (_config.typer == 18) {
						if (ch == 'l' || ch == 'i') _nextLetterPosition.x += 2;
						if (ch == 'I') _nextLetterPosition.x += 2;
						if (ch == '!') _nextLetterPosition.x += 2;
						if (ch == '.') _nextLetterPosition.x += 2;
						if (ch == 'S') _nextLetterPosition.x++;
						if (ch == '?') _nextLetterPosition.x += 2;
						if (ch == 'D') _nextLetterPosition.x++;
						if (ch == 'A') _nextLetterPosition.x++;
						if (ch == '\'') _nextLetterPosition.x++;
					}
				}
				push_back(t.value(), _currentColor);

				int kern = 0;
				if (_font->getIndex() == 8) { // only two cases that have kernings?
					if (ch == 'w') kern += 2;
					if (ch == 'm') kern += 2;
					if (ch == 'i') kern -= 2;
					if (ch == 'l') kern -= 2;
					if (ch == 's') kern--;
					if (ch == 'j') kern--;
				}
				if (_font->getIndex() == 9) { // only two cases that have kernings?
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
				_nextLetterPosition.x += kern;
				return;
			}
			}
		}
		_isTyping = false;
	}
}