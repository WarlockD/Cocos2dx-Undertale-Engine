#include "Global.h"
#include "Components.h"
#include "UndertaleLoader.h"
#include "obj_dialoger.h"
#include "UndertaleLabel.h"

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
		//systems.add<RenderSystem>(target, font);
	//	systems.add<ParticleRenderSystem>(target);
		systems.configure();
	}

	void update(ex::TimeDelta dt) {
		systems.update_all(dt);
	}
};




namespace ex = entityx;
void gameLoop() {
	sf::RenderWindow& window = global::getWindow();
	sf::Clock clock;
	bool isPlaying = false;
	auto font = UFont::LoadUndertaleFont(4);
	sf::View view(sf::FloatRect(0, 0, 640, 480));
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
	Systems::t_dumb_batch batch;

	Engine::USprite sprite0;
	Engine::USprite sprite1;

	sprite1.setSprite(1986);
	sprite1.body().setScale(2.0f,2.0f);
	sprite1.body().setPosition(50.0f, 50.0f);
	sprite0.setSprite(1987);
	sprite0.body().setScale(2.0f, 2.0f);
	sprite0.body().setPosition(50.0f, 150.0f);

	sprite0[Components::velocity] = sf::Vector2f(1.0f, 0.0f);
	sprite1[Components::animation] = 0.25;
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
			
			
			float dt = time.asSeconds();
			Systems::velocity_system(dt);
			Systems::animation_system(dt);



			// handle rendering system
			batch.clear();
			Systems::rendering_system(batch);
		}
		// update as fast as we can?
		window.clear();
		sf::RenderStates states = sf::RenderStates::Default;
		//window.draw(debug_label);
		for (auto& kv : batch) {
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