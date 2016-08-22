#include "Global.h"
#include "Drawables.h"
#include <array>
#include <chrono>

#if _WIN32
#include <Windows.h>
#endif
using namespace sf;

namespace {


};
namespace global {
	void convert(
		const std::vector<sf::Vertex>& from, sf::PrimitiveType from_type,
		std::vector<sf::Vertex>& to, sf::PrimitiveType to_type, bool clear) {
		if (from_type == to_type) {
			if (clear) to = from; else to.insert(to.end(),from.begin(),from.end());
		}
		else {
			if (clear) to.clear();
			// FUCK MEE
			assert(false);
		}
	}

	std::vector<sf::Vertex> convert(const std::vector<sf::Vertex>& from, sf::PrimitiveType from_type, sf::PrimitiveType to_type) {
		std::vector<sf::Vertex> to;
		convert(from, from_type, to, to_type);
		return to;
	}
	void convert(const sf::VertexArray& from, std::vector<sf::Vertex>& to, sf::PrimitiveType to_type, bool clear) {
		if (from.getPrimitiveType() == to_type) {
			const sf::Vertex* bptr = &from[0];
			const sf::Vertex* eptr = bptr + from.getVertexCount();
			if (clear) to.assign(bptr,eptr); else to.insert(to.end(), bptr,eptr);
		}
		else {
			if (clear) to.clear();
			// FUCK MEE
			assert(false);
		}
	}
	std::vector<sf::Vertex> convert(const sf::VertexArray& from, sf::PrimitiveType to_type) {
		std::vector<sf::Vertex> to;
		convert(from, to, to_type);
		return to;
	}
};

const std::string global::empty_string;
struct VertexRef;
namespace cvt_triangles {
	struct Vertex;
	struct Triangle {
		Vertex *FinalVert;
	};
	struct Edge {
		Vertex *V2;
		int Count;
		int TriangleCount;
		Triangle *Triangles;
	};
	struct Vertex {
		
		size_t hash;  // vertex hash
		sf::Vertex V;
		size_t count;
		std::vector<size_t> PointsToMe;
		std::vector<size_t> Edges;
		Vertex *next;
		Vertex(const sf::Vertex& v) : V(v), count(1), next(nullptr), hash(std::hash<sf::Vertex>()(v)) {}
	};
	struct vertex_hasher {
		constexpr size_t operator()(const Vertex& v)  const { return v.hash; }
	};
	struct vertex_equals {
		constexpr bool operator()(const Vertex& l, const Vertex& r)  const { return almost_equal_to<sf::Vertex>()(l.V, r.V); }
	};
	struct Book {
		typedef std::unordered_set<Vertex, vertex_hasher, vertex_equals> vertex_container;
		typedef vertex_container::iterator iterator;

		std::unordered_set<Vertex, vertex_hasher> _vertexs;
		//iterator incVertex(const sf::Vertex& v) {
			//std::pair<iterator, bool>  it = _vertexs.emplace(v);
		//	if (!it.second) {
		//		(*it.first).count++;
		//	}
		//}
	};
};






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
