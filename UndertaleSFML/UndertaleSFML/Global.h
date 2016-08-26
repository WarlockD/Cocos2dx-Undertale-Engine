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
#include "Math.h"
namespace ex = entityx;


namespace global {
	sf::RenderWindow& getWindow();
	ex::EventManager& getEventManager();
	ex::EntityManager& getEntities();
	void convert(const std::vector<sf::Vertex>& from, sf::PrimitiveType from_type, std::vector<sf::Vertex>& to, sf::PrimitiveType to_type, bool clear = false);
	std::vector<sf::Vertex> convert(const std::vector<sf::Vertex>& from, sf::PrimitiveType from_type, sf::PrimitiveType to_type);
	void convert(const sf::VertexArray& from, std::vector<sf::Vertex>& to, sf::PrimitiveType to_type, bool clear = false);
	std::vector<sf::Vertex> convert(const sf::VertexArray& from, sf::PrimitiveType to_type);

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
	inline int8_t hash_color_to_8bits(const sf::Color& color) {
		uint32_t i = color.toInteger();
		return ((color.r / 4) << 6) | ((color.g / 4) << 4) | ((color.b / 4) << 2) | (color.a / 4);
	}
	inline int16_t hash_float_to_12bits(float x) {
		return (0xFFF & static_cast<int16_t>(x));
	}
	inline int16_t hash_vector_to_12bit(const sf::Vector2f& v) {
		return hash_float_to_12bits(v.x) ^ hash_float_to_12bits(v.y);
	}
	inline size_t hash_vector(const sf::Vector2f& v) {
		return static_cast<int16_t>(v.x) | (static_cast<int16_t>(v.y) << 16);
	}
	// I am figuring the positions are most important followed by the texture positions then color
	// in eather case I am kind of proud of this thing
	inline size_t hash_vertex(const sf::Vertex& v) {
		return hash_vector_to_12bit(v.position) << 20 | hash_vector_to_12bit(v.position) << 8 | hash_color_to_8bits(v.color);
	}
};

// some standard defines
namespace std { // try to detect if iliterator
	template<typename T, typename = void>
	struct is_iterator
	{
		static constexpr bool value = false;
	};
	template<typename T>
	struct is_iterator<T, typename std::enable_if<!std::is_same<typename std::iterator_traits<T>::value_type, void>::value>::type>
	{
		static constexpr bool value = true;
	};
	// hash specializtiong for vertex and vectors
	template <> struct hash<sf::Vector2f> {
		typedef sf::Vector2f argument_type;
		typedef size_t result_type;
		size_t operator()(const argument_type& v) const { return global::hash_vector(v); }
	};

	template <> struct hash<sf::Vertex> {
		typedef sf::Vertex argument_type;
		typedef size_t result_type;
		size_t operator()(const argument_type& v) const {
			return global::hash_vector_to_12bit(v.position) << 20 & global::hash_vector_to_12bit(v.texCoords) << 8 & global::hash_color_to_8bits(v.color);
		}
	};
};

template<typename T>
struct almost_equal_to {
	constexpr bool operator()(const T &l, const T &r) const { return std::equal_to<T>(l, r); }
};



template<> struct almost_equal_to<float> {
	static constexpr float maxdiff = 0.0001f;
	bool operator()(const float & l, const float& r) const
	{
		return umath::compare(l, r, maxdiff);
	}
};


template<> struct almost_equal_to<sf::Vector2f> {
	 bool operator()(const sf::Vector2f& l, const sf::Vector2f& r) const
	{
		return almost_equal_to<float>()(l.x, r.x) && almost_equal_to<float>()(l.y, r.y);
	}
};

template<> struct almost_equal_to<sf::Vertex> {
	 bool operator()(const sf::Vertex& l, const sf::Vertex& r) const
	{
		return l.color.toInteger() == r.color.toInteger() && almost_equal_to<sf::Vector2f>()(l.texCoords, r.texCoords) && almost_equal_to<sf::Vector2f>()(l.position, r.position);
	}
};

template<typename T>
struct almost_zero {
	bool operator()(const T &l) const { return almost_equal_to<T>()(l, static_cast<T>(0)); }
};
template<>
struct almost_zero<sf::Vector2f> {
	bool operator()(const sf::Vector2f &l) const { return almost_zero<float>()(l.x) && almost_zero<float>()(l.y); }
};

class ChangedCass {
public:
	ChangedCass() : _changed(true) {} // set true caues at the start of the class it is changed
	bool changed() const { return _changed; }
protected:
	mutable bool _changed;
	bool changed(bool value) const { bool ret = _changed; _changed = value; return ret; }
	friend class RenderableCache; // all the friends that can reset _changed
};

// generic iliterator that uses a position and the "at" function of a class
template<class C, typename T>
class generic_iterator {
public:
	typedef typename generic_iterator<C,T> iterator;
	typedef typename C container;
	typedef typename container& container_refrence;
	typedef typename T value_type;
	typedef typename std::bidirectional_iterator_tag iterator_category;
	typedef typename size_t size_type;    // C::size_type size_type; no gurnetee that C will have type trates
	typedef typename ptrdiff_t difference_type; // C::difference_type difference_type; or diffrence type for that matter
	typedef typename std::conditional<std::is_const<T>::value, typename std::remove_const<T>::type * , T*>::type pointer;
	typedef typename std::conditional<std::is_const<T>::value, typename std::remove_const<T>::type & , T&>::type reference;
	typedef typename std::conditional<std::is_const<T>::value, T*, const T*>::type const_pointer;
	typedef typename std::conditional<std::is_const<T>::value, T&, const T&>::type const_reference;
	inline iterator &operator ++() { ++_pos; return *this; }
	inline iterator &operator --() { --_pos;; return *this; }
	inline iterator operator ++(int) { ++_pos; return *this; }
	inline iterator operator --(int) { --_pos;; return *this; }
	inline bool operator ==(const iterator &other) const { return _pos == other._pos; }
	inline bool operator !=(const iterator &other) const { return _pos != other._pos; }
	inline bool operator <(const iterator &other) const { return _pos< other._pos; }
	inline bool operator >(const iterator &other) const { return _pos> other._pos; }
	inline bool operator <=(const iterator &other) const { return _pos <= other._pos; }
	inline bool operator >=(const iterator &other) const { return _pos >= other._pos; }
	inline iterator & operator +(const difference_type &add) const { const_iterator copy(*this); copy += add; return copy; }
	inline iterator & operator -(const difference_type &add) const { const_iterator copy(*this); copy -= add; return copy; }
	inline iterator & operator +=(const difference_type &add) { _pos += add; return *this; }
	inline iterator & operator -=(const difference_type &add) { _pos -= add; return *this; }
	inline const_reference operator*() const { return _ref.at(_pos); }
	inline const_pointer operator->() const { return &_ref.at(_pos); }
	template<typename Q> typename std::enable_if<!std::is_const<Q>::value, reference>::type inline operator*() { return _ref.at(_pos); }
	template<typename Q> typename std::enable_if<!std::is_const<Q>::value, pointer>::type inline operator->() { return &_ref.at(_pos); }
	generic_iterator(container_refrence ref, difference_type pos) : _pos(pos), _ref(ref) {}
protected:
	difference_type _pos;
	container_refrence _ref;
};


// This is a wraper for std::vector<sf::Vertex> being that while I like sf::VertexArray, I like
// the vector interface more.  We template this and created a bunch of conversions
// with overloaded functions
class RawVertices :  public sf::Drawable {
public:
	// vector wrapper
	// Traits
	typedef std::vector<sf::Vertex> vector;
	typedef typename vector::size_type size_type;
	typedef typename vector::value_type value_type;
	typedef typename vector::difference_type difference_type;
	typedef typename vector::const_iterator const_iterator;
	typedef typename vector::iterator iterator;
	typedef typename vector::reverse_iterator reverse_iterator;
	typedef typename vector::const_reverse_iterator const_reverse_iterator;
	typedef typename vector::const_pointer const_pointer;
	typedef typename vector::const_reference const_reference;
	typedef typename vector::pointer pointer;
	typedef typename vector::reference reference;
	typedef typename std::pair<iterator, iterator> range_iterator;
	typedef typename std::pair<const_iterator, const_iterator> const_range_iterator;

	// constructors
	RawVertices() : _ptype(sf::PrimitiveType::Triangles) {} // we always default to triangles
	RawVertices(const RawVertices& copy) = default;
	RawVertices(RawVertices&& copy) = default;


	RawVertices(sf::PrimitiveType ptype) : _ptype(ptype) {}
	RawVertices(const vector& verts) : _verts(verts), _ptype(sf::PrimitiveType::Triangles) {}
	RawVertices(sf::PrimitiveType ptype, const vector& verts) : _verts(verts), _ptype(ptype) {}
	RawVertices(vector&& verts) : _verts(std::move(verts)), _ptype(sf::PrimitiveType::Triangles) {}
	RawVertices(sf::PrimitiveType ptype, vector&& verts) : _verts(std::move(verts)), _ptype(ptype) {}
	template<class IT, class = typename enable_if<std::is_iterator<IT>::value, void>::type>
	RawVertices(IT begin, IT end) : _verts(begin, end), _ptype(sf::PrimitiveType::Triangles) {}
	template<class IT, class = typename enable_if<std::is_iterator<IT>::value, void>::type>
	RawVertices(sf::PrimitiveType ptype, IT begin, IT end) : _verts(begin, end), _ptype(ptype) {}


	RawVertices(const sf::VertexArray& verts) : _ptype(verts.getPrimitiveType()), _verts(global::convert(verts, primitive_type())) {}

	// ok, these two, might never want to use.  SUPER HACKY.  I put them in global.cpp because I just don't want anyone to see..
	// basicly I recast the verts, skip the vtable, then extract the vector directly from vertexArray..  yea:P
	RawVertices(sf::VertexArray&& verts);
	RawVertices& operator=(sf::VertexArray&& right);

	// vector function wrap
	reference operator[](size_t index) { return _verts[index]; }
	const_reference operator[](size_t index) const { return _verts[index]; }
	reference at(size_t index) { return _verts.at(index); }
	const_reference at(size_t index) const { return _verts.at(index); }
	size_t size() const { return _verts.size(); }
	pointer data() { return _verts.data(); }
	void resize(size_type size) { _verts.resize(size); }
	void reserve(size_type size) { _verts.reserve(size); }
	const_pointer data() const { return _verts.data(); }
	void push_back(const value_type& v) { _verts.push_back(v); }
	void push_back(value_type&& v) { _verts.push_back(v); }
	template<class... Args> void emplace_back(Args&&... args) { _verts.emplace_back(std::forward<Args>(args)...); }
	template<class... Args> iterator emplace(const_iterator where, Args&&... args) { return _verts.emplace(where, std::forward<Args>(args)...); }
	void pop_back() { _verts.pop_back(); }
	reference front() { return _verts.front(); }
	reference back() { return _verts.back(); }
	iterator begin() { return _verts.begin(); }
	iterator end() { return _verts.end(); }
	reverse_iterator rbegin() { return _verts.rbegin(); }
	reverse_iterator rend() { return _verts.rend(); }
	const_reference front() const { return _verts.front(); }
	const_reference back() const { return _verts.back(); }
	const_iterator begin() const { return _verts.begin(); }
	const_iterator end() const { return _verts.end(); }
	const_reverse_iterator rbegin() const { return _verts.rbegin(); }
	const_reverse_iterator rend() const { return _verts.rend(); }
	const_iterator cbegin() const { return _verts.begin(); }
	const_iterator cend() const { return _verts.end(); }
	void clear() { _verts.clear(); }
	iterator insert(const_iterator where, value_type&& val) { return _verts.insert(where, val); }
	iterator insert(const_iterator where, const value_type& val) { return _verts.insert(where, val); }
	iterator insert(const_iterator where, std:: initializer_list<value_type> list) { return _verts.insert(where, list); }
	iterator erase(const_iterator first, const_iterator last) { return _verts.erase(first, last); }
	iterator erase(const_iterator at) { return _verts.erase(at); }
	void swap(RawVertices& other) { _verts.swap(other._verts); std::swap(_ptype, other._ptype); }

	template<class IT> 
	typename std::enable_if<std::is_iterator<IT>::value, iterator>::type 
		insert(const_iterator where, IT first, IT last) { return _verts.insert(where, first, last); }
	template<class IT>
	typename std::enable_if<std::is_iterator<IT>::value, void>::type 
		assign(IT first, IT last) { _verts.assign(first, last); }

	// extensions
	template<class IT>
	typename std::enable_if<std::is_iterator<IT>::value, void>::type
		append(IT begin, IT end) { _verts.insert(_verts.begin(), begin, end); }

	void append(const vector& verts) { _verts.insert(_verts.begin(), verts.begin(), verts.end()); }
	void append(const RawVertices& verts) { global::convert(verts._verts, verts.primitive_type(), _verts, primitive_type()); }
	void append(const sf::VertexArray& verts) { global::convert(verts, _verts, primitive_type()); }

	range_iterator range(size_type start, size_type end) { return std::make_pair(_verts.begin() + start, _verts.end() + end); }
	const_range_iterator range(size_type start, size_type end) const { return std::make_pair(_verts.begin() + start, _verts.end() + end); }

	// we want to convert as there is no way to change this type once its made without doing an explicit construction
	RawVertices& operator=(const sf::VertexArray& right) { _verts.clear(); append(right); return *this; }
	RawVertices& operator=(const RawVertices& right) { _verts.clear(); append(right); return *this; }
	RawVertices& operator+=(const sf::VertexArray& right) { append(right); return *this; }
	RawVertices& operator+=(const RawVertices& right) { append(right); return *this; }
	RawVertices& operator*=(const sf::Transform& t) { transform(t); return *this; }
	RawVertices& operator=(RawVertices&& right) { 
		if (primitive_type() == right.primitive_type()) {
			_verts = std::move(right._verts);
		}
		else {
			// else just copy,bleh
			_verts.clear();
			append(right);
		}
		return *this;
	}
	
	void transform(const sf::Transform& transform) { for (auto& v : _verts) v.position = transform * v.position ; }
	RawVertices transform_copy(const sf::Transform& transform) const {
		RawVertices copy(*this);
		copy *= transform;
		return copy;
	}
	void fill_color(const sf::Color& color) { for (auto& v : _verts) v.color = color; }
	sf::FloatRect bounds() const {
		sf::Vector2f vmin;
		sf::Vector2f vmax;
		for (auto& it : _verts) {
			auto& v = it.position;
			vmin.x = std::min(vmin.x, v.x);
			vmin.y = std::min(vmin.y, v.y);
			vmax.x = std::max(vmin.x, v.x);
			vmax.y = std::max(vmin.y, v.y);
		}
		return sf::FloatRect(vmin, vmax - vmin);
	}
	 sf::PrimitiveType primitive_type() const { return _ptype; }
protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		if (size() > 0) target.draw(data(), size(), primitive_type(), states);
	}
	vector _verts;
	sf::PrimitiveType _ptype;
};


namespace global {
	 std::array<sf::Vertex, 6> CreateRectangle(const sf::FloatRect& rect, sf::Color fill_color);
	 void InsertRectangle(sf::Vertex  * verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles); 
	 void InsertRectangle(RawVertices& verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	 void InsertRectangle(sf::VertexArray& verts, const sf::FloatRect& rect, sf::Color fill_color);// convert
};

/*

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
*/
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


#if 0
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