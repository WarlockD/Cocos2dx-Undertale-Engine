#pragma once
#include "Global.h"

// This is the basic rendering class.  None of the verts are transformed, no position data, nothing.  Just the raw verts
// has some simple helper functions
struct Renderable {
protected:
	virtual bool hasChanged() const { return true; } // defaults to always trasform this thing
	virtual int depth() const { return 0; } // used for depth sorting
	// sent by the engine in an atempt to optimize draw calls by recreating the texture
	// not sure if I will use
	virtual bool changeTexture(const sf::Texture& texture, const sf::IntRect& textRect) { return false; }
public:
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

typedef LightWeightRef<Renderable> RenderableRef;

class Mesh : public Renderable {
	std::vector<sf::Vertex> _verts;
	const sf::Texture* _texture;
public:
	Mesh() : _texture(nullptr) {}
	Mesh(const sf::Texture* texture, const std::vector<sf::Vertex>& verts) : _texture(texture), _verts(verts) {}
	Mesh(const sf::Texture* texture, std::vector<sf::Vertex>&& verts) : _texture(texture), _verts(std::move(verts)) {}
	Mesh(const std::vector<sf::Vertex>& verts) : _texture(nullptr), _verts(verts) {}
	Mesh(const std::vector<sf::Vertex>&& verts) : _texture(nullptr), _verts(std::move(verts)) {}
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
	explicit SpriteFrameCollection() : _texture(nullptr), _current(nullptr), _image_index(0) {}
	explicit SpriteFrameCollection(const sf::Texture* texture, const std::vector<sf::Vertex>& frames, sf::Vector2f& size) : _frames(frames), _current(_frames.data()), _image_index(0), _size(size), _texture(texture) {}
	explicit SpriteFrameCollection(const sf::Texture* texture, std::vector<sf::Vertex>&& frames, sf::Vector2f& size) : _frames(std::move(frames)), _current(_frames.data()), _image_index(0), _size(size), _texture(texture) {}
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

class Shape : public Renderable {

};
