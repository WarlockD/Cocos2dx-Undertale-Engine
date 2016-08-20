#pragma once
#include "Global.h"

// This is the basic rendering class.  None of the verts are transformed, no position data, nothing.  Just the raw verts
// has some simple helper functions
struct Renderable {
	bool debug_draw_box = false;
	// starting to get the hang of templates
	// traites and other good bits
	std::vector<sf::Vertex> test;

	class const_iterator : public std::iterator<std::bidirectional_iterator_tag,sf::Vertex> {
	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef const sf::Vertex value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef const sf::Vertex*pointer;
		typedef const sf::Vertex& reference;
		//typedef const pointer const_pointer;
		//typedef const reference const_reference;
		inline const_iterator &operator ++() { ++_pos; return *this; }
		inline const_iterator &operator --() { --_pos;; return *this; }
		inline const_iterator operator ++(int) { ++_pos; return *this; }
		inline const_iterator operator --(int) { --_pos;; return *this; }
		inline bool operator ==(const const_iterator &other) const { return _pos == other._pos; }
		inline bool operator !=(const const_iterator &other) const { return _pos != other._pos; }
		inline bool operator <(const const_iterator &other) const { return _pos< other._pos; }
		inline bool operator >(const const_iterator &other) const { return _pos> other._pos; }
		inline bool operator <=(const const_iterator &other) const { return _pos <= other._pos; }
		inline bool operator >=(const const_iterator &other) const { return _pos >= other._pos; }
		inline const_iterator & operator +(const difference_type &add) const { const_iterator copy(*this); copy += add; return copy; }
		inline const_iterator & operator -(const difference_type &add) const { const_iterator copy(*this); copy -= add; return copy; }
		inline const_iterator & operator +=(const difference_type &add) { _pos += add; return *this; }
		inline const_iterator & operator -=(const difference_type &add) { _pos -= add; return *this; }
		inline reference operator*() const { return _ref[_pos]; }
		inline pointer operator->() const { return &_ref[_pos]; }
		const_iterator(const Renderable& ref, difference_type pos) : _pos(pos), _ref(ref) {}
	protected:
		difference_type _pos;
		const Renderable& _ref;
	};
	class iterator : public const_iterator {
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef sf::Vertex value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef sf::Vertex *pointer;
		typedef sf::Vertex& reference;
		typedef const sf::Vertex* const_pointer;
		typedef const sf::Vertex& const_reference;
		// I still beleve using const_cast is a hack, but I want my iterator to be inhertertable
		inline reference operator*()  { return const_cast<reference>(_ref[_pos]); }
		inline pointer operator->()  { return const_cast<pointer>(&_ref[_pos]); }
		iterator(const Renderable& ref, difference_type pos) : const_iterator(ref,pos) {}
	};

protected: // protected interface
	virtual bool hasChanged() const { return true; } // defaults to always trasform this thing
	virtual int depth() const { return 0; } // used for depth sorting
											// sent by the engine in an atempt to optimize draw calls by recreating the texture
											// not sure if I will use
	virtual bool changeTexture(const sf::Texture& texture, const sf::IntRect& textRect) { return false; }
	virtual iterator begin()  { return iterator(*this, 0); }
	virtual iterator end()  { return iterator(*this, (int)size()); }
	// Interface
public:
	virtual const sf::Texture* texture() const = 0;
	virtual size_t size() const = 0;
	virtual const sf::Vertex& at(size_t index) const = 0;
	// end interface, the rest is just optimiztion
	const sf::Vertex& operator[](size_t index)  const { return at(index); }
	virtual const_iterator begin() const { return const_iterator(*this,0); }
	virtual const_iterator end() const { return const_iterator(*this, (int)size());}

	// Extra stuff with some defaults, override for preformance
	virtual sf::FloatRect bounds() const {
		sf::Vector2f vmin;
		sf::Vector2f vmax;
		for (auto& it : *this) {
			auto& v = it.position;
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
	std::reference_wrapper<Renderable> ref() { return std::ref(*this); }
	std::reference_wrapper<const Renderable> ref() const { return std::ref(*this); }
};

// use this template class for a Renderable object to test drawing direct to sfml
template<class C> class SFMLDrawable : public sf::Transformable, public sf::Drawable {
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.texture = = texture();
		states.transform *= getTransform();
		target.draw(data(), size(), sf::PrimitiveType::Triangles, states);
	}
};

typedef std::reference_wrapper<Renderable> RenderableRef;

class Mesh : public Renderable {
	std::vector<sf::Vertex> _verts;
	const sf::Texture* _texture;
public:
	virtual const sf::Vertex& at(size_t index) const { return _verts[index]; }
	virtual const sf::Texture* texture() const override { return _texture; }
	virtual size_t size() const override { return _verts.size(); } // we only return the current frame

	Mesh() : _texture(nullptr) {}
	Mesh(const sf::Texture* texture, const std::vector<sf::Vertex>& verts) : _texture(texture), _verts(verts) {}
	Mesh(const sf::Texture* texture, std::vector<sf::Vertex>&& verts) : _texture(texture), _verts(std::move(verts)) {}
	Mesh(const std::vector<sf::Vertex>& verts) : _texture(nullptr), _verts(verts) {}
	Mesh(const std::vector<sf::Vertex>&& verts) : _texture(nullptr), _verts(std::move(verts)) {}
};
class SpriteFrameRef : public Renderable {
protected:
	sf::Vertex* _ptr;
	const sf::Texture* _texture;
	explicit SpriteFrameRef(const sf::Texture* texture, sf::Vertex* ptr) : _texture(texture), _ptr(ptr) {}
public:
	virtual const sf::Vertex& at(size_t i) const override { return _ptr[i]; }
	virtual const sf::Texture* texture() const override { return _texture; }
	virtual size_t size() const override { return 6; }
	sf::FloatRect bounds() const override { return sf::FloatRect(_ptr[0].position, _ptr[5].position- _ptr[0].position); }
	sf::IntRect texRect() const { return sf::IntRect(sf::FloatRect(_ptr[0].texCoords, _ptr[5].texCoords - _ptr[0].texCoords));}
	sf::Color color() const { return _ptr[0].color; }
	std::reference_wrapper<SpriteFrameRef> ref() { return std::ref(*this); }
	std::reference_wrapper<const SpriteFrameRef> ref() const { return std::ref(*this); }
};

class SpriteFrame : public SpriteFrameRef {
	std::array<sf::Vertex, 6> _verts;
public:
	SpriteFrame() : SpriteFrameRef(nullptr,_verts.data()) {}
	explicit SpriteFrame(const sf::Texture* texture, const sf::Vertex* vert) : SpriteFrameRef(texture, _verts.data()) { std::copy(vert, vert + 6, _verts.begin()); }
	explicit SpriteFrame(const sf::Texture* texture, const sf::IntRect& textureRect, const sf::FloatRect& bounds);
};

class SpriteFrameCollection : public SpriteFrameRef {
private:
	std::vector<sf::Vertex> _frames;
	size_t _image_index;
	sf::Vector2f _size;
public:
	virtual const sf::Texture* texture() const override { return _texture; }
	sf::FloatRect bounds() const override { return sf::FloatRect(sf::Vector2f(), _size); }
	explicit SpriteFrameCollection() : SpriteFrameRef(nullptr,_frames.data()), _image_index(0), _size(0,0) {}
	explicit SpriteFrameCollection(const sf::Texture* texture, const std::vector<sf::Vertex>& frames, sf::Vector2f& size) : SpriteFrameRef(texture, _frames.data()), _frames(frames), _image_index(0), _size(size) {}
	explicit SpriteFrameCollection(const sf::Texture* texture, std::vector<sf::Vertex>&& frames, sf::Vector2f& size) : SpriteFrameRef(texture, _frames.data()), _frames(std::move(frames)), _image_index(0), _size(size) {}
	size_t getImageIndex() const { return _image_index; }
	void setImageIndex(size_t image_index) { _image_index = image_index % (_frames.size() / 6); _ptr = &_frames[_image_index * 6]; }
	// cheat and use the iliterator from an array
	SpriteFrame copy_frame() const { return SpriteFrame(_texture, _ptr); }
	size_t total_frames() const { return _frames.size()/6; }
};


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
	void move(const sf::Vector2f& v) { setPosition(getPosition() + v); }
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

