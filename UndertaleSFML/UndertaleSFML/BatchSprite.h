#pragma once

#include <SFML\Graphics.hpp>

class BatchSprite : public sf::Drawable {
	mutable bool _needUpdate; // changed in draw
	sf::Vector2f _position;
	sf::Vector2f _scale;
	sf::Vector2f _origin;
	sf::IntRect _texRect;
	float _rotation;
	sf::Color _color;
	bool _visiable;
	sf::Vertex* _verts;
	sf::Vector2f _offset;                     ///< Sprite Offset from texture
public:
	BatchSprite() :_verts(nullptr), _offset(0.0f, 0.0f), _needUpdate(false), _position(0.0f, 0.0f), _scale(1.0f, 1.0f), _rotation(0.0f), _color(sf::Color::White), _visiable(true) {}
	void setVerts(sf::Vertex& verts) { _verts = &verts; }

	void setColor(const sf::Color color);
	const sf::Color& getColor() const { return _color; }

	void setVisible(bool visible);
	bool getVisible() const { return _visiable; }
	void setTextureRect(const sf::IntRect& v);
	void setTextureRect(float x, float y, int width, int height) { setTextureRect(sf::IntRect(x, y, width, height)); }
	const sf::IntRect& getTextureRect() const { return _texRect; }

	void setPosition(const sf::Vector2f& v);
	void setPosition(float x, float y) { setPosition(sf::Vector2f(x, y)); }
	const sf::Vector2f& getPosition() const { return _position; }

	void setOffset(const sf::Vector2f& v);
	void setOffset(float x, float y) { setOffset(sf::Vector2f(x, y)); }
	const sf::Vector2f& getOffset() const { return _offset; }

	void setOrigin(const sf::Vector2f& v);
	void setOrigin(float x, float y) { setOrigin(sf::Vector2f(x, y)); }
	const sf::Vector2f& getOrigin() const { return _origin; }

	void setScale(const sf::Vector2f& v);
	void setScale(float x, float y) { setScale(sf::Vector2f(x, y)); }
	void setScale(float s) { setScale(sf::Vector2f(s, s)); }
	const sf::Vector2f& getScale() const { return _scale; }

	void setRotation(float rot);
	float getRotation() const { return _rotation; }

	// yes, we need to draw, but what this does is update the vertexs
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};