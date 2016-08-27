#pragma once

// some custome math routines
#include <cmath>
#include <cinttypes>
#include <type_traits>
#include <limits>
#include <cassert>
#include  <algorithm>

#ifdef INFINITY
#undef INFINITY
#endif

#ifdef FLT_EPSILON
#undef FLT_EPSILON
#endif


namespace umath {


#define FLOATSIGNBITSET(f)		((*(const unsigned long *)&(f)) >> 31)
#define FLOATSIGNBITNOTSET(f)	((~(*(const unsigned long *)&(f))) >> 31)
#define FLOATNOTZERO(f)			((*(const unsigned long *)&(f)) & ~(1<<31) )
#define INTSIGNBITSET(i)		(((const unsigned long)(i)) >> 31)
#define INTSIGNBITNOTSET(i)		((~((const unsigned long)(i))) >> 31)
		constexpr unsigned int  IEEE_FLT_MANTISSA_BITS = 23;
		constexpr unsigned int  IEEE_FLT_EXPONENT_BITS = 8;
		constexpr unsigned int IEEE_FLT_EXPONENT_BIAS = 127;
		constexpr unsigned int  IEEE_FLT_SIGN_BIT = 31;

		constexpr unsigned int IEEE_DBL_MANTISSA_BITS = 52;
		constexpr unsigned int IEEE_DBL_EXPONENT_BITS = 11;
		constexpr unsigned int  IEEE_DBL_EXPONENT_BIAS = 1023;
		constexpr unsigned int  IEEE_DBL_SIGN_BIT = 63;

		constexpr unsigned int  IEEE_DBLE_MANTISSA_BITS = 63;
		constexpr unsigned int  IEEE_DBLE_EXPONENT_BITS = 15;
		constexpr unsigned int  IEEE_DBLE_EXPONENT_BIAS = 0;
	constexpr unsigned int IEEE_DBLE_SIGN_BIT = 79;
	constexpr bool FLOAT_IS_NAN(float x) { return (((*(const unsigned long *)&x) & 0x7f800000) == 0x7f800000); }
	constexpr bool FLOAT_IS_INF(float x) { return (((*(const unsigned long *)&x) & 0x7fffffff) == 0x7f800000); }
	constexpr bool FLOAT_IS_IND(float x) { return ((*(const unsigned long *)&x) == 0xffc00000); }
	constexpr bool FLOAT_IS_DENORMAL(float x) { return (((*(const unsigned long *)&x) & 0x7f800000) == 0x00000000 && ((*(const unsigned long *)&x) & 0x007fffff) != 0x00000000); }

	template<class T> inline int	MaxIndex(T x, T y) { return  (x > y) ? 0 : 1; }
	template<class T> inline int	MinIndex(T x, T y) { return (x < y) ? 0 : 1; }

	template<class T> inline T	Max3(T x, T y, T z) { return (x > y) ? ((x > z) ? x : z) : ((y > z) ? y : z); }
	template<class T> inline T	Min3(T x, T y, T z) { return (x < y) ? ((x < z) ? x : z) : ((y < z) ? y : z); }
	template<class T> inline int	Max3Index(T x, T y, T z) { return (x > y) ? ((x > z) ? 0 : 2) : ((y > z) ? 1 : 2); }
	template<class T> inline int	Min3Index(T x, T y, T z) { return (x < y) ? ((x < z) ? 0 : 2) : ((y < z) ? 1 : 2); }

	template<class T> inline T	Sign(T f) { return (f > 0) ? 1 : ((f < 0) ? -1 : 0); }
	template<class T> inline T	Square(T x) { return x * x; }
	template<class T> inline T	Cube(T x) { return x * x * x; }
	constexpr float	PI = 3.14159265358979323846264338327950288f; // 3.14159265358979323846f;				// pi
	constexpr float	TWO_PI = 2.0f * PI;							// pi * 2
	constexpr float	HALF_PI = 0.5f * PI;						// pi / 2
	constexpr float	ONEFOURTH_PI = 0.25f * PI;					// pi / 4
	constexpr float	E = 2.71828182845904523536f;				// e
	constexpr float	SQRT_TWO = 1.41421356237309504880f;			// sqrt( 2 )
	constexpr float	SQRT_THREE = 1.73205080756887729352f;		// sqrt( 3 )
	constexpr float	SQRT_1OVER2 = 0.70710678118654752440f;		// sqrt( 1 / 2 )
	constexpr float	SQRT_1OVER3 = 0.57735026918962576450f;		// sqrt( 1 / 3 )
	constexpr float	M_DEG2RAD = PI / 180.0f;					// degrees to radians multiplier
	constexpr float	M_RAD2DEG = 180.0f / PI;					// radians to degrees multiplier
	constexpr float	M_SEC2MS = 1000.0f;							// seconds to milliseconds multiplier
	constexpr float	M_MS2SEC = 0.001f;							// milliseconds to seconds multiplier
	constexpr float	INFINITY = 1.0e30f;							// huge number which should be larger than any valid number used
	constexpr float	FLT_EPSILON = 1.192092896e-07f;				// smallest positive number such that 1.0+FLT_EPSILON != 1.0
	// id Rsquare root...just wow
	inline float rsqrt(float x) {
		long i;
		float y, r;

		y = x * 0.5f;
		i = *reinterpret_cast<long *>(&x);
		i = 0x5f3759df - (i >> 1);
		r = *reinterpret_cast<float *>(&i);
		r = r * (1.5f - r * r * y);
		return r;
	}
	inline float isqrt(float x) { return 1.0f / std::sqrtf(x); }
	inline bool is_power_of_two(int x) {
		return (x & (x - 1)) == 0 && x > 0;
	}
	inline int BitReverse(int x) {
		x = (((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1));
		x = (((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2));
		x = (((x >> 4) & 0x0f0f0f0f) | ((x & 0x0f0f0f0f) << 4));
		x = (((x >> 8) & 0x00ff00ff) | ((x & 0x00ff00ff) << 8));
		return ((x >> 16) | (x << 16));
	}
	
#if 0
	inline float Fabs(float f) {
		int tmp = *reinterpret_cast<int *>(&f);
		tmp &= 0x7FFFFFFF;
		return *reinterpret_cast<float *>(&tmp);
	}
#endif
	constexpr float DegreesToRadians(float a) { return a*M_DEG2RAD; }
	constexpr float RadiansToDegrees(float a) { return a*M_RAD2DEG; }
	template<class T, class Compare>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi, Compare comp)
	{
		return assert(!comp(hi, lo)),
			comp(v, lo) ? lo : comp(hi, v) ? hi : v;
	}
	template<class T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi)
	{
		return clamp(v, lo, hi, std::less<>());
	}
	template<typename T> constexpr T pow_const(T x, unsigned int y) { return y > 1 ? x * pow_const(x, y - 1) : x; }
	template<typename T> inline T pow(T x, unsigned int y) { T r = x; while (y-- > 1) r *= x; return r; }
	template<typename T> constexpr T abs_const(T v) { return v >= 0 ? v : -v; }
	template<typename T> inline T abs(T v) { return std::abs(v); }
	// crazy, but works?
	template<> constexpr int abs_const<int>(int x) { return (pow_const(x, (x >> 31)) - (x >> 31)); }
	template<> inline float abs<float>(float f) { int t = *reinterpret_cast<int *>(&f) & 0x7FFFFFFF;  return  *reinterpret_cast<float *>(&t); }
	template<> inline int abs<int>(int x) { int y = x >> 31; return ((x ^ y) - y); }
	template<typename T> inline T floor(T v) { return std::floor(v); }
	template<typename T> inline T ceil(T v) { return std::ceil(v); }
	template<typename T> inline T round(T v) { return umath::floor(v + ((T)0.5)); }
	//template<typename T> T clamp(T v, T min, T max) { return v < min ? min : v > max ? max : v; }

	
	// constexpr sin
	//http://forum.devmaster.net/t/fast-and-accurate-sine-cosine/9648
	// fast and accurate sine/cosine
	// used to get constexpr from it
	// this is like 12% off
	constexpr float sin_const(float x) {
#define _B (4.0f / PI)
#define _C (-4.0f / (PI*PI))
		return _B * x + _C * x * abs_const(x);
#undef _B
#undef _C
	}
	// better but close to 0.2%
	constexpr float sin_accurate_const(float x,bool accurate=false) {
#define _Y (sin_const(x))
		return  0.225f * (_Y *  abs_const(_Y)- _Y) + _Y;
#undef _Y
	}
	inline float sin_cos(float a, float &s, float &c) {
		// looking this up, but a few people say its slow? humm
#ifdef _WIN32
		_asm {
			fld		a
			fsincos
			mov		ecx, c
			mov		edx, s
			fstp	dword ptr[ecx]
			fstp	dword ptr[edx]
		}
#else
		s = std::sinf(a);
		c = std::cosf(a);
#endif
	}
	inline int FtoiFast(float f) {
#ifdef _WIN32
		int i;
		__asm fld		f
		__asm fistp		i		// use default rouding mode (round nearest)
		return i;
#else
		return (int)f;
#endif
	}
	template<typename T> inline T sqrt(T a) { return std::sqrt(a); }
	template<typename T> inline T lerp(T v1, T v2, T amount) { return v1 + (amount * (v2 - v1)); }
	inline float angle_normalize_full(float angle) { return ((angle >= 360.0f) || (angle < 0.0f)) ? angle - std::floorf(angle / 360.0f) * 360.0f : angle; }
	inline float angle_normalize_half(float angle) { return angle_normalize_full(angle) > 180.f ? angle_normalize_full(angle) - 360.0f : angle_normalize_full(angle); }
	inline float angle_delta(float a1, float a2) { return angle_normalize_half(a1 - a2); }

	inline int SecondsToMilliSeconds(float t) { return FtoiFast(t* M_SEC2MS); }
	inline float MilliSecondsToSeconds(int t) { return t* M_MS2SEC; }
	inline uint16_t ANGLE2SHORT(float x) { return FtoiFast((x) * 65536.0f / 360.0f) & 65535; }
	inline float SHORT2ANGLE(uint16_t x) { return (x) * (360.0f / 65536.0f); }
	inline uint8_t ANGLE2BYTE(float x) { return FtoiFast((x) * 256.0f / 360.0f) & 255; }
	inline float BYTE2ANGLE(uint8_t x) { return (x) * (360.0f / 256.0f); }
	// C++14 stuff
	template<class T> inline const T& max(const T& a, const T& b) { return (a < b) ? b : a; }
	template<class T, class Compare> inline const T& max(const T& a, const T& b, Compare comp) { return (comp(a, b)) ? b : a; }
	template<class T> inline const T& min(const T& a, const T& b) { return (b < a) ? b : a; }
	template<class T, class Compare> const T& min(const T& a, const T& b, Compare comp) { return (comp(b, a)) ? b : a; }
	template<class T> inline std::pair<const T&, const T&> minmax(const T& a, const T& b) { return (b < a) ? std::pair<const T&, const T&>(b, a) : std::pair<const T&, const T&>(a, b); }
	template<class T, class Compare> inline std::pair<const T&, const T&> minmax(const T& a, const T& b, Compare comp){return comp(b, a) ? std::pair<const T&, const T&>(b, a): std::pair<const T&, const T&>(a, b);}

	template<typename T> bool compare(T a, T b) { return std::equal_to<T>(a, b); }
	template<typename T> 
	typename std::enable_if<std::is_floating_point<T>::value,bool>::type 
		compare(T a, T b, T epslon) { 
		// Calculate the difference.
		T diff = umath::abs(a - b); 
		a = umath::abs(a); 
		b = umath::abs(b);
		return diff <= (umath::max(a, b) * epslon);
	}
	template<typename T>
	typename std::enable_if<std::is_floating_point<T>::value, bool>::type
		compare(const T* a, const T* b, size_t count, T epslon) {
		while (count-- > 0) {
			if (!compare(*a++, *b++, epslon)) return false;
		}
		return true;
	}
	// tired of the limitations, so my personal vec class
	// the vec class just adds some traits

	
	template<class, class = void> struct vector_traits_base {};// empty for non-vectors
	template<class V> struct vector_traits_base<V, std::void_t<typename V::type, typename V::vec_base_type>>
	{	// defined if types exist
		typedef	typename V::type type;
		typedef	typename V::base_type vec_base_type;
	};
	template<class V>struct vector_traits : vector_traits_base<V> {};	// get traits from iterator _Iter, if possible
	template<class T, class = void> struct is_vector : std::false_type {};// default definition
	template<class T> struct is_vector<T, std::void_t<typename vector_traits<T>::vec_base_type>> : std::true_type {};

	template<typename T, size_t DIM> struct vec_base {
		typedef typename T type;
		typedef typename vec_base<T, DIM> vec_base_type;
		static constexpr size_t dimensions = DIM;
		T ptr[DIM];
		T& operator[](size_t i) { assert(i < dimensions); return ptr[i]; }
		const T& operator[](size_t i) const { assert(i < dimensions); return ptr[i]; }
		vec_base() { std::memset(ptr, 0.0f, sizeof(float)*DIM); }
	};
	template<typename T> struct vec_base<T,2> {
		typedef typename T type;
		typedef typename vec_base<T, 2> vec_base_type;
		static constexpr size_t dimensions = 2;
		union { struct { T x; T y; }; T ptr[2]; };
		vec_base() : x((T)0), y((T)0) {}
		vec_base(T x, T y) : x(x), y(y) {}
		T& operator[](size_t i) { assert(i < dimensions); return ptr[i]; }
		const T& operator[](size_t i) const { assert(i < dimensions); return ptr[i]; }
		template<typename D> typename std::enable_if<std::is_base_of<vec_base_type, D>::value, bool>::type
		inline compare(const D &l) const { return x == l.x && y == l.y; }
		template<typename D> typename std::enable_if<std::is_base_of<vec_base_type, D>::value && std::is_floating_point<T>::value,bool>::type
		inline compare(const D &l, T epsilon) const { return umath::compare(x, l.x, epsilon) && umath::compare(y, l.y, epsilon); }
		template<typename D> typename std::enable_if<std::is_base_of<vec_base_type, D>::value, D&>::type
			inline set(T X, T Y) const { x = X; y = Y;  return *((D*)this); }
		template<typename D> typename std::enable_if<std::is_base_of<vec_base_type, D>::value, bool>::type
			inline operator==(const D &r) const { return compare(r); }
		template<typename D> typename std::enable_if<std::is_base_of<vec_base_type, D>::value, bool>::type
			inline operator!=(const D &r) const { return !compare(r); }
		template<typename D> typename std::enable_if<std::is_base_of<vec_base_type, D>::value, D>::type
			inline operator-() const { return D(-x, -y); }
		template<typename D, typename = typename std::enable_if<std::is_convertible<D,T>::value>::type>
			inline void set(D X, D Y) { x = static_cast<T>(X); y = static_cast<T>(Y);  }
	};
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D&>::type
		inline operator*=(D &l, const T &r) { l.x *= r; l.y *= r; return l; }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D&>::type
		inline operator/=(D &l, const T &r) { l.x /= r; l.y /= r; return l; }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D&>::type
		inline operator+=(D &l, const D &r) { x += r.x; y += r.y; return l; }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D&>::type
		inline operator-=(D &l, const D &r) { x -= r.x; y -= r.y; return l; }

	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator+(const D &l, const D &r) { return D(l.x + r.x, l.y + r.y); }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator-(const D &l, const D &r) { return D(l.x - r.x, l.y - r.y); }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator*(const D &l, const D &r) { return D(l.x * r.x, l.y * r.y); }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator*(const D &l, const T &r) { return D(l.x * r, l.y * r); }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator*(const T &l, const D &r) { return D(l * r.x, l * r.x); }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator/(const D &l, const T &r) { return D(l.x / r, l.y / r); }
	template<typename T, typename D> typename std::enable_if<std::is_base_of<vec_base<T, 2>, D>::value, D>::type
		inline operator/(const T &l, const D &r) { return D(l / r.x, l / r.x); }
	

	template<typename T> struct vec_base<T, 3> {
		typedef typename T type;
		typedef typename vec_base<T, 3> vec_base_type;
		static constexpr size_t dimensions = 3;
		union { struct { T x; T y; T z; }; T ptr[3]; };
		vec_base() : x((T)0), y((T)0), z((T)0) {}
		vec_base(T x, T y, T z) : x(x), y(y), z(z) {}
		T& operator[](size_t i) { assert(i < dimensions); return ptr[i]; }
		const T& operator[](size_t i) const { assert(i < dimensions); return ptr[i]; }
		template<typename D>
		typename std::enable_if<std::is_base_of<vec_base_type, D>::value, bool>::type
			inline compare(const D &l) const { return x == l.x && y == l.y && z == l.z; }
		template<typename D>
		typename std::enable_if<std::is_base_of<vec_base_type, D>::value && std::is_floating_point<T>::value, bool>::type
			inline compare(const D &l, T epsilon) const { return umath::compare(x, l.x, epsilon) && umath::compare(y, l.y, epsilon) && umath::compare(z, l.z, epsilon);}
	
		
	};

	template<typename T> struct vec_base<T, 4> {
		typedef typename T type;
		typedef typename vec_base<T, 4> vec_base_type;
		static constexpr size_t dimensions = 4;
		union { struct { T x; T y; T z; T w; }; T ptr[4]; };
		
		vec_base() : x((T)0), y((T)0), z((T)0), w((T)0) {}
		vec_base(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
		T& operator[](size_t i) { assert(i < dimensions); return ptr[i]; }
		const T& operator[](size_t i) const { assert(i < dimensions); return ptr[i]; }
		template<typename D>
		typename std::enable_if<std::is_base_of<vec_base_type, D>::value, bool>::type
		inline compare(const D &l) const { return x == l.x && y == l.y&& z == l.z&& w == l.w; }
		template<typename D>
		typename std::enable_if<std::is_base_of<vec_base_type, D>::value && std::is_floating_point<T>::value, bool>::type
		inline  compare(const D &l, T epsilon) const { return umath::compare(x, l.x, epsilon) && umath::compare(y, l.y, epsilon) && umath::compare(z, l.z, epsilon) && umath::compare(w, l.w, epsilon); }


	};

	template<typename T,typename D> struct color4_base {
		typedef typename T type;
		static constexpr T max = std::is_floating_point<T>::value ? ((T)1) : ((T)std::numeric_limits<T>::max());
		static constexpr T min = std::is_floating_point<T>::value ? ((T)0) : ((T)std::numeric_limits<T>::min());
		typedef typename color4_base<T,D> color_base_type;
		static constexpr size_t dimensions = 4;
		union { struct { T r; T g; T b; T a; }; T ptr[4]; };
		color4_base() : r((T)0), g((T)0), b((T)0), a((T)0) {}
		explicit color4_base(T r, T g, T b, T a=max) : r(r), g(g), b(b), a(a) {}
		explicit color4_base(const T* arr) : r(arr[0]), g(arr[1]), b(arr[2]), a(arr[3]) {}
		T& operator[](size_t i) { assert(i < dimensions); return ptr[i]; }
		const T& operator[](size_t i) const { assert(i < dimensions); return ptr[i]; }
		static inline D blend(const D& l, const D& r) {
			return blend((l.r + r.r) / color4_base::max, (l.g + r.g) / color4_base::max, (l.b + r.b) / color4_base::max);
		}
		static inline D blend(const color4_base& l, const color4_base& r, T alpha) {
			return blend((l.r * alpha + r.r*(max - alpha)) / max, (l.g * alpha + r.g*(max - alpha)) / max, (l.b * alpha + r.b*(max - alpha)) / max);
		}
		inline D& blend(const D& rv) {
			r = (r + rv.r) / max;
			g = (g + rv.g) / max;
			b = (b + rv.b) / max;
			return *this;
		}
		inline D& blend(const D& rv, T alpha) {
			r = (r * alpha + rv.r*(max - alpha)) / max;
			g = (g * alpha + rv.g*(max - alpha)) / max;
			b = (b * alpha + rv.b*(max - alpha)) / max;
			return *this;
		}
	};

	struct color4b : public color4_base<uint8_t, color4b> {
		color4b() : color4_base() {}
		explicit color4b(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : color4_base(r,g,b,a) {}
		explicit color4b(uint32_t v) : color4_base(reinterpret_cast<const uint8_t*>(&v)) {}
		uint32_t as_int() const { return *reinterpret_cast<const uint32_t*>(ptr); }
		color4b& operator*=(uint8_t o) { r *= o; g *= o; b *= b; return *this; }
	};
	struct color4f : public color4_base<float, color4f> {
		color4f() : color4_base() {}
		explicit color4f(float r, float g, float b, float a) : color4_base(r, g, b, a) {}
		explicit color4f(const float* ptr) : color4_base(ptr) {}
		uint32_t as_int() const { return *reinterpret_cast<const uint32_t*>(ptr); }
		color4f& operator*=(float o) { r *= o; g *= o; b *= b; return *this; }
	};

	struct vec2i : public vec_base<int, 2> {
		static const vec2i zero;
		static const vec2i one;
		static const vec2i one_x;
		static const vec2i one_y;
		static const vec2i max;
		static const vec2i min;
		//constructor
		vec2i() : vec_base() {}
		vec2i(int x, int y) : vec_base(x, y) {}

	//	vec2i& set(int X, int Y) { x = X; y = Y; return *this; }
	//	vec2i& set(const vec2i& r) { x = r.x; y = r.y; return *this; }
	//	inline bool operator==(const vec2i &r) const { return compare(r); }
	//	inline bool operator!=(const vec2i &r) const { return !compare(r); }
		

	};


	struct vec2f :public vec_base<float,2> {
		// some simple constants
		static const vec2f zero;
		static const vec2f one;
		static const vec2f one_x;
		static const vec2f one_y;
		static const vec2f max;
		static const vec2f min;
		static const vec2f infinity;
		//constructor
		vec2f() : vec_base() {}
		vec2f(float x, float y) : vec_base(x,y) {}


	//	vec2f& set(float X, float Y) { x = X; y = Y; return *this; }
	//	vec2f& set(const vec2f& r) { x = r.x; y = r.y; return *this; }
	//	inline bool operator==(const vec2f &r) const { return compare(r); }
		//inline bool operator!=(const vec2f &r) const { return !compare(r); }
		//inline vec2f operator-() const { return vec2f(-x, -y); }
		/*
		inline vec2f& operator+=(const vec2f &r) { x += r.x; y += r.y; return *this; }
		inline vec2f& operator-=(const vec2f &r) { x -= r.x; y -= r.y; return *this; }
		inline vec2f& operator*=(const vec2f &r) { x *= r.x; y *= r.y; return *this; }
		inline vec2f& operator*=(const float r) { x *= r; y *= r; return *this; }
		inline vec2f& operator/=(const float r) { float ir = 1.0f / r;  x /= ir; y /= ir; return *this; }
		*/
		inline float length_sqr() const { return (x * x + y * y); }
		inline float length() const { return umath::sqrt(length_sqr()); }
		inline vec2f& length(float new_length) { // sets the lengh of the vector
			if (new_length > 0.0f) {
				float len = length();
				if (len > 0.0f) {
					new_length /= len;
					*this *= new_length; // scale it
				}
				else {
					x = new_length;
					y = 0.0f; // make it a streight line
				}
			}
			else *this = vec2f(); // zero it out
			return *this;
		}
		inline float dot(const vec2f& r)  const { return (r.x * x) + (r.y* y); }
		inline float cross(const vec2f& r)  const { return (r.x * y) - (r.x* y); }
		inline float determinant(const vec2f& a, const vec2f& b) const { return ((a.x - x)*(b.y - y)) - ((a.y - y)*(b.x - x)); }
		inline bool point_in_triangle(const vec2f& a, const vec2f& b, const vec2f& c) const {
			// Use >= to include tests that are on the arc
			return a.determinant(b, *this) >= 0.0f && b.determinant(c, *this) >= 0.0f && c.determinant(a, *this) >= 0.0f;
		}
		inline vec2f& normalize() {
			*this *= isqrt(length_sqr());
			return *this;
		}
		inline vec2f normalize() const {
			float invLength = isqrt(length_sqr());
			return vec2f(x * invLength, y * invLength);
		}
		inline float normalize_fast() {
			float sqrLength = length_sqr();
			float invLength = rsqrt(sqrLength);
			x *= invLength; y *= invLength;
			return invLength * sqrLength;
		}
		inline vec2f& interpolate(const vec2f& from,  float factor) {
			x = ((x - from.x) * factor) + from.x;
			y = ((x - from.y) * factor) + from.y;
			return *this;
		}
		inline vec2f interpolate(const vec2f& from, float factor) const {
			return vec2f(((x - from.x) * factor) + from.x, ((y - from.y) * factor) + from.y);
		}

		inline vec2f& truncate(float length) {
			if (length == 0.0f) x = y = 0.0f;
			else {
				float length2 = length_sqr();
				if (length2 > length * length) {
					float ilength = length * isqrt(length2);
					x *= ilength;
					y *= ilength;
				}
			}
			return *this;
		}
		inline vec2f& clamp(const vec2f &min, const vec2f &max) {
			x = umath::clamp(x, min.x, max.x);
			y = umath::clamp(y, min.y, max.y);
			return *this;
		}
		inline vec2f& snap() { x = std::floor(x + 0.5f);  y = std::floor(y + 0.5f); return *this; }
		inline vec2f& snap_int() { x = static_cast<float>(static_cast<int>(x));  y = static_cast<float>(static_cast<int>(y));; return *this; }
#if 0
		void lerp(const vec2 &v1, const vec2 &v2, const float l) {
			if (l <= 0.0f)  (*this) = v1;
			else if (l >= 1.0f) (*this) = v2;
			else (*this) = v1 + l * (v2 - v1);

		}
#endif
	};
	/*

	inline vec2f operator+(const vec2f&l, const vec2f&r) { return vec2f(l.x + r.x, l.y + r.y); }
	inline vec2f operator-(const vec2f&l, const vec2f&r) { return vec2f(l.x - r.x, l.y - r.y); }
	inline float operator*(const vec2f&l, const vec2f&r) { return l.dot(r); }
	inline vec2f operator*(const vec2f&l, const float r) { return vec2f(l.x*r, l.y*r); }
	inline vec2f operator*(const float r, const vec2f&l) { return vec2f(l.x*r, l.y*r); }
	inline vec2f operator/(const vec2f&l, const float r) { return l * (1.0f / r); }
	*/
	struct vec3f :public vec_base<float, 3> {
		// some simple constants
		static const vec3f zero;
		static const vec3f one;
		static const vec3f one_x;
		static const vec3f one_y;
		static const vec3f one_z;
		static const vec3f max;
		static const vec3f min;
		static const vec3f infinity;
		vec3f() : vec_base() {}
		vec3f(float x, float y, float z) : vec_base(x,y,z) {}

		vec3f& set(float X, float Y, float Z) { x = X; y = Y; z = Z; return *this; }
		vec3f& set(const vec3f& r) { x = r.x; y = r.y; z = r.z; return *this; }
		inline bool operator==(const vec3f &r) const { return compare(r); }
		inline bool operator!=(const vec3f &r) const { return !compare(r); }
		inline vec3f operator-() const { return vec3f(-x, -y, -z); }
		inline vec3f& operator+=(const vec3f &r) { x += r.x; y += r.y; z += r.z;  return *this; }
		inline vec3f& operator-=(const vec3f &r) { x -= r.x; y -= r.y; z -= r.z; return *this; }
		inline vec3f& operator*=(const vec3f &r) { x *= r.x; y *= r.y; z *= r.z; return *this; }
		inline vec3f& operator*=(const float r) { x *= r; y *= r; z *= r; return *this; }
		inline vec3f& operator/=(const float r) { float ir = 1.0f / r;  x /= ir; y /= ir; return *this; }
		inline float length_sqr() const { return (x * x + y * y + z*z); }
		inline float length() const { return umath::sqrt(length_sqr()); }
		inline float dot(const vec3f& r)  const { return (r.x * x) + (r.y* y) + (r.z* z); }
		inline vec3f& cross(const vec3f& r)  {
			x = (y * r.z) - (z * r.y);
			y = (z * r.x) - (x * r.z);
			z = (x * r.y) - (y  *r.x);
			return *this;
		}
		inline vec3f cross(const vec3f& r) const {return vec3f((y * r.z) - (z * r.y), (z * r.x) - (x * r.z), (x * r.y) - (y *r.x)); }
		inline float determinant(const vec3f& a, const vec3f& b) const { return ((a.x - x)*(b.y - y)) - ((a.y - y)*(b.x - x)); }
		inline vec3f& interpolate(const vec3f& from, float factor) {
			x = ((x - from.x) * factor) + from.x;
			y = ((x - from.y) * factor) + from.y;
			z = ((z - from.z) * factor) + from.z;
			return *this;
		}
		inline vec3f interpolate(const vec3f& from, float factor) const {
			return vec3f(((x - from.x) * factor) + from.x, ((y - from.y) * factor) + from.y,((z - from.z) * factor) + from.z);
		}
		inline vec3f& normalize() {
			*this *= isqrt(length_sqr());
			return *this;
		}
		inline vec3f normalize() const {
			float invLength = isqrt(length_sqr());
			return vec3f(x * invLength, y * invLength, z * invLength);
		}
		static inline vec3f plane_normal(const vec3f& origin, const vec3f& p1, const vec3f& p2);
	};
	inline vec3f operator+(const vec3f&l, const vec3f&r) { return vec3f(l.x + r.x, l.y + r.y, l.z + r.z); }
	inline vec3f operator-(const vec3f&l, const vec3f&r) { return vec3f(l.x - r.x, l.y - r.y, l.z + r.z); }
	inline float operator*(const vec3f&l, const vec3f&r) { return l.dot(r); }
	inline vec3f operator*(const vec3f&l, const float r) { return vec3f(l.x*r, l.y*r, l.z*r); }
	inline vec3f operator*(const float r, const vec3f&l) { return vec3f(l.x*r, l.y*r, l.z*r); }
	inline vec3f vec3f::plane_normal(const vec3f& origin, const vec3f& p1, const vec3f& p2) {
		vec3f v0 = origin - p1;
		vec3f v1 = origin - p2;
		return v0.cross(v1).normalize();
	}

	struct quat : public vec_base<float, 4> {
		static const quat Zero;
		quat() : vec_base<float, 4>() {}
		quat(float x, float y, float z, float w) : vec_base<float, 4>(x,y,z,w) {}
		quat operator-() const { return quat(-x, -y, -z, -w); }
		quat& operator+=(const quat &a) { x += a.x;  y += a.y; z += a.z; w += a.w; return *this; }
		quat& operator-=(const quat &a) { x -= a.x;  y -= a.y; z -= a.z; w -= a.w; return *this; }
		quat& operator*=(float a) { x *= a; y *= a; z *= a; w *= a; return *this; }
		quat& operator*=(const quat &b) {
			quat a = *this;
			x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
			y = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
			z = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
			w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
			return *this;
		}
		inline bool operator==(const quat &l) const { return compare(l); }
		inline bool operator!=(const quat &l) const { return !compare(l); }
		inline quat inverse() const { return quat(-x, -y, -z, w); }
		inline quat& inverse() { x = -x; y = -y; z = -z; return *this; }
		inline float length() const { return umath::sqrt(x * x + y * y + z * z + w * w); }
		inline quat& normalize() { float len = 1.0f / this->length();  *this *= len; return *this; }

	};

	inline quat operator+(const quat &a, const quat &b) { return quat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
	inline quat operator-(const quat &a, const quat &b) { return quat(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
	inline quat operator*(const quat &a, const quat &b) {
		return quat( // can do in one SSE asembly?
			a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
			a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z,
			a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x,
			a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z
		);
	}
	inline quat operator*(const quat &a, float b) { return quat(a.x * b, a.y * b, a.z * b, a.w * b); }

	inline vec3f operator*(const quat& a, const vec3f&b) {
		// result = this->Inverse() * idQuat( a.x, a.y, a.z, 0.0f ) * (*this)
		float xxzz = a.x*a.x - a.z*a.z;
		float wwyy = a.w*a.w - a.y*a.y;

		float xw2 = a.x*a.w*2.0f;
		float xy2 = a.x*a.y*2.0f;
		float xz2 = a.x*a.z*2.0f;
		float yw2 = a.y*a.w*2.0f;
		float yz2 = a.y*a.z*2.0f;
		float zw2 = a.z*a.w*2.0f;

		return vec3f(
			(xxzz + wwyy)*b.x + (xy2 + zw2)*b.y + (xz2 - yw2)*b.z,
			(xy2 - zw2)*b.x + (a.y*a.y + a.w*a.w - a.x*a.x - a.z*a.z)*b.y + (yz2 + xw2)*b.z,
			(xz2 + yw2)*b.x + (yz2 - xw2)*b.y + (wwyy - xxzz)*b.z
		);
	}
	typedef std::tuple<vec2f, vec2f, vec2i> vertex;
	inline void test2() {
		color4b test2;
		color4f test4;
		vec2f test5;
		vec2f test6;
		vec2i test7;
		test7.compare(test7);
		test6.compare(test6,3.0f);
		test6 *= 3.3f;
		test6.set(3.0f, 2.0f);

		auto m = color4f::max;
		auto m22 = color4b::max;
		//test2.blend()
		printf("max check");
	}

};




