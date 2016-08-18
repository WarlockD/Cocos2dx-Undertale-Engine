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
#include <SFML\Graphics.hpp>
#include <map>  // Component-entity system in 16 lines of C++11. 2013 rlyeh, MIT licensed
#include <set>  // Code fragment from kult engine     -    https://github.com/r-lyeh/kult
#include "kult.hpp"
#include <entityx/entityx.h>
namespace ex = entityx;


typedef std::function<void(int)> IntSetCallback;
namespace global {
	sf::RenderWindow& getWindow();
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
};
// This is the basic rendering class.  None of the verts are transformed, no position data, nothing.  Just the raw verts
// has some simple helper functions
struct Renderable {
	typedef const sf::Vertex* const_iterator;
	virtual const sf::Texture* texture() const = 0;
	virtual const sf::Vertex* data() const = 0;
	virtual size_t size() const = 0;
	virtual const_iterator begin() const { return data(); }
	virtual const_iterator end() const { return data() + size(); }
	virtual sf::FloatRect bounds() const {
		sf::Vector2f vmin;
		sf::Vector2f vmax;
		for (auto& vert : *this) {
			auto& v = vert.position;
			vmin.x = std::min(vmin.x, v.x);
			vmin.y = std::min(vmin.y, v.y);
			vmax.x = std::max(vmin.x, v.x);
			vmax.y = std::max(vmin.y, v.y);
		}
		return sf::FloatRect(vmin, vmax - vmin);
	}
	virtual sf::FloatRect bounds(const sf::Transform&t) const { return t.transformRect(bounds()); }
	virtual void insert(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at) const { verts.insert(at, begin(), end()); }
	virtual void insert(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at, const sf::Transform& t) const { for (auto& v : *this) verts.emplace(at, t * v.position, v.color, v.texCoords); }
	virtual void insert(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at, const sf::Transform& t, const sf::Color& color) const { for (auto& v : *this) verts.emplace(at, t * v.position, color, v.texCoords); }
	virtual void insert(std::vector<sf::Vertex>& verts) const { verts.insert(verts.end(), begin(), end()); }
	virtual void insert(std::vector<sf::Vertex>& verts, const sf::Transform& t) const { 
		for (auto& v : *this) verts.emplace_back(t * v.position, v.color, v.texCoords); 
	}
	virtual void insert(std::vector<sf::Vertex>& verts, const sf::Transform& t, const sf::Color& color) const { for (auto& v : *this) verts.emplace_back(t * v.position, color, v.texCoords); }
	virtual ~Renderable() {}
};

// use this template class for a Renderable object to test drawing direct to sfml
template<class C> class SFMLDrawable : public sf::Transformable, public sf::Drawable {
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.texture = = texture();
		states.transform *= getTransform();
		target.draw(data(), size(), sf::PrimitiveType::Triangles, states);
	}
};


template <typename T>
class abstract_ptr : public std::unique_ptr<T>
{
	std::function<T*(const T&)> _create_copy;

	template<class B, class = typename std::enable_if<std::is_copy_constructible<B>::value && std::is_base_of<T, B>::value, B>::type>
	void SetCopyFunctions() {
		_create_copy = [](const T& copy) { return new B(static_cast<const B&>(copy)); };
	}
public:
	using std::unique_ptr<T>::unique_ptr;
	abstract_ptr() {}

	//	typename std::enable_if<!std::is_abstract<T>::value && std::is_base_of<Base, T>::value && std::is_copy_assignable<T>, PtrComponent<Base>&>::type
//	typename std::enable_if<std::is_copy_constructible<B>>::type
		
		//class = typename std::enable_if<!std::is_abstract<T>::value && std::is_base_of<Base, T>::value, PtrComponent<Base>&>::type>
	template<class B, class G=T, class = typename std::enable_if<std::is_copy_constructible<B>::value && std::is_base_of<G, B>::value, B>::type>
	abstract_ptr(abstract_ptr<B> const& other)
	{
		SetCopyFunctions<B>();
		auto value = *other.get();
		this->reset(new B(value));
	}

	abstract_ptr(abstract_ptr<T> const& other) : _create_copy(other._create_copy)
	{
		if (_create_copy) this->reset(_create_copy(*other.get()));
	}
	abstract_ptr<T>& operator=(abstract_ptr<T> const& other)
	{
		_create_copy = other._create_copy;
		if (_create_copy) this->reset(_create_copy(*other.get()));
		return *this;
	}

	template<class B, class G = typename std::enable_if<std::is_copy_constructible<B>::value && std::is_base_of<T,B>::value, T>::type>
	abstract_ptr(const B& other) {
		SetCopyFunctions<B>(*this);
		this->reset(new B(other));
	}

	template<class B>
	typename std::enable_if<std::is_copy_constructible<B>::value && std::is_base_of<T, B>::value, abstract_ptr<T>&>::type
	 operator=(const B& other)
	{
		SetCopyFunctions<B>();
		this->reset(new B(other));
		return *this;
	}

	template<class B,  class = typename std::enable_if<std::is_copy_constructible<B>::value && std::is_base_of<T, B>::value, , T>::type>
	abstract_ptr<T>& operator=(abstract_ptr<B> const& other)
	{
		SetCopyFunctions<B>();
		this->reset(new B(&other.get()));
		return *this;
	}
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
	StopWatch() : _toAmount(Zero), _current(Zero) {}
	StopWatch(Type init) : _toAmount(init), _current(Zero) {}
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
class Mesh : public Renderable {
	std::vector<sf::Vertex> _verts;
	const sf::Texture* _texture;
public:
	Mesh() : _texture(nullptr) {}
	Mesh(const sf::Texture* texture, const std::vector<sf::Vertex>& verts) : _texture(texture) , _verts(verts) {}
	Mesh(const sf::Texture* texture, std::vector<sf::Vertex>&& verts) : _texture(texture), _verts(std::move(verts)) {}
	virtual const sf::Texture* texture() const override { return _texture; }
	virtual const sf::Vertex* data() const override { return _verts.data(); }
	virtual size_t size() const override { return _verts.size(); } // we only return the current frame
};

class SpriteFrame : public Renderable {
	const sf::Texture* _texture;
	sf::IntRect _textureRect;
	sf::FloatRect _bounds;
	std::array<sf::Vertex, 6> _verts;
public:
	SpriteFrame() : _texture(nullptr) {}
	SpriteFrame(const sf::Texture* texture, const sf::IntRect& textureRect, const sf::FloatRect& bounds);
	SpriteFrame(const sf::Texture* texture, const sf::Vertex*  verts);  // build from triangles
																		// we want to default to triangles for compatablity with opengles
	const sf::IntRect& textureRect() const { return _textureRect; }
	sf::FloatRect bounds() const override { return _bounds; }
	// interface 
	virtual const sf::Texture* texture() const override { return _texture; }
	virtual const sf::Vertex* data() const override { return _verts.data(); }
	virtual size_t size() const override { return _verts.size(); }
};

class SpriteFrameCollection : public Renderable {
private:
	std::vector<sf::Vertex> _frames;
	const sf::Vertex* _current;
	size_t _image_index;
	sf::Vector2f _size;
	const sf::Texture* _texture;
public:
	SpriteFrameCollection() : _texture(nullptr), _current(nullptr),_image_index(0) {}
	SpriteFrameCollection(const sf::Texture* texture, const std::vector<sf::Vertex>& frames, sf::Vector2f& size) : _frames(frames), _current(_frames.data()), _image_index(0),_size(size), _texture(texture) {}
	SpriteFrameCollection(const sf::Texture* texture, std::vector<sf::Vertex>&& frames, sf::Vector2f& size) : _frames(std::move(frames)), _current(_frames.data()), _image_index(0), _size(size), _texture(texture) {}
	size_t getImageIndex() const { return _image_index; }
	void setImageIndex(size_t image_index) { _image_index = image_index % (_frames.size() / 6); _current = _frames.data() + (_image_index * 6); }	
	// cheat and use the iliterator from an array
	SpriteFrame getSpriteFrame() const { return SpriteFrame(_texture, _current); }
	virtual const sf::Texture* texture() const override { return _texture; }
	virtual const sf::Vertex* data() const override { return _current; }
	virtual size_t size() const override { return 6; } // we only return the current frame
	sf::FloatRect bounds() const override { return sf::FloatRect(sf::Vector2f(), _size); }
	size_t total_size() const { return _frames.size(); }
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
class Body : public LockingObject {
	sf::Vector2f _position;
	sf::Vector2f _origin;
	sf::Vector2f _scale;
	float _rotation;
	bool _changed;
	mutable bool _transformNeedUpdate;
	mutable sf::Transform _transform;
public:
	Body();
	void setPosition(const sf::Vector2f& v);
	void setPosition(float x, float y) { setPosition(sf::Vector2f(x, y)); }
	void setOrigin(const sf::Vector2f& v);
	void setOrigin(float x, float y) { setOrigin(sf::Vector2f(x, y)); }
	void setScale(const sf::Vector2f& v);
	void setScale(float x, float y) { setScale(sf::Vector2f(x, y)); }
	void setScale(float x) { setScale(sf::Vector2f(x, x)); }
	void setRotation(float angle);
	void move(const sf::Vector2f& v) {setPosition(getPosition() + v); }
	const sf::Vector2f& getPosition() const { return _position; }
	const sf::Vector2f& getOrigin() const { return _origin; }
	const sf::Vector2f& getScale() const { return _scale; }
	float getRotation() const { return _rotation; }
	bool hasChanged() const { return _changed; }
	void resetChanged() { _changed = false; }
	const sf::Transform& getTransform() const;

};
inline std::ostream& operator<<(std::ostream& os, const Body& body) {
	os << "Position" << body.getPosition() << " ,Origin" << body.getOrigin() << " ,Scale" << body.getScale() << ", Rotation(" << body.getRotation() << ")";
	return os;
}
class Animation {
	float _speed;
	float _last;
public:
	Animation() : _speed(0), _last(0) {}
	void setSpeed(float speed) { _speed = speed; }
	float getSpeed() const {
		return _speed;
	}
	Animation& operator=(float f) {
		_speed = f;
		_last = 0.0f;
		return *this;
	}
	bool checkFrame(float dt) {
		_last += dt;
		if (_last + 0.01 > _speed) {
			_last = 0.0;
			return true;
		}
		return false;
	}
};
inline std::ostream& operator<<(std::ostream& os, const Animation& a) {
	os << "Speed(" << a.getSpeed() << ")";
	return os;
}





// Emitted when two entities collide.
struct TransformChanged {
	TransformChanged(ex::Entity enity, ex::Entity right) : left(left), right(right) {}

	ex::Entity left, right;
};

#if 0

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




class ComponentBase {
public:
	typedef std::unique_ptr<ComponentBase> ptr_type;
	static constexpr int ID = -1;
	static constexpr int ParentID = -1;
	virtual void update(sf::Time dt) {}
	virtual ~ComponentBase() { }
};
template<int _ID, class D,class P = ComponentBase> class Component : public P {
public:
	typedef D type;
	typedef P parent_type;
	typedef std::unique_ptr<type> ptr_type;
	typedef std::unique_ptr<parent_type> ptr_parent_type;
	static constexpr int ID = _ID;
	static constexpr int ParentID = P::ID;
	static ptr_type create() { return std::make_unique<D>(); }
	static ptr_type create(const D& copy) { return std::make_unique<D>(copy); }
	ptr_type clone() const { return create(static_cast<const D&>(*this)); }
	virtual void update(sf::Time dt) override { P::update(dt); }
};

class ComponentContainer {
	typedef std::unordered_multimap<size_t, std::unique_ptr<ComponentBase>> container;
	typedef container::iterator iterator;
	typedef container::const_iterator const_iterator;
	typedef std::pair<iterator, iterator> range_iterator;
	container _list;
public: // can't be copyied
	ComponentContainer() {}
	ComponentContainer(const ComponentContainer& copy) = delete;
	ComponentContainer& operator=(const ComponentContainer& copy) = delete;
	ComponentContainer(ComponentContainer&& move) = default;
	ComponentContainer& operator=(ComponentContainer&& move) = default;
	~ComponentContainer() {}

	bool exists(size_t id) const { return _list.find(id) != _list.end(); }
	size_t count(size_t id) const { return _list.bucket_size(_list.bucket(id)); }
	std::pair<iterator, iterator> equal_range(size_t id) { return _list.equal_range(id); }
	std::pair<const_iterator, const_iterator> equal_range(size_t id) const { return _list.equal_range(id); }

	template<class T>
	typename std::enable_if<std::is_base_of<ComponentBase, T*>::value>::type
	findone(size_t id) const { 
		assert(T::ID == id);
		auto it = _list.find(id);
		if(it != _list.end()) return static_cast<T*>(_list.begin(index)->second.get());
		else return nullptr;
	}
	template<class T>
	typename std::enable_if<std::is_base_of<ComponentBase, std::vector<T*>>::value>::type
		findmany(size_t id) const {
		assert(T::ID == id);
		std::vector<T*> many;
		auto range = _list.equal_range(id);
		for (auto it = range.first; it != range.second; it++) 
			many.push_back(static_cast<T*>(it->second.get()));
		return many;
	}
	template<class T>
	typename std::enable_if<std::is_base_of<ComponentBase, T*>::value>::type
		create() {
		auto ptr = T::create();
		return return static_cast<T*>(_list.emplace(T::ID, ptr)->second.get());
	}
	iterator find(size_t id)  { return _list.find(id); }
	const_iterator find(size_t id) const { return _list.find(id); }
	void erase(size_t id) { _list.erase(id); }
	iterator begin() { return _list.begin(); }
	const_iterator begin() const { return _list.begin(); }
	iterator end() { return _list.end(); }
	const_iterator end() const { return _list.end(); }
	size_t size() const { return _list.size(); }

};
class GameObject {
public:
	
	std::unordered_multimap<size_t, size_t> _lookp; // lookup
	std::vector<std::unique_ptr<ComponentBase>> _order;
};
class Node : public sf::Transformable, public sf::Drawable {
	std::vector<Node*> _children;
	Node* _parent;
public:
	void AddChild(Node* child) { }
	Node() : _parent(nullptr) {}
	virtual ~Node() {}
};

namespace console {

};



namespace logging {
	void init_cerr();
	void init_cout();
	bool init();
	void error(const std::string& message);
};