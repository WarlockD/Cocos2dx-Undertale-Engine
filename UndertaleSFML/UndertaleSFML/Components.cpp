#include "Components.h"
#include "UndertaleLoader.h"
#include <SFML/OpenGL.hpp>
#include <map>

using namespace sf;




bool PlayerControl::DirectionDown[4]{ false,false,false,false };
std::unordered_set<sf::Keyboard::Key> PlayerControl::keys_down;
void PlayerControl::update_keys(const sf::Event &event) {
	switch (event.type) {
	case sf::Event::EventType::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Key::A:
			DirectionDown[(int)Direction::Left] = true;
			break;
		case sf::Keyboard::Key::D:
			DirectionDown[(int)Direction::Right] = true;
			break;
		case sf::Keyboard::Key::W:
			DirectionDown[(int)Direction::Up] = true;
			break;
		case sf::Keyboard::Key::S:
			DirectionDown[(int)Direction::Down] = true;
			break;
		}
		keys_down.emplace(event.key.code);
		break;
	case sf::Event::EventType::KeyReleased:
		switch (event.key.code) {
		case sf::Keyboard::Key::A:
			DirectionDown[(int)Direction::Left] = false;
			break;
		case sf::Keyboard::Key::D:
			DirectionDown[(int)Direction::Right] = false;
			break;
		case sf::Keyboard::Key::W:
			DirectionDown[(int)Direction::Up] = false;
			break;
		case sf::Keyboard::Key::S:
			DirectionDown[(int)Direction::Down] = false;
			break;
		}
		keys_down.erase(event.key.code);
		break;
	}
}

sf::Vector2f PlayerControl::getMovement() {
	sf::Vector2f movement;
	if (isMoving()) {
		bool turned = false;
		if (isMoving(Direction::Left)) {
			//if (self.xprevious == self.x + 3) self.x -= 2; else self.x -= 3;
			movement.x -= moving_speed;
			turned = true;
			if (isMoving(Direction::Up) && facing == Direction::Up) turned = false;
			if (isMoving(Direction::Down) && facing == Direction::Down) turned = false;
			if (turned) facing = Direction::Left;
		}
		if (isMoving(Direction::Up)) {
			
			movement.y -= moving_speed;
			turned = true;
			if (isMoving(Direction::Right) && facing == Direction::Right) turned = false;
			if (isMoving(Direction::Left) && facing == Direction::Left) turned = false;
			if (turned) facing = Direction::Up;
		}
		if (isMoving(Direction::Right) && !isMoving(Direction::Left)) {
			movement.x += moving_speed;
			turned = true;
			if (isMoving(Direction::Up) && facing == Direction::Up) turned = false;
			if (isMoving(Direction::Down) && facing == Direction::Down) turned = false;
			if (turned) facing = Direction::Right;
		}
		if (isMoving(Direction::Down) && !isMoving(Direction::Up)) {
			movement.y += moving_speed;
			turned = true;
			if (isMoving(Direction::Right) && facing == Direction::Right) turned = false;
			if (isMoving(Direction::Left) && facing == Direction::Left) turned = false;
			if (turned) facing = Direction::Down;
		}
	}
	return movement;
}



/*as a fall back to line()*/
void line_raw(float x1, float y1, float x2, float y2,
	float w,
	float Cr, float Cg, float Cb,
	float, float, float, bool)
{
	glLineWidth((float)w);
	float line_vertex[] =
	{
		x1,y1,
		x2,y2
	};
	float line_color[] =
	{
		Cr,Cg,Cb,
		Cr,Cg,Cb
	};
	// Bind no texture
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glVertexPointer(2, GL_FLOAT, 0, line_vertex);
	glColorPointer(3, GL_FLOAT, 0, line_color);
	glDrawArrays(GL_LINES, 0, 2);
}

void draw_box_old(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green) {
	float left = rect.left;
	float top = rect.top;
	float right = rect.left + rect.width;
	float bottom = rect.top + rect.height;
	// Add a quad for the current character
	verts.append(Vertex(Vector2f(left, top), color)); verts.append(Vertex(Vector2f(left, bottom), color));
	verts.append(Vertex(Vector2f(left, top), color)); verts.append(Vertex(Vector2f(right, top), color));
	verts.append(Vertex(Vector2f(right, bottom), color)); verts.append(Vertex(Vector2f(left, bottom), color));
	verts.append(Vertex(Vector2f(right, bottom), color)); verts.append(Vertex(Vector2f(right, top), color));
}
void draw_box(RawVertices& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green) {
	float left = rect.left;
	float top = rect.top;
	float right = rect.left + rect.width;
	float bottom = rect.top + rect.height;
	global::insert_line(verts, sf::Vector2f(left, top), sf::Vector2f(right, top), thickness, color);
	global::insert_line(verts, sf::Vector2f(left, top), sf::Vector2f(left, bottom), thickness, color);
	global::insert_line(verts, sf::Vector2f(right, bottom), sf::Vector2f(left, bottom), thickness, color);
	global::insert_line(verts, sf::Vector2f(right, bottom), sf::Vector2f(right, top), thickness, color);
	// Add a quad for the current character
 
}
void draw_box(const sf::FloatRect& rect,  float thickness = 4.0f, const sf::Color color = sf::Color::Green) {
	sf::VertexArray test(sf::PrimitiveType::TrianglesStrip);
	glLineWidth(thickness);
	sf::VertexArray array(sf::PrimitiveType::Lines);
	float line_vertex[] =
	{
		rect.left,rect.top, rect.left,rect.height-rect.top,
		rect.left,rect.top, rect.width-rect.left, rect.top,
		rect.width-rect.left,rect.height-rect.top,rect.left,rect.height - rect.top,
		rect.width-rect.left,rect.height-rect.top,rect.width-rect.left, rect.top,
	};
	sf::Color line_color[] = { color, color,color, color, color, color,color, color };


}
RawVertices createTest() {
	RawVertices vect;
	global::insert_line(vect, sf::Vector2f(100.0f, 100.0f), sf::Vector2f(200.0f, 200.0f), 10.0f, sf::Color::White);
	global::insert_hair_line(vect, sf::Vector2f(200.0f, 150.0f), sf::Vector2f(50.0f, 50.0f), 10.0f, sf::Color::Green);
	return vect;
}
RawVertices test;


std::unordered_map<size_t, size_t> debug_handles;
static size_t lineindex = 0;
size_t findDebugLine(size_t value) {
	auto it = debug_handles.find(value);
	if (it == debug_handles.end()) {
		it = debug_handles.emplace(value, lineindex).first;
		it->second = lineindex++;
	}
	return it->second;
}


Application::Application(sf::RenderWindow &window) : _window(window), draw_count(0), frame_count(0) {
	_transform.scale(1.5f, 1.5f);
	//systems.add<PlayerOverWorldSystem>(*this, _window);
//	systems.add<RenderSystem>(_window);
//	render_system = systems.system<RenderSystem>().get();
	systems.configure();

	if (!_font.loadFromFile("LiberationSans-Regular.ttf")) {
		std::cerr << "error: failed to load LiberationSans-Regular.ttf" << std::endl;
		exit(1);
	}
	_text.setFont(_font);
	_text.setPosition(sf::Vector2f(2, 2));
	_text.setCharacterSize(18);
	_text.setColor(sf::Color::White);

}
void Application::init(ex::EntityX& app) {
//	systems.system<PlayerOverWorldSystem>()->init(app);
	
}
std::set<ex::Entity> Application::findOjbects(size_t index) {
	std::set<ex::Entity> enity;
	auto it_pair = _roomObjects.equal_range(index);
	if (it_pair.first != it_pair.second) {
		for (auto it = it_pair.first; it != it_pair.second; it++)
			enity.emplace(it->second);
	}
	return enity;
}

ex::Entity Application::findSingleObject(size_t index) {
	auto it = _roomObjects.find(index);
	if (it != _roomObjects.end()) return it->second;
	else return ex::Entity();
}

void Application::LoadRoom(size_t index) {
	for (auto& e : _roomEntitys)  e.destroy();
	_roomEntitys.clear();
	sortedVerts.clear();
	_roomObjects.clear();
	 _static_entitys.clear();
	_dynamic_enitys.clear();
	_room.reset();
	_room = UndertaleRoom::LoadRoom(index);
	if (_room) {
		if (_room->objects().size() > 0) {
			for (auto& o : _room->objects()) {
				ex::Entity e = entities.create();
				auto obj = e.assign<UndertaleObject>(o.obj);
				auto body = e.assign<Body>(o.body);
				e.assign<Layer>(o.obj.depth());
				if (o.obj.sprite_index() >= 0) {
					e.assign<UndertaleSprite>(o.obj.sprite_index());
				}
				_roomEntitys.emplace_back(e);
				_roomObjects.emplace(std::make_pair(o.obj.index(), e));
				for (auto p : obj->parents) _roomObjects.emplace(std::make_pair(p.index(), e));
			}
			auto& player = findSingleObject(1570);// find player
			if (player.valid()) {
				player.assign<PlayerControl>(30.0f); // we can move it
				auto facing = player.assign<SpriteFacing>(1043, 1045, 1044, 1046);
				player.assign<SpriteAnimation>(0.2f, facing->facing_sprites[0].image_count());
			}
			auto static_it = findOjbects(820); // find all static objects
			if (static_it.size() > 0) {
				_static_entitys.assign(static_it.begin(), static_it.end());
			}
		}
	}
}
struct Candidate {
	sf::Vector2f position;
	float radius;
	ex::Entity entity;
};

static size_t frame_count = 0;

void Application::update_verts(ex::TimeDelta dt) {
	
	
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
		}
		if (_room->tiles().size() > 0) {
			for (auto& t : _room->tiles()) {
				auto& verts = (sortedVerts[0])[t.first];
				const auto& f = t.second;
				verts.append(f.verts());
			}
		}
	}

	entities.each<SpriteFacing, UndertaleSprite, Body, PlayerControl, SpriteAnimation>([this, dt](ex::Entity entity, SpriteFacing& spritefacing, UndertaleSprite& sprite, Body& body, PlayerControl &control, SpriteAnimation& animation) {
		if (PlayerControl::isMoving()) {
			body.move(control.getMovement()*dt);
			if (control.facing != spritefacing.direction) {
				sprite = spritefacing.getCurrentFace(control.facing);
				spritefacing.direction = control.facing;
			}
			if (!animation.is_running())animation.start();
		}
		else {
			if (animation.is_running()) {
				animation.reset();
				animation.stop();
				sprite.image_index(0);
			}
		}
	});

	static bool last_state = false;

	
	entities.each<UndertaleSprite, SpriteAnimation>([dt](ex::Entity entity, UndertaleSprite& sprite, SpriteAnimation &animation) {
		bool pressed = sf::Keyboard::isKeyPressed(sf::Keyboard::N);
		if (pressed) {
			if (!last_state) {
				last_state = true;
				sprite.image_index(sprite.image_index() + 1);
			}
		}
		else last_state = false;
		if (animation.update(dt)) {
			sprite.image_index(animation.current_frame());
		}
	});

	entities.each<Body, UndertaleSprite>([this](ex::Entity entity, Body& body, UndertaleSprite &sprite) {
		constexpr bool draw_all_boxes = true;
		int layer = entity.has_component<Layer>() ? entity.component<Layer>() : 0;
		auto& verts = (sortedVerts[layer])[sprite.texture()];
		temp_verts.assign(sprite.ptr(), sprite.ptr() + 6);
		temp_verts.transform(body.getTransform());
		body.fixBounds(sprite.frame_size());
		verts += temp_verts;
	});
//	Candidate canadates;

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

	
//	info_window.refresh(10, 5);
}
void Application::draw() {
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

static console::Window info_window(80, 50);
void Application::update(ex::TimeDelta dt) {
	sf::Vector2i mouse_old;
	sf::Time current = _clock.getElapsedTime();
	if (current.asMilliseconds() > room_fps) {
		update_count++;
		float delta = _clock.restart().asSeconds();

		//systems.system<PlayerOverWorldSystem>()->update(entities, events, delta);
		//	systems.system<VelocitySystem>()->update(entities, events, delta);
		//	systems.system<AnimationSystem>()->update(entities, events, delta);
		//systems.system<RenderSystem>()->update(entities, events, delta);
		update_verts(delta);
	}
	sf::Vector2i mouse_current = sf::Mouse::getPosition(_window);
	if (mouse_current != mouse_old) {
		info_window.clearline(2);
		info_window.cursor(0,2);
		info_window.print("mouse (%2.2i,%2.2i)", mouse_current.x, mouse_current.y);
		// object debug
		entities.each<Body, UndertaleObject, UndertaleSprite>([this, mouse_current](ex::Entity entity, Body &body, UndertaleObject& obj, UndertaleSprite &sprite) {
			sf::FloatRect bounds = body.getBounds();
			if (bounds.contains(sf::Vector2f(mouse_current))) {
				auto& verts = (sortedVerts[100])[nullptr];
				draw_box(verts, bounds);
				auto& o = obj.obj;
				size_t line = 3;
				if (o.valid()) {
					info_window.cursor(0, line);
					info_window.clearline(line++);
					info_window.print("Object(%i, %s)\r\nBox(%2.2f, %2.2f, %2.2f, 2.2f)", o.index(), o.name().c_str(), bounds.left, bounds.top, bounds.width, bounds.height);
					if (obj.parents.size() > 0) {
						for (auto& p : obj.parents) {
							info_window.cursor(0, line);
							info_window.clearline(line++);
							info_window.print("->(%i, %s)   \r\n", p.index(), p.name().c_str());
						}
					}
				}
				else info_window.print("invalid obj");
			}
		});
	}

	if (_debugUpdate.getElapsedTime().asSeconds() >= 0.1) {
		float last_update = _debugUpdate.restart().asSeconds();
		info_window.clearline(0);
		info_window.cursor(0, 0);
		info_window.print("FPS(%2.2f) Update(%2.2f) Objects(%i)", (float)((float)frame_count / last_update), (float)((float)update_count / last_update, entities.size()));
		info_window.refresh(5, 5);
		/*
		
		info_window.refresh(10, 10);
		std::ostringstream out;
		const float fps = frame_count / last_update;
		out << "Draw FPS(";
		out << std::setprecision(2) << std::fixed << (float)((float)frame_count / last_update);
		out << ") Update FPS(";
		out << std::setprecision(2) << std::fixed << (float)((float)update_count / last_update);
		out << ")" << std::endl;
		out << "Objects(" << entities.size() << ") DrawCount(" << draw_count << ")" << std::endl;
		_text.setString(out.str());
		*/
		
		draw_count = 0;
		update_count = 0;
		frame_count = 0;
	}
	draw();
}