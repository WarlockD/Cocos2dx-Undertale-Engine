#pragma once
#include "Global.h"
#include "Drawables.h"
#include "UndertaleLoader.h"

struct Bounds {
	explicit Bounds(sf::FloatRect bounds) :bounds(bounds) {}
	sf::FloatRect bounds;
};

struct Velocity {
	explicit Velocity(sf::Vector2f velocity) :velocity(velocity) {}
	explicit Velocity() :velocity(0.0f,0.0f) {}
	sf::Vector2f velocity;
};

struct Layer {
	explicit Layer(int layer) :layer(layer) {}
	int layer;
};

class RenderableCache :public ChangedCass {
	const Renderable& _renderable;
	const ChangedCass* _renderable_can_change;
	mutable RawVertices _cache;
public:
	explicit RenderableCache(const Renderable& ref) : _renderable(ref), _renderable_can_change(dynamic_cast<const ChangedCass*>(&ref)) { _cache.assign(_renderable.begin(), _renderable.end()); }
	void update_cache() const {
		if ((_renderable_can_change && _renderable_can_change->changed())) {
			_cache.assign(_renderable.begin(), _renderable.end());
		}
	}
	void update_cache(const Body& body) const {
		if (body.changed() || (_renderable_can_change && _renderable_can_change->changed())) {
			_cache.assign(_renderable.begin(), _renderable.end());
			_cache.transform(body.getTransform());
			body.changed(false);
		}
	}
	// helper functions
	const sf::Texture* texture() const { return _renderable.texture(); }
	const RawVertices& cache() {
		return _cache;
	}
	const Renderable& renderable() const { return _renderable; }
};

struct SystemEvent : public ex::Event<SystemEvent> {
	sf::Event event;
	explicit SystemEvent() : event() {}
	explicit SystemEvent(const sf::Event& event) : event(event) {}
};
class Player : public SpriteFrameBase, public ex::Receiver<Player> {
	enum class  PlayerFacing : char {
		DOWN = 0, RIGHT = 1, UP = 2, LEFT = 3
	};
	std::array<UndertaleSprite, 4> _sprites;

	PlayerFacing _facing;
	int health;
	int status;
	bool in_overworld;
	int _direction;
	ex::Entity _enity;
public:
	virtual const sf::Vertex*  ptr() const { return _sprites[(char)_facing].ptr(); }
	const sf::Texture* texture() const override final { return _sprites[(char)_facing].texture(); }
	Player() {} // does NOTHING  use load
	virtual ~Player();
	virtual bool load_resources(ex::EntityX& app);
	virtual void receive(const SystemEvent &event);
};


class Animation {
	StopWatch<float> _watch;
	bool _reverse;
public:
	explicit Animation(float fps, bool reverse) :  _watch(std::fabs(fps)), _reverse(reverse) {}
	explicit Animation(float fps) :  _watch(std::fabs(fps)), _reverse(fps > 0.0f ? false : true) {}
	bool update(Renderable& renderable, float dt);
};

class PlayerOverWorldSystem : public ex::System<PlayerOverWorldSystem> {
	sf::RenderTarget &target;
	ex::EntityX& app;
	Player _player;
public:
	explicit PlayerOverWorldSystem(ex::EntityX& app, sf::RenderTarget &target);
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override;
	void init(ex::EntityX& app) { _player.load_resources(app); }
};

class RenderSystem : public ex::System<RenderSystem> {
	typedef std::unordered_map<const sf::Texture*, RawVertices> t_dumb_batch;
	typedef std::vector<sf::FloatRect> t_debug_boxes;
	std::map<int, t_dumb_batch> sortedVerts;
	sf::VertexArray debug_lines;
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
	void init(ex::EntityX& app); ;
	void update(ex::TimeDelta dt) { systems.update_all(dt); }
};

