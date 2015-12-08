// this class is from QRepRap project

#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <cstddef>
#include <ostream>

template<typename T, int N>
struct Vec {
    public:
        typedef T   component_type;
        enum { components = N };

    public:
        inline Vec()    {}
        inline Vec(const T &x0)
        {
            _v[0] = x0;
        }
        inline Vec(const T &x0, const T &x1)
        {
            _v[0] = x0;
            _v[1] = x1;
        }
        inline Vec(const T &x0, const T &x1, const T &x2)
        {
            _v[0] = x0;
            _v[1] = x1;
            _v[2] = x2;
        }
        inline Vec(const T &x0, const T &x1, const T &x2, const T &x3)
        {
            _v[0] = x0;
            _v[1] = x1;
            _v[2] = x2;
            _v[3] = x3;
        }
        inline Vec(const Vec &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] = v._v[i];
            }
        }

        inline Vec &operator=(const Vec &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] = v._v[i];
            }

            return *this;
        }

        inline bool operator==(const Vec &v) const
        {
            for(size_t i = 0 ; i < N ; ++i)
                if (_v[i] != v._v[i]) {
                    return false;
                }

            return true;
        }

        inline Vec &operator+=(const Vec &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] += v._v[i];
            }

            return *this;
        }

        inline Vec &operator-=(const Vec &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] -= v._v[i];
            }

            return *this;
        }

        inline Vec &operator*=(const Vec &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] *= v._v[i];
            }

            return *this;
        }

        inline Vec &operator/=(const Vec &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] /= v._v[i];
            }

            return *this;
        }

        inline Vec &operator*=(const T &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] *= v;
            }

            return *this;
        }

        inline Vec &operator/=(const T &v)
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] /= v;
            }

            return *this;
        }

#define IMPLEMENT_OP(op)\
    inline Vec operator op(const Vec &v) const\
    {\
        Vec r;\
        for(size_t i = 0 ; i < N ; ++i)\
            r[i] = _v[i] op v._v[i];\
        return r;\
    }

        IMPLEMENT_OP(+)
        IMPLEMENT_OP(-)
        IMPLEMENT_OP(*)
        IMPLEMENT_OP( / )

        inline Vec operator *(const T &v) const
        {
            Vec r;

            for(size_t i = 0 ; i < N ; ++i) {
                r[i] = _v[i] * v;
            }

            return r;
        }

        inline Vec operator /(const T &v) const
        {
            Vec r;

            for(size_t i = 0 ; i < N ; ++i) {
                r[i] = _v[i] / v;
            }

            return r;
        }

        inline Vec operator -() const
        {
            Vec r;

            for(size_t i = 0 ; i < N ; ++i) {
                r[i] = -_v[i];
            }

            return r;
        }

#undef IMPLEMENT_OP

        inline T operator |(const Vec &v) const
        {
            T acc(0);

            for(size_t i = 0 ; i < N ; ++i) {
                acc += _v[i] * v._v[i];
            }

            return acc;
        }

        inline Vec operator ^(const Vec &v) const
        {
            return Vec(_v[1] * v[2] - _v[2] * v[1],
                       _v[2] * v[0] - _v[0] * v[2],
                       _v[0] * v[1] - _v[1] * v[0]);
        }

        inline T sq() const
        {
            return *this | *this;
        }
        inline T length() const
        {
            return std::sqrt(sq());
        }

        inline void normalize()
        {
            const T l = T(1.0) / length();

            if (!isinf(l) && !isnan(l))
                for(size_t i = 0 ; i < N ; ++i) {
                    _v[i] *= l;
                }
        }

        inline const T &operator[](size_t idx) const
        {
            return _v[idx];
        }
        inline T &operator[](size_t idx)
        {
            return _v[idx];
        }

        inline void clear()
        {
            for(size_t i = 0 ; i < N ; ++i) {
                _v[i] = T(0);
            }
        }

        inline T &x()
        {
            return _v[0];
        }
        inline const T &x() const
        {
            return _v[0];
        }
        inline T &y()
        {
            return _v[1];
        }
        inline const T &y() const
        {
            return _v[1];
        }
        inline T &z()
        {
            return _v[2];
        }
        inline const T &z() const
        {
            return _v[2];
        }

    private:
        T _v[N];
};

template<typename T, int N>
inline Vec<T, N> operator *(const T &s, const Vec<T, N> &v)
{
    return v * s;
}

typedef Vec<double, 2>  Vec2d;
typedef Vec<float, 2>   Vec2f;
typedef Vec<double, 2>  Vec2;
typedef Vec<double, 3>  Vec3d;
typedef Vec<float, 3>   Vec3f;
typedef Vec<double, 3>  Vec3;
typedef Vec<unsigned char, 3>   Vec3ub;
typedef Vec<double, 4>  Vec4d;
typedef Vec<float, 4>   Vec4f;
typedef Vec<Vec4d, 4>  Vec4x4d;
typedef Vec<Vec4f, 4>  Vec4x4f;

// typedef std::array<TFloat4, 4> TFloat4x4;

// namespace std
// {
// template<typename T, int N>
// struct is_pod<Vec<T, N> > {
//     enum { value = true };
// };
// }


template<typename T, int N>
std::ostream &operator<<(std::ostream &out, const Vec<T, N> &rhs)
{
    out << '[';

    for(size_t i = 0 ; i < N - 1 ; ++i) {
        out << rhs[i] << ' ';
    }

    out << rhs[N - 1];
    out << ']';
    return out;
}

#endif // VEC_H
