#include "Global.h"
#include "Drawables.h"
#include <array>
#include <chrono>

#if _WIN32
#include <Windows.h>
#endif
using namespace sf;

struct triangle_stripper {
	// http://www.plunk.org/~grantham/public/meshifier/oldmesh.html
	typedef float REAL;
	struct Vertex {
		sf::Vertex v;
		std::vector<std::array<int,2>> conn; /* [0] == index of connected vertex */
									/* [1] == index of connecting edge */
		bool operator==(const Vertex& other) const { return almost_equal_to<sf::Vertex>()(v,other.v); }
		Vertex(const sf::Vertex& v) : v(v) {}
	};
	struct Edge { std::array<int, 2> t; Edge(int a, int b) { t[0] = a; t[1] = b; } };/* triangles sharing this edge */
	struct Triangle {
		std::array<int, 3>	v;			/* indices of vertices */
		int	tcnt;						/* # of adjacent triangles */
		std::array<int, 3>	t;			/* adjacent triangles */
		bool done;					/* been inserted into strip? */
		int	next;					/* next triangle in strip */
		Triangle(int v1, int v2, int v3) : tcnt(0), done(false), next(-1) {
			v[0] = v1; v[1] = v2; v[2] = v3;
		}
	};

#define	MESH_DRAWSTRIPOUTLINES	0
#define	MESH_DRAWCONNECTIVITY	1
#define	MESH_PRINTSTRIPCONTENTS	2
#define	MESH_PRINTADDTRIANGLES	3
#define MAX_TRIS 100000
	typedef void(*meshDrawTextProc)(REAL v[3], const char *text);
	typedef void(*meshDrawLineProc)(REAL v1[3], REAL v2[3]);
	typedef void(*meshDrawEdgeProc)(Vertex *v1, Vertex *v2);
	typedef void(*meshBeginStripProc)(Vertex *v1, Vertex *v2);
	typedef void(*meshContStripProc)(Vertex *v);
	typedef void(*meshEndStripProc)(void);
	std::vector<Vertex>	verts;
	std::vector<Triangle> ts;
	std::vector<Edge> edges;

	int 		doDrawStripOutlines = 0;
	int 		doDrawConnectivity = 0;
	int 		doPrintStripContents = 0;
	int 		doPrintAddTriangles = 0;
	size_t notSharedVertex(size_t	t1, size_t	t2)
	{
		if ((ts[t1].v[0] != ts[t2].v[0]) &&
			(ts[t1].v[0] != ts[t2].v[1]) &&
			(ts[t1].v[0] != ts[t2].v[2]))
		{
			return(ts[t1].v[0]);
		}

		if ((ts[t1].v[1] != ts[t2].v[0]) &&
			(ts[t1].v[1] != ts[t2].v[1]) &&
			(ts[t1].v[1] != ts[t2].v[2]))
		{
			return(ts[t1].v[1]);
		}

		/*
		* Well, must be the last one; if they shared all three, then they
		*  would be the same triangle.
		*/

		return(ts[t1].v[2]);
	}
	int firstSharedVertex(size_t t1, size_t t2)
	{
		if ((ts[t1].v[0] == ts[t2].v[0]) ||
			(ts[t1].v[0] == ts[t2].v[1]) ||
			(ts[t1].v[0] == ts[t2].v[2]))
		{
			return(ts[t1].v[0]);
		}

		if ((ts[t1].v[1] == ts[t2].v[0]) ||
			(ts[t1].v[1] == ts[t2].v[1]) ||
			(ts[t1].v[1] == ts[t2].v[2]))
		{
			return(ts[t1].v[1]);
		}

		/*
		* Well, can't be either; if this was the first shared, it's the ONLY
		* one shared.
		*/
		throw std::exception("error here, might not ever get here");
		fprintf(stderr, "firstSharedVertex: only one shared?  Internal error.\n");
		exit(1);
	}
	int secondSharedVertex(size_t t1, size_t t2)
	{
		if ((ts[t1].v[2] == ts[t2].v[0]) ||
			(ts[t1].v[2] == ts[t2].v[1]) ||
			(ts[t1].v[2] == ts[t2].v[2]))
		{
			return(ts[t1].v[2]);
		}

		if ((ts[t1].v[1] == ts[t2].v[0]) ||
			(ts[t1].v[1] == ts[t2].v[1]) ||
			(ts[t1].v[1] == ts[t2].v[2]))
		{
			return(ts[t1].v[1]);
		}

		/*
		* Well, can't be either; if we think the first vertex was the
		* SECOND shared, something is way out of whack.
		*/
		throw std::exception("error here, might not ever get here");
		fprintf(stderr, "secondSharedVertex: less than two shared?  Internal "
			"error.\n");
		exit(1);
	}

	bool triangleUsesVertex(size_t t, size_t v)
	{
		if ((v == ts[t].v[0]) || (v == ts[t].v[1]) || (v == ts[t].v[2]))
			return true;
		return false;
	}
	void followStrip(std::vector<int>& strip, int last)
	{
		int	i;
		int	next;

		while (true) { // *count < MAX_STRIP) {

			/* find next triangle to add */
			next = -1;
			if (strip.size() == 0) {
				/* for 2nd tri, just pick one. */
				for (i = 0; i < ts[last].tcnt; i++)
					if (!ts[ts[last].t[i]].done) {
						next = ts[last].t[i];
						break;
					}
				if (next != -1) {
					strip.push_back(notSharedVertex(last, next));
					strip.push_back(firstSharedVertex(last, next));
					strip.push_back(secondSharedVertex(last, next));
				}
				else {
					strip.push_back(ts[last].v[0]);
					strip.push_back(ts[last].v[1]);
					strip.push_back(ts[last].v[2]);
				}
			}
			else {
				/* third and later tris must share prev two verts */
				for (i = 0; i < ts[last].tcnt; i++)
					if (!ts[ts[last].t[i]].done &&
						triangleUsesVertex(ts[last].t[i], strip[strip.size() - 2]) &&
						triangleUsesVertex(ts[last].t[i], strip[strip.size() - 1]))
					{
						next = ts[last].t[i];
						break;
					}
			}

			if (next == -1) break;

			//	if (doPrintAddTriangles) fprintf(stderr, "adding %d to strip.\n", next);

			strip.push_back(notSharedVertex(next, last));
			ts[next].done = true;
			last = next;
		}
	}
	void getNextStrip(int start, std::vector<int> strip, bool gready = false, bool nostrips = false)
	{
		std::vector<int> istrip;
#if !GREEDY

#endif /* GREEDY */

		if (nostrips) {
			strip.push_back(ts[start].v[0]);
			strip.push_back(ts[start].v[1]);
			strip.push_back(ts[start].v[2]);
			ts[start].done = true;
		}
		else {
			/* do greedy strips */

			/* start is the start triangle */

			if (!gready) {
				int min = -1, mincnt = 4;
				/* do slightly more wise strips; pick one of least connected
				triangles */

				for (size_t i = 0; i < ts.size(); i++)
					if ((!ts[i].done) && (ts[i].tcnt < mincnt))
					{
						min = i;
						mincnt = ts[i].tcnt;
					}

				if (min != -1) start = min;
				else
				{
					throw new std::exception("internal error");
					fprintf(stderr, "getNextStrip: min == -1?  Internal Error!\n");
					exit(1);
				}
			}

			ts[start].done = true;
			followStrip(istrip, start);
			strip.reserve(istrip.size() * 3);
			for (size_t i = 0; i < istrip.size(); i++)
				strip.push_back(istrip[istrip.size() - i - 1]);

			followStrip(strip, start);
		}
	}
	void addEdge(int tnum, int vnum1, int vnum2)
	{
		Triangle		*t, *t2;
		Vertex		*v1, *v2;
		size_t			e;	/* index of edge record */

		t = &ts[tnum];
		v1 = &verts[vnum1];
		v2 = &verts[vnum2];

		/* set up edge between v1 and v2 unless one exists */
		
		for (e = 0; (e < v1->conn.size()) && (v1->conn.at(e).at(0) != vnum2); e++);

		if (e < v1->conn.size()) {
			/* found existing edge */
			e = v1->conn.at(e).at(1);
			edges.at(e).t[1] = tnum;
			t2 = &ts[edges[e].t[0]];
			t->t[t->tcnt++] = edges[e].t[0];
			t2->t[t2->tcnt++] = tnum;

		}
		else {
			/* have to make new edge */
			v1->conn.emplace_back(std::array<int, 2>{vnum2, (int)e});
			v2->conn.emplace_back(std::array<int, 2>{vnum1, (int)e});
			edges.emplace_back(tnum, -1);   
		}
	}
	void reset(void)
	{
		ts.clear();
		this->ts.clear();
		this->verts.clear();
	}
	void prepareTriangle(int t)
	{
		/* set up edge connectivity */

		addEdge(t, ts[t].v[0], ts[t].v[1]);
		addEdge(t, ts[t].v[1], ts[t].v[2]);
		addEdge(t, ts[t].v[2], ts[t].v[0]);
	}
	void addVert(const sf::Vertex& v) {
		verts.emplace_back(v);
	}
		
	void addTriangle(size_t v1, size_t v2, size_t v3) {
		ts.emplace_back(v1, v2, v3);
		prepareTriangle(ts.size() - 1);
	}

};
namespace global {
	
	// just a hack for now, need to build more conversions
	
	void convert(
		const std::vector<sf::Vertex>& from, sf::PrimitiveType from_type,
		std::vector<sf::Vertex>& to, sf::PrimitiveType to_type, bool clear) {
		if (from_type == to_type) {
			if (clear) to = from; else to.insert(to.end(),from.begin(),from.end());
		}
		else {
			if (clear) to.clear();
			switch (to_type) {
			case sf::PrimitiveType::Triangles:
				switch (from_type) {
				case sf::PrimitiveType::TrianglesStrip:
					to.reserve(to.size() + from.size()+2);
					for (size_t v = 0; v < from.size() - 2; v++) {
						if (v & 1) {
							to.push_back(from[v]);
							to.push_back(from[v + 1]);
							to.push_back(from[v + 2]);
						}
						else {
							to.push_back(from[v]);
							to.push_back(from[v + 2]);
							to.push_back(from[v + 1]);
						}
					}
					return;
				case sf::PrimitiveType::Quads:
					to.reserve(to.size() + (from.size()/4) *6);
					for (size_t v = 0; v < from.size(); v += 4) {
						to.push_back(from[v]);
						to.push_back(from[v + 1]);
						to.push_back(from[v + 2]);
						to.push_back(from[v + 2]);
						to.push_back(from[v + 3]);
						to.push_back(from[v + 1]);
					}
					return;
				}
			}
			// FUCK MEE
			assert(false);
		}
	}

	std::vector<sf::Vertex> convert(const std::vector<sf::Vertex>& from, sf::PrimitiveType from_type, sf::PrimitiveType to_type) {
		std::vector<sf::Vertex> to;
		convert(from, from_type, to, to_type);
		return to;
	}
	void convert(const sf::VertexArray& from, std::vector<sf::Vertex>& to, sf::PrimitiveType to_type, bool clear ) {
		if (from.getPrimitiveType() == to_type) {
			const sf::Vertex* bptr = &from[0];
			const sf::Vertex* eptr = bptr + from.getVertexCount();
			if (clear) to.assign(bptr, eptr); else to.insert(to.end(), bptr, eptr);
		}
		else {
			if (clear) to.clear();
			switch (to_type) {
			case sf::PrimitiveType::Triangles:
				switch (from.getPrimitiveType()) {
				case sf::PrimitiveType::TrianglesStrip:
					to.reserve(to.size() + from.getVertexCount());
					for (size_t v = 0; v < from.getVertexCount() - 2; v++) {
						if (v & 1) {
							to.push_back(from[v]);
							to.push_back(from[v + 1]);
							to.push_back(from[v + 2]);
						}
						else {
							to.push_back(from[v]);
							to.push_back(from[v + 2]);
							to.push_back(from[v + 1]);
						}
					}
					return;
				case sf::PrimitiveType::Quads:
					to.reserve(to.size() + from.getVertexCount());
					for (size_t v = 0; v < from.getVertexCount() ; v+=4) {
						to.push_back(from[v]);
						to.push_back(from[v + 1]);
						to.push_back(from[v + 2]);
						to.push_back(from[v + 2]);
						to.push_back(from[v + 3]);
						to.push_back(from[v + 1]);
					}
					return;
				}
				break;
			}
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



struct VertexArrayHack { // : public sf::Drawable {
	sf::Drawable* vtable_ptr_to_drawable;
	//virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {}
	std::vector<sf::Vertex> m_vertices;      ///< Vertices contained in the array
	sf::PrimitiveType       m_primitiveType; ///< Type of primitives to draw
};
RawVertices::RawVertices(sf::VertexArray&& right) : _verts(std::move(reinterpret_cast<VertexArrayHack*>(&right)->m_vertices)), _ptype(right.getPrimitiveType()) {}

RawVertices& RawVertices::operator=(sf::VertexArray&& right) {
	if (primitive_type() == right.getPrimitiveType()) {
		// this is SOO super hacky to get the vertices from sf::VertexArray, but I want to move it!
		VertexArrayHack* ptr = reinterpret_cast<VertexArrayHack*>(&right);
		_verts = std::move(ptr->m_vertices);
	}
	else {
		// else just copy,bleh
		_verts.clear();
		append(right);
	}
	return *this;
}

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
