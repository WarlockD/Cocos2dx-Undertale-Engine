#include <iostream>
#include <algorithm>
#include <array>

//http://stackoverflow.com/questions/19936841/initialize-a-constexpr-array-as-sum-of-other-two-constexpr-arrays
//http://en.cppreference.com/w/cpp/language/parameter_pack
// just beutiful
namespace array_helpers {
	template<int... Is>
	struct seq {};
	template<int I, int... Is>
	struct gen_seq : gen_seq<I - 1, I - 1, Is...> {};
	template<int... Is>
	struct gen_seq<0, Is...> : seq<Is...> {};
#pragma pack(push,1)
	template<typename T, size_t SIZE>
	struct _vec {
		typedef T element_type;
		T ptr[SIZE];
		static constexpr size_t dimensions = SIZE;
		template<typename... Targs>
		constexpr _vec(Targs... Fargs) : ptr{ static_cast<T>(Fargs)... } {}
	};
	template<typename T>
	struct _vec<T, 2> {
		typedef T element_type;
		typedef T element_type;  struct { union { struct { T x; T y; }; T ptr[2]; }; };
		static constexpr size_t dimensions = 2;
		template<typename... Targs>
		constexpr _vec(Targs... Fargs) : ptr{ static_cast<T>(Fargs)... } {}
		//template<typename A, typename B>
		//constexpr _vec(A x,B x) : ptr{ static_cast<T>(x), static_cast<T>(y) } {}
	};

	template<typename T, size_t N>
	struct vec : public _vec<T, N> {
		using _vec::_vec;
		size_t constexpr size() const { return dimensions; }
		constexpr T operator[](size_t i) const { return ptr[i]; }
		T& operator[](size_t i) { return ptr[i]; }
	};
#pragma pack(pop)

	template<class CHAR, class TRAITS, typename T, size_t N, int... Is>
	void print_vect(std::basic_ostream<CHAR, TRAITS>& os, vec<T, N> const& v, seq<Is...>) {
		using swallow = int[];
		(void)swallow {
			0, (void(os << (Is == 0 ? "" : ", ") << v.ptr[Is]), 0)...
		};
	}
	template<class CHAR, class TRAITS, size_t N, int... Is>
	void print_vect(std::basic_ostream<CHAR, TRAITS>& os, vec<float, N> const& v, seq<Is...>) {
		auto pbackup = os.precision();
		auto fbackup = os.flags();
		os.flags(fbackup | std::ios::fixed);
		os.precision(2);
		using swallow = int[];
		(void)swallow {
			0, (void(os << (Is == 0 ? "" : ", ") << v.ptr[Is] << "f"), 0)...
		};
		os.flags(fbackup);
		os.precision(pbackup);
	}

	template<class CHAR, class TRAITS, typename T, size_t N>
	std::basic_ostream<CHAR, TRAITS>& operator<<(std::basic_ostream<CHAR, TRAITS>& os, vec<T, N> const& v)
	{
		os << "(";
		print_vect(os, v, gen_seq<static_cast<int>(N)>{});
		return os << ")";
	}

	template<class T, int N, class F, int... Is> constexpr inline vec<T, N> transform(vec<T, N> const &lhs, vec<T, N>const &rhs, F f, seq<Is...>) { return vec<T, N>(f(lhs[Is], rhs[Is])...); }
	template<class T, int N, class F> constexpr inline vec<T, N>  transform(vec<T, N> const &lhs, vec<T, N>const &rhs, F f) { return transform(lhs, rhs, f, gen_seq<N>{}); }
	template<class T, int N, class F, int... Is> constexpr inline vec<T, N>& transform(vec<T, N>  &lhs, vec<T, N>const &rhs, F f, seq<Is...>) {
		using swallow = int[];
		(void)swallow {
			0, (void(f(lhs[Is], rhs[Is])), 0)...
		};
		return lhs;
	}
	template<class T, int N, class F> constexpr inline vec<T, N>&  transform(vec<T, N>  &lhs, vec<T, N>const &rhs, F f) { return transform(lhs, rhs, f, gen_seq<N>{}); }


	//template<typename T, size_t N> constexpr vec<T, N> operator+(const vec<T, N>& l, const vec<T, N>&r) { return vec_add(l, r, gen_seq<N>{}); }
	template<typename T, size_t N> constexpr inline vec<T, N> operator+(const vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T a, T b) { return a + b; }); }
	template<typename T, size_t N> constexpr inline vec<T, N>& operator+=(vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T& a, T b) { a += b; }); }
	template<typename T, size_t N> constexpr inline vec<T, N> operator-(const vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T a, T b) { return a - b; }); }
	template<typename T, size_t N> constexpr inline vec<T, N>& operator-=(vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T& a, T b) { a -= b; }); }


};
#pragma pack(push,1)

typedef std::array<float, 4> svec;

#pragma pack(pop)
template<class CHAR, class TRAITS>
std::basic_ostream<CHAR, TRAITS>& operator<<(std::basic_ostream<CHAR, TRAITS>& os, const svec const& v)
{
	os << "(" << v[0] <<", " << v[1] << ", " << v[2];
	return os << ")";
}
//http://neilkemp.us/src/sse_tutorial/sse_tutorial.html#D
 inline  void __fastcall asm_add(svec& l, const svec& r) {
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		movups	xmm0, [ecx]
		movups	xmm1, [edx]
		addps xmm0, xmm1
		movups [ecx], xmm0
	}
}
void __fastcall sub_vec(svec& l, const svec& r) {
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		movups	xmm0, [ecx]
		movups	xmm1, [edx]
		subps xmm0, xmm1
		movups[ecx], xmm0
	}
}
void __fastcall div_vec(svec& l, const svec& r) {
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		movups	xmm0, [ecx]
		movups	xmm1, [edx]
		divps xmm0, xmm1
		movups[ecx], xmm0
		ret // added because of nakid
	}
}
// you have to make it a const float& other wise ecx dosn't get a pointer
void __fastcall div_vec(svec& l, const float& r) {
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		movups	xmm0, [ecx]
		movss	xmm1, [edx]
		shufps xmm1, xmm1, 0
		divps xmm0, xmm1
		movups[ecx], xmm0
		ret // added because of nakid
	}
}
void __fastcall mul_vec(svec& l, const svec& r) {
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		movups	xmm0, [ecx]
		movups	xmm1, [edx]
		mulps xmm0, xmm1
		movups[ecx], xmm0
		ret // added because of nakid
	}
}
// you have to make it a const float& other wise ecx dosn't get a pointer
//__declspec(naked) 
//inline  void  mul_vec(svec& l, const float& r) {__fastcall
// we have to force inline here cause I am not sure why its failing
__forceinline  void  __fastcall  mul_vec(svec& l, const float& r) {
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		movups	xmm0, [ecx]
		movss	xmm1, [edx]
		shufps xmm1, xmm1, 0
		mulps xmm0, xmm1
		movups[ecx], xmm0
	}
}

__declspec(noinline) void __vectorcall normal_add(svec& l, const svec& r) {
	for (int i = 0; i < l.size(); i++) l[i] += r[i];
}


svec operator+(const svec& l, const svec& r) { 
	return svec{ l[0] + r[0], l[1] + r[1], l[2] + r[2] }; 
}
svec& operator+=(svec& l, const svec& r) {  }
using namespace array_helpers;
// used to force the asembler to not treat eveything as constexpr


__declspec(noinline) vec<float, 3> template_test() {
	vec<float, 3> test1(1.0f, 1.0f, 1.0f);
	vec<float, 3> test2(2.0f, 2.0f, 2.0f);
	return test1 + test2;
}
volatile float meh = 50.0f;

__declspec(noinline) void simple_test() {
	svec test1s{ 1.0f, 1.0f, 1.0f };
	svec test1c{ 1.0f, 1.0f, 1.0f };
	svec test2{ 2.0f, 2.0f, 2.0f };
	normal_add(test1s, test2);
	asm_add(test1c, test2);
	mul_vec(test1c, 50.0f);
	std::cout << " more " << test1s << " " << test1c;
}

int main(int argc, const char* argv[]) {
	simple_test();
	return 0;
}