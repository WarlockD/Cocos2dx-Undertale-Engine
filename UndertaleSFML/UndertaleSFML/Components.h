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
	explicit Velocity(float x, float y) :velocity(x, y) {}
	explicit Velocity() :velocity(0.0f,0.0f) {}
	sf::Vector2f velocity;
};
/*

struct UndertaleObject {

};
*/
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
//	explicit RenderableCache(const Renderable* ref) : _renderable(&ref), _renderable_can_change(dynamic_cast<const ChangedCass*>(&ref)) { _cache.assign(_renderable.begin(), _renderable.end()); }

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
class Animation {
	StopWatch<float> _watch;
	bool _reverse;
public:
	explicit Animation(float fps, bool reverse) : _watch(std::fabs(fps)), _reverse(reverse) {}
	explicit Animation(float fps) : _watch(std::fabs(fps)), _reverse(fps > 0.0f ? false : true) {}
	bool update(Renderable& renderable, float dt);
};

template<class T, typename EQUAL = std::equal_to<T>>
class ValueMonitor {
	T _value;
	mutable   bool _changed;
public:
	explicit ValueMonitor(T value) : _value(value), _changed(true) {}
	ValueMonitor& operator=(const T& value) { if (!EQUAL(_value, value)) { _changed = true; _value = value; } return *this; }
	operator T&() { return _value; }
	operator const T&() const { return _value; }
	bool changed() const { return _changed; }
	void reset() const { _changed = false; }
};

using kBounds = kult::component<'bond', sf::FloatRect>;
using kPosition = kult::component<'pos', ValueMonitor<sf::Vector2f>>;
using kVelocity = kult::component<'velc', sf::Vector2f>;
using kDepth = kult::component<'dept', ValueMonitor<int>>;
using kBody = kult::component<'body', Body>;
using kSprite = kult::component<'sprt', UndertaleSprite>;
enum class  Direction : char {
	DOWN = 0, RIGHT = 1, UP = 2, LEFT = 3
};
class EnityMovement : public ex::Receiver<EnityMovement> {

};
class Player : public SpriteFrameBase {
	enum class  PlayerFacing : char {
		DOWN = 0, RIGHT = 1, UP = 2, LEFT = 3
	};
	std::array<UndertaleSprite, 4> _sprites;
	float _frameTime;
	PlayerFacing _facing;
	int health;
	int status;
	sf::Vector2f _moving_speed;
	bool _directionDown[4];
	bool in_overworld;
	int _direction;
	ex::Entity _enity;
	bool _ismoving;
	friend class PlayerOverWorldSystem;
public:
	bool isMoving() const {
		return _directionDown[0] || _directionDown[1] || _directionDown[2] || _directionDown[3];
	}
	bool isMoving(PlayerFacing direction) const { return _directionDown[(char)direction]; }
	virtual bool next_frame() { 
		bool moving = isMoving(); if (moving) _sprites[(int)_facing].next_frame(); return moving; }; // This interface just tells the sprite to do next frame
	virtual bool prev_frame() { bool moving = isMoving(); if (moving) _sprites[(int)_facing].prev_frame(); return moving; }; // This interface just tells the sprite to do prev frame
	virtual const sf::Vertex*  ptr() const { return _sprites[(char)_facing].ptr(); }
	const sf::Texture* texture() const override final { return _sprites[(char)_facing].texture(); }
	Player() {} // does NOTHING  use load
	virtual ~Player();
	virtual bool load_resources(ex::EntityX& app);
	virtual void receive(const sf::Event &event);
};




class PlayerOverWorldSystem : public ex::System<PlayerOverWorldSystem> {
	sf::RenderTarget &target;
	ex::EntityX& app;
	Player _player;
public:
	Player& getPlayer()  { return _player; }
	explicit PlayerOverWorldSystem(ex::EntityX& app, sf::RenderTarget &target);
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override;
	void init(ex::EntityX& app) { _player.load_resources(app); }
};
struct UndertaleObject {
	UndertaleLib::Object object;
	explicit UndertaleObject(UndertaleLib::Object o) : object(o) {}
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
	std::vector<ex::Entity> _roomObjects;
	sf::Transform _transform;
public:
	UndertaleRoom::type _room;

	const std::map<int, t_dumb_batch>& get_verts() const { return sortedVerts; }
	explicit RenderSystem(sf::RenderTarget &target);
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override;
	void LoadRoom(size_t index);
	size_t getRoomIndex() const { return _room->index(); }
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

class RoomSystem {

	float last_update = 0.0;
	float frame_count = 0.0;
public:
//	RoomSystem(sf::RenderTarget& target) : target(target) {}
	
//	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	
};

class Application : public ex::EntityX {
	sf::Clock _clock;
	sf::Clock _debugUpdate;
	sf::RenderWindow& _window;
	sf::Time _lastTime;
	RenderSystem* render_system;
	size_t frame_count;
	size_t draw_count;
	size_t update_count;
	sf::Font _font;
	sf::Text _text;
	sf::Transform _transform;
	typedef std::unordered_map<const sf::Texture*, RawVertices> t_dumb_batch;
	typedef std::vector<sf::FloatRect> t_debug_boxes;
	std::map<int, t_dumb_batch> sortedVerts;
	std::unordered_multimap<size_t, ex::Entity::Id> _roomObjects;
	std::vector< ex::Entity> _roomEntitys;
	UndertaleRoom::type _room;
public:
	explicit Application(sf::RenderWindow &window);
	void LoadRoom(size_t index) {
		ex::EntityManager &es = global::getEntities();
		for (auto& e : _roomEntitys) e.destroy();
		_roomEntitys.clear();
		sortedVerts.clear();
		_roomObjects.clear();
		_room.reset();
		_room = UndertaleRoom::LoadRoom(index);
		if (_room) {
			for (auto& o : _room->objects()) {
				ex::Entity e = es.create();
				e.assign<UndertaleObject>(o.obj);
				e.assign<Body>(o.body);
				e.assign<Layer>(o.obj.depth());
				if (o.obj.sprite_index() >= 0) {
					e.assign<UndertaleSprite>(o.obj.sprite_index());
				}
				_roomEntitys.emplace_back(e);
				auto parent = o.obj;
				while (parent.valid()) {
					_roomObjects.emplace(std::make_pair((size_t)parent.index(), e.id()));
					parent = Global::LookupObject(parent.parent_index());
				}				
			}
		}
	}
	size_t getRoomIndex() const { return _room ? _room->index() : 0; }
	void init(ex::EntityX& app); 
	void draw() {
		draw_count = 0;
		_window.clear();
		sf::RenderStates states = sf::RenderStates::Default;
		states.transform = _transform;
		frame_count++;
		auto& verts = sortedVerts;
		if (verts.size() > 0) {
			for (auto& sv : verts) {
				if (sv.second.size() == 0) continue;
				for (auto& b : sv.second) {
					draw_count++;
					states.texture = b.first;
					_window.draw(b.second.data(), b.second.size(), sf::PrimitiveType::Triangles, states);
				}
			}
		}
		_window.draw(_text);
		_window.display();
	}
	void update_verts(ex::EntityManager& es) {
		RawVertices temp_verts;
		sortedVerts.clear();
		if (_room) {
			if (_room->backgrounds().size() > 0) {
				for (auto& t : _room->backgrounds()) {
					if (t.forground) continue;
					int layer = t.depth;
					auto& verts = (sortedVerts[layer])[t.frame.texture()];
					temp_verts.assign(t.frame.ptr(), t.frame.ptr() + t.frame.size());
					temp_verts.traslate(t.pos);
					verts += temp_verts;
				}
				if (_room->tiles().size() > 0) {
					for (auto& t : _room->tiles()) {
						auto& verts = (sortedVerts[0])[t.first];
						const auto& f = t.second;
						verts.append(f.verts());
					}
				}
			}
		}
		es.each<Body, UndertaleSprite>([this, &temp_verts](ex::Entity entity, Body& body, UndertaleSprite &sprite) {
			constexpr bool draw_all_boxes = true;
			int layer = entity.has_component<Layer>() ? entity.component<Layer>() : 0;
			auto& verts = (sortedVerts[layer])[sprite.texture()];
			temp_verts.assign(sprite.ptr(), sprite.ptr() + 6);
			temp_verts.transform(body.getTransform());
			verts += temp_verts;
		});

		if (_room && _room->backgrounds().size() > 0) {
			for (auto& t : _room->backgrounds()) {
				if (!t.forground) continue;
				int layer = t.depth;
				auto& verts = (sortedVerts[layer])[t.frame.texture()];
				temp_verts.assign(t.frame.ptr(), t.frame.ptr() + t.frame.size());
				temp_verts.traslate(t.pos);
				verts += temp_verts;
			}
		}
	}
	static constexpr size_t room_fps = (1000/ 30);
	void update(ex::TimeDelta dt) { 
		sf::Time current = _clock.getElapsedTime();
		if (current.asMilliseconds() > room_fps) {
			update_count++;
			float delta = _clock.restart().asSeconds();
			systems.system<PlayerOverWorldSystem>()->update(entities, events, delta);
			systems.system<VelocitySystem>()->update(entities, events, delta);
			systems.system<AnimationSystem>()->update(entities, events, delta);
			//systems.system<RenderSystem>()->update(entities, events, delta);
			update_verts(entities);
		}
		if (_debugUpdate.getElapsedTime().asSeconds() >= 1.0) {
			float last_update = _debugUpdate.restart().asSeconds();
			std::ostringstream out;
			const float fps = frame_count / last_update;
			out << "Draw FPS(";
			out << std::setprecision(2) << std::fixed << (float)((float)frame_count / last_update);
			out << ") Update FPS(";
			out << std::setprecision(2) << std::fixed << (float)((float)update_count / last_update);
			out << ")" << std::endl;
			out << "Objects(" << entities.size() << ") DrawCount(" << draw_count << ")" << std::endl;
			_text.setString(out.str());
			draw_count = 0;
			update_count = 0;
			frame_count = 0;
		}
		draw();
	}
};

