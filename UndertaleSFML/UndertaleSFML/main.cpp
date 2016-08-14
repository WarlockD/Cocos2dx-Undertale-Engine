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


constexpr size_t UpdateTime = 30 / 1000; // 30 frames a second
void gameLoop() {
	sf::RenderWindow& window = global::getWindow();
	sf::Clock clock;
	bool isPlaying = false;
	auto font = UFont::LoadUndertaleFont(4);
	sf::View view(sf::FloatRect(0, 0, 320, 240));
	window.setView(view);
	auto writer = obj_dialoger::create();
	//writer.setFont(4);
	writer->setConfig();
	writer->setText("* mind your p \\Yand\n\r q's and I");
	writer->start_typing();
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
		if (time.asMilliseconds() > UpdateTime) {
			writer->update(time);
			clock.restart();
		}
		window.draw(*writer);
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