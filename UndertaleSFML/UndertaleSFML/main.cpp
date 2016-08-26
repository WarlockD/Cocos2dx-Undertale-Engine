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
static std::unique_ptr<Application> s_app;
#if 0
namespace umath {
	// ALIAS TEMPLATE bool_constant

	template<class _Iter>
	void_t test;
	std::void_t<> test;

	template<typename T, size_t DIM> struct vector_traits<T, void_t<
		typename _Iter::iterator_category,
		typename _Iter::value_type,
		typename _Iter::difference_type,
		typename _Iter::pointer,
		typename _Iter::reference
	> >
	{	// defined if _Iter::* types exist
		typedef typename _Iter::iterator_category iterator_category;
		typedef typename _Iter::value_type value_type;
		typedef typename _Iter::difference_type difference_type;

		typedef typename _Iter::pointer pointer;
		typedef typename _Iter::reference reference;
	};

	// TEMPLATE CLASS iterator_traits
	
	template<class, class = void> struct vector_traits_base {};// empty for non-vectors
	template<class V> struct vector_traits_base<V, std::void_t<typename V::type, typename V::base_type>>
	{	// defined if types exist
		typedef	typename V::type type;
		typedef	typename V::base_type base_type;
	};
	template<class V>struct vector_traits : vector_traits_base<V> {};	// get traits from iterator _Iter, if possible

	template<class T, class = void> struct is_vector : std::false_type { static constexpr size_t dimensions = 0; };// default definition
	template<class T> struct is_vector<T, std::void_t<typename vector_traits<T>::base_type>> : std::true_type { };

	template<typename T, size_t DIM> struct vec_base { 
		typedef T type; 
		typedef vec_base<T, DIM> base_type; 
		T ptr[DIM];  
		static constexpr size_t dimensions = DIM; 
		T operator[](size_t i) const { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		T& operator[](size_t i) { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		inline bool compare(const base_type &r) const { std::equal(&ptr[0], &ptr[DIM], &r.ptr[0], &r.ptr[DIM], [](const T a, const T b) { return a == b; }); }
		inline  bool compare(const base_type &r, T epsilon) const {
			std::equal(&ptr[0], &ptr[DIM], &r.ptr[0], &r.ptr[DIM], [epsilon](const T a, const T b) { return umath::compare(a, b, epsilon); });
		}
		bool operator==(const base_type &r) const { return compare(r); }
		bool operator!=(const base_type &r) const { return !compare(r); }
		
	};
	template<typename T> struct vec_base<T, 2> { 
		typedef typename T type; 
		typedef typename vec_base<T, 2> base_type; 
		static constexpr size_t dimensions = 2;
		union { struct { T x; T y; }; T ptr[2]; }; 
		T operator[](size_t i) const { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		T& operator[](size_t i) { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		inline  bool compare(const base_type &r) const { return x == r.x && y == r.y; }
		inline  bool compare(const base_type &r, T epsilon) const { return umath::compare(x, r.x, epsilon) && umath::compare(y, r.y, epsilon); }
		bool operator==(const base_type &r) const { return compare(r); }
		bool operator!=(const base_type &r) const { return !compare(r); }
		base_type& operator+=(const base_type&r) { x.}
	};
	template<typename T> struct vec_base<T, 3> { 
		typedef typename T type; 
		typedef typename vec_base<T, 3> base_type; 
		static constexpr size_t dimensions = 3;
		union { struct { T x; T y; T z; }; T ptr[3]; }; 
		T operator[](size_t i) const { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		T& operator[](size_t i) { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		inline  bool compare(const base_type &r) const { return x == r.x && y == r.y && z == r.z; }
		inline  bool compare(const base_type &r, T epsilon) const { return umath::compare(x, r.x, epsilon) && umath::compare(y, r.y, epsilon) && umath::compare(z, r.z, epsilon); }
		bool operator==(const base_type &r) const { return compare(r); }
		bool operator!=(const base_type &r) const { return !compare(r); }
	};
	template<typename T> struct vec_base<T, 4> { 
		typedef typename T type; 
		typedef typename vec_base<T, 4> base_type; 
		static constexpr size_t dimensions = 4;
		union { struct { T x; T y; T z; T w; }; T ptr[4]; }; 
		T operator[](size_t i) const { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		T& operator[](size_t i) { assert((i >= 0) && (i < dimensions)); ptr[i]; }
		inline bool compare(const base_type &r) const { return x == r.x && y == r.y && z == r.z && z == r.w; }
		inline bool compare(const base_type &r, T epsilon) const { umath::compare(x, r.x, epsilon) && umath::compare(y, r.y, epsilon) && umath::compare(z, r.z, epsilon) && umath::compare(w, r.w, epsilon); }
		bool operator==(const base_type &r) const { return compare(r); }
		bool operator!=(const base_type &r) const { return !compare(r); }
	};

	template<typename T, size_t DIM>
	struct vec : public vec_base<T, DIM>{
		
#if 0
		template<typename O>
		typename std::enable_if<std::is_same<O, vector_type>::value && (DIM == 2), bool>::type
			inline compare(const O &r, const T epsilon) const
		{
			return umath::compare(x, r.x, epsilon) && umath::compare(y, r.y, epsilon);
		}
		template<typename O>
		typename std::enable_if<std::is_same<O, vector_type>::value && (DIM == 3), bool>::type
			inline compare(const O &r, const T epsilon) const
		{
			return umath::compare(x, r.x, epsilon) && umath::compare(y, r.y, epsilon) && umath::compare(z, r.z, epsilon);
		}
		template<typename O>
		typename std::enable_if<std::is_same<O, vector_type>::value && (DIM == 4), bool>::type
			inline compare(const O &r, const T epsilon) const
		{
			return umath::compare(x, r.x, epsilon) && umath::compare(y, r.y, epsilon) && umath::compare(z, r.z, epsilon) && umath::compare(w, r.w, epsilon);
		}
#endif
	};
};
#endif
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


namespace ex = entityx;
void gameLoop() {
	sf::RenderWindow& window = global::getWindow();
	auto& systems = global::getSystems();
	auto& events = global::getEventManager();
	sf::Clock clock;
	bool isPlaying = false;
	auto font = UFont::LoadUndertaleFont(4);
	sf::View view(sf::FloatRect(0, 0, 640, 480));
//	window.setView(view);
//	auto writer = obj_dialoger::create();
	//writer.setFont(4);
//	writer->setConfig();
//	writer->setText("* mind your p \\Yand\n\r q's and I");
//	writer->start_typing();
//	//UndertaleLabel debug_label;
	
	UndertaleSprite raw_sprite(1986); 
	UndertaleSprite raw_sprite2(1982);

	//SpriteEnity teste = SpriteEnity::create(1986)

	ex::Entity sprite = global::getEntities().create();
	sprite.assign<Body>();
	sprite.component<Body>()->setPosition(20, 50);
	sprite.component<Body>()->setScale(2.0, 2.0);
	//sprite.assign<RenderableRef>(raw_sprite);
//	sprite.assign<LightRenderable>(raw_sprite);
	//sprite.assign<Animation>(0.25f);

	ex::Entity sprite2 = global::getEntities().create();
	sprite2.assign<Body>();
	sprite2.component<Body>()->setPosition(20, 80);
	sprite2.component<Body>()->setPosition(20, 60);
	sprite2.component<Body>()->setScale(2.0, 2.0);
	//sprite2.assign<RenderableRef>(raw_sprite2);
//	sprite2.assign<LightRenderable>(raw_sprite);
	//sprite2.component<Animation>() = Animation(0.25f;

	// don't want to invalidate the sprite entry
	//SpriteEnity sprite(&global::getEntities(), sprite_id.id());
	//sprite.setSpriteIndex(sprite_index);
	///sprite._sprite_index = 0;
	//sprite._body = sprite.assign<Body>().get();
	//return sprite;

	s_app->init(*s_app.get());
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
			events.emit<SystemEvent>(event);
		}
		window.clear();
		sf::Time elapsed = clock.restart();
		systems.update_all(elapsed.asSeconds());
		window.display();
	}
}
int main(int argc, const char* argv[]) {
	testVec();
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