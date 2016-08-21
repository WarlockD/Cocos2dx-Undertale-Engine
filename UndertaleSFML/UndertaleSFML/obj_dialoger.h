#pragma once
#include "obj_writer.h"

class sprite_component : public sf::Sprite {

};
class obj_dialoger : public  obj_writer {
	static sf::View _dialog_vew;
	std::vector<sf::Sprite> _face;
	sf::RectangleShape _blackInside;
	sf::RectangleShape _whiteOutside;
	bool _faceChange;
	void fixFace() {
		if (_face.size() > 0)
			setTextOffset(sf::Vector2f(68.0f, -5.0f));
		else
			setTextOffset(sf::Vector2f(10.0, -5.0f));
	}
	
public:
	obj_dialoger()  {
		_blackInside.setFillColor(sf::Color::Black);
		_whiteOutside.setFillColor(sf::Color::White);
		_whiteOutside.setSize(sf::Vector2f(304 - 16, 80 - 5));
		_blackInside.setSize(_whiteOutside.getSize() - sf::Vector2f(6, 6));
		_blackInside.setPosition(3, 3);
		
		fixFace();
	}
	void setConfig(const TEXTTYPE& type = TEXTTYPE()) override { 
		obj_writer::setConfig(type);
		fixFace();
	}
	void update(sf::Time dt) {
		obj_writer::update(dt);
	}// { _writer.update(dt); }
//	void start_typing() { _writer.start_typing(); }
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override  {
		states.transform *= getTransform();
		target.draw(_whiteOutside, states);
		target.draw(_blackInside, states);
		obj_writer::draw(target, states);
	}
};