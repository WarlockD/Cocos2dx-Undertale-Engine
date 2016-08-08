#pragma once
#include "UndertaleLoader.h"
#include "UndertaleLib.h"


class UndertaleLabel : public sf::Drawable, public sf::Transformable {
protected:
	std::shared_ptr<UFont> _font;
	std::vector<sf::Vertex> _verts;
	sf::Vector2f _nextLetterPosition;
	sf::FloatRect _bounds;
	
public:
	UndertaleLabel();
	virtual void setText(const std::string& text);
	
	void setFont(size_t index);
	virtual void clear();
	virtual void push_back(int a, const sf::Color& color = sf::Color::White);
	virtual void pop_back();
	sf::FloatRect getLocalBounds() const { return _bounds; }
	sf::FloatRect getGlobalBounds() const { return  getTransform().transformRect(getLocalBounds()); }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};