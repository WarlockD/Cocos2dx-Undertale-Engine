#pragma once
#include "UndertaleLoader.h"
#include "UndertaleLib.h"


class obj_writer : public sf::Drawable, public sf::Transformable {
	class Letter : public sf::Transformable {
		std::vector<sf::Vertex>& _verts;
		size_t _index;
		const sf::Glyph& _glyph;
		bool _visible;
		sf::Color _color;
		void updateColor();
	public:
		void updateTransform(); // needs to be called before drawing
		Letter(std::vector<sf::Vertex>& verts, const sf::Glyph& glyph, const sf::Color& color = sf::Color::White); // pushes new letter
		void setColor(const sf::Color& color) { if (color != _color) { _color = color; updateColor(); } }
		void setVisible(bool visible) { if (visible != _visible) { _visible = visible; updateColor(); } }
		bool getVisible() const { return _visible; }
	};
	std::shared_ptr<UFont> _font;
	std::vector<sf::Vertex> _verts;
	sf::Vector2f _nextLetterPosition;
	sf::FloatRect _bounds;
	//UndertaleLib::UndertaleText _text;
	void push_back(int a, const sf::Color& color = sf::Color::White);
	void pop_back();
public:
	obj_writer();
	virtual void setText(const std::string& text);
	
	void setFont(size_t index);
	void clear();
	const sf::FloatRect& getBounds() const { return _bounds; }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};