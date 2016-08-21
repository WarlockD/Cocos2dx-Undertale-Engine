#pragma once
#include <cassert>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <atomic>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <type_traits>
#include <SFML\Graphics.hpp>
#include <map>  // Component-entity system in 16 lines of C++11. 2013 rlyeh, MIT licensed
#include <set>  // Code fragment from kult engine     -    https://github.com/r-lyeh/kult
#include "kult.hpp"
#include <entityx/entityx.h>
namespace ex = entityx;

// This is the basic rendering class.  None of the verts are transformed, no position data, nothing.  Just the raw verts
// has some simple helper functions
// I just have been using std::vector<sf::vertex> eveywhere os might as well subclass it
class RawVertices :  public sf::Drawable {
public:
	typedef std::vector<sf::Vertex> vector;
	typedef typename vector::size_type size_type;
	typedef typename vector::value_type value_type;
	typedef typename vector::difference_type difference_type;
	typedef typename vector::const_iterator const_iterator;
	typedef typename vector::iterator iterator;
	typedef typename vector::const_pointer const_pointer;
	typedef typename vector::const_reference const_reference;
	typedef typename vector::pointer pointer;
	typedef typename vector::reference reference;
	RawVertices() { }
	explicit RawVertices(const vector& verts) : _verts(verts) {}
	explicit RawVertices(vector&& verts) : _verts(std::move(verts)) {}
	explicit RawVertices(const sf::VertexArray& verts) { insert(begin(), verts); }
	iterator begin() { return _verts.begin(); }
	iterator end() { return _verts.end(); }
	const_iterator begin() const { return _verts.begin(); }
	const_iterator end() const { return _verts.end(); }
	reference operator[](size_t index) { return _verts[index]; }
	const_reference operator[](size_t index) const { return _verts[index]; }
	size_t size() const { return _verts.size(); }
	pointer data() { return _verts.data(); }
	void resize(size_type size) { _verts.resize(size); }
	void reserve(size_type size) { _verts.reserve(size); }
	const_pointer data() const { return _verts.data(); }
	void push_back(const value_type& v) { _verts.push_back(v); }
	void push_back(const sf::VertexArray& verts) { insert(end(), verts); }
	void push_back(const vector& verts) { _verts.insert(_verts.end(), verts.begin(), verts.end()); }
	void push_back(const RawVertices& verts) { _verts.insert(_verts.end(), verts._verts.begin(),verts._verts.end()); }
	void clear() { _verts.clear(); }
	void insert(const_iterator where, const value_type& v) { _verts.insert(where, v); }
	void insert(const_iterator where, const RawVertices& v) { _verts.insert(where, v._verts.begin(), v._verts.end()); }
	void insert(const_iterator where, const sf::VertexArray& verts);
	RawVertices& operator=(vector&& v) { _verts = std::move(v); return *this; }
	RawVertices& operator=(const vector& v) { _verts = v; return *this; }
	RawVertices& operator=(const sf::VertexArray& v) { _verts.clear(); push_back(v); return *this; }
	RawVertices& operator+=(const vector& v) { push_back(v); return *this; }
	RawVertices& operator+=(const sf::VertexArray& v) { push_back(v); return *this; }
	RawVertices& operator+=(const RawVertices& v) { push_back(v); return *this; }
	RawVertices& operator+=(const value_type& v) { push_back(v); return *this; }
protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	vector _verts;
};

namespace global {
	sf::RenderWindow& getWindow();
	ex::EventManager& getEventManager();
	ex::EntityManager& getEntities();

	constexpr float smallest_pixel = 0.001f; // smallest movement of a pixel
	extern const std::string empty_string; // used for empty const std::string refrences

	inline bool AlmostEqualRelative(float A, float B)
	{
		// Calculate the difference.
		float diff = std::fabs(A - B);
		A = std::fabs(A);
		B = std::fabs(B);
		// Find the largest
		float largest = (B > A) ? B : A;
		if (diff <= largest * smallest_pixel) return true;
		return false;
	}
	 inline bool AlmostEqualRelative(const sf::Vector2f& l, const sf::Vector2f& r) {
		 return AlmostEqualRelative(l.x, r.x) && AlmostEqualRelative(l.y, r.y);
	 }

	 std::array<sf::Vertex, 6> CreateRectangle(const sf::FloatRect& rect, sf::Color fill_color);
	 void InsertRectangle(sf::Vertex  * verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles); 
	 void InsertRectangle(RawVertices& verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	 void InsertRectangle(sf::VertexArray& verts, const sf::FloatRect& rect, sf::Color fill_color);// convert
};



template<typename T> class StopWatch {
public:
	typedef T Type;
	static constexpr Type Zero = ((Type)0);
	static constexpr Type One = ((Type)1); // used for incrment
protected:
	Type _toAmount;
	Type _current;
public:
	explicit StopWatch() : _toAmount(Zero), _current(Zero) {}
	explicit StopWatch(Type init) : _toAmount(init), _current(Zero) {}
	template<class B, class = typename std::enable_if<std::is_convertible<B, T>::value, B>::type>
	void setTime(B v) { _toAmount = static_cast<T>(v); }
	template<class B, class = typename std::enable_if<std::is_convertible<B, T>::value, B>::type>
	void addCurrent(B v) { _current += static_cast<T>(v); }
	template<class B, class = typename std::enable_if<std::is_convertible<B, T>::value, B>::type>
	void setCurrent(B v) { _current += static_cast<T>(v); }
	Type getTime() const { return _toAmount; }
	Type getCurrent() const { return _current; }
	void reset() { _current = Zero; }
	bool test() const { return _current >= _toAmount; }
	bool test_then_reset() {
		if (test()) { reset(); return true; }
		else return false;
	}
	template<class B>
	bool test_then_reset(B v) {
		addCurrent(v);
		if (test()) { reset(); return true; }
		else return false;
	}
};

class LockingObject {
	mutable std::atomic_flag _flag = ATOMIC_FLAG_INIT; // want it mutable so it can be used in cost functions that need a lock
public:
	// We don't want this thing to ever copy the _flag
	LockingObject()  {  }
	LockingObject(const LockingObject& copy)  {  }
	LockingObject(LockingObject&& move)  {  }
	LockingObject& operator=(const LockingObject& copy) { return *this; }
	LockingObject& operator=(LockingObject&& move) { return *this; }
	~LockingObject() { unlock(); }
	void lock() const { while (_flag.test_and_set(std::memory_order_acquire)); }
	void unlock() const { _flag.clear(std::memory_order_release); }// release lock
	class SafetyLock {
		const LockingObject* _obj;
	public:
		// can only be moved, never copyied
		SafetyLock() : _obj(nullptr) {}
		SafetyLock(const LockingObject& obj) : _obj(&obj) { _obj->lock(); }
		SafetyLock(const SafetyLock& copy) = delete;
		SafetyLock& operator=(const SafetyLock& copy) = delete;
		SafetyLock(SafetyLock&& move) : _obj(move._obj) { move._obj = nullptr; }
		SafetyLock& operator=(SafetyLock&& move) { _obj = move._obj; move._obj = nullptr; return *this; }
		~SafetyLock() { if(_obj) _obj->unlock(); }
	};
	// Returns an object that auto locks and auto unlocks
	// to make it simple just do auto lock = safeLock()
	SafetyLock safeLock() const { return SafetyLock(*this); }
};


inline std::ostream& operator<<(std::ostream& os, const sf::Vector2f& v) {
	os << std::fixed << std::setprecision(3) << '('  << v.x << ',' << v.y << ')';
	return os;
}

template<class T>
class LightWeightRef {
public:
	typedef LightWeightRef<T> light_weight_type;
	typedef T element_type;
	typedef T* pointer;
	typedef const T* const_pointer;
private:
	pointer _ref;
public:
	LightWeightRef() : _ref(nullptr) {}
	template<class C> explicit LightWeightRef(C* ptr) : _ref(ptr) {}
	template<class C> explicit LightWeightRef(C& ptr) : _ref(&ptr) {}
	
	//explicit LightWeightRef(pointer* ptr) : _ref(ptr) {}
	//explicit LightWeightRef(pointer& ptr) : _ref(&ptr) {}
	virtual ~LightWeightRef() { _ref = nullptr; }
	pointer get() { return _ref; }
	element_type& operator*() const { return *get(); }
	pointer operator->() const { return _ref; }
	explicit operator bool() const { return _ref != nullptr; }

	//template<class C> explicit LightWeightRef(C* ptr) : _ref(ptr) {}
	//template<class C> explicit LightWeightRef(C& ptr) : _ref(&ptr) {}
	template<class B, class = typename std::enable_if<std::is_base_of<T, B>::value, , T>::type>
	LightWeightRef<T>& operator=(const LightWeightRef<B>& other) {
		_ref = other._ref;
		return *this;
	}
	template<class B, class = typename std::enable_if<std::is_base_of<T, B>::value, , T>::type>
	LightWeightRef<T>& operator=(LightWeightRef<B>&& other) {
		_ref = other._ref;
		other._ref = nullptr;
		return *this;
	}

	inline LightWeightRef& operator=(pointer ptr) { _ref = ptr; return *this; }
	inline LightWeightRef& operator=(element_type& ref) { _ref = &ptr; return *this; }

	class WeakRef {
		light_weight_type& _ref;
	public:
		WeakRef(light_weight_type& ref) : _ref(ref) {}
		inline operator bool() const { return _ref._ref != nullptr; }
		const const_pointer*  operator->() const { return _ref; }
	};
	inline WeakRef weakRef() const { return WeakRef(*this); }
};



#if 0
// http://stackoverflow.com/questions/87372/check-if-a-class-has-a-member-function-of-a-given-signature
// really nice idea, works well too
template<typename T>
struct HasUpdateMethod
{
	template<typename U, void(U::*)(float)> struct SFINAE {};
	template<typename U> static char Test(SFINAE<U, &U::update>*);
	template<typename U> static int Test(...);
	static const bool value = sizeof(Test<T>(0)) == sizeof(char);
};
namespace SmalleEntiySystem {



	enum { JOIN, MERGE, EXCLUDE };
	typename std::set<unsigned> t_entities;
	template<typename T> set&system() {
		static t_entities entities; return entities;
	}
	template<typename T, int MODE>t_entities subsystem(const t_entities &B) {
		t_entities newset;
		const t_entities&A = system<T>();
		if (MODE == MERGE) {
			newset = B; for (auto&id : A)newset.insert(id);
		}
		else if (MODE == EXCLUDE) { newset = B; for (auto&id : A)newset.erase(id); }
		else if (A.size()
			< B.size()) {
			for (auto&id : A)if (B.find(id) != B.end())newset.insert(id);
		}
		else {
			for (auto&id : B)if (
				A.find(id) != A.end())newset.insert(id);
		}return newset;
	}template<typename T>std::map<unsigned, T>&components() {
		static std::map<unsigned, T>objects; return objects;
	}
	template<typename T>bool has(unsigned id) { return components<T>().find(id) != components<T>().end(); }
	template<typename T>decltype(T::value_type)&get(unsigned id) {
		static decltype(T::value_type)invalid, reset; return has<T>(id) ? components<T>()[id].value_type : invalid = reset;
	}
	template<typename T>decltype(T::value_type)&add(unsigned id) { return system<T>().insert(id), components<T>()[id] = components<T>()[id], get<T>(id); }
	template<typename T>bool del(unsigned id) { return add<T>(id), components<T>().erase(id), system<T>().erase(id), !has<T>(id); }
	template<typename T, int>struct component { T value_type; };

};
#endif




namespace console {

};



namespace logging {
	void init_cerr();
	void init_cout();
	bool init();
	void error(const std::string& message);
};