#include "Global.h"
#include "UndertaleLoader.h"
#include "obj_dialoger.h"
#include <cassert>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "flac.lib")
#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "freetype.lib")
#pragma comment(lib, "openal32.lib")
#pragma comment(lib, "vorbis.lib")
#pragma comment(lib, "vorbisenc.lib")
#pragma comment(lib, "vorbisfile.lib")

#pragma comment(lib, "sfml-main-d.lib")
#pragma comment(lib, "sfml-system-s-d.lib")
#pragma comment(lib, "sfml-graphics-s-d.lib")
#pragma comment(lib, "sfml-window-s-d.lib")

sf::RenderWindow* s_window = nullptr; // global window

namespace global {
	sf::RenderWindow& getWindow() {
		assert(s_window);
		return *s_window;
	}
};
struct Velocity  { 
	sf::Vector2f velocity; 
	explicit Velocity(float x, float y) : velocity(x, y) {}
};
// Render all Renderable entities and draw some informational text.
class VelocitySystem :public ex::System<VelocitySystem> {
private:
	float last_update = 0.0f;
	sf::RenderTarget &target;
public:
	explicit VelocitySystem(sf::RenderTarget &target) : target(target) {
	}
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override {
		last_update += dt;
		if (last_update >= (1.0 / 30.0)) {
			
			es.each<Body, Velocity>([this](ex::Entity entity, Body &body, Velocity& velocity) {
				body.move(velocity.velocity*last_update);
			});
			last_update = 0.0;
		}
	}
};
class RenderSystem :public ex::System<RenderSystem> {
private:
	double last_update = 0.0;
	double frame_count = 0.0;
	sf::RenderTarget &target;
	sf::Text text;
	std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> draw_verts;
public:
	explicit RenderSystem(sf::RenderTarget &target, sf::Font &font) : target(target) {
		text.setFont(font);
		text.setPosition(sf::Vector2f(2, 2));
		text.setCharacterSize(9);
		text.setColor(sf::Color::White);
	}
	
	void update(ex::EntityManager &es, ex::EventManager &events, ex::TimeDelta dt) override {
		draw_verts.clear();
		es.each<Body, Mesh,const sf::Texture*>([this](ex::Entity entity, Body &body, Mesh &mesh, const sf::Texture* texture) {
			auto& verts = draw_verts[texture];
			mesh.insert(verts, body.getTransform());
		//	target.draw(verts.data(), 6, sf::PrimitiveType::Triangles, states);
			//
		});
		for (auto& kv : draw_verts) {
			auto& verts = kv.second;
			
			target.draw(verts.data(), verts.size(), sf::PrimitiveType::Triangles, sf::RenderStates(kv.first));
		}

		// , sf::RenderStates(body.getTransform()));
		
		last_update += dt;
		frame_count++;
		if (last_update >= 0.5) {
			std::ostringstream out;
			const double fps = frame_count / last_update;
			out << es.size() << " entities (" << static_cast<int>(fps) << " fps)";
			text.setString(out.str());
			last_update = 0.0;
			frame_count = 0.0;
		}
		target.draw(text);
	}
};


constexpr size_t UpdateTime = 30 / 1000; // 30 frames a second

class Application : public ex::EntityX {
public:
	explicit Application(sf::RenderTarget &target, sf::Font &font) {
		//systems.add<SpawnSystem>(target, 500);
		//systems.add<BodySystem>();
		//systems.add<BounceSystem>(target);
	//	systems.add<CollisionSystem>(target);
	//	systems.add<ExplosionSystem>();
	//	systems.add<ParticleSystem>();

		
		systems.add<VelocitySystem>(target);
		systems.add<RenderSystem>(target, font);
	//	systems.add<ParticleRenderSystem>(target);
		systems.configure();
	}

	void update(ex::TimeDelta dt) {
		systems.update_all(dt);
	}
};
kult::component<'body', Body> kbody;
kult::component<'anim', StopWatch<float>> kanimation;
kult::component<'vel', sf::Vector2f> kvelocity;
kult::component<'pos', sf::Vector2f> kposition;
kult::component<'fcol', SpriteFrameCollection> kframes;
kult::component<'fsng', SpriteFrame> kframe;
kult::component<'text', UndertaleLabelBuilder> ktext;
//kult::component<'ani1', PtrComponent<Renderable2>> krenderable;

kult::system<float> velocity_system([](float dt) {
	for (auto &entitys : kult::join(kvelocity, kbody)) {
		auto& velocity = entitys[kvelocity];
		auto& body = entitys[kbody];
		body.move(velocity*dt);
	//	std::stringstream ss;
	//	ss << body.getPosition();
	//	debug_label.setText(ss.str());
	//	debug_label_entity[ktext].setText(ss.str());
	}
});

// animation system
kult::system<float> animation_system([](float dt) {
	for (auto &entitys : kult::join(kanimation, kframes)) {
		auto& animation = entitys[kanimation];
		auto& frames = entitys[kframes];
		if (animation.test_then_reset(dt)) {
			frames.setImageIndex(frames.getImageIndex() + 1);
		}
	}
});
typedef std::unordered_map<const sf::Texture*, std::vector<sf::Vertex>> t_dumb_batch;

// rendering_system
kult::system<t_dumb_batch&> rendering_system([](t_dumb_batch& batch) {
	for (auto &entitys : kult::join(kframes, kbody)) {
		auto& frame = entitys[kframes];
		auto& verts = batch[frame.getTexture()];
		frame.insert(verts, entitys[kbody].getTransform());
	}
	for (auto &entitys : kult::join(ktext, kbody)) {
		auto& text = entitys[ktext];
		auto& verts = batch[&text.getTexture()];
		text.insert(verts, entitys[kbody].getTransform());
	}
});


namespace ex = entityx;
void gameLoop() {
	sf::RenderWindow& window = global::getWindow();
	sf::Clock clock;
	bool isPlaying = false;
	auto font = UFont::LoadUndertaleFont(4);
	sf::View view(sf::FloatRect(0, 0, 320, 240));
//	window.setView(view);
	auto writer = obj_dialoger::create();
	//writer.setFont(4);
	writer->setConfig();
	writer->setText("* mind your p \\Yand\n\r q's and I");
	writer->start_typing();
	UndertaleLabel debug_label;
	
	sf::Font debug_font;
	if (!debug_font.loadFromFile("LiberationSans-Regular.ttf")) {
		std::cerr << "error: failed to load LiberationSans-Regular.ttf" << std::endl;
		return;
	}
	SpriteFrameCollection raw_sprite = Global::LoadSprite(1986);
	kult::entity etest;
	etest[kanimation] = 0.5f;
	
	etest[kvelocity] = sf::Vector2f(1.0f, 0.0f);
	etest[kframes] = raw_sprite;
	etest[kbody].setPosition(10.0f, 100.0f);

//	etest[kposition] = sf::Vector2f(10.0f, 100.0f);
	etest[kbody].setScale(4.0f);
	t_dumb_batch draw_verts;

	kult::entity debug_label_entity;
	debug_label_entity[kbody].setPosition(0.0f, 0.0f);
	//debug_label_entity[kbody].setScale(2.0f);
	debug_label_entity[ktext] = "Happy Happy Joy joy";

	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				return;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape) {
					window.close();
					return;
				}
				break;
			}
		}
		auto time = clock.getElapsedTime();
		if (time.asMilliseconds() > 60/1000) { // 60 times a second
			clock.restart();
			draw_verts.clear();
			
			float dt = time.asSeconds();
			velocity_system(dt);
			animation_system(dt);



			// handle rendering system
			draw_verts.clear();
			rendering_system(draw_verts);
		}
		// update as fast as we can?
		window.clear();
		sf::RenderStates states = sf::RenderStates::Default;
		//window.draw(debug_label);
		for (auto& kv : draw_verts) {
			auto& verts = kv.second;
			states.texture = kv.first;
			window.draw(verts.data(), verts.size(), sf::PrimitiveType::Triangles, states);
		}
		
		window.display();
	}
}
int main(int argc, const char* argv[]) {
	if (argc != 2 || !Global::LoadUndertaleDataWin(argv[1])) return -1;
	logging::init_cerr();
	logging::init_cout();
	const int gameWidth = 800;
	const int gameHeight = 600;

	// Create the window of the application
	s_window = new sf::RenderWindow(sf::VideoMode(800, 600, 32), "SFML Pong", sf::Style::Titlebar | sf::Style::Close);
	gameLoop();
	// we got to run this to delete all the loaded textures we have or visual studio blows a fit
	Global::DestroyEveything();
	delete s_window;
	s_window = nullptr;
	
	return 0;
}