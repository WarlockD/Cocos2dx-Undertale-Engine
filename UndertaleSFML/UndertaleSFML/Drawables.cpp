#include "Drawables.h"

using namespace sf;

namespace global {
	void InsertFrame(sf::Vertex* verts, const sf::FloatRect& bounds, const sf::FloatRect& texRect, sf::Color fill_color, sf::PrimitiveType type) {
		float left = bounds.left;
		float top = bounds.top;
		float right = bounds.left + bounds.width;
		float bottom = bounds.top + bounds.height;

		float u1 = texRect.left;
		float v1 = texRect.top;
		float u2 = texRect.left + texRect.width;
		float v2 = texRect.top + texRect.height;
		// Add a quad for the current character
		if (type == sf::PrimitiveType::Triangles) {
			*verts++ = (Vertex(Vector2f(left, top), fill_color, Vector2f(u1, v1)));
			*verts++ = (Vertex(Vector2f(right, top), fill_color, Vector2f(u2, v1)));
			*verts++ = (Vertex(Vector2f(left, bottom), fill_color, Vector2f(u1, v2)));
			*verts++ = (Vertex(Vector2f(left, bottom), fill_color, Vector2f(u1, v2)));
			*verts++ = (Vertex(Vector2f(right, top), fill_color, Vector2f(u2, v1)));
			*verts++ = (Vertex(Vector2f(right, bottom), fill_color, Vector2f(u2, v2)));
		}
		else {
			*verts++ = (Vertex(Vector2f(left, top), fill_color, Vector2f(u1, v1)));
			*verts++ = (Vertex(Vector2f(right, top), fill_color, Vector2f(u2, v1)));
			*verts++ = (Vertex(Vector2f(right, bottom), fill_color, Vector2f(u2, v2)));
			*verts++ = (Vertex(Vector2f(left, bottom), fill_color, Vector2f(u1, v2)));
		}
	}
	void InsertFrame(sf::Vertex* verts, const sf::FloatRect& bounds, const sf::IntRect& texRect, sf::Color fill_color, sf::PrimitiveType type ) {
		InsertFrame(verts, bounds, sf::FloatRect(static_cast<float>(texRect.left), static_cast<float>(texRect.top), static_cast<float>(texRect.width), static_cast<float>(texRect.height)), fill_color, type);
	}
	void InsertFrame(std::vector<sf::Vertex>& verts, const sf::FloatRect& bounds, const sf::FloatRect& texRect, sf::Color fill_color, sf::PrimitiveType type) {
		size_t pos = verts.size();
		verts.resize(pos + (type != sf::PrimitiveType::Triangles ? 4 : 6));
		return InsertFrame(verts.data()+pos, bounds, texRect, fill_color, type);
	}
	void InsertFrame(std::vector<sf::Vertex>& verts, const sf::FloatRect& bounds, const sf::IntRect& texRect, sf::Color fill_color, sf::PrimitiveType type) {
		InsertFrame(verts, bounds, sf::FloatRect(static_cast<float>(texRect.left), static_cast<float>(texRect.top), static_cast<float>(texRect.width), static_cast<float>(texRect.height)), fill_color, type);
	}

	void InsertRectangle(sf::Vertex * verts, const sf::FloatRect& bounds, sf::Color fill_color, sf::PrimitiveType type) {
		InsertFrame(verts, bounds, sf::IntRect(0, 0, 1, 1), fill_color, type);
	}
	void draw_box(sf::VertexArray& verts, const sf::FloatRect& rect, float thickness, const sf::Color color) {
		float left = rect.left;
		float top = rect.top;
		float right = rect.left + rect.width;
		float bottom = rect.top + rect.height;
		// Add a quad for the current character
		verts.append(Vertex(Vector2f(left, top), color)); verts.append(Vertex(Vector2f(left, bottom), color));
		verts.append(Vertex(Vector2f(left, top), color)); verts.append(Vertex(Vector2f(right, top), color));
		verts.append(Vertex(Vector2f(right, bottom), color)); verts.append(Vertex(Vector2f(left, bottom), color));
		verts.append(Vertex(Vector2f(right, bottom), color)); verts.append(Vertex(Vector2f(right, top), color));
	}
	std::array<sf::Vertex, 6> CreateRectangle(const sf::FloatRect& rect, sf::Color fill_color) {
		std::array<sf::Vertex, 6> verts;
		InsertRectangle(verts.data(), rect, fill_color);
		return verts;
	}
	void InsertRectangle(RawVertices& verts, const sf::FloatRect& rect, sf::Color fill_color, sf::PrimitiveType type) {
		const size_t pos = verts.size();
		verts.resize(pos + type == sf::PrimitiveType::Triangles ? 6 : 4);
		InsertRectangle(verts.data() + pos, rect, fill_color,type);
	}
	void InsertRectangle(sf::VertexArray&  verts, const sf::FloatRect& rect, sf::Color fill_color) {
		const size_t pos = verts.getVertexCount();
		verts.resize(pos + verts.getPrimitiveType() == sf::PrimitiveType::Triangles ? 6 : 4);
		InsertRectangle(&verts[pos], rect, fill_color);
	}
	void InsertRectangle(sf::VertexArray& verts, const sf::FloatRect& rect, sf::Color fill_color);
	sf::Color  blend(sf::Color fg, sf::Color  bg)
	{
		sf::Color r;
		unsigned int alpha = fg.a + 1;
		unsigned int inv_alpha = 256 - fg.a;
		r.r = (unsigned char)((alpha * fg.r + inv_alpha * bg.r) >> 8);
		r.g = (unsigned char)((alpha * fg.g + inv_alpha * bg.g) >> 8);
		r.b = (unsigned char)((alpha * fg.b + inv_alpha * bg.b) >> 8);
		r.a = 0xFF;
		return r;
	}
	sf::Color color_from_float(float r, float g, float b, float a = 1.0) {
		return sf::Color(static_cast<uint8_t>(r * 255.0f), static_cast<uint8_t>(g * 255.0f), static_cast<uint8_t>(b * 255.0f), static_cast<uint8_t>(a * 255.0f));
	}
	sf::Color color_from_float(float amount) {
		uint8_t shade = static_cast<uint8_t>(amount * 255.0f);
		return sf::Color(shade, shade, shade, shade);
	}
	RawVertices create_line(sf::Vector2f v1, sf::Vector2f v2, float w, sf::Color color) {
		RawVertices verts;
		insert_line(verts, v1, v2, w, color);
		return verts;
	}
	void insert_line(RawVertices& verts, sf::Vector2f v1, sf::Vector2f v2, float w,sf::Color color)
	{
		float t; float R; float f = w- static_cast<int>(w);
		float A = 1.0f;
		sf::Color alpha(color);
		sf::Color noalpha(color.r, color.g, color.b, 0);
		//determine parameters t,R
		/*   */
		if (w >= 0.0 && w < 1.0) {
			t = 0.05f; R = 0.48f + 0.32f*f;
			noalpha.a = static_cast<unsigned char>((((float)color.a / 255.0f) * f) * 255);
		}
		else if (w >= 1.0f && w < 2.0f) {
			t = 0.05f + f*0.33f; R = 0.768f + 0.312f*f;
		}
		else if (w >= 2.0f && w < 3.0f) {
			t = 0.38f + f*0.58f; R = 1.08f;
		}
		else if (w >= 3.0f && w < 4.0f) {
			t = 0.96f + f*0.48f; R = 1.08f;
		}
		else if (w >= 4.0f && w < 5.0f) {
			t = 1.44f + f*0.46f; R = 1.08f;
		}
		else if (w >= 5.0f && w < 6.0f) {
			t = 1.9f + f*0.6f; R = 1.08f;
		}
		else if (w >= 6.0f) {
			float ff = w - 6.0f;
			t = 2.5f + ff*0.50f; R = 1.08f;
		}

		//determine angle of the line to horizontal
		sf::Vector2f tv;//core thinkness of a line
		sf::Vector2f Rv;//fading edge of a line
		sf::Vector2f cv;//cap of a line
		const float ALW = 0.01f;
		sf::Vector2f dv = v2 - v1;
		if (std::fabsf(dv.x) < ALW) {
			//vertical
			if (w > 0.0 && w <= 1.0) {
				tv = sf::Vector2f(0.5f,0.0f);
				Rv = sf::Vector2f(0.0f, 0.0f);
			}
			else {
				tv = sf::Vector2f(0.0f, 0.0f);
				Rv = sf::Vector2f(R, 0.0f);
			}
		}
		else if (std::fabsf(dv.y) < ALW) {
			//horizontal
			if (w > 0.0 && w <= 1.0) {
				tv = sf::Vector2f(0.0f, 0.5f);
				Rv = sf::Vector2f(0.0f, 0.0f);
			}
			else {
				tv = sf::Vector2f(0.0f, t);
				Rv = sf::Vector2f(0.0f, R);
			}
		}
		else {
			if (w < 3) { //approximate to make things even faster
				float m = dv.y / dv.x;
				//and calculate tx,ty,Rx,Ry
				if (m > -0.4142 && m <= 0.4142) {
					// -22.5< angle <= 22.5, approximate to 0 (degree)
					tv = sf::Vector2f(t*0.1f, t);
					Rv = sf::Vector2f(R*0.6f, R);
				}
				else if (m > 0.4142 && m <= 2.4142) {
					// 22.5< angle <= 67.5, approximate to 45 (degree)
					tv = sf::Vector2f(t*-0.7071f, t*0.7071f);
					Rv = sf::Vector2f(R*-0.7071f, R*0.7071f);
				}
				else if (m > 2.4142 || m <= -2.4142) {
					// 67.5 < angle <=112.5, approximate to 90 (degree)
					tv = sf::Vector2f(t, t*0.1f);
					Rv = sf::Vector2f(R, R*0.6f);
				}
				else if (m > -2.4142 && m < -0.4142) {
					// 112.5 < angle < 157.5, approximate to 135 (degree)
					tv = sf::Vector2f(t*0.7071f, t*0.7071f);
					Rv = sf::Vector2f(R*0.7071f, R*0.7071f);
				}
				else {
					// error in determining angle
					//printf( "error in determining angle: m=%.4f\n",m);
				}
			}
			else { //calculate to exact
				dv.x = v1.y - v2.y;
				dv.y = v2.x - v1.x;
				const float L = std::sqrtf(dv.x*dv.x + dv.y*dv.y);
				dv /= L;
				cv = sf::Vector2f(-dv.y, dv.x);
				tv = dv * t;
				Rv = dv * R;
			}
		}
		v1 += cv * 0.5f;
		v2 -= cv * 0.5f;
		RawVertices strip(sf::PrimitiveType::TrianglesStrip);
		strip.reserve(w >= 3 ? 12 : 8);
		//draw the line by triangle strip
		strip.emplace_back(v1 - tv - Rv - cv, noalpha); //fading edge1
		strip.emplace_back(v2 - tv - Rv + cv, noalpha);
		strip.emplace_back(v1 - tv - cv, alpha);	  //core
		strip.emplace_back(v2 - tv + cv, alpha);
		strip.emplace_back(v1 + tv - cv, alpha);
		strip.emplace_back(v2 + tv + cv, alpha);
		strip.emplace_back(v1 + tv + Rv - cv, noalpha); //fading edge2
		strip.emplace_back(v2 + tv + Rv + cv, noalpha);
		//cap

		if (w >= 3) {
			strip.emplace_back(v1 - tv - Rv - cv, noalpha); //cap1
			strip.emplace_back(v1 - tv - Rv, noalpha);
			strip.emplace_back(v1 - tv - cv,  alpha);
			strip.emplace_back(v1 + tv + Rv,  noalpha);
			strip.emplace_back(v1 + tv - cv,  alpha);
			strip.emplace_back(v1 + tv + Rv - cv,  noalpha);

			strip.emplace_back(v2 - tv - Rv + cv,  noalpha); //cap2
			strip.emplace_back(v2 - tv - Rv,  noalpha);
			strip.emplace_back(v2 - tv + cv,  alpha);
			strip.emplace_back(v2 + tv + Rv,  noalpha);
			strip.emplace_back(v2 + tv + cv,  alpha);
			strip.emplace_back(v2 + tv + Rv + cv,  noalpha);
		}
		verts += strip;
	}
	void insert_hair_line(RawVertices& verts, sf::Vector2f v1, sf::Vector2f v2, float width, sf::Color color) {
		//float t = width;
		float t = 0.05f; float R = 0.768f;
		const float ALW = 0.01f;
		sf::Vector2f tv, Rv, dv = v2 - v1;
		//determine angle of the line to horizontal
		if (std::fabsf(dv.x) < ALW) {
			//vertical
			tv.x = t*0.5f;
		}
		else if (std::fabsf(dv.y) < ALW) {
			//horizontal
			tv.y = t*0.5f;
		}
		else {
			float m = dv.y / dv.x;
			if (m > -0.4142 && m <= 0.4142) {
				// -22.5< angle <= 22.5, approximate to 0 (degree)
				tv = sf::Vector2f(t*0.1f, t);
				Rv = sf::Vector2f(R*0.6f, R);
			}
			else if (m > 0.4142 && m <= 2.4142) {
				// 22.5< angle <= 67.5, approximate to 45 (degree)
				tv = sf::Vector2f(t*-0.7071f, t*0.7071f);
				Rv = sf::Vector2f(R*-0.7071f, R*0.7071f);
			}
			else if (m > 2.4142 || m <= -2.4142) {
				// 67.5 < angle <=112.5, approximate to 90 (degree)
				tv = sf::Vector2f(t, t*0.1f);
				Rv = sf::Vector2f(R, R*0.6f);
			}
			else if (m > -2.4142 && m < -0.4142) {
				// 112.5 < angle < 157.5, approximate to 135 (degree)
				tv = sf::Vector2f(t*0.7071f, t*0.7071f);
				Rv = sf::Vector2f(R*0.7071f, R*0.7071f);
			}
		}
		sf::Color alpha(color.r, color.g, color.b, 255);
		sf::Color noalpha(color.r, color.g, color.b, 0);
		RawVertices strip(sf::PrimitiveType::TrianglesStrip);
		strip.reserve(8);
		strip.emplace_back(v1 - tv - Rv, noalpha); //fading edge1
		strip.emplace_back(v2 - tv - Rv, noalpha);
		strip.emplace_back(v1 - tv, alpha);	  //core
		strip.emplace_back(v2 - tv, alpha);
		strip.emplace_back(v1 + tv, alpha);
		strip.emplace_back(v2 + tv, alpha);
		strip.emplace_back(v1 + tv + Rv, noalpha); //fading edge2
		strip.emplace_back(v2 + tv + Rv, noalpha);
		verts += strip;
	}
};
void Mesh::push_back(const sf::FloatRect& bounds, const sf::FloatRect& texRect, const sf::Color& color) {
	global::InsertFrame(_verts.vector(), bounds, texRect, color);
}
void Mesh::push_back(const sf::FloatRect& bounds, const sf::IntRect& texRect, const sf::Color& color) {
	global::InsertFrame(_verts.vector(), bounds, texRect, color);
}

SpriteFrame::SpriteFrame(const sf::FloatRect& bounds, const sf::Texture* texture, const sf::IntRect& textureRect, const sf::Color& color) : _texture(texture) {
	global::InsertFrame(_verts.data(), bounds, textureRect, color);
}
SpriteFrameRef TileMap::tile_create(const sf::Vector2f& pos, const sf::IntRect& rect) {
	
	sf::Vector2f size(rect.width, rect.height);
	sf::Vector2f offset(_textureRect.left + rect.left, _textureRect.top + rect.top);
	global::InsertFrame(_verts.vector(), sf::FloatRect(pos, size), sf::FloatRect(offset, size),sf::Color::White);
	SpriteFrameRef tile(_verts.data() + _verts.size() - 6, _texture);
	_tiles.push_back(tile);
	return tile;
}
SpriteFrame::SpriteFrame(const sf::FloatRect& bounds, const sf::Color& color) : SpriteFrame(bounds, nullptr, sf::IntRect(0, 0, 1, 1), color) {}
SpriteFrame::SpriteFrame(const sf::FloatRect& bounds, const sf::Texture* texture, const sf::Color& color) : SpriteFrame(bounds, texture, sf::IntRect(0, 0, texture->getSize().x, texture->getSize().y), color) {}

Body::Body() : LockingObject(), _transform(sf::Transform::Identity), _position(), _origin(), _scale(1.0f, 1.0f), _rotation(0),  _transformNeedUpdate(true), _bounds() {}
void Body::setPosition(const sf::Vector2f& v) {
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(v, _position)) {
		_position = v; _transformNeedUpdate = true; changed(true);
	}
}
void Body::setOrigin(const sf::Vector2f& v) {
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(v, _origin)) { _origin = v; _transformNeedUpdate   = true; }
}

void Body::setScale(const sf::Vector2f& v) {
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(v, _scale)) { _scale = v; _transformNeedUpdate = true; changed(true);
	}
}

void Body::setRotation(float angle) {
	angle = static_cast<float>(std::fmod(angle, 360));
	if (angle < 0) angle += 360.f;
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(angle, _rotation)) { _rotation = angle; _transformNeedUpdate  = true; changed(true);
	}
}


const sf::Transform& Body::getTransform() const {
	auto lock = safeLock();
	if (_transformNeedUpdate) {
		float angle = -_rotation * 3.141592654f / 180.f;
		float cosine = static_cast<float>(std::cos(_rotation));
		float sine = static_cast<float>(std::sin(_rotation));
		float sxc = _scale.x * cosine;
		float syc = _scale.y * cosine;
		float sxs = _scale.x * sine;
		float sys = _scale.y * sine;
		//float tx = -_origin.x * sxc - _origin.y * sys + (int)(_position.x + 0.5f);// _position.x;
		//float ty = _origin.x * sxs - _origin.y * syc + (int)(_position.y + 0.5f); // _position.y;
		float tx = -_origin.x * sxc - _origin.y * sys + _position.x;
		float ty = _origin.x * sxs - _origin.y * syc + _position.y;
		_transform = sf::Transform(sxc, sys, tx,
			-sxs, syc, ty,
			0.f, 0.f, 1.f);
		_transformNeedUpdate = false;
	}
	return _transform;
}
