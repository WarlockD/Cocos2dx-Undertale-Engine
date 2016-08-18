#include "Components.h"
#include "UndertaleLoader.h"

namespace Components {
	kult::component<'name', std::string> name;
	kult::component<'desc', std::string> description;
	kult::component<'matx', sf::Transformable> transform;
	kult::component<'body', Body> body;
	kult::component<'vel', sf::Vector2f> velocity;
	kult::component<'cfrm', SpriteFrameCollection> frames;
	kult::component<'sfrm', SpriteFrame> frame;
	kult::component<'anim', StopWatch<float>> animation;
	kult::component<'rend', RenderableRef> render;
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

	// animation system
	kult::system<float> collision_system([](float dt) {
		std::unordered_map<kult::entity*, sf::FloatRect> rects;
		for (auto &entitys : kult::join(Components::bounds, Components::transform)) {
			const auto& bounds = entitys[Components::bounds];
			auto& transform = entitys[Components::transform].getTransform();
			rects[&entitys] = transform.transformRect(bounds);
		}
		// now we serch the map
		for (auto a : rects) {
			for (auto b : rects) {
				if (a != b && a.second.intersects(b.second.intersects)) {
					a.first->emit(CollisionEvent);
				}
			}
		}
	});


	
	typedef std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> t_dumb_batch;

	// rendering_system
	kult::system<t_dumb_batch&> rendering_system([](t_dumb_batch& batch) {
		// majority of sprites use this
		for (auto &entitys : kult::join(Components::render, Components::transform)) {
			auto& frame = entitys[Components::render];
			auto& verts = batch[frame->texture()];
			frame->insert(verts, entitys[Components::transform].getTransform());
		}
	});



};

namespace Engine {

	constexpr size_t SpriteEnity = 10000;
	/*

	class USprite : kult::entity {
		size_t _sprite_index;
		sf::Transformable* _body;
		SpriteFrameCollection* _frames;
	public:
	*/
	USprite::USprite() : kult::entity() , _sprite_index(0), _frames(nullptr) {
		_body = &(*this)[Components::transform];
	}
	void USprite::setSprite(size_t sprite_index) {
		if (sprite_index != _sprite_index) {
			if (!kult::has<SpriteFrameCollection>(id)) {
				_frames = &(*this)[Components::frames];
				(*this)[Components::render] = _frames;
			}
			*_frames = Global::LoadSprite(sprite_index);
		}
	}
};