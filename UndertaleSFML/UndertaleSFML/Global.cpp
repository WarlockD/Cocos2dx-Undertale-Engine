#include "Global.h"
#include <array>
#include <chrono>

#if _WIN32
#include <Windows.h>
#endif
using namespace sf;


void Mesh::push_back(const std::vector<sf::Vertex>& v) {
	auto lock = safeLock();
	_verts.insert(_verts.end(), v.begin(), v.end());
}
void Mesh::push_back(const Mesh& mesh) {
	auto mlock = mesh.safeLock();
	push_back(mesh.getVerteicsVector());
}
Mesh::Mesh(const Mesh& copy, const sf::Transform& t) : _verts(copy._verts) {
	transformPositions(t);
}

SpriteFrame::SpriteFrame(const sf::Texture* texture, const sf::IntRect& textureRect, const sf::FloatRect& bounds) :_texture(texture), _textureRect(textureRect), _bounds(bounds) {
	float left = bounds.left;
	float top = bounds.top;
	float right = bounds.left + bounds.width;
	float bottom = bounds.top + bounds.height;

	float u1 = static_cast<float>(textureRect.left);
	float v1 = static_cast<float>(textureRect.top);
	float u2 = static_cast<float>(textureRect.left + textureRect.width);
	float v2 = static_cast<float>(textureRect.top + textureRect.height);
	sf::Vertex* verts = _verts;
	// Add a quad for the current character
	*verts++ = (Vertex(Vector2f(left, top), sf::Color::White, Vector2f(u1, v1)));
	*verts++ = (Vertex(Vector2f(right, top), sf::Color::White, Vector2f(u2, v1)));
	*verts++ = (Vertex(Vector2f(left, bottom), sf::Color::White, Vector2f(u1, v2)));
	*verts++ = (Vertex(Vector2f(left, bottom), sf::Color::White, Vector2f(u1, v2)));
	*verts++ = (Vertex(Vector2f(right, top), sf::Color::White, Vector2f(u2, v1)));
	*verts++ = (Vertex(Vector2f(right, bottom), sf::Color::White, Vector2f(u2, v2)));
}


sf::FloatRect Mesh::getVerteicsBounds() const {
	auto lock = safeLock();
	if (!_verts.empty())
	{
		float left = _verts[0].position.x;
		float top = _verts[0].position.y;
		float right = _verts[0].position.x;
		float bottom = _verts[0].position.y;
		for (const auto& v : _verts) {
			sf::Vector2f position = v.position;
			// Update left and right
			if (position.x < left)
				left = position.x;
			else if (position.x > right)
				right = position.x;

			// Update top and bottom
			if (position.y < top)
				top = position.y;
			else if (position.y > bottom)
				bottom = position.y;
		}
		return sf::FloatRect(left, top, right - left, bottom - top);
	}
	else
	{
		// Array is empty
		return sf::FloatRect();
	}
}
void Mesh::transformPositions(const sf::Transform& trasform) {
	auto lock = safeLock();
	if (!_verts.empty()) {
		for (auto& v : _verts) v.position = trasform.transformPoint(v.position);
	}
}
Body::Body() : LockingObject(), _transform(sf::Transform::Identity),_position(), _origin(), _scale(1.0f,1.0f), _rotation(0), _changed(false), _transformNeedUpdate(true) {}
void Body::setPosition(const sf::Vector2f& v) { 
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(v, _position)) { 
		_position = v; _transformNeedUpdate = _changed = true; 
	} 
}
void Body::setOrigin(const sf::Vector2f& v) { 
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(v, _origin)) { _origin = v; _transformNeedUpdate = _changed = true; } 
}

void Body::setScale(const sf::Vector2f& v) { 
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(v, _scale)) { _scale = v; _transformNeedUpdate = _changed = true; } 
}

void Body::setRotation(float angle) {
	angle = static_cast<float>(std::fmod(angle, 360));
	if (angle < 0) angle += 360.f;
	auto lock = safeLock();
	if (!global::AlmostEqualRelative(angle, _rotation)) { _rotation = angle; _transformNeedUpdate = _changed = true; }
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
		float tx = -_origin.x * sxc - _origin.y * sys + _position.x;
		float ty = _origin.x * sxs - _origin.y * syc + _position.y;

		_transform = sf::Transform(sxc, sys, tx,
			-sxs, syc, ty,
			0.f, 0.f, 1.f);
		_transformNeedUpdate = false;
	}
	return _transform;
}


const std::string global::empty_string;

// all from http://www.codeproject.com/Articles/1053/Using-an-output-stream-for-debugging
class basic_debugbuf : public std::streambuf {
protected:
	std::array<char,1024> _buffer;
	std::streambuf * _oldBuffer;
	int _lastc;
	int_type overflow(int_type ch) override
	{
		if (ch == '\n' || ch == '\r')
		{
			if (_lastc != ch && (_lastc == '\n' || _lastc == '\r')) {
				// skip, throw it away
				_lastc = 0;
				return 0;
			} 
			*pptr() = '\n';
			pbump(1);
			sync();
		}
		else if (ch == traits_type::eof()) {
			*pptr() = '\n';
			pbump(1);
			sync();
			return traits_type::eof();
		}
		else {
			*pptr() = ch;
			pbump(1);
		}
		return 0;
	}
	static bool is_string_empty_or_whitespace(const std::string& str) {
		bool empty = true;
		if (str.length() > 0) {
			for (char c : str) if (!isspace(c)) { empty = false; break; }
		}
		return empty;
	}
	int sync() override
	{
		std::string str(pbase(), pptr());
		if (!is_string_empty_or_whitespace(str)) { // check for empty sync
			auto now = std::chrono::system_clock::now();
			auto in_time_t = std::chrono::system_clock::to_time_t(now);
			std::stringstream ss;
			ss << "[" << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << "]:" << str << std::endl;

			output_debug_string(ss.str().c_str());
			if (_oldBuffer) _oldBuffer->sputn(ss.str().c_str(), ss.str().length());
		}
		std::ptrdiff_t n = pptr() - pbase();
		pbump(-n); // clear the buffer
		return 0;
	}

	void output_debug_string(const char *text) {
		::OutputDebugStringA(text);
	}

public:
	virtual ~basic_debugbuf()
	{
		sync();
	}
	basic_debugbuf() : std::streambuf(), _oldBuffer(nullptr), _lastc(0) {
		char *base = _buffer.data();
		setp(base, base + _buffer.size() - 1); // -1 to make overflow() easier
	}
	void setOldBuffer(std::streambuf* old_buffer) { _oldBuffer = old_buffer; }
};




template<class CharT, class TraitsT = std::char_traits<CharT> >
class basic_dostream : public std::basic_ostream<CharT, TraitsT>
{
public:
	basic_dostream() : std::basic_ostream<CharT, TraitsT>
		(new basic_debugbuf<CharT, TraitsT>()) {}
	~basic_dostream()
	{
		delete rdbuf();
	}

	 void setOldBuffer(std::basic_stringbuf<CharT, TraitsT>* old_buffer) {
		auto debug_stream = dynamic_cast<basic_debugbuf<CharT, TraitsT>*>(rdbuf());
		debug_stream->setOldBuffer(old_buffer);
	}
};

typedef basic_dostream<char>    dostream;
typedef basic_dostream<wchar_t> wdostream;

basic_debugbuf s_cerr_debug_buffer;
basic_debugbuf s_cout_debug_buffer;

namespace logging {
	void init_cerr() {
		s_cerr_debug_buffer.setOldBuffer(std::cerr.rdbuf());
		std::cerr.rdbuf(&s_cerr_debug_buffer);
		std::cerr << "cerr redirected" << std::endl;
	}
	void init_cout() {
		s_cout_debug_buffer.setOldBuffer(std::cout.rdbuf());
		std::cout.rdbuf(&s_cout_debug_buffer);
		std::cout << "cout redirected" << std::endl;
	}
	bool init() {
		return true;
	}
	void error(const std::string& message) {

	}
};
