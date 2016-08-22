#pragma once
#include "Global.h"


namespace global {
	void draw_box(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green);
	RawVertices<sf::PrimitiveType::TrianglesStrip> create_line(sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color);
	void insert_line(sf::VertexArray& verts, sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color);
	void insert_hair_line(sf::VertexArray& verts, sf::Vector2f v1, sf::Vector2f v2, float width, sf::Color color);
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
protected: // protected interface
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
class SpriteFrameBase : public Renderable ,public sf::Drawable , public ChangedCass {
public:
	const sf::Vertex& at(size_t i) const override final { return ptr()[i]; }
	size_t size() const override final { return 6; }
	virtual sf::FloatRect bounds() const override  { return sf::FloatRect(at(0).position, at(5).position- at(0).position); }
	sf::IntRect texRect() const { return sf::IntRect(sf::FloatRect(at(0).texCoords, at(5).texCoords - at(0).texCoords));}
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
	bool next_frame() override final { image_index(image_index() + 1); _changed = true;  return true; }
	bool prev_frame() override final { image_index(image_index() - 1); _changed = true; return true; }
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


class Body : public LockingObject , public ChangedCass {
	sf::Vector2f _position;
	sf::Vector2f _origin;
	sf::Vector2f _scale;
	float _rotation;
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
	const sf::Transform& getTransform() const;
};
inline std::ostream& operator<<(std::ostream& os, const Body& body) {
	os << "Position" << body.getPosition() << " ,Origin" << body.getOrigin() << " ,Scale" << body.getScale() << ", Rotation(" << body.getRotation() << ")";
	return os;
}

