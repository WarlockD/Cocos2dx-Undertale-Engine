#pragma once
#include <cassert>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <SFML\Graphics.hpp>


#include <map>  // Component-entity system in 16 lines of C++11. 2013 rlyeh, MIT licensed
#include <set>  // Code fragment from kult engine     -    https://github.com/r-lyeh/kult
enum { JOIN, MERGE, EXCLUDE }; 
using set = std::set<unsigned>; 
template<typename T> set&system() {
	static set entities; return entities;
}
template<typename T, int MODE>set subsystem(const set &B) {
	set newset; 
	const set&A = system<T>(); 
	if (MODE == MERGE) {
		newset = B; for (auto&id : A)newset.insert(id);
	}
	else if (MODE == EXCLUDE) { newset = B; for (auto&id : A)newset.erase(id); }
	else if (A.size()
		<B.size()) {
		for (auto&id : A)if (B.find(id) != B.end())newset.insert(id);
	}
	else {
		for (auto&id : B)if (
			A.find(id) != A.end())newset.insert(id);
	}return newset;
}template<typename T>std::map<unsigned, T>&components() { 
	static std::map<unsigned, T>objects; return objects; }
template<typename T>bool has(unsigned id) { return components<T>().find(id) != components<T>().end(); }
template<typename T>decltype(T::value_type)&get(unsigned id) {
	static decltype(T::value_type)invalid, reset; return has<T>(id) ? components<T>()[id].value_type : invalid = reset;
}
template<typename T>decltype(T::value_type)&add(unsigned id) { return system<T>().insert(id), components<T>()[id] = components<T>()[id], get<T>(id);}
template<typename T>bool del(unsigned id) { return add<T>(id), components<T>().erase(id), system<T>().erase(id), !has<T>(id); }
template<typename T, int>struct component { T value_type; };

typedef std::function<void(int)> IntSetCallback;
namespace global {
	sf::RenderWindow& getWindow();
	extern const std::string empty_string; // used for empty const std::string refrences
};



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