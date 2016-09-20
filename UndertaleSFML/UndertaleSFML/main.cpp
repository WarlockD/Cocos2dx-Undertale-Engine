#include "Global.h"
#include "Components.h"
#include "UndertaleLoader.h"
#include "obj_dialoger.h"
#include "UndertaleLabel.h"
#include  <vt100.h>
#include <cassert>
#include <iostream>
#include <functional>

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

//#pragma comment(lib, "sfml-main-d.lib")
//#pragma comment(lib, "sfml-system-s-d.lib")
//#pragma comment(lib, "sfml-graphics-s-d.lib")
//#pragma comment(lib, "sfml-window-s-d.lib")

static std::unique_ptr<sf::RenderWindow> s_window;// global window
static std::unique_ptr<Application> s_app;


//http://stackoverflow.com/questions/19936841/initialize-a-constexpr-array-as-sum-of-other-two-constexpr-arrays
//http://en.cppreference.com/w/cpp/language/parameter_pack
// just beutiful
namespace array_helpers {
	template<int... Is>
	struct seq {};
	template<int I, int... Is>
	struct gen_seq : gen_seq<I - 1, I - 1, Is...> {};
	template<int... Is>
	struct gen_seq<0, Is...> : seq<Is...> {};

	template<typename T,size_t SIZE>
	struct _vec { 
		typedef T element_type;  
		T ptr[SIZE]; 
		static constexpr size_t dimensions = SIZE; 
		template<typename... Targs>
		constexpr _vec(Targs... Fargs) : ptr{ static_cast<T>(Fargs)... } {}
	};
	template<typename T>
	struct _vec<T, 2> {
		typedef T element_type;
		typedef T element_type;  struct { union { struct { T x; T y; }; T ptr[2]; }; }; 
		static constexpr size_t dimensions = 2;
		template<typename... Targs>
		constexpr _vec(Targs... Fargs) : ptr{ static_cast<T>(Fargs)... } {}
		//template<typename A, typename B>
		//constexpr _vec(A x,B x) : ptr{ static_cast<T>(x), static_cast<T>(y) } {}
	};

	template<typename T, size_t N>
	struct vec : public _vec<T,N> {
		using _vec::_vec;
		size_t constexpr size() const { return dimensions; }
		constexpr T operator[](size_t i) const { return ptr[i]; }
		T& operator[](size_t i) { return ptr[i]; }
	};

	template<class CHAR, class TRAITS, typename T, size_t N, int... Is>
	void print_vect(std::basic_ostream<CHAR, TRAITS>& os, vec<T, N> const& v, seq<Is...>) {
		using swallow = int[];
		(void)swallow { 0, (void(os << (Is == 0 ? "" : ", ") << v.ptr[Is]), 0)... };
	}
	template<class CHAR, class TRAITS,  size_t N, int... Is>
	void print_vect(std::basic_ostream<CHAR, TRAITS>& os, vec<float, N> const& v, seq<Is...>) {
		auto pbackup = os.precision();
		auto fbackup = os.flags();
		os.flags(fbackup | std::ios::fixed);
		os.precision(2);
		using swallow = int[];
		(void)swallow {
			0, (void(os << (Is == 0 ? "" : ", ") << v.ptr[Is] << "f"), 0)...
		};
		os.flags(fbackup);
		os.precision(pbackup);
	}

	template<class CHAR, class TRAITS, typename T, size_t N>
	std::basic_ostream<CHAR, TRAITS>& operator<<(std::basic_ostream<CHAR, TRAITS>& os, vec<T, N> const& v) 
	{
		os << "(";
		print_vect(os, v, gen_seq<static_cast<int>(N)>{});
		return os << ")";
	}

	template<typename LT, typename RT, size_t N, int... Is>
	constexpr vec<LT, N> vec_add(vec<LT, N> const &lhs, vec<RT, N> const &rhs, seq<Is...>) { return vec<LT, N>(lhs[Is]+rhs[Is]...); }
	template<typename LT, typename RT, size_t N, int... Is>
	vec<LT, N>& vec_addeq(vec<LT, N> &lhs, vec<RT, N> const &rhs, seq<Is...>) { 
		using swallow = int[];
		(void)swallow {
			0, (void(lhs[Is] += rhs[Is]), 0)...
		};
		return lhs;
		//return vec<LT, N>(lhs[Is] + rhs[Is]...); 
	}
	template<class T, int N, class F, int... Is> constexpr vec<T, N> transform(vec<T, N> const &lhs, vec<T, N>const &rhs, F f, seq<Is...>) { return vec<T, N>( f(lhs[Is], rhs[Is])... ); }
	template<class T, int N, class F> constexpr vec<T, N>  transform(vec<T, N> const &lhs, vec<T, N>const &rhs, F f) { return transform(lhs, rhs, f, gen_seq<N>{}); }
	template<class T, int N, class F, int... Is> constexpr vec<T, N>& transform(vec<T, N>  &lhs, vec<T, N>const &rhs, F f, seq<Is...>) { 
		using swallow = int[];
		(void)swallow {
			0, (void(f(lhs[Is], rhs[Is])), 0)...
		};
		return lhs;
	}
	template<class T, int N, class F> constexpr vec<T, N>&  transform(vec<T, N>  &lhs, vec<T, N>const &rhs, F f) { return transform(lhs, rhs, f, gen_seq<N>{}); }


	//template<typename T, size_t N> constexpr vec<T, N> operator+(const vec<T, N>& l, const vec<T, N>&r) { return vec_add(l, r, gen_seq<N>{}); }
	template<typename T, size_t N> constexpr vec<T, N> operator+(const vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T a, T b) { return a + b; }); }
	template<typename T, size_t N> constexpr vec<T, N>& operator+=(vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T& a, T b) { a += b; }); }
	template<typename T, size_t N> constexpr vec<T, N> operator-(const vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T a, T b) { return a - b; }); }
	template<typename T, size_t N> constexpr vec<T, N>& operator-=(vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T& a, T b) { a -= b; }); }


	void example_sum() {
		vec<float, 2> test1(30.0f, 20.0f);
		vec<float, 2> test2(12, 32);
		auto add_test = test1 + test2;
		std::cout << add_test;
		add_test += test1;
		std::cout  << " more " << add_test;
		//constexpr auto c = sum(a, b);
		//auto test = sum(a, b);
		std::cout << std::endl;
		std::cout << "End of example" << std::endl;
	}
};

void testVec() {
	
	/*
	
	umath::vec<float, 3> test;
	umath::vec<float, 3> test3;
	umath::vec<float, 2> test2;
	umath::vec<float, 2> test22;
	constexpr auto isv = umath::is_vector<float>::value;
	constexpr auto isv2 = umath::is_vector<umath::vec<float, 2>>::value;
	*/
	//umath::vec2<float> vtest(3.2f, 43.3f);
//	float test[2] = { vtest.ptr()[0],vtest.ptr()[1] };
	//test.compare(test3,6.7f);


	//test22.compare(test22);
//printf("wee");

}



namespace global {
	sf::RenderWindow& getWindow() {
		assert(s_window);
		return *s_window;
	}
	ex::EventManager& getEventManager() { return s_app->events; }
	ex::EntityManager& getEntities() { return s_app->entities; }
	ex::SystemManager& getSystems() { return s_app->systems; }
};


console::VT00Window winerr(0, 15, 50, 10);

namespace ex = entityx;
void gameLoop() {
	sf::RenderWindow& window = global::getWindow();
	//auto& systems = global::getSystems();
	auto& events = global::getEventManager();
	sf::Clock clock;
	bool isPlaying = false;
	auto font = UFont::LoadUndertaleFont(4);
	sf::View view(sf::FloatRect(0, 0, 640, 480));

//	UndertaleSprite raw_sprite(1986);
//	UndertaleSprite raw_sprite2(1982);

	//SpriteEnity teste = SpriteEnity::create(1986)

//	Player& player = systems.system<PlayerOverWorldSystem>()->getPlayer();
	//auto render_system = systems.system<RenderSystem>();
	s_app->LoadRoom(52);
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
				switch (event.key.code) {
				case sf::Keyboard::Y:
					(std::ostream&)winerr << "Testing" << std::endl;
					break;
				case sf::Keyboard::Down:
					//console::test_vt("\33[B");
					break;
				case sf::Keyboard::Up:
				//	console::test_vt("\33[A");
					break;
				case sf::Keyboard::Left:
				//	console::test_vt("\33[D");
					break;
				case sf::Keyboard::Right:
				//	console::test_vt("\33[C");
					break;
				case sf::Keyboard::Return:
				//	console::test_vt("Testing");
					break;
				case sf::Keyboard::Num9:
					
					s_app->LoadRoom(s_app->getRoomIndex()-1);
				break;
				case sf::Keyboard::Num0:
					s_app->LoadRoom(s_app->getRoomIndex() +1);
				break;
				}
			}
			PlayerControl::update_keys(event);
		}
		//	window.clear();
		
		sf::Time elapsed = clock.restart();
		s_app->update(elapsed.asSeconds());
		//	window.display();
	}
}
const char* test_files[] = { "colors.vt", "decdemo.vt","torture.vt", "xmas.vt" } ;
#include <chrono>
#include <fstream>

class QuickTimer {
	std::chrono::time_point<std::chrono::system_clock> _start;
public:
	QuickTimer() : _start(std::chrono::system_clock::now()) {}
	void restart() { _start = std::chrono::system_clock::now(); }
	double elapsed() const { return std::chrono::duration<double>(std::chrono::system_clock::now() - _start).count(); }
};
void test_vt() {
	
	QuickTimer timer;
	static constexpr int debug_line = 0;// 645; // champain glass
	std::ifstream file(test_files[0]);
	int line = 1;
	int pos = 0;
	con::parm_csi_command<int,int> tt('m', 31, 42);
//	std::cout << tt << "This is a happy test" << std::endl;
	console::VT00Window window(10, 10, 50, 50);
	window.scroll(false);
	std::ostream& stream = window;
	stream << "This is a test" << std::endl;
	stream << "this is also a test" << std::endl;
	window.paint();
	//vt100::init();
	while (line < debug_line && !file.eof()) {
		int c = file.get();
		if (c == '\n') line++;
	}
	while (!file.eof()) {
		assert(file.good() && !file.eof());
		if (timer.elapsed() > 0.001) {
			timer.restart();
			int c = file.get();
			if (c == '\n') {
				line++; pos = 0;
			}
			else pos++;
			stream << (char)c;
		//	std::cout << (char)c;
			window.paint();
		}
	}
	while (true) {} // loop

}
int main(int argc, const char* argv[]) {
	
	console::init();
	//test_vt();
	//array_helpers::example_sum();
	if (argc != 2 || !Global::LoadUndertaleDataWin(argv[1])) return -1;
	
	const int gameWidth = 800;
	const int gameHeight = 600;

	// Create the window of the application
	s_window.reset(new sf::RenderWindow(sf::VideoMode(800, 600, 32), "SFML Pong", sf::Style::Titlebar | sf::Style::Close));
	s_app.reset(new Application(*s_window));
//	s_app->init(*s_app.get());
	
	gameLoop();
	// we got to run this to delete all the loaded textures we have or visual studio blows a fit
	s_window->close();
	Global::DestroyEveything();

	s_app.release();
	s_window.release();


	return 0;
}