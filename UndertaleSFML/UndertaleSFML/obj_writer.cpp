#include "obj_writer.h"

using namespace sf;

obj_writer::obj_writer() : _nextLetterPosition(0.0f,0.0f) , _bounds(), _verts(sf::PrimitiveType::Triangles) {

}
void obj_writer::setText(const std::string& text) { 
	clear();
	for (char c : text) push_back(c); 
}
void obj_writer::setFont(size_t index) { 
	_font = UFont::LoadUndertaleFont(index); 
	clear(); 
}
void obj_writer::clear() {
	_verts.clear();
	_bounds.height = _font ? _font->getFontSize() : 0.0f;
	_bounds.width = 0.0f;
}
void obj_writer::push_back(int a, const sf::Color& color) {

	if (a == '\n') {
		_nextLetterPosition.y += _font->getFontSize()+2;
		_bounds.height += _font->getFontSize()+2;
	}
	else if(a == '\r')
		_nextLetterPosition.x = 0.0f;
	else {
		auto& glyph = _font->getGlyph(a);
		float x = _nextLetterPosition.x;// +glyph.bounds.width / 2; //+_linesOffsetX[letterInfo.lineIndex];
		float y = _nextLetterPosition.y;// +glyph.bounds.height / 2.0f;// +glyph.bounds.height / 2;// +glyph.bounds.top;

		float left = x + glyph.bounds.left;
		float top = y + glyph.bounds.top;
		float right = x + glyph.bounds.left + glyph.bounds.width;
		float bottom = y + glyph.bounds.top + glyph.bounds.height;

		float u1 = static_cast<float>(glyph.textureRect.left);
		float v1 = static_cast<float>(glyph.textureRect.top);
		float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width);
		float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height);

		// Add a quad for the current character
		_verts.push_back(Vertex(Vector2f(left, top), color, Vector2f(u1, v1)));
		_verts.push_back(Vertex(Vector2f(right, top), color, Vector2f(u2, v1)));
		_verts.push_back(Vertex(Vector2f(left, bottom), color, Vector2f(u1, v2)));
		_verts.push_back(Vertex(Vector2f(left, bottom), color, Vector2f(u1, v2)));
		_verts.push_back(Vertex(Vector2f(right, top), color, Vector2f(u2, v1)));
		_verts.push_back(Vertex(Vector2f(right, bottom), color, Vector2f(u2, v2)));
		_nextLetterPosition.x += glyph.advance;

		// Update the current bounds
		_bounds.width += std::max(_bounds.width, _nextLetterPosition.x);
		_bounds.height = y + _font->getFontSize();
	}
	
	
}
void obj_writer::pop_back() {
	_verts.resize(_verts.size() - 6);
}

void obj_writer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_font && _verts.size() > 0) {
		states.transform *= getTransform();
		states.texture = &_font->getTexture();
		target.draw(_verts.data(), _verts.size(), sf::PrimitiveType::Triangles, states);
	}
}