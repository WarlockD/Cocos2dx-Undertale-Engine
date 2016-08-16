#include "UndertaleLabel.h"

using namespace sf;

UndertaleLabelBuilder::UndertaleLabelBuilder() : _writing(0.0f, 0.0f), _textBounds(0.0f, 0.0f), _offset(0.0f, 0.0f) {
	_textVerts.reserve(6 * 40);
}
void UndertaleLabelBuilder::setText(const std::string& text) {
	clear();
	for (char c : text) push_back(c);
}
void UndertaleLabelBuilder::setFont(size_t index) {
	_font = UFont::LoadUndertaleFont(index);
	clear();
}
void UndertaleLabelBuilder::setTextOffset(const sf::Vector2f& v) {
	if (_textVerts.size() >0) for (auto& vert : _textVerts) vert.position = (vert.position - _offset) + v;
	_offset = v;
}
UndertaleLabelBuilder& UndertaleLabelBuilder::operator+=(const std::string& text) {
	if (!_font) setFont(2);
	for (char c : text) push_back(c);
	return *this;
}
UndertaleLabelBuilder& UndertaleLabelBuilder::operator=(const std::string& text) {
	if (!_font) setFont(2);
	setText(text);
	return *this;
}
void UndertaleLabelBuilder::clear() {
	_textVerts.clear();
	if (!_font) setFont(2);
	_textBounds.y = _font ? _font->getFontSize() : 0.0f;
	_textBounds.x = 0.0f;
	_writing = _offset;
}
void UndertaleLabelBuilder::newline() {
	_writing.y += _font->getFontSize() + 2;
	_textBounds.y += _font->getFontSize() + 2;
	_writing.x = _offset.x;
}
void UndertaleLabelBuilder::push_back(int a, const sf::Color& color) {
	if (a == '\n') {
		newline();
	}
	else if (a == '\b') {
		pop_back();
	} else {
		auto& glyph = _font->getGlyph(a);
		float x = _writing.x +glyph.bounds.width / 2.0f; //+_linesOffsetX[letterInfo.lineIndex];
		float y = _writing.y + _font->getFontSize() / 2.0f;// +glyph.bounds.height / 2;// +glyph.bounds.top;

		float left = x + glyph.bounds.left;
		float top = y + glyph.bounds.top ;
		float right = x + glyph.bounds.left + glyph.bounds.width;
		float bottom = y + glyph.bounds.top + glyph.bounds.height;

		float u1 = static_cast<float>(glyph.textureRect.left);
		float v1 = static_cast<float>(glyph.textureRect.top);
		float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width);
		float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height);

		// Add a quad for the current character
		_textVerts.push_back(Vertex(Vector2f(left, top), color, Vector2f(u1, v1)));
		_textVerts.push_back(Vertex(Vector2f(right, top), color, Vector2f(u2, v1)));
		_textVerts.push_back(Vertex(Vector2f(left, bottom), color, Vector2f(u1, v2)));
		_textVerts.push_back(Vertex(Vector2f(left, bottom), color, Vector2f(u1, v2)));
		_textVerts.push_back(Vertex(Vector2f(right, top), color, Vector2f(u2, v1)));
		_textVerts.push_back(Vertex(Vector2f(right, bottom), color, Vector2f(u2, v2)));
		_writing.x += glyph.advance;

		// Update the current bounds
		_textBounds.x += std::max(_textBounds.x, _writing.x);
	}
}
void UndertaleLabelBuilder::pop_back() {
	if(_textVerts.size() > 0) _textVerts.resize(_textVerts.size() - 6);
}

void UndertaleLabel::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_font && _textVerts.size() > 0) {
		states.transform *= getTransform();
		states.texture = &_font->getTexture();
		target.draw(_textVerts.data(), _textVerts.size(), sf::PrimitiveType::Triangles, states);
	}
}