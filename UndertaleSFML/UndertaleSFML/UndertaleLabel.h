#pragma once
#include "Global.h"
#include "UndertaleLoader.h"
#include "UndertaleLib.h"

class UndertaleLabel 
{
protected:
	std::shared_ptr<UFont> _font;
	std::vector<sf::Vertex> _textVerts;
	sf::Vector2f _writing;
	sf::Vector2f _offset; // starting offset
	sf::Vector2f _textBounds;
public:
	UndertaleLabel();
	UndertaleLabel& operator=(const std::string& text);
	UndertaleLabel& operator+=(const std::string& text);
	virtual ~UndertaleLabel() {}
	virtual void setTextOffset(const sf::Vector2f& v);
	const sf::Vector2f getTextOffset() const { return _offset; }
	virtual void setText(const std::string& text);
	virtual void setFont(size_t index);
	virtual void clear();
	virtual void newline();
	virtual void push_back(int a, const sf::Color& color = sf::Color::White);
	virtual void pop_back();

	const sf::Vector2f& getTextSize() const { return _textBounds; }
	const sf::Vertex* data() const { return _textVerts.data(); }
	size_t size() const { return _textVerts.size(); }
	const sf::Texture* texture() const { return &_font->getTexture(); }
};


