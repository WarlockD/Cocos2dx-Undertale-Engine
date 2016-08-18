#pragma once
#include "Global.h"



namespace Components {
	// simple class to pass the raw verts to the rendering engine
	class RenderableRef {
		const Renderable* _ref;
	public:
		RenderableRef() : _ref(nullptr) {}
		RenderableRef(RenderableRef&& move) = default;
		RenderableRef& operator=(RenderableRef&& move) = default;
		RenderableRef(const RenderableRef& copy) : _ref(nullptr) { assert(false); }
		RenderableRef& operator=(const RenderableRef& copy)  { _ref = nullptr;  assert(false); return *this; }

		RenderableRef(const Renderable* ref) : _ref(ref) {}
		RenderableRef& operator=(const Renderable& ref) { _ref = &ref; return *this; }

		const Renderable*  operator->() const { return _ref; }
	};
	extern kult::component<'name', std::string> name;
	extern kult::component<'rect', sf::FloatRect> bounds;
	extern kult::component<'desc', std::string> description;
	extern kult::component<'matx', sf::Transformable> transform;
	extern kult::component<'body', Body> body;
	extern kult::component<'vel', sf::Vector2f> velocity;
	extern kult::component<'cfrm', SpriteFrameCollection> frames;
	extern kult::component<'sfrm', SpriteFrame> frame;
	extern kult::component<'anim', StopWatch<float>> animation;
	extern kult::component<'rend', RenderableRef> render;
};

namespace Systems {
	constexpr size_t CollisionEvent = 1000;
	typedef std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> t_dumb_batch;
	extern kult::system<float> velocity_system;
	extern kult::system<float> animation_system;
	extern kult::system<float> collision_system;
	extern kult::system<t_dumb_batch&> rendering_system;
}

namespace Engine {
	class USprite : public kult::entity {
		size_t _sprite_index;
		sf::Transformable* _body;
		SpriteFrameCollection* _frames;
	public:
		USprite();
		void setSprite(size_t sprite_index);
		sf::Transformable& body() { return *_body; }
		const sf::Transformable& body() const { return *_body; }
		SpriteFrameCollection& frames() { return *_frames; }
		const SpriteFrameCollection& frames() const { return *_frames; }
		size_t getSprite() const { return _sprite_index; }
	};
};

