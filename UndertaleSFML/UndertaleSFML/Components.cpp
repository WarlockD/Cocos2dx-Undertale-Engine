#include "Components.h"
#include "UndertaleLoader.h"
#include <SFML/OpenGL.hpp>
#include <map>

using namespace sf;


PlayerOverWorldSystem::PlayerOverWorldSystem(ex::EntityX& app,sf::RenderTarget &target) : target(target), app(app) {
	//_player.load_resources(app);
}
void PlayerOverWorldSystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt)  {

}

bool Player::load_resources(ex::EntityX& app) {
	app.events.subscribe<SystemEvent>(*this);
	_enity = app.entities.create();
	_enity.assign<Body>();
	_enity.assign<RenderableRef>(*this);
	_enity.assign<Velocity>();
	_facing = PlayerFacing::DOWN;

	_sprites[0].sprite_index(1043); 
	_sprites[1].sprite_index(1045); 
	_sprites[2].sprite_index(1044); 
	_sprites[3].sprite_index(1046);
	_enity.component<Body>()->setPosition(180, 180);
	return true;
}
Player::~Player() {
	if (_enity.valid()) {
		global::getEventManager().unsubscribe<SystemEvent>(*this);
		_enity.destroy();
	}
}
void Player::receive(const SystemEvent &sevent) {
	auto& event = sevent.event;
	switch (event.type) {
	case sf::Event::EventType::KeyPressed:
		switch (event.key.code) {
		case sf::Keyboard::Key::A:
			_enity.component<Velocity>()->velocity.x = -5.0f; 
			_facing = PlayerFacing::LEFT;
			break;
		case sf::Keyboard::Key::D:
			_enity.component<Velocity>()->velocity.x = 5.0f;
			_facing = PlayerFacing::RIGHT;
			break;
		}
	};
}




bool Animation::update(Renderable& renderable, float dt) {
//	if (_watch.test_then_reset(dt)) 
//		return _reverse ? renderable.prev_frame() : renderable.next_frame();
	return true; // we havn't gotten to the next frame yet so just assume true
}
/*as a fall back to line()*/
void line_raw(double x1, double y1, double x2, double y2,
	double w,
	double Cr, double Cg, double Cb,
	double, double, double, bool)
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
void draw_box(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green) {
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
RawTriangles createTest() {
	sf::VertexArray test(sf::PrimitiveType::TrianglesStrip);
	RawTriangles vect;
	global::insert_line(test, sf::Vector2f(100.0f, 100.0f), sf::Vector2f(200.0f, 200.0f), 10.0f, sf::Color::White);
	vect += test;
	test.clear();
	global::insert_hair_line(test, sf::Vector2f(200.0f, 150.0f), sf::Vector2f(50.0f, 50.0f), 10.0f, sf::Color::White);
	vect += test;
	test.clear();
	return vect;
}
RawTriangles test;
RenderSystem::RenderSystem(sf::RenderTarget &target) : target(target), debug_lines(sf::PrimitiveType::Lines) {
	if (!_font.loadFromFile("LiberationSans-Regular.ttf")) {
		std::cerr << "error: failed to load LiberationSans-Regular.ttf" << std::endl;
		exit(1);
	}
	text.setFont(_font);
	text.setPosition(sf::Vector2f(2, 2));
	text.setCharacterSize(18);
	text.setColor(sf::Color::White);
	test = createTest();
}
std::multimap<int, std::reference_wrapper<const sf::Drawable>> sorting;
void RenderSystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	
	sortedVerts.clear();
	debug_lines.clear();
	
	sf::VertexArray temp_verts(sf::PrimitiveType::TrianglesStrip);
	es.each<Body, RenderableCache>([this, &temp_verts](ex::Entity entity, Body& body, RenderableCache &lrenderable) {
		const auto& transform = body.getTransform();
		sf::RenderStates states(transform); 
		target.draw(lrenderable, states);
	});
	return;
	es.each<Body, RenderableRef>([this,&temp_verts](ex::Entity entity, Body& body, RenderableRef &renderable_ref) {
		constexpr bool draw_all_boxes = true;
		int layer = entity.has_component<Layer>() ? entity.component<Layer>() : 0;
		const auto& transform = body.getTransform();
		Renderable& renderable = renderable_ref;
		auto& verts = (sortedVerts[layer])[renderable.texture()];
		renderable.insert(verts, transform);
		if (draw_all_boxes || renderable.debug_draw_box) {
			draw_box(temp_verts, transform.transformRect(renderable.bounds()));
		}
	});
	size_t draw_count = 0;
	sf::RenderStates states = sf::RenderStates::Default;
	
	
	//target.pushGLStates();
	for (auto& sv : sortedVerts) {
		for (auto& b : sv.second) {
			draw_count++;
			states.texture = b.first;
			target.draw(b.second.data(), b.second.size(), sf::PrimitiveType::Triangles, states);
		}
	}
	RawTriangles boxes = temp_verts;
	if(debug_lines.getVertexCount() > 0) {
		target.draw(debug_lines);
	}
	target.draw(boxes);

	last_update += dt;
	frame_count++;
	if (last_update >= 0.5) {
		std::ostringstream out;
		const double fps = frame_count / last_update;
		out << es.size() << " entities (" << static_cast<int>(fps) << " fps)" << std::endl;
		out << "draws " << draw_count;
		text.setString(out.str());
		last_update = 0.0;
		frame_count = 0.0;
	}
	target.draw(text);
}

void AnimationSystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	
	es.each<RenderableRef, Animation>([this,dt](ex::Entity entity, RenderableRef &renderable_ref, Animation &animation) {
		if (!animation.update(renderable_ref, dt)) entity.remove<Animation>();
	
	});
}

void VelocitySystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	es.each<Body, Velocity>([this, dt](ex::Entity entity, Body& body, Velocity &velocity) {
		body.setPosition(body.getPosition() + (velocity.velocity * dt));
	});
}

Application::Application(sf::RenderTarget &target) {
	systems.add<PlayerOverWorldSystem>(*this,target);
	systems.add<VelocitySystem>(target);
	systems.add<AnimationSystem>(target);
	systems.add<RenderSystem>(target);
	systems.configure();
}
void Application::init(ex::EntityX& app) {
	systems.system<PlayerOverWorldSystem>()->init(app);
}