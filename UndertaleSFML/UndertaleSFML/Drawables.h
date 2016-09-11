#pragma once
#include "Global.h"


namespace global {
	void draw_box(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green);
	RawVertices create_line(sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color);
	void insert_line(RawVertices& verts, sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color);
	void insert_hair_line(RawVertices& verts, sf::Vector2f v1, sf::Vector2f v2, float width, sf::Color color);

	void InsertFrame(sf::Vertex* verts, const sf::FloatRect& bounds, const sf::IntRect& texRect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	void InsertFrame(sf::Vertex* verts, const sf::FloatRect& bounds, const sf::FloatRect& texRect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);

	void InsertFrame(std::vector<sf::Vertex>& verts, const sf::FloatRect& bounds, const sf::IntRect& texRect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	void InsertFrame(std::vector<sf::Vertex>& verts, const sf::FloatRect& bounds, const sf::FloatRect& texRect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);

	std::array<sf::Vertex, 6> CreateRectangle(const sf::FloatRect& rect, sf::Color fill_color);
	void InsertRectangle(sf::Vertex  * verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	void InsertRectangle(RawVertices& verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	void InsertRectangle(sf::VertexArray& verts, const sf::FloatRect& rect, sf::Color fill_color);// convert
};



struct Renderable  {
	bool debug_draw_box = false;
	// starting to get the hang of templates
	// traites and other good bits
	typedef sf::Vertex element_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef element_type* element_pointer;
	typedef element_type& element_reference;
	typedef const element_type* const_element_pointer;
	typedef const element_type& const_element_reference;
	typedef generic_iterator<Renderable, sf::Vertex> iterator;
	typedef generic_iterator<const Renderable, const sf::Vertex> const_iterator;
private:
	int _depth = 0;
protected: // protected interface
	
	
											// sent by the engine in an atempt to optimize draw calls by recreating the texture
											// not sure if I will use
	virtual bool changeTexture(const sf::Texture& texture, const sf::IntRect& textRect) { return false; }
	virtual iterator begin()  { return iterator(*this, 0); }
	virtual iterator end()  { return iterator(*this, (int)size()); }
	// Interface
public:
	int depth() const { return _depth; } // used for depth sorting
	int& depth() { return _depth; }
	void depth(size_t depth) { _depth = depth; }
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
	virtual void copy(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at) const { verts.insert(at, begin(), end()); }
	virtual void copy(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at, const sf::Transform& t) const { for (auto& v : *this) verts.emplace(at, t * v.position, v.color, v.texCoords); }
	virtual void copy(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at, const sf::Transform& t, const sf::Color& color) const { for (auto& v : *this) verts.emplace(at, t * v.position, color, v.texCoords); }
	virtual void copy(std::vector<sf::Vertex>& verts) const { verts.insert(verts.end(), begin(), end()); }
	virtual void copy(std::vector<sf::Vertex>& verts, const sf::Transform& t) const {
		for (auto& v : *this) verts.emplace_back(t * v.position, v.color, v.texCoords);
	}
	virtual void copy(std::vector<sf::Vertex>& verts, const sf::Transform& t, const sf::Color& color) const { for (auto& v : *this) verts.emplace_back(t * v.position, color, v.texCoords); }
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

// sprite frames and mesh have continusious illterators, passed by pointer
class SpriteFrameBase : public Renderable ,public sf::Drawable , public ChangedCass {
public:
	const sf::Vertex& at(size_t i) const override final { return ptr()[i]; }
	size_t size() const override final { return 6; }
	const sf::Vector2f frame_size() const { return at(5).position; }
	virtual sf::FloatRect bounds() const override  { return sf::FloatRect(sf::Vector2f(), frame_size());  }
	sf::IntRect texRect() const { return sf::IntRect(sf::FloatRect(at(0).texCoords, at(5).texCoords - at(0).texCoords));}
	const sf::Vector2f texture_offset() const { return at(0).texCoords; } // offset of the start of texture, useful for tiles
	sf::Color color() const { return ptr()[0].color; }
	std::reference_wrapper<SpriteFrameBase> ref() { return std::ref(*this); }
	std::reference_wrapper<const SpriteFrameBase> ref() const { return std::ref(*this); }
	virtual const sf::Vertex* ptr() const = 0;
	virtual ~SpriteFrameBase() {}
	// return false if no more frames
	virtual bool next_frame() { return false; }; // This interface just tells the sprite to do next frame
	virtual bool prev_frame() { return false; }; // This interface just tells the sprite to do prev frame
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.texture = texture();
		//states.transform *= getTransform();
		target.draw(ptr(), size(), sf::PrimitiveType::Triangles, states);
	}
};
// simple ref to data, used for extra stuff
// assumes the _ptr is valid.

class SpriteFrameRef : public SpriteFrameBase {
	const sf::Vertex* _ptr;
	const sf::Texture* _texture;
public:
	explicit SpriteFrameRef(const SpriteFrameBase& base) : _ptr(base.ptr()), _texture(base.texture()) {}
	SpriteFrameRef& operator=(const SpriteFrameBase& other) { _ptr = other.ptr(); _texture = other.texture(); return *this; }
	explicit SpriteFrameRef(const sf::Vertex* ptr, const sf::Texture* texture=nullptr) : _ptr(ptr), _texture(texture) {}
	explicit SpriteFrameRef(const sf::Vertex* ptr, const sf::Texture& texture) : SpriteFrameRef(ptr, &texture) {}
	void ptr(const sf::Vertex* ptr) { _ptr = ptr; }
	void texture(const sf::Texture* texture) { _texture = texture; }
	const sf::Texture* texture() const override final { return _texture; }
	const sf::Vertex* ptr() const override final { return _ptr; }
};


class Mesh : public Renderable {
	RawVertices _verts;
	const sf::Texture* _texture;
	typedef std::vector<sf::Vertex>::const_iterator vector_const_iterator;
	typedef std::vector<sf::Vertex>::iterator vector_iterator;
public:
	RawVertices& verts() { return _verts; }
	const RawVertices& verts() const { return _verts; }
	virtual const sf::Vertex& at(size_t index) const { return _verts[index]; }
	sf::Vertex& at(size_t index) { return _verts[index]; }
	sf::Vertex& operator[](size_t index) { return at(index); }
	iterator begin() { return iterator(*this, 0); }
	iterator end() { return iterator(*this, (int)size()); }

	virtual const sf::Texture* texture() const override { return _texture; }
	void texture(const sf::Texture* texture) { _texture = texture; }
	virtual size_t size() const override { return _verts.size(); } // we only return the current frame
	void clear() { _verts.clear(); }
	void resize(size_t t) { _verts.resize(t); }
	void reserve(size_t t) { _verts.reserve(t); }

	// end interface, the rest is just optimiztion

	Mesh() : _texture(nullptr) {}
	Mesh(const sf::Texture* texture, const std::vector<sf::Vertex>& verts) : _texture(texture), _verts(verts) {}
	Mesh(const sf::Texture* texture, std::vector<sf::Vertex>&& verts) : _texture(texture), _verts(std::move(verts)) {}
	Mesh(const std::vector<sf::Vertex>& verts) : _texture(nullptr), _verts(verts) {}
	Mesh(const std::vector<sf::Vertex>&& verts) : _texture(nullptr), _verts(std::move(verts)) {}
	template<typename... Targs>
	void emplace_back(Targs&&... Fargs) { _verts.emplace_back(Fargs...); }
	void push_back(sf::Vertex&&v) { _verts.push_back(std::move(v)); }
	void push_back(const sf::Vertex&v) { _verts.push_back(v); }
	// faster to use the ptr offsets for frames as Renderable has added costs
	void push_back(const SpriteFrameBase& frame) { assert(frame.texture() == _texture);  _verts.insert(_verts.end(), frame.ptr(), frame.ptr() + frame.size()); }
	void push_back(const Mesh& mesh) { assert(mesh.texture() == _texture);  _verts.insert(_verts.end(), mesh._verts.begin(),mesh._verts.end()); }
	///void push_back(const Renderable& r) { assert(r.texture() == _texture);  _verts.emplace(_verts.end(), r.begin(),r.end()); }
	void push_back(const sf::FloatRect& bounds, const sf::FloatRect& texRect, const sf::Color& color = sf::Color::White);
	void push_back(const sf::FloatRect& bounds, const sf::IntRect& texRect, const sf::Color& color = sf::Color::White);
	iterator insert(const_iterator where, sf::Vertex&&v) { _verts.insert(_verts.begin() + where.pos(), std::move(v)); }
	iterator insert(const_iterator where, const sf::Vertex&v) { _verts.insert(_verts.begin() + where.pos(), v); }
	template<typename... Targs>
	iterator emplace(const_iterator where, Targs&&... Fargs) { return _verts.emplace(_verts.begin() + where.pos(), Fargs...); }

};

class SpriteFrame : public SpriteFrameBase {
	std::array<sf::Vertex, 6> _verts;
	const sf::Texture* _texture;
public:
	const sf::Texture* texture() const override final { return _texture; }
	const sf::Vertex* ptr() const override final { return _verts.data(); }
	SpriteFrame() : _texture(nullptr) {}
	explicit SpriteFrame(const sf::Texture* texture, const sf::Vertex* vert) : _texture(texture) { std::copy(vert, vert + 6, _verts.begin()); }
	explicit SpriteFrame(const sf::FloatRect& bounds, const sf::Color& color = sf::Color::White); // if you just want a box, you can use this to get the verts
	explicit SpriteFrame(const sf::FloatRect& bounds, const sf::Texture* texture, const sf::Color& color = sf::Color::White);
	explicit SpriteFrame(const sf::FloatRect& bounds, const sf::Texture* texture, const sf::IntRect& textureRect, const sf::Color& color = sf::Color::White);
	
};

class TileMap : public Renderable {
	sf::IntRect _textureRect;
	const sf::Texture* _texture;
	std::vector<SpriteFrameRef> _tiles;
	RawVertices _verts;
public:
	TileMap() : _texture(nullptr), _textureRect() {}
	explicit TileMap(const sf::Texture* texture, const sf::IntRect& textureRect) : _texture(texture), _textureRect(textureRect) {}
	const sf::Vertex& at(size_t i) const override final { return _verts[i]; }
	size_t size() const override final { return _verts.size(); }
	virtual sf::FloatRect bounds() const override { return sf::FloatRect(_textureRect); }
	const sf::Vertex* ptr() const { return _verts.data(); }
	const sf::Texture* texture() const override final { return _texture; }
	void texture(const sf::Texture* texture) { _texture = texture; }
	
	size_t tile_count() const { return _tiles.size(); }
	SpriteFrameRef tile_at(size_t index) const { return _tiles.at(index); }
	SpriteFrameRef tile_create(const sf::Vector2f& pos, const sf::IntRect& rect);
	RawVertices& verts() { return _verts; }
	const RawVertices& verts() const { return _verts; }
};

class SpriteFrameCollection : public SpriteFrameBase  {
private:
	std::vector<SpriteFrame> _frames;
	size_t _image_index;
	sf::Vector2f _size;
public:
	explicit SpriteFrameCollection() : _frames(), _image_index(0), _size(0, 0) {}
	explicit SpriteFrameCollection(const std::vector<SpriteFrame>& frames, sf::Vector2f& size) : _frames(frames), _image_index(0), _size(size) {}
	explicit SpriteFrameCollection(const std::vector<SpriteFrame>& frames) : _frames(frames), _image_index(0) {
		auto bounds = SpriteFrameBase::bounds();
		_size = sf::Vector2f(bounds.width, bounds.height);
	}
	explicit SpriteFrameCollection(std::vector<SpriteFrame>&& frames, sf::Vector2f& size) : _frames(std::move(frames)), _image_index(0), _size(size) {}
	explicit SpriteFrameCollection(std::vector<SpriteFrame>&& frames) : _frames(std::move(frames)), _image_index(0) {
		auto bounds = SpriteFrameBase::bounds();
		_size = sf::Vector2f(bounds.width, bounds.height);
	}
	// interface
	const sf::Texture* texture() const override final { return _frames[_image_index].texture(); }
	const sf::Vertex*  ptr() const override final { return _frames[_image_index].ptr(); }
	sf::FloatRect bounds() const override final { return sf::FloatRect(sf::Vector2f(), _size); }
	// custom stuff
	size_t image_index() const { return _image_index; }
	void image_index(size_t index) { _image_index = index % _frames.size(); }
	bool next_frame() override final { image_index(image_index() + 1); changed(true);  return true; }
	bool prev_frame() override final { image_index(image_index() - 1); changed(true); return true; }
	void push_back(const SpriteFrame& frame) { _frames.push_back(frame); }
	SpriteFrameCollection& operator+=(const SpriteFrame& frame){_frames.push_back(frame); return *this; }
	SpriteFrameCollection& operator=(const std::vector<SpriteFrame>& frames) { _frames = frames; return *this; }
	SpriteFrameCollection& operator=(std::vector<SpriteFrame>&& frames) { _frames = std::move(frames); return *this; }
	void clear() { _frames.clear(); }
	void bounds(sf::Vector2f size) { _size = size; }
	// cheat and use the iliterator from an array
	const SpriteFrame& current_frame() const { return _frames[_image_index]; }
	size_t total_frames() const { return _frames.size(); }
};


class Body : public LockingObject, public ChangedCass {
protected:
	sf::Vector2f _position;
	sf::Vector2f _origin;
	sf::Vector2f _scale;
	sf::Vector2f _size;
	float _rotation;
	mutable bool _transformNeedUpdate;
	mutable sf::Transform _transform;
public:
	Body();
	virtual void setPosition(const sf::Vector2f& v);
	virtual void setOrigin(const sf::Vector2f& v);
	virtual void setScale(const sf::Vector2f& v);
	// helpers
	template<typename TX, typename TY, typename = std::enable_if<std::is_arithmetic<TX>::value&&std::is_arithmetic<TY>::value>::type>
	void setPosition(TX x, TY y) { setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y))); }
	template<typename TX, typename TY, typename = std::enable_if<std::is_arithmetic<TX>::value&&std::is_arithmetic<TY>::value>::type>
	void setOrigin(TX x, TY y) { setOrigin(sf::Vector2f(static_cast<float>(x), static_cast<float>(y))); }
	template<typename TX, typename TY, typename = std::enable_if<std::is_arithmetic<TX>::value&&std::is_arithmetic<TY>::value>::type>
	void setScale(TX x, TY y) { setScale(sf::Vector2f(static_cast<float>(x), static_cast<float>(y))); }
	template<typename TX, typename = std::enable_if<std::is_arithmetic<TX>::value>::type>
	void setScale(TX x) { setScale(sf::Vector2f(static_cast<float>(x), static_cast<float>(x))); }
	virtual void setRotation(float angle);
	void move(const sf::Vector2f& v) { setPosition(getPosition() + v); }
	const sf::Vector2f& getPosition() const { return _position; }
	const sf::Vector2f& getOrigin() const { return _origin; }
	const sf::Vector2f& getScale() const { return _scale; }
	sf::Vector2f getSize() const { return _size * _scale; }
	float getRotation() const { return _rotation; }
	const sf::Transform& getTransform() const;
};
inline std::ostream& operator<<(std::ostream& os, const Body& body) {
	os << "Position" << body.getPosition() << " ,Origin" << body.getOrigin() << " ,Scale" << body.getScale() << ", Rotation(" << body.getRotation() << ")";
	return os;
}

class  GameObject : public Body {
	sf::Vector2f _size;
	sf::FloatRect _bounds;
public:
	GameObject() : _size(0.0f, 0.0f), _bounds() {}
	void setSize(const sf::Vector2f& size) { _size = size; }
	void setSize(float x, float y) { _size = sf::Vector2f(x, y); }
	const sf::Vector2f& getSize() const { return _size; }
	virtual void setPosition(const sf::Vector2f& v) override { 
		_bounds.left = v.x; _bounds.top = v.y;
		Body::setPosition(v);
	}
	virtual void setScale(const sf::Vector2f& v) override { 
		_bounds.width = _size.x * v.x;
		_bounds.height = _size.y * v.y;
		Body::setScale(v);
	}
	const sf::FloatRect& getBounds() const { return _bounds; }
	virtual ~GameObject() {}
};

