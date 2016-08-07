#include "UndertaleLoader.h"
#include "obj_writer.h"
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


void gameLoop(sf::RenderWindow& window) {
	sf::Clock clock;
	bool isPlaying = false;
	auto font = UFont::LoadUndertaleFont(4);
	sf::View view(sf::FloatRect(0, 0, 320, 240));
	window.setView(view);
	obj_writer writer;
	writer.setFont(2);
	writer.setText("* mind your p and q's and I");
	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// Window closed or escape key pressed: exit
			if ((event.type == sf::Event::Closed) ||
				((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
			{
				window.close();
				break;
			}

		}
		if (!window.isOpen()) break;
		window.draw(writer);
		window.display();
	}
}
int main(int argc, const char* argv[]) {
	if (argc != 2 || !Global::LoadUndertaleDataWin(argv[1])) return -1;
	const int gameWidth = 800;
	const int gameHeight = 600;

	// Create the window of the application
	sf::RenderWindow window(sf::VideoMode(800, 600, 32), "SFML Pong", sf::Style::Titlebar | sf::Style::Close);
	gameLoop(window);
	

	// we got to run this to delete all the loaded textures we have or visual studio blows a fit
	Global::DestroyEveything();
	return 0;
}