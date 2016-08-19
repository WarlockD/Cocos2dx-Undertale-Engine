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

static std::unique_ptr<sf::RenderWindow> s_window;// global window
static std::unique_ptr<ex::EntityX> s_app;

namespace global {
	sf::RenderWindow& getWindow() {
		assert(s_window);
		return *s_window;
	}
	ex::EventManager& getEventManager() { return s_app->events; }
	ex::EntityManager& getEntities() { return s_app->entities; }
	ex::SystemManager& getSystems() { return s_app->systems; }
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
	
	SpriteFrameCollection raw_sprite = Global::LoadSprite(1986);

	auto& systems = global::getSystems();
	//SpriteEnity teste = SpriteEnity::create(1986)

	ex::Entity sprite = global::getEntities().create();
	sprite.assign<SpriteFrameCollection>(raw_sprite);
	sprite.assign<Body>();
	sprite.component<Body>()->setPosition(20, 50);
	sprite.component<Body>()->setScale(2.0, 2.0);
	sprite.assign<RenderableRef>(sprite.component<SpriteFrameCollection>().get());
	sprite.assign<Animation>(0.25f);

	ex::Entity sprite2 = global::getEntities().create_from_copy(sprite);
	sprite2.component<Body>()->setPosition(20, 80);
	sprite2.component<SpriteFrameCollection>() = Global::LoadSprite(1982);
	sprite2.component<Body>()->setPosition(20, 60);
	sprite2.component<Body>()->setScale(2.0, 2.0);
	sprite2.component<RenderableRef>() = RenderableRef(sprite2.component<SpriteFrameCollection>().get());
	//sprite2.component<Animation>() = Animation(0.25f;

	// don't want to invalidate the sprite entry
	//SpriteEnity sprite(&global::getEntities(), sprite_id.id());
	//sprite.setSpriteIndex(sprite_index);
	///sprite._sprite_index = 0;
	//sprite._body = sprite.assign<Body>().get();
	//return sprite;


	//teste->setPosition(20, 20);
	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type) {
			case sf::Event::Closed:
				return;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Escape) {
					return;
				}
				break;
			}
		}
		window.clear();
		sf::Time elapsed = clock.restart();
		systems.update_all(elapsed.asSeconds());
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
	s_window.reset(new sf::RenderWindow(sf::VideoMode(800, 600, 32), "SFML Pong", sf::Style::Titlebar | sf::Style::Close));
	s_app.reset(new Application(*s_window));
	gameLoop();
	// we got to run this to delete all the loaded textures we have or visual studio blows a fit
	s_window->close();
	Global::DestroyEveything();

	s_app.release();
	s_window.release();


	return 0;
}