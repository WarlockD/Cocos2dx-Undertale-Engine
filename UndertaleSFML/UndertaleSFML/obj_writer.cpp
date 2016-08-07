#include "obj_writer.h"

using namespace sf;

obj_writer::Letter::Letter(std::vector<sf::Vertex>& verts, const sf::Glyph& glyph, const sf::Color& color) : _verts(verts), _index(verts.size()), _glyph(glyph), _color(color), _visible(true) {
	// Extract the current glyph's description
	float left = glyph.bounds.left;
	float top = glyph.bounds.top;
	float right = glyph.bounds.left + glyph.bounds.width;
	float bottom = glyph.bounds.top + glyph.bounds.height;

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
}
void obj_writer::Letter::updateColor() {
	float displayedOpacity = _visible ? 1.0f : 0.0; //  _displayedOpacity;
	Color color = _visible ? _color : Color::Transparent;
	_verts[_index + 0].color = color;
	_verts[_index + 1].color = color;
	_verts[_index + 2].color = color;
	_verts[_index + 3].color = color;
	_verts[_index + 4].color = color;
	_verts[_index + 5].color = color;
}
void obj_writer::Letter::updateTransform() {
	auto matrix = getTransform().getMatrix();

	float x1 = _glyph.bounds.left;
	float y1 = _glyph.bounds.top;
	float x2 = _glyph.bounds.left + _glyph.bounds.width;
	float y2 = _glyph.bounds.top + _glyph.bounds.height;

	float x = matrix[12];
	float y = matrix[13];

	float cr = matrix[0];
	float sr = matrix[1];
	float cr2 = matrix[5];
	float sr2 = -matrix[4];
	float ax = x1 * cr - y1 * sr2 + x;
	float ay = x1 * sr + y1 * cr2 + y;

	float bx = x2 * cr - y1 * sr2 + x;
	float by = x2 * sr + y1 * cr2 + y;
	float cx = x2 * cr - y2 * sr2 + x;
	float cy = x2 * sr + y2 * cr2 + y;
	float dx = x1 * cr - y2 * sr2 + x;
	float dy = x1 * sr + y2 * cr2 + y;

	// Add a quad for the current character
	_verts[_index + 0].position = Vector2f(dx, dy);
	_verts[_index + 1].position = Vector2f(cx, cy);
	_verts[_index + 2].position = Vector2f(ax, ay);
	_verts[_index + 3].position = Vector2f(ax, ay);
	_verts[_index + 4].position = Vector2f(cx, cy);
	_verts[_index + 5].position = Vector2f(bx, by);
}


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
	auto& glyph = _font->getGlyph(a);
	if (a == '\n') {
		_nextLetterPosition.y += _font->getFontSize();
		_bounds.height += _font->getFontSize();
	}
	else if(a == '\r')
		_nextLetterPosition.x = 0.0f;
	else {
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