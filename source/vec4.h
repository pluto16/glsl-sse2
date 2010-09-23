#ifndef __VEC4_H__
#define __VEC4_H__

#include <emmintrin.h>

class vec4
{
	private:
			// Most compilers don't use pshufd (SSE2) when _mm_shuffle(x, x, mask) is used
			// This macro saves 2-3 movaps instructions when shuffling
			// This has to be a macro since mask HAS to be an immidiate value
		#define _mm_shufd(xmm, mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(xmm), mask))
		
			// Merges mask `target` with `m` into one unified mask that does the same sequential shuffle
		template <unsigned target, unsigned m>
		struct _mask_merger
		{
			enum
			{
				ROW0 = ((target >> (((m >> 0) & 3) << 1)) & 3) << 0,
				ROW1 = ((target >> (((m >> 2) & 3) << 1)) & 3) << 2,
				ROW2 = ((target >> (((m >> 4) & 3) << 1)) & 3) << 4,
				ROW3 = ((target >> (((m >> 6) & 3) << 1)) & 3) << 6,

				MASK = ROW0 | ROW1 | ROW2 | ROW3,
			};

			private:
				_mask_merger();
		};

			// Since we are working in little endian land, this reverses the shuffle mask
		template <unsigned m>
		struct _mask_reverser
		{
			enum
			{
				ROW0 = 0 << (((m >> 0) & 3) << 1),
				ROW1 = 1 << (((m >> 2) & 3) << 1),
				ROW2 = 2 << (((m >> 4) & 3) << 1),
				ROW3 = 3 << (((m >> 6) & 3) << 1),

				MASK = ROW0 | ROW1 | ROW2 | ROW3,
			};

			private:
				_mask_reverser();
		};

			// Swizzle helper (Read only)
		template <unsigned mask>
		struct _swzl_ro
		{
			friend class vec4;

			public:
				inline operator const vec4 () const {
					return _mm_shufd(v.m, mask);
				}

					// Swizzle of the swizzle, read only const
				template<unsigned other_mask>
				inline _swzl_ro<_mask_merger<mask, other_mask>::MASK> shuffle4_ro() const {
					typedef _mask_merger<mask, other_mask> merged;
					return _swzl_ro<merged::MASK>(v);
				}

					// Swizzle of the swizzle, read/write const
				template<unsigned other_mask>
				inline _swzl_ro<_mask_merger<mask, other_mask>::MASK> shuffle4_rw() const {
					typedef _mask_merger<mask, other_mask> merged;
					return _swzl_ro<merged::MASK>(v);
				}

				const float &x, &y, &z, &w;
				const float &r, &g, &b, &a;
				const float &s, &t, &p, &q;

			private:
					// This massive constructor maps a vector to references
				inline _swzl_ro(const vec4 &v):
					x(v[(mask >> 0) & 0x3]), y(v[(mask >> 2) & 0x3]),
					z(v[(mask >> 4) & 0x3]), w(v[(mask >> 6) & 0x3]),

					r(v[(mask >> 0) & 0x3]), g(v[(mask >> 2) & 0x3]),
					b(v[(mask >> 4) & 0x3]), a(v[(mask >> 6) & 0x3]),

					s(v[(mask >> 0) & 0x3]), t(v[(mask >> 2) & 0x3]),
					p(v[(mask >> 4) & 0x3]), q(v[(mask >> 6) & 0x3]),

					v(v) {
						// Empty
				}

					// Reference to unswizzled self
				const vec4 &v;
		};

			// Swizzle helper (Read/Write)
		template <unsigned mask>
		struct _swzl_rw
		{
			friend class vec4;

			public:
				inline operator const vec4 () const {
					return _mm_shufd(v.m, mask);
				}

					// Swizzle from vec4
				inline _swzl_rw& operator = (const vec4 &r) {
					v.m = _mm_shufd(r.m, _mask_reverser<mask>::MASK);
					return *this;
				}

					// Swizzle from same r/o mask (v1.xyzw = v2.xyzw)
				inline _swzl_rw& operator = (const _swzl_ro<mask> &s) {
					v.m = s.v.m;
					return *this;
				}

					// Swizzle from same mask (v1.xyzw = v2.xyzw)
				inline _swzl_rw& operator = (const _swzl_rw &s) {
					v.m = s.v.m;
					return *this;
				}

					// Swizzle mask => other_mask, r/o (v1.zwxy = v2.xyxy)
				template<unsigned other_mask>
				inline _swzl_rw& operator = (const _swzl_ro<other_mask> &s) {
					typedef _mask_merger<other_mask, _mask_reverser<mask>::MASK> merged;
					v.m = _mm_shufd(s.v.m, merged::MASK);
					return *this;
				}

					// Swizzle mask => other_mask (v1.zwxy = v2.xyxy)
				template<unsigned other_mask>
				inline _swzl_rw& operator = (const _swzl_rw<other_mask> &s) {
					typedef _mask_merger<other_mask, _mask_reverser<mask>::MASK> merged;
					v.m = _mm_shufd(s.v.m, merged::MASK);
					return *this;
				}

					// Swizzle of the swizzle, read only (v.xxxx.yyyy)
				template<unsigned other_mask>
				inline _swzl_ro<_mask_merger<mask, other_mask>::MASK> shuffle4_ro() const {
					typedef _mask_merger<mask, other_mask> merged;
					return _swzl_ro<merged::MASK>(v);
				}

					// Swizzle of the swizzle, read/write (v1.zyxw.wzyx = ...)
				template<unsigned other_mask>
				inline _swzl_rw<_mask_merger<mask, other_mask>::MASK> shuffle4_rw() {
					typedef _mask_merger<mask, other_mask> merged;
					return _swzl_rw<merged::MASK>(v);
				}

				float &x, &y, &z, &w;
				float &r, &g, &b, &a;
				float &s, &t, &p, &q;

			private:
					// This massive contructor maps a vector to references
				inline _swzl_rw(vec4 &v):
					x(v[(mask >> 0) & 0x3]), y(v[(mask >> 2) & 0x3]),
					z(v[(mask >> 4) & 0x3]), w(v[(mask >> 6) & 0x3]),

					r(v[(mask >> 0) & 0x3]), g(v[(mask >> 2) & 0x3]),
					b(v[(mask >> 4) & 0x3]), a(v[(mask >> 6) & 0x3]),

					s(v[(mask >> 0) & 0x3]), t(v[(mask >> 2) & 0x3]),
					p(v[(mask >> 4) & 0x3]), q(v[(mask >> 6) & 0x3]),

					v(v) {
						// Empty
				}

					// Refrence to unswizzled self
				vec4 &v;
		};

		// ----------------------------------------------------------------- //

	public:
			// Empty constructor
		inline vec4() {
			m = _mm_setzero_ps();
		}

			// Fill constructor
		explicit inline vec4(float f) {
			m = _mm_set1_ps(f);
		}

			// 4 var init constructor
		inline vec4(float _x, float _y, float _z, float _w) {
			m = _mm_setr_ps(_x, _y, _z, _w);
		}

			// Float array constructor
		inline vec4(const float* fv) {
			m = _mm_loadu_ps(fv);
		}

			// Copy constructor
		inline vec4(const vec4 &v) {
			m = v.m;
		}

			// SSE compatible constructor
		inline vec4(const __m128 &_m) {
			m = _m;
		}

		// ----------------------------------------------------------------- //

			// Read-write swizzle
		template<unsigned mask>
		inline _swzl_rw<mask> shuffle4_rw() {
			return _swzl_rw<mask>(*this);
		}

			// Read-write swizzle, const, actually read only
		template<unsigned mask>
		inline _swzl_ro<mask> shuffle4_rw() const {
			return _swzl_ro<mask>(*this);
		}

			// Read-only swizzle
		template<unsigned mask>
		inline _swzl_ro<mask> shuffle4_ro() const {
			return _swzl_ro<mask>(*this);
		}

		// ----------------------------------------------------------------- //

			// Write direct access operator
		inline float& operator[](int index) {
			return ((float*)this)[index];
		}

			// Read direct access operator
		inline const float& operator[](int index) const {
			return ((const float*)this)[index];
		}

			// Cast operator
		inline operator float* () {
			return (float*)this;
		}

			// Const cast operator
		inline operator const float* () const {
			return (const float*)this;
		}

		// ----------------------------------------------------------------- //

		friend inline vec4& operator += (vec4 &v, float f) {
			v.m = _mm_add_ps(v.m, _mm_set1_ps(f));
			return v;
		}

		friend inline vec4& operator += (vec4 &v0, const vec4 &v1) {
			v0.m = _mm_add_ps(v0.m, v1.m);
			return v0;
		}

		friend inline vec4& operator -= (vec4 &v, float f) {
			v.m = _mm_sub_ps(v.m, _mm_set1_ps(f));
			return v;
		}

		friend inline vec4& operator -= (vec4 &v0, const vec4 &v1) {
			v0.m = _mm_sub_ps(v0.m, v1.m);
			return v0;
		}

		friend inline vec4& operator *= (vec4 &v, float f) {	
			v.m = _mm_mul_ps(v.m, _mm_set1_ps(f));
			return v;
		}

		friend inline vec4& operator *= (vec4 &v0, const vec4 &v1) {
			v0.m = _mm_mul_ps(v0.m, v1.m);
			return v0;
		}

		friend inline vec4& operator /= (vec4 &v, float f) {
			v.m = _mm_div_ps(v.m, _mm_set1_ps(f));
			return v;
		}

		friend inline vec4& operator /= (vec4 &v0, const vec4 &v1) {
			v0.m = _mm_div_ps(v0.m, v1.m);
			return v0;
		}

		// ----------------------------------------------------------------- //

		friend inline const vec4 operator + (float f, const vec4 &v) {
			return _mm_add_ps(_mm_set1_ps(f), v.m);
		}

		friend inline const vec4 operator + (const vec4 &v, float f) {
			return _mm_add_ps(v.m, _mm_set1_ps(f));
		}

		friend inline const vec4 operator + (const vec4 &v0, const vec4 &v1) {
			return _mm_add_ps(v0.m, v1.m);
		}

		friend inline const vec4 operator - (const vec4 &v) {
			return _mm_xor_ps(v.m, _mm_set1_ps(-0.f));
		}

		friend inline const vec4 operator - (float f, const vec4 &v) {
			return _mm_sub_ps( _mm_set1_ps(f), v.m);
		}

		friend inline const vec4 operator - (const vec4 &v, float f) {
			return _mm_sub_ps(v.m, _mm_set1_ps(f));
		}

		friend inline const vec4 operator - (const vec4 &v0, const vec4 &v1) {
			return _mm_sub_ps(v0.m, v1.m);
		}

		friend inline const vec4 operator * (float f, const vec4 &v) {
			return _mm_mul_ps(_mm_set1_ps(f), v.m);
		}

		friend inline const vec4 operator * (const vec4 &v, float f) {
			return _mm_mul_ps(v.m, _mm_set1_ps(f));
		}

		friend inline const vec4 operator * (const vec4 &v0, const vec4 &v1) {
			return _mm_mul_ps(v0.m, v1.m);
		}

		friend inline const vec4 operator / (float f, const vec4 &v) {
			return _mm_div_ps(_mm_set1_ps(f), v.m);
		}

		friend inline const vec4 operator / (const vec4 &v, float f) {
			return _mm_div_ps(v.m, _mm_set1_ps(f));
		}

		friend inline const vec4 operator / (const vec4 &v0, const vec4 &v1) {
			return _mm_div_ps(v0.m, v1.m);
		}

		// ----------------------------------------------------------------- //

		friend inline const vec4 sqrt(const vec4 &v) {
			return _mm_sqrt_ps(v.m);
		}

		friend inline const vec4 inversesqrt(const vec4 &v) {
			return _mm_div_ps(_mm_set1_ps(1.0f), _mm_sqrt_ps(v.m));
		}

		// ----------------------------------------------------------------- //

		friend inline const vec4 abs(const vec4 &v) {
			return _mm_andnot_ps(_mm_set1_ps(-0.f), v.m);
		}

		friend inline const vec4 ceil(const vec4 &v) {
			return _mm_cvtepi32_ps(_mm_cvtps_epi32(
								   _mm_add_ps(v.m, _mm_set1_ps(0.5f))));
		}

		friend inline const vec4 clamp(const vec4 &v0, float f1, float f2) {
			return _mm_max_ps(_mm_set1_ps(f1),
			                  _mm_min_ps(_mm_set1_ps(f2), v0.m));
		}

		friend inline const vec4 clamp(const vec4 &v0,
		                               const vec4 &v1, const vec4 &v2) {
			return _mm_max_ps(v1.m, _mm_min_ps(v2.m, v0.m));
		}

		friend inline const vec4 floor(const vec4 &v) {
			return _mm_cvtepi32_ps(_mm_cvtps_epi32(
								   _mm_sub_ps(v.m, _mm_set1_ps(0.5f))));
		}

		friend inline const vec4 fract(const vec4 &v) {
			return _mm_sub_ps(v.m, _mm_cvtepi32_ps(_mm_cvtps_epi32(
								   _mm_sub_ps(v.m, _mm_set1_ps(0.5f)))));
		}

		friend inline const vec4 max(const vec4 &v, float f) {
			return _mm_max_ps(v.m, _mm_set1_ps(f));
		}

		friend inline const vec4 max(const vec4 &v0, const vec4 &v1) {
			return _mm_max_ps(v0.m, v1.m);
		}

		friend inline const vec4 min(const vec4 &v, float f) {
			return _mm_min_ps(v.m, _mm_set1_ps(f));
		}

		friend inline const vec4 min(const vec4 &v0, const vec4 &v1) {
			return _mm_min_ps(v0.m, v1.m);
		}

		friend inline const vec4 mix(const vec4 &v0, const vec4 &v1,
									 float f) {
			__m128 ff = _mm_set1_ps(f);
			return _mm_add_ps(_mm_mul_ps(v0.m, _mm_sub_ps(_mm_set1_ps(1.f), ff)),
							  _mm_mul_ps(v1.m, ff));
		}

		friend inline const vec4 mix(const vec4 &v0, const vec4 &v1,
		                             const vec4 &v2) {
			return _mm_add_ps(_mm_mul_ps(v0.m, _mm_sub_ps(_mm_set1_ps(1.f), v1.m)),
							  _mm_mul_ps(v1.m, v2.m));
		}

		friend inline const vec4 mod(const vec4 &v, float f) {
			__m128 ff = _mm_set1_ps(f);
			return _mm_sub_ps(v.m, _mm_mul_ps(ff, _mm_cvtepi32_ps(
								   _mm_cvtps_epi32(_mm_sub_ps(_mm_div_ps(v.m, ff),
												   _mm_set1_ps(0.5f))))));
		}

		friend inline const vec4 mod(const vec4 &v0, const vec4 &v1) {
			return _mm_sub_ps(v0.m, _mm_mul_ps(v1.m, _mm_cvtepi32_ps(
									_mm_cvtps_epi32(_mm_sub_ps(_mm_div_ps(v0.m, v1.m),
													_mm_set1_ps(0.5f))))));
		}

		friend inline const vec4 sign(const vec4 &v) {
			return _mm_and_ps(_mm_or_ps(_mm_and_ps(v.m, _mm_set1_ps(-0.f)),
			                            _mm_set1_ps(1.0f)),
			                  _mm_cmpneq_ps(v.m, _mm_setzero_ps()));
		}

		friend inline const vec4 smoothstep(float f1, float f2,
		                                    const vec4 &v) {
			__m128 ff1 = _mm_set1_ps(f1);
			 __m128 c = _mm_max_ps(_mm_min_ps(_mm_div_ps(_mm_sub_ps(v.m, ff1),
								   _mm_sub_ps(_mm_set1_ps(f2), ff1)),
								   _mm_set1_ps(1.f)), _mm_setzero_ps());
			return _mm_mul_ps(_mm_mul_ps(c, c),
			                  _mm_sub_ps(_mm_set1_ps(3.0f), _mm_add_ps(c, c)));
		}

		friend inline const vec4 smoothstep(const vec4 &v0,
		                                    const vec4 &v1, const vec4 &v2) {
			 __m128 c = _mm_max_ps(_mm_min_ps(_mm_div_ps(_mm_sub_ps(v2.m, v0.m),
								   _mm_sub_ps(v1.m, v0.m)), _mm_set1_ps(1.f)),
								   _mm_setzero_ps());
			return _mm_mul_ps(_mm_mul_ps(c, c),
			                  _mm_sub_ps(_mm_set1_ps(3.0f), _mm_add_ps(c, c)));
		}

		friend inline const vec4 step(float f, const vec4 &v) {
			return _mm_and_ps(_mm_cmple_ps(v.m, _mm_set1_ps(f)),
			                  _mm_set1_ps(1.0f));
		}

		friend inline const vec4 step(const vec4 &v0, const vec4 &v1) {
			return _mm_and_ps(_mm_cmple_ps(v0.m, v1.m), _mm_set1_ps(1.0f));
		}

		friend inline const vec4 trunc(const vec4 &v) {
			return _mm_cvtepi32_ps(_mm_cvtps_epi32(_mm_sub_ps(v.m,
								   _mm_or_ps(_mm_and_ps(v.m, _mm_set1_ps(-0.f)),
															 _mm_set1_ps(0.5f)))));
		}

		// ----------------------------------------------------------------- //

		friend inline float distance(const vec4 &v0, const vec4 &v1) {
			__m128 l = _mm_sub_ps(v0.m, v1.m);
			l = _mm_mul_ps(l, l);
			l = _mm_add_ps(l, _mm_shufd(l, 0x4E));
			return _mm_cvtss_f32(_mm_sqrt_ss(_mm_add_ss(l,
			                                 _mm_shufd(l, 0x11))));
		}

		friend inline float dot(const vec4 &v0, const vec4 &v1) {
			__m128 l = _mm_mul_ps(v0.m, v1.m);
			l = _mm_add_ps(l, _mm_shufd(l, 0x4E));
			return _mm_cvtss_f32(_mm_add_ss(l, _mm_shufd(l, 0x11)));
		}

		friend inline const vec4 faceforward(const vec4 &v0,
		                                     const vec4 &v1, const vec4 &v2) {
			__m128 l = _mm_mul_ps(v2.m, v1.m);
			l = _mm_add_ps(l, _mm_shufd(l, 0x4E));
			return _mm_xor_ps(_mm_and_ps(_mm_cmpnlt_ps(
			        _mm_add_ps(l, _mm_shufd(l, 0x11)),
			        _mm_setzero_ps()), _mm_set1_ps(-0.f)), v0.m);
		}

		friend inline float length(const vec4 &v) {
			__m128 l = _mm_mul_ps(v.m, v.m);
			l = _mm_add_ps(l, _mm_shufd(l, 0x4E));
			return _mm_cvtss_f32(_mm_sqrt_ss(_mm_add_ss(l,
			                                 _mm_shufd(l, 0x11))));
		}

		friend inline const vec4 normalize(const vec4 &v) {
			__m128 l = _mm_mul_ps(v.m, v.m);
			l = _mm_add_ps(l, _mm_shufd(l, 0x4E));
			return _mm_div_ps(v.m, _mm_sqrt_ps(_mm_add_ps(l,
			                                   _mm_shufd(l, 0x11))));
		}

		friend inline const vec4 reflect(const vec4 &v0, const vec4 &v1) {
			__m128 l = _mm_mul_ps(v1.m, v0.m);
			l = _mm_add_ps(l, _mm_shufd(l, 0x4E));
			l = _mm_add_ps(l, _mm_shufd(l, 0x11));
			return _mm_sub_ps(v0.m, _mm_mul_ps(_mm_add_ps(l, l), v1.m));
		}

		friend inline const vec4 refract(const vec4 &v0, const vec4 &v1,
										 float f) {
			__m128 o = _mm_set1_ps(1.0f);
			__m128 e = _mm_set1_ps(f);
			__m128 d = _mm_mul_ps(v1.m, v0.m);
			d = _mm_add_ps(d, _mm_shufd(d, 0x4E));
			d = _mm_add_ps(d, _mm_shufd(d, 0x11));
			__m128 k = _mm_sub_ps(o, _mm_mul_ps(_mm_mul_ps(e, e),
									 _mm_sub_ps(o, _mm_mul_ps(d, d))));
			return _mm_and_ps(_mm_cmpnlt_ps(k, _mm_setzero_ps()),
			                  _mm_mul_ps(_mm_mul_ps(e, _mm_sub_ps(v0.m,
			                  _mm_mul_ps(_mm_mul_ps(e, d), _mm_sqrt_ps(k)))),
			                             v1.m));
		}

		// ----------------------------------------------------------------- //

		friend inline bool operator == (const vec4 &v0, const vec4 &v1) {
			return (_mm_movemask_ps(_mm_cmpeq_ps(v0.m, v1.m)) == 0xF);
		}

		friend inline bool operator != (const vec4 &v0, const vec4 &v1) {
			return (_mm_movemask_ps(_mm_cmpneq_ps(v0.m, v1.m)) != 0x0);
		}

		// ----------------------------------------------------------------- //

		union {
				// Vertex / Vector 
			struct {
				float x, y, z, w;
			};
				// Color
			struct {
				float r, g, b, a;
			};
				// Texture coordinates
			struct {
				float s, t, p, q;
			};

				// SSE register
			__m128	m;
		};

		// Avoid pollution
	#undef _mm_shufd
};

#include "swizzle.h"

#endif
