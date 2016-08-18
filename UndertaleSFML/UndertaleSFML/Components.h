#pragma once
#include "Global.h"

namespace Components {
	extern kult::component<'name', std::string> name;
	extern kult::component<'desc', std::string> description;
	extern kult::component<'matx', sf::Transformable> transform;
	extern kult::component<'body', Body> body;
	extern kult::component<'vel', sf::Vector2f> velocity;
	extern kult::component<'cfrm', SpriteFrameCollection> frames;
	extern kult::component<'sfrm', SpriteFrame> frame;
	extern kult::component<'anim', StopWatch<float>> animation;
};

namespace Systems {
	typedef std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> t_dumb_batch;
	extern kult::system<float> velocity_system;
	extern kult::system<float> animation_system;
	extern kult::system<t_dumb_batch&> rendering_system;
}



