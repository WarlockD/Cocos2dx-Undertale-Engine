#pragma once
#include "Global.h"
#include "Drawables.h"

struct Bounds {
	explicit Bounds(sf::FloatRect bounds) :bounds(bounds) {}
	sf::FloatRect bounds;
};

struct Velocity {
	explicit Velocity(sf::Vector2f velocity) :velocity(velocity) {}
	sf::Vector2f velocity;
};

struct Layer {
	explicit Layer(int layer) :layer(layer) {}
	int layer;
};


struct Animation {
	explicit Animation(float fps, bool reverse) :watch(fps), reverse(reverse) {}
	explicit Animation(float fps) :watch(fps), reverse(false) {}
	StopWatch<float> watch;
	bool reverse = false;
};

class RenderSystem : public ex::System<RenderSystem> {
	typedef std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> t_dumb_batch;
	typedef std::vector<sf::Vertex> vert_vector;
	std::map<int, t_dumb_batch> sortedVerts;
	float last_update = 0.0;
	float frame_count = 0.0;
	sf::RenderTarget &target;
	sf::Font _font; 
	sf::Text text;
public:
	explicit RenderSystem(sf::RenderTarget &target);
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override;
};

class AnimationSystem : public ex::System<AnimationSystem> {
	sf::RenderTarget &target;
public:
	explicit AnimationSystem(sf::RenderTarget &target) :target(target) {}
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override;
};

class VelocitySystem : public ex::System<VelocitySystem> {
	sf::RenderTarget &target;
public:
	explicit VelocitySystem(sf::RenderTarget &target) :target(target) {}
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override;
};



class Application : public ex::EntityX {
public:
	explicit Application(sf::RenderTarget &target);
	void update(ex::TimeDelta dt) { systems.update_all(dt); }
};

