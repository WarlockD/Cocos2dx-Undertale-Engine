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
	template<typename TT, size_t NN>
	struct vec_choise {
		template<typename T, size_t N> 
		struct _vec {
			union { T ptr[N]; struct {  }; };
			static constexpr size_t dimension = N;
			typedef T element_type;
			typedef _vec<T, N> vec_type;
			template<typename... Targs, typename = std::enable_if<sizeof...(Targs) == N>::type>
			constexpr _vec(Targs... Fargs) :ptr{ static_cast<T>(Fargs)... } {}
			_vec() : ptr{ gen_seq<N>((T)0) } {}
		};
		template<typename T> 
		struct _vec<T,4> {
			union { T ptr[4]; struct { T x; T y; T z; T w; }; }; 
			static constexpr size_t dimension = 4; 
			typedef T element_type; 
			typedef _vec<T, 4> vec_type;
			template<typename... Targs, typename = std::enable_if<(sizeof...(Targs)) == 4>::type>
			constexpr _vec(Targs... Fargs) :ptr{ static_cast<T>(Fargs)... } {}
			_vec() : ptr{ gen_seq<N>((T)0) } {}
		};
		template<typename T> 
		struct _vec<T, 3> {
			union { T ptr[3]; struct { T x; T y; T z;  }; }; 
			static constexpr size_t dimension = 3; 
			typedef T element_type; 
			typedef _vec<T, 3> vec_type;
			template<typename... Targs, typename = std::enable_if<(sizeof...(Targs)) == 3>::type>
			constexpr _vec(Targs... Fargs) :ptr{ static_cast<T>(Fargs)... } {}
			_vec() : ptr{ gen_seq<N>((T)0) } {}
		};
		template<typename T> 
		struct _vec<T, 2> {
		 union { T ptr[2]; struct { T x; T y;  }; }; 
		 static constexpr size_t dimension = 2; 
		 typedef T element_type; 
		 typedef _vec<T,2> vec_type;
		 template<typename... Targs, typename = std::enable_if<(sizeof...(Targs)) == 2>::type>
		 constexpr _vec(Targs... Fargs) :ptr{ static_cast<T>(Fargs)... } {}
		 _vec() : ptr{ gen_seq<N>((T)0) } {}
		};
		
		typedef typename _vec<TT, NN> type;
	};

	template<typename T, size_t N>
	struct vec : public vec_choise<T, N>::type {
		template<typename... Targs>
		constexpr vec(Targs... Fargs) : _vec(static_cast<T>(Fargs)...) {} 

		T& operator[](size_t i) { return ptr[i]; }
		constexpr const T& operator[](size_t i) const { return ptr[i];  }
	};

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

	/*

	template<class T, int N, class F, int... Is> 
	constexpr inline vec<T, N> transform(vec<T, N> const &lhs, vec<T, N>const &rhs, F f, seq<Is...>) { 
		return vec<T, N>(f(lhs[Is], rhs[Is])...); 
	}
	template<class T, int N, class F> constexpr inline vec<T, N>  
		transform(vec<T, N> const &lhs, vec<T, N>const &rhs, F f) { 
			return transform(lhs, rhs, f, gen_seq<N>{}); 
		}
	template<class T, int N, class F, int... Is> constexpr inline vec<T, N>& transform(vec<T, N>  &lhs, vec<T, N>const &rhs, F f, seq<Is...>) {
		using swallow = int[];
		(void)swallow {
			0, (void(f(lhs[Is], rhs[Is])), 0)...
		};
		return lhs;
	}
	*/
	template<typename T, size_t N> constexpr inline vec<T, N> operator+(const vec<T, N>& l, const vec<T, N>&r) { return transform([](T a, T b) { return a + b; }, l, r); }
	template<typename T, size_t N> constexpr inline vec<T, N>& operator+=(vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T& a, T b) { a += b; }); }
	template<typename T, size_t N> constexpr inline vec<T, N> operator-(const vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T a, T b) { return a - b; }); }
	template<typename T, size_t N> constexpr inline vec<T, N>& operator-=(vec<T, N>& l, const vec<T, N>&r) { return transform(l, r, [](T& a, T b) { a -= b; }); }


};
using namespace array_helpers;

typedef vec<float, 4> svec;
#pragma pack(pop)
template<class CHAR, class TRAITS>
std::basic_ostream<CHAR, TRAITS>& operator<<(std::basic_ostream<CHAR, TRAITS>& os, const svec const& v)
{
	os << "(" << v[0] <<", " << v[1] << ", " << v[2];
	return os << ")";
}
//http://neilkemp.us/src/sse_tutorial/sse_tutorial.html#D


inline  void  __vectorcall asm_add(svec& l, const svec& r) {
	//
	// fast call makes the argumetns on edx and ecx, left to right
	// naked dosn't write a preamble or post so we have to return ourselves
	__asm {
		//movups	xmm0, [ecx]
		//movups	xmm1, [edx]
		addps xmm0, xmm1
	//	movups[ecx], xmm0
	}
}
/*

inline  void  __fastcall asm_add(svec& l, const svec& r) {
	__asm {
		movups	xmm0, [ecx]
		movups	xmm1, [edx]
		addps xmm0, xmm1
		movups [ecx], xmm0
	}
}
*/
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
//__forceinline  void  __fastcall
inline void __vectorcall normal_add(float l[4], const float  r[4]) {
	for (int i = 0; i < 4; i++) l[i] += r[i];
}


//using namespace array_helpers;
// used to force the asembler to not treat eveything as constexpr


__declspec(noinline) vec<float, 3> template_test() {
	vec<float, 3> test1(1.0f, 1.0f, 1.0f);
	vec<float, 3> test2(2.0f, 2.0f, 2.0f);
	return test1 + test2;
}
volatile float meh = 50.0f;

__declspec(noinline) void simple_test() {
	svec test1s( 1.0f, 1.0f, 1.0f, 1.0f);
	svec test1c( 1.0f, 1.0f, 1.0f, 1.0f);
	svec test2(2.0f, 2.0f, 2.0f, 1.0f);
	///humm
	//normal_add(test1s.ptr.m128_f32, test2.ptr);
	///humm
	asm_add(test1c, test2);
	///humm
	std::cout << " more " << test1s << " " << test1c;
}

int __cdecl main(int argc, const char* argv[]) {
	simple_test();
	return 0;
}