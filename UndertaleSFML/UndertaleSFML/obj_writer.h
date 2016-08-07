#pragma once
#include "UndertaleLoader.h"
#include "UndertaleLib.h"


class obj_writer : public sf::Drawable, public sf::Transformable {
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