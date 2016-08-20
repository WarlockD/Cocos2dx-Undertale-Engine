#include "Components.h"
#include "UndertaleLoader.h"
#include <SFML/OpenGL.hpp>
#include <map>

using namespace sf;

RenderSystem::RenderSystem(sf::RenderTarget &target) : target(target), debug_lines(sf::PrimitiveType::Lines) {
	if (!_font.loadFromFile("LiberationSans-Regular.ttf")) {
		std::cerr << "error: failed to load LiberationSans-Regular.ttf" << std::endl;
		exit(1);
	}
	text.setFont(_font);
	text.setPosition(sf::Vector2f(2, 2));
	text.setCharacterSize(18);
	text.setColor(sf::Color::White);
}
/*as a fall back to line()*/
void line_raw(double x1, double y1, double x2, double y2,
	double w,
	double Cr, double Cg, double Cb,
	double, double, double, bool)
{
	glLineWidth(w);
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

void draw_box(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green) {
	float left = rect.left;
	float top = rect.top;
	float right = rect.left + rect.width;
	float bottom = rect.top + rect.height;
	// Add a quad for the current character
	verts.append(Vertex(Vector2f(left, top), color)); verts.append(Vertex(Vector2f(left, bottom), color));
	verts.append(Vertex(Vector2f(left, top), color)); verts.append(Vertex(Vector2f(right, top), color));
	verts.append(Vertex(Vector2f(right, top), color)); verts.append(Vertex(Vector2f(right, bottom), color));
	verts.append(Vertex(Vector2f(right, top), color)); verts.append(Vertex(Vector2f(left, top), color));
}
void draw_box(const sf::FloatRect& rect,  float thickness = 4.0f, const sf::Color color = sf::Color::Green) {
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
void RenderSystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	
	sortedVerts.clear();
	debug_lines.clear();
	es.each<Body, RenderableRef>([this](ex::Entity entity, Body& body, RenderableRef &renderable) {
		constexpr bool draw_all_boxes = true;
		int layer = entity.has_component<Layer>() ? entity.component<Layer>() : 0;
		const auto& transform = body.getTransform();
		auto& verts = (sortedVerts[layer])[renderable->texture()];
		renderable->insert(verts, transform);
		if (draw_all_boxes || renderable->debug_draw_box) {
			draw_box(debug_lines, transform.transformRect(renderable->bounds()));
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
	//target.popGLStates();
	if(debug_lines.getVertexCount() > 0) {
		glLineWidth(3.0f);
		target.draw(debug_lines);
	}

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

	es.each<SpriteFrameCollection, Animation>([this,dt](ex::Entity entity, SpriteFrameCollection& frames, Animation &animation) {
		if (animation.watch.test_then_reset(dt)) {
			if(animation.reverse)
				frames.setImageIndex(frames.getImageIndex() - 1);
			else
				frames.setImageIndex(frames.getImageIndex() + 1);

		}
	});
}

void VelocitySystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	es.each<Body, Velocity>([this, dt](ex::Entity entity, Body& body, Velocity &velocity) {
		body.setPosition(body.getPosition() + (velocity.velocity * dt));
	});
}

Application::Application(sf::RenderTarget &target) {
	systems.add<VelocitySystem>(target);
	systems.add<AnimationSystem>(target);
	systems.add<RenderSystem>(target);
	systems.configure();
}
