#pragma once
#include "Global.h"


namespace global {
	void draw_box(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness = 4.0f, const sf::Color color = sf::Color::Green);
	RawVertices create_line(sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color);
	void insert_line(RawVertices& verts, sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color);
	void insert_hair_line(RawVertices& verts, sf::Vector2f v1, sf::Vector2f v2, float width, sf::Color color);

	template<typename T1, typename T2> 
	//sf::Vertex* InsertFrame(sf::Vertex* verts, sf::Rect<T1> const & bounds, sf::Rect<T2> const & texRect, sf::Color fill_color, sf::PrimitiveType type) {
	sf::Vertex* InsertFrame(sf::Vertex* verts, const sf::Rect<T1>  & bounds, const sf::Rect<T2>  & texRect, sf::Color fill_color, sf::PrimitiveType type) {
		const float left = static_cast<float>(bounds.left);
		const float top = static_cast<float>(bounds.top);
		const float right = static_cast<float>(bounds.left + bounds.width);
		const float bottom = static_cast<float>(bounds.top + bounds.height);
		const float u1 = static_cast<float>(texRect.left);
		const float v1 = static_cast<float>(texRect.top);
		const float u2 = static_cast<float>(texRect.left + texRect.width);
		const float v2 = static_cast<float>(texRect.top + texRect.height);
		// Add a quad for the current character
		if (type == sf::PrimitiveType::Triangles) {
			*verts++ = (sf::Vertex(sf::Vector2f(left, top), fill_color, sf::Vector2f(u1, v1)));
			*verts++ = (sf::Vertex(sf::Vector2f(right, top), fill_color, sf::Vector2f(u2, v1)));
			*verts++ = (sf::Vertex(sf::Vector2f(left, bottom), fill_color, sf::Vector2f(u1, v2)));
			*verts++ = (sf::Vertex(sf::Vector2f(left, bottom), fill_color, sf::Vector2f(u1, v2)));
			*verts++ = (sf::Vertex(sf::Vector2f(right, top), fill_color, sf::Vector2f(u2, v1)));
			*verts++ = (sf::Vertex(sf::Vector2f(right, bottom), fill_color, sf::Vector2f(u2, v2)));
		}
		else {
			*verts++ = (sf::Vertex(sf::Vector2f(left, top), fill_color, sf::Vector2f(u1, v1)));
			*verts++ = (sf::Vertex(sf::Vector2f(right, top), fill_color, sf::Vector2f(u2, v1)));
			*verts++ = (sf::Vertex(sf::Vector2f(right, bottom), fill_color, sf::Vector2f(u2, v2)));
			*verts++ = (sf::Vertex(sf::Vector2f(left, bottom), fill_color, sf::Vector2f(u1, v2)));
		}
		return verts;
	}
	template<typename T1, typename T2>
	inline sf::Vertex* InsertFrame(std::vector<sf::Vertex>& verts, const sf::Rect<T1> & bounds, const sf::Rect<T2>& texRect, sf::Color fill_color, sf::PrimitiveType type) {
			size_t pos = verts.size();
			verts.resize(pos + (type != sf::PrimitiveType::Triangles ? 4 : 6));
			return InsertFrame(verts.data() + pos, bounds, texRect, fill_color, type);
	}
	template<typename V, typename T1, typename T2>
	auto inline InsertFrame(V verts, const sf::Rect<T1> & bounds, const sf::Rect<T2>& texRect, sf::Color fill_color)
		->decltype(InsertFrame(verts, bounds, texRect, fill_color, sf::PrimitiveType::Triangles)) {
		return InsertFrame(verts, bounds, texRect, fill_color, sf::PrimitiveType::Triangles);
	}
	template<typename V, typename T1, typename T2>
	auto inline InsertFrame(V verts, const sf::Rect<T1> & bounds, const sf::Rect<T2>& texRect, sf::PrimitiveType type)
		->decltype(InsertFrame(verts, bounds, texRect, sf::Color::White, type)) {
		return InsertFrame(verts, bounds, texRect, sf::Color::White, type);
	}
	template<typename V, typename T1, typename T2>
	auto inline InsertFrame(V verts, const sf::Rect<T1> & bounds, const sf::Rect<T2>& texRect)
		->decltype(InsertFrame(verts, bounds, texRect, sf::Color::White, sf::PrimitiveType::Triangles)) {
		return InsertFrame(verts, bounds, texRect, sf::Color::White, sf::PrimitiveType::Triangles);
	}
	//template<typename T1, typename T2>
	//sf::Vertex* InsertFrame(sf::Vertex* verts, sf::Rect<T1> const & bounds, sf::Rect<T2> const & texRect, sf::Color fill_color, sf::PrimitiveType type) {
	//sf::Vertex* InsertFrame(sf::Vertex* verts, sf::Rect<T1>&& bounds, sf::Rect<T2>&& texRect, sf::Color fill_color, sf::PrimitiveType type) {
	//	return InsertFrameImpl(verts, bounds, textRect, color, type);
	//}

	std::array<sf::Vertex, 6> CreateRectangle(const sf::FloatRect& rect, sf::Color fill_color);
	void InsertRectangle(sf::Vertex  * verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	void InsertRectangle(RawVertices& verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type = sf::PrimitiveType::Triangles);
	void InsertRectangle(sf::VertexArray& verts, const sf::FloatRect& rect, sf::Color fill_color);// convert
};



// use this template class for a Renderable object to test drawing direct to sfml
template<class C> class SFMLDrawable : public sf::Transformable, public sf::Drawable {
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		states.texture = = texture();
		states.transform *= getTransform();
		target.draw(data(), size(), sf::PrimitiveType::Triangles, states);
	}
};


class Body : public LockingObject, public ChangedCass {
protected:
	sf::Vector2f _position;
	sf::Vector2f _origin;
	sf::Vector2f _scale;
	float _rotation;
	mutable bool _transformNeedUpdate;
	mutable sf::Transform _transform;
	sf::FloatRect _bounds;
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
	const sf::FloatRect& getBounds() const { return _bounds; }
	void setBounds(const sf::FloatRect& rect) { _bounds = rect; }
	void fixBounds(const sf::FloatRect& rect) { _bounds = rect; }
	template<typename U>
	void fixBounds(const sf::Vector2<U>& size) {
		_bounds = getTransform().transformRect(sf::FloatRect(sf::Vector2f(), size));
	}
	float getRotation() const { return _rotation; }
	const sf::Transform& getTransform() const;
};
inline std::ostream& operator<<(std::ostream& os, const Body& body) {
	os << "Position" << body.getPosition() << " ,Origin" << body.getOrigin() << " ,Scale" << body.getScale() << ", Rotation(" << body.getRotation() << ")";
	return os;
}

// sprite frames and mesh have continusious illterators, passed by pointer
class SpriteFrameRef : public sf::Drawable , public Body {
public:
	class trasform_const_iterator {
		const SpriteFrameRef& _ref;
		size_t _pos;
	public:
		using difference_type = size_t;
		using size_type = size_t;
		using value_type = sf::Vertex;
		using pointer = const sf::Vertex*;
		using reference = const sf::Vertex&;
		using iterator_category = std::bidirectional_iterator_tag;
		trasform_const_iterator(const SpriteFrameRef& ref, size_t pos) : _ref(ref), _pos(pos) {}
		trasform_const_iterator& operator++() { _pos++; return *this; }
		trasform_const_iterator operator++(int) { return trasform_const_iterator(_ref, _pos++); }
		trasform_const_iterator& operator--() { _pos--; return *this; }
		trasform_const_iterator operator--(int) { return trasform_const_iterator(_ref, _pos--); }
		value_type operator[](int pos) const { return _ref.at(_pos+pos) * _ref.getTransform(); }
		value_type operator*() const { return _ref.at(_pos) * _ref.getTransform(); }
		bool operator==(const trasform_const_iterator& r) { return _pos == r._pos; }
		bool operator!=(const trasform_const_iterator& r) { return _pos != r._pos; }
		bool operator<(const trasform_const_iterator& r) { return _pos < r._pos; }
		bool operator>(const trasform_const_iterator& r) { return _pos > r._pos; }
	};
	typedef sf::Vertex element_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef element_type* element_pointer;
	typedef element_type& element_reference;
	typedef const element_type* const_element_pointer;
	typedef const element_type& const_element_reference;
	typedef sf::Vertex* iterator;
	typedef const sf::Vertex* const_iterator;
protected:
	sf::Vertex* _verts;
	const sf::Texture* _texture;
	void  data(sf::Vertex* verts) {  _verts=verts; }
public:
	const sf::Vertex* data() const { return _verts; }
	sf::Vertex* data() { return _verts; }
	const sf::Texture* texture() const { return _texture; }
	void texture(const sf::Texture* texture)  {  _texture= texture; }

	virtual size_t size() const { return data() ? 6 : 0; }

	void assign(const sf::Vertex* verts) { std::memcpy(_verts, data(), sizeof(sf::Vertex) * 6); }
	void assign(const SpriteFrameRef& ref) { _verts = ref._verts; _texture = ref._texture; }
	void assign(const Body& ref) { Body::operator=(ref);  }
	void addOffset(const sf::Vector2f& offset) { for (auto& v : *this) v.position += offset; }
	virtual const_iterator begin() const { return data() ? data() : nullptr; }
	virtual const_iterator end() const { return data() ? data() + size() : nullptr; }
	trasform_const_iterator tbegin() const { return trasform_const_iterator(*this, 0); }
	trasform_const_iterator tend() const { return trasform_const_iterator(*this, size()); }
	virtual iterator begin() { return data() ? _verts : nullptr; }
	virtual iterator end() { return data() ? _verts + size() : nullptr; }
	SpriteFrameRef() : _verts(nullptr), _texture(nullptr) {}
	SpriteFrameRef(sf::Vertex* verts, const sf::Texture* texture) : _verts(verts), _texture(texture) {}
	// the old pointers are nulified on moves fyi
	SpriteFrameRef(SpriteFrameRef&& move) : _verts(move._verts), _texture(move._texture) { move._verts = nullptr; move._texture = nullptr; }
	SpriteFrameRef(const SpriteFrameRef& copy) : _verts(copy._verts), _texture(copy._texture) { }
	SpriteFrameRef& operator=(SpriteFrameRef rhs) { using std::swap; swap(_verts, rhs._verts); swap(_texture, rhs._texture); return *this; }

	// return state from a body
	SpriteFrameRef& operator=(const Body&body) { Body::operator=(body); return *this; }
	virtual ~SpriteFrameRef() {}
	

	
	const sf::Vertex& at(size_t i) const { return _verts[i]; }
	virtual sf::Vector2f sprite_size() const { return _verts[5].position- _verts[0].position; }
	virtual sf::FloatRect bounds_local() const  { return sf::FloatRect(0.0f, 0.0f, sprite_size().x, sprite_size().y); }
	virtual sf::FloatRect bounds_global() const { return getTransform().transformRect(bounds_local()); }
	sf::IntRect texRect() const { return sf::IntRect(sf::FloatRect(at(0).texCoords, at(5).texCoords - at(0).texCoords));}
	const sf::Vector2f texture_offset() const { return at(0).texCoords; } // offset of the start of texture, useful for tiles
	const sf::Color& color() const { return _verts[0].color; }
	void color(const sf::Color& c) { for (auto& v : *this) v.color = c; }
	// return false if at the end or could not advance
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		if (data()) {
			states.texture = texture();
			states.transform *= getTransform();
			target.draw(data(), size(), sf::PrimitiveType::Triangles, states);
		}
	}
	virtual void copy(std::vector<sf::Vertex>::iterator at, std::vector<sf::Vertex>& verts) const { verts.insert(at, begin(), end()); }
	virtual void copy(std::vector<sf::Vertex>& verts) const { verts.insert(verts.end(), begin(), end()); }
	virtual void transform_copy(std::vector<sf::Vertex>& verts, std::vector<sf::Vertex>::iterator at) const { 
		const sf::Transform& t = getTransform(); 
		for (auto& v : *this) verts.emplace(at, t * v.position, v.color, v.texCoords); 
	}
	virtual void transform_copy(std::vector<sf::Vertex>& verts) const {
		const sf::Transform& t = getTransform();
		for (auto& v : *this) verts.emplace_back(t * v.position, v.color, v.texCoords);
	}
};



class SpriteFrame : public SpriteFrameRef {
public:
	SpriteFrame() : SpriteFrameRef(new sf::Vertex[6], nullptr) {  }
	SpriteFrame(const sf::Texture* texture) : SpriteFrameRef(new sf::Vertex[6], texture) {  }
	SpriteFrame(const sf::Vertex* verts, const sf::Texture* texture) : SpriteFrameRef(new sf::Vertex[6], texture) { assign(verts);}
	SpriteFrame(SpriteFrame&& move) : SpriteFrameRef(move) { move._verts = nullptr; move._texture = nullptr; }
	SpriteFrame(const SpriteFrame& copy) : SpriteFrame(copy._verts,copy._texture) {  }
	SpriteFrame& operator=(SpriteFrame move) { using std::swap; swap(move._verts, _verts); swap(move._texture,_texture); return *this; }
	virtual ~SpriteFrame() { if (_verts) { delete _verts; _verts = nullptr; } }
	static SpriteFrame create(const sf::Vector2f& size, const sf::Color& color); // if you just want a colored box
	static SpriteFrame create(const sf::Texture* texture, const sf::IntRect& textureRect, const sf::Vector2f& offset=sf::Vector2f()); // standard sprite
	SpriteFrameRef ref() { return SpriteFrameRef(_verts, _texture); } // we make a refrence
};

class TileMap : public SpriteFrameRef {
	sf::IntRect _textureRect;
	sf::Vector2f _size;
	RawVertices _tile_verts;
public:
	TileMap() : SpriteFrameRef(), _textureRect() {}
	explicit TileMap(const sf::Texture* texture, const sf::IntRect& textureRect) : SpriteFrameRef(nullptr,texture), _textureRect(textureRect) {}
	size_t size() const override final { return _tile_verts.size(); }
	virtual sf::Vector2f sprite_size() const { return _size; }
	size_t tile_count() const { return _tile_verts.size()/6; }
	SpriteFrameRef tile_at(size_t index) { return SpriteFrameRef(_tile_verts.data() + (index * 6), _texture); }
	SpriteFrameRef tile_create(const sf::Vector2f& pos, const sf::IntRect& rect);
	RawVertices& tile_verts() { return _tile_verts; }
	const RawVertices& tile_verts() const { return _tile_verts; }
};

class SpriteFrameCollection : public SpriteFrameRef  {
private:
	std::vector<SpriteFrame> _frames;
	size_t _image_index;
	sf::Vector2f _size;
public:
	explicit SpriteFrameCollection() : _frames(), _image_index(0), _size(0, 0) {}
	explicit SpriteFrameCollection(const std::vector<SpriteFrame>& frames, sf::Vector2f& size) : _frames(frames), _image_index(0), _size(size) {}
	explicit SpriteFrameCollection(std::vector<SpriteFrame>&& frames, sf::Vector2f& size) : _frames(std::move(frames)), _image_index(0), _size(size) {}
	virtual size_t image_count() const { return _frames.size(); } // we default to only having a single frame
	virtual size_t image_index() const { return _image_index; } // always on frame zero
	virtual void image_index(size_t image) {
		if (_image_index != image) {
			_image_index = image % _frames.size();
			auto& frame = _frames.at(_image_index);
			_verts = frame.data();
			_texture = frame.texture();
		}
	} // dosn't do anything
	void push_back(const SpriteFrame& frame) { _frames.push_back(frame); }
	SpriteFrameCollection& operator+=(const SpriteFrame& frame){_frames.push_back(frame); return *this; }
	SpriteFrameCollection& operator=(const std::vector<SpriteFrame>& frames) { _frames = frames; return *this; }
	SpriteFrameCollection& operator=(std::vector<SpriteFrame>&& frames) { _frames = std::move(frames); return *this; }
	void clear() { _frames.clear(); _verts = nullptr; _texture = nullptr; }
};



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

