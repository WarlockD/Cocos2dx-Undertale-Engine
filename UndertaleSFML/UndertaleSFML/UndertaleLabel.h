#pragma once
#include "UndertaleLoader.h"
#include "UndertaleLib.h"

class UndertaleLabelBuilder {
protected:
	std::shared_ptr<UFont> _font;
	std::vector<sf::Vertex> _textVerts;
	sf::Vector2f _writing;
	sf::Vector2f _offset; // starting offset
	sf::Vector2f _textBounds;
public:
	UndertaleLabelBuilder();
	virtual ~UndertaleLabelBuilder() {}
	virtual void setTextOffset(const sf::Vector2f& v);
	const sf::Vector2f getTextOffset() const { return _offset; }
	virtual void setText(const std::string& text);
	virtual void setFont(size_t index);
	virtual void clear();
	virtual void newline();
	virtual void push_back(int a, const sf::Color& color = sf::Color::White);
	virtual void pop_back();
	const sf::Vector2f& getTextSize() const { return _textBounds; }
};

class UndertaleLabel : public UndertaleLabelBuilder,  public sf::Drawable, public sf::Transformable {
public:
	sf::FloatRect getLocalBounds() const { return sf::FloatRect(sf::Vector2f(), getTextSize()); }
	sf::FloatRect getGlobalBounds() const { return  getTransform().transformRect(getLocalBounds()); }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

