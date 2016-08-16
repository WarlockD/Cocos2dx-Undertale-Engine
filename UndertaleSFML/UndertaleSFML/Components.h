#pragma once
#include "Global.h"


struct transformWrapper : public sf::Transformable {}
;
kult::component<'name', std::string> name;
kult::component<'desc', std::string> description;
kult::component<'pos2', sf::Vector2f> position;
kult::component<'org2', sf::Vector2f> origin;
kult::component<'org2', sf::Vector2f> origin;
kult::component<'vel2', sf::Vector2f> velocity;

// systems
kult::system<float> movement = [&](float dt) {
	for (auto &entity : kult::join(position, velocity)) {
		entity[position].x += entity[velocity].x * dt;
		entity[position].y += entity[velocity].y * dt;
	}
};
