#include "Components.h"
#include "UndertaleLoader.h"
#include <map>


void RenderSystem::OrderedVerts::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	for (auto& b : batch) {
		states.texture = b.first;
		target.draw(b.second.data(), b.second.size(), sf::PrimitiveType::Triangles, states);
	}
}
RenderSystem::RenderSystem(sf::RenderTarget &target) : target(target) {
	if (!_font.loadFromFile("LiberationSans-Regular.ttf")) {
		std::cerr << "error: failed to load LiberationSans-Regular.ttf" << std::endl;
		exit(1);
	}
	text.setFont(_font);
	text.setPosition(sf::Vector2f(2, 2));
	text.setCharacterSize(18);
	text.setColor(sf::Color::White);
}
void RenderSystem::update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) {
	sortedVerts.clear();
	es.each<Body, RenderableRef>([this](ex::Entity entity, Body& body, RenderableRef &renderable) {
		int layer = entity.has_component<Layer>() ? entity.component<Layer>() : 0;
		auto& verts = (sortedVerts[layer])[(renderable->texture()];
		renderable->insert(verts, body.getTransform());
	});
	size_t draw_count = 0;
	sf::RenderStates states = sf::RenderStates::Default;
	for (auto& sv : sortedVerts) {
		for (auto& b : sv.second) {
			draw_count++;
			states.texture = b.first;
			target.draw(b.second.data(), b.second.size(), sf::PrimitiveType::Triangles, states);
		}
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
