#include "Components.h"

namespace Components {
	kult::component<'name', std::string> name;
	kult::component<'desc', std::string> description;
	kult::component<'matx', sf::Transformable> transform;
	kult::component<'body', Body> body;
	kult::component<'vel', sf::Vector2f> velocity;
	kult::component<'cfrm', SpriteFrameCollection> frames;
	kult::component<'sfrm', SpriteFrame> frame;
	kult::component<'anim', StopWatch<float>> animation;
};
namespace Systems {
	kult::system<float> velocity_system([](float dt) {
		for (auto &entitys : kult::join(Components::velocity, Components::transform)) {
			auto& velocity = entitys[Components::velocity];
			auto& body = entitys[Components::transform];
			body.move(velocity*dt);
		}
	});

	// animation system
	kult::system<float> animation_system([](float dt) {
		for (auto &entitys : kult::join(Components::animation, Components::frames)) {
			auto& animation = entitys[Components::animation];
			auto& frames = entitys[Components::frames];
			if (animation.test_then_reset(dt)) {
				frames.setImageIndex(frames.getImageIndex() + 1);
			}
		}
	});
	typedef std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> t_dumb_batch;

	// rendering_system
	kult::system<t_dumb_batch&> rendering_system([](t_dumb_batch& batch) {
		// majority of sprites use this
		for (auto &entitys : kult::join(Components::frames, Components::transform)) {
			auto& frame = entitys[Components::frames];
			auto& verts = batch[frame.getTexture()];
			frame.insert(verts, entitys[Components::transform].getTransform());
		}
		for (auto &entitys : kult::join(ktext, Components::transform)) {
			auto& text = entitys[ktext];
			auto& verts = batch[&text.getTexture()];
			text.insert(verts, entitys[kbody].getTransform());
		}
	});
};
