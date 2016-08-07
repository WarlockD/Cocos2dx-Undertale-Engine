#include "BatchSprite.h"

using namespace sf;


void BatchSprite::setPosition(const sf::Vector2f& v) {
	_position = v;
	_needUpdate = true;
}

void BatchSprite::setOffset(const sf::Vector2f& v) {
	_offset = v;
	_needUpdate = true;
}

void BatchSprite::setOrigin(const sf::Vector2f& v) {
	_origin = v;
	_needUpdate = true;
}

void BatchSprite::setScale(const sf::Vector2f& v) {
	_scale = v;
	_needUpdate = true;
}
void BatchSprite::setRotation(float rot) {
	_rotation = static_cast<float>(std::fmod(rot, 360));
	if (_rotation < 0)
		_rotation += 360.f;
	_needUpdate = true;
}
void BatchSprite::setTextureRect(const sf::IntRect& v) {
	float u1 = static_cast<float>(_texRect.left);
	float v1 = static_cast<float>(_texRect.top);
	float u2 = static_cast<float>(_texRect.left + _texRect.width);
	float v2 = static_cast<float>(_texRect.top + _texRect.height);
	sf::Vertex* verts = _verts;
	verts++->texCoords = Vector2f(u1, v1);
	verts++->texCoords = Vector2f(u2, v1);
	verts++->texCoords = Vector2f(u1, v2);
	verts++->texCoords = Vector2f(u1, v2);
	verts++->texCoords = Vector2f(u2, v1);
	verts++->texCoords = Vector2f(u2, v2);
}
// yes, we need to draw, but what this does is update the vertexs
void BatchSprite::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (_needUpdate) {
		_needUpdate = false;
		float angle = -_rotation * 3.141592654f / 180.f;
		float cosine = static_cast<float>(std::cos(angle));
		float sine = static_cast<float>(std::sin(angle));
		float sxc = _scale.x * cosine;
		float syc = _scale.y * cosine;
		float sxs = _scale.x * sine;
		float sys = _scale.y * sine;
		float tx = -_origin.x * sxc - _origin.y * sys + _position.x;
		float ty = _origin.x * sxs - _origin.y * syc + _position.y;

		Transform transform(sxc, sys, tx,
			-sxs, syc, ty,
			0.f, 0.f, 1.f);
		const float* matrix = transform.getMatrix();
		float x1 = _offset.x;
		float y1 = _offset.y;
		float x2 = x1 + static_cast<float>(_texRect.width);
		float y2 = y2 + static_cast<float>(_texRect.height);

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
		sf::Vertex* verts = _verts;
		verts++->position = Vector2f(dx, dy);
		verts++->position = Vector2f(cx, cy);
		verts++->position = Vector2f(ax, ay);
		verts++->position = Vector2f(ax, ay);
		verts++->position = Vector2f(cx, cy);
		verts++->position = Vector2f(bx, by);

	}
}
void BatchSprite::setVisible(bool visible) {
	if (visible != _visiable) {
		_visiable = visible;
		Color color = _visiable ? _color : Color::Transparent;
		sf::Vertex* verts = _verts;
		verts++->color = color;
		verts++->color = color;
		verts++->color = color;
		verts++->color = color;
		verts++->color = color;
		verts++->color = color;
	}
}
void BatchSprite::setColor(const sf::Color color) {
	_color = color;
	if (_visiable) {
		sf::Vertex* verts = _verts;
		verts++->color = _color;
		verts++->color = _color;
		verts++->color = _color;
		verts++->color = _color;
		verts++->color = _color;
		verts++->color = _color;
	}
}
