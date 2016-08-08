#pragma once
#include "obj_writer.h"

class obj_dialoger : public sf::Drawable, public sf::Transformable {
	obj_writer _writer;
	sf::RectangleShape _blackInside;
	sf::RectangleShape _whiteOutside;

public:
	obj_dialoger() {
		_blackInside.setFillColor(sf::Color::Black);
		_whiteOutside.setFillColor(sf::Color::White);
		_whiteOutside.setSize(sf::Vector2f(304 - 16, 80 - 5));
		_blackInside.setSize(_whiteOutside.getSize() - sf::Vector2f(6,6));
		_blackInside.setPosition(3, 3);
		_writer.setPosition(20, 20);
	}
	void setText(const std::string& text) { _writer.setText(text); _writer.setPosition(20, 20); }
	void setConfig(const TEXTTYPE& type= TEXTTYPE()) { _writer.setConfig(type); }
	void update(float dt) { _writer.update(dt); }
	void start_typing() { _writer.start_typing(); }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= getTransform();
		target.draw(_whiteOutside);
		target.draw(_blackInside);
		target.draw(_writer);
	}
};