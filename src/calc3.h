#ifndef CALC3
#define CALC3

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <cmath>
#include <numeric>
#include <algorithm>
#include <vector>
#include <cstring>

namespace c3d
{

////
// Constants.
////

constexpr float pi = 3.1415926535897932384626f;
constexpr float eps = 16 * std::numeric_limits<float>::epsilon();

////
// Mat class.
////

template<typename ValType, int NumRows, int NumCols>
class Mat;

template<typename ValType, int NumRows>
void MemCopy(Mat<ValType, NumRows, NumRows>& result, const ValType* p);

template<typename ValType, int NumRows>
void MemCopyRowMajor(Mat<ValType, NumRows, NumRows>& result, const ValType* p);

template<typename ValType>
class Mat<ValType, 2, 1> {
public:

	ValType x;
	ValType y;

	const ValType& operator[](const int i) const { return (&x)[i]; }
	ValType& operator[](const int i) { return (&x)[i]; }

	static auto FromMemory(const ValType* p)
	{
		Mat<ValType, 2, 1> result;
		std::memcpy(&result, p, sizeof(result));
		return result;
	}
};

using Vec2 = Mat<float, 2, 1>;
using iVec2 = Mat<int, 2, 1>;

template<typename ValType>
class Mat<ValType, 3, 1> {
public:

	ValType x;
	ValType y;
	ValType z;

	const ValType& operator[](const int i) const { return (&x)[i]; }
	ValType& operator[](const int i) { return (&x)[i]; }

	static auto FromMemory(const ValType* p)
	{
		Mat<ValType, 3, 1> result;
		std::memcpy(&result, p, sizeof(result));
		return result;
	}
};

using Vec3 = Mat<float, 3, 1>;
using iVec3 = Mat<int, 3, 1>;

template<typename ValType>
class Mat<ValType, 4, 1> {
public:

	ValType x;
	ValType y;
	ValType z;
	ValType w;

	const ValType& operator[](const int i) const { return (&x)[i]; }
	ValType& operator[](const int i) { return (&x)[i]; }

	static auto FromMemory(const ValType* p)
	{
		Mat<ValType, 4, 1> result;
		std::memcpy(&result, p, sizeof(result));
		return result;
	}
};

using Vec4 = Mat<float, 4, 1>;
using iVec4 = Mat<int, 4, 1>;

template<typename ValType>
class Mat<ValType, 2, 2> {
public:

	Mat<ValType, 2, 1> col_vec_0;
	Mat<ValType, 2, 1> col_vec_1;

	const auto& operator[](const int i) const { return (&col_vec_0)[i]; }
	auto& operator[](const int i) { return (&col_vec_0)[i]; }

	auto& T()
	{
		std::swap((*this)[0][1], (*this)[1][0]);
		return *this;
	}

	static Mat<ValType, 2, 2> Eye()
	{
		Mat<ValType, 2, 2> m{};
		for (int i = 0; i < 2; ++i)
			m[i][i] = 1;
		return m;
	}

	static auto FromMemory(const ValType* p)
	{
		Mat<ValType, 2, 2> result;
		std::memcpy(&result, p, sizeof(result));
		return result;
	}

	static auto FromMemoryRowMajor(const ValType* p)
	{
		Mat<ValType, 2, 2> result;
		for (int row = 0; row < NumRows; ++row)
			for (int col = 0; col < NumRows; ++col)
				result[col][row] = p[row*NumRows+col];
		return result;
	}
};

using Mat2 = Mat<float, 2, 2>;
using iMat2 = Mat<int, 2, 2>;

template<typename ValType>
class Mat<ValType, 3, 3> {
public:

	Mat<ValType, 3, 1> col_vec_0;
	Mat<ValType, 3, 1> col_vec_1;
	Mat<ValType, 3, 1> col_vec_2;

	const auto& operator[](const int i) const { return (&col_vec_0)[i]; }
	auto& operator[](const int i) { return (&col_vec_0)[i]; }

	auto& T()
	{
		std::swap((*this)[0][1], (*this)[1][0]);
		std::swap((*this)[0][2], (*this)[2][0]);
		std::swap((*this)[1][2], (*this)[2][1]);
		return *this;
	}

	static Mat<ValType, 3, 3> Eye()
	{
		Mat<ValType, 3, 3> m{};
		for (int i = 0; i < 3; ++i)
			m[i][i] = 1;
		return m;
	}

	static auto FromMemory(const ValType* p)
	{
		Mat<ValType, 3, 3> result;
		std::memcpy(&result, p, sizeof(result));
		return result;
	}

	static auto FromMemoryRowMajor(const ValType* p)
	{
		Mat<ValType, 3, 3> result;
		for (int row = 0; row < 3; ++row)
			for (int col = 0; col < 3; ++col)
				result[col][row] = p[row*3+col];
		return result;
	}
};

using Mat3 = Mat<float, 3, 3>;
using iMat3 = Mat<int, 3, 3>;

template<typename ValType>
class Mat<ValType, 4, 4> {
public:

	Mat<ValType, 4, 1> col_vec_0;
	Mat<ValType, 4, 1> col_vec_1;
	Mat<ValType, 4, 1> col_vec_2;
	Mat<ValType, 4, 1> col_vec_3;

	const auto& operator[](const int i) const { return (&col_vec_0)[i]; }
	auto& operator[](const int i) { return (&col_vec_0)[i]; }

	auto& T()
	{
		std::swap((*this)[0][1], (*this)[1][0]);
		std::swap((*this)[0][2], (*this)[2][0]);
		std::swap((*this)[0][3], (*this)[3][0]);
		std::swap((*this)[1][2], (*this)[2][1]);
		std::swap((*this)[1][3], (*this)[3][1]);
		std::swap((*this)[2][3], (*this)[3][2]);
		return *this;
	}

	static Mat<ValType, 4, 4> Eye()
	{
		Mat<ValType, 4, 4> m{};
		for (int i = 0; i < 4; ++i)
			m[i][i] = 1;
		return m;
	}

	static auto FromMemory(const ValType* p)
	{
		Mat<ValType, 4, 4> result;
		std::memcpy(&result, p, sizeof(result));
		return result;
	}

	static auto FromMemoryRowMajor(const ValType* p)
	{
		Mat<ValType, 4, 4> result;
		for (int row = 0; row < 4; ++row)
			for (int col = 0; col < 4; ++col)
				result[col][row] = p[row*4+col];
		return result;
	}
};

using Mat4 = Mat<float, 4, 4>;
using iMat4 = Mat<int, 4, 4>;

////
// begin & end functions:
// Iterating each values in Mat in column major order.
////

template<typename ValType, int NumRows, int NumCols>
std::enable_if_t<NumCols == 1, ValType*>
	begin(Mat<ValType, NumRows, NumCols>& a)
{
	return (ValType*)(&a[0]);
}

template<typename ValType, int NumRows, int NumCols>
std::enable_if_t<NumCols != 1, ValType*>
	begin(Mat<ValType, NumRows, NumCols>& a)
{
	return (ValType*)(&a[0][0]);
}

template<typename ValType, int NumRows, int NumCols>
std::enable_if_t<NumCols == 1, const ValType*>
	begin(const Mat<ValType, NumRows, NumCols>& a)
{
	return (const ValType*)(&a[0]);
}

template<typename ValType, int NumRows, int NumCols>
std::enable_if_t<NumCols != 1, const ValType*>
	begin(const Mat<ValType, NumRows, NumCols>& a)
{
	return (const ValType*)(&a[0][0]);
}

template<typename ValType, int NumRows, int NumCols>
ValType* end(Mat<ValType, NumRows, NumCols>& a)
{
	return begin(a) + NumRows * NumCols;
}

template<typename ValType, int NumRows, int NumCols>
const ValType* end(const Mat<ValType, NumRows, NumCols>& a)
{
	return begin(a) + NumRows * NumCols;
}

////
// Element-wise operations.
////

#define ElementwiseOpsForMat(Op) \
template<typename ValType1, typename ValType2, int NumRows, int NumCols> \
auto operator##Op##=( \
	Mat<ValType1, NumRows, NumCols>& lhs, \
	const Mat<ValType2, NumRows, NumCols>& rhs) \
{ \
for (int i = 0; i < NumRows * NumCols; ++i) \
	* (begin(lhs) + i) Op##= *(begin(rhs) + i); \
return lhs; \
} \
\
template<typename ValType, typename ScalarType, int NumRows, int NumCols> \
std::enable_if_t< \
	std::is_same<float, ScalarType>::value \
	|| std::is_same<int, ScalarType>::value, \
	Mat<ValType, NumRows, NumCols>> \
operator##Op##=(Mat<ValType, NumRows, NumCols> & lhs, const ScalarType & rhs) \
{ \
for (int i = 0; i < NumRows * NumCols; ++i) \
	* (begin(lhs) + i) Op##= rhs; \
return lhs; \
} \
\
template<typename ValType1, typename ValType2, int NumRows, int NumCols> \
auto operator##Op( \
	const Mat<ValType1, NumRows, NumCols> & lhs, \
	const Mat<ValType2, NumRows, NumCols> & rhs) \
{ \
Mat<decltype(ValType1() + ValType2()), NumRows, NumCols> tmp{}; \
for (int i = 0; i < NumRows * NumCols; ++i) \
	* (begin(tmp) + i) = *(begin(lhs) + i) Op *(begin(rhs) + i); \
return tmp; \
} \
\
template<typename ValType, typename ScalarType, int NumRows, int NumCols> \
std::enable_if_t< \
	std::is_same<float, ScalarType>::value \
	|| std::is_same<int, ScalarType>::value, \
	Mat<decltype(ValType() + ScalarType()), NumRows, NumCols>> \
operator##Op( \
const Mat<ValType, NumRows, NumCols> & lhs, \
const ScalarType & rhs) \
{ \
Mat<decltype(ValType() + ScalarType()), NumRows, NumCols> tmp{}; \
for (int i = 0; i < NumRows * NumCols; ++i) \
	* (begin(tmp) + i) = *(begin(lhs) + i) Op rhs; \
return tmp; \
} \
\
template<typename ValType, typename ScalarType, int NumRows, int NumCols> \
std::enable_if_t< \
	std::is_same<float, ScalarType>::value \
	|| std::is_same<int, ScalarType>::value, \
	Mat<decltype(ValType() * ScalarType()), NumRows, NumCols>> \
operator##Op( \
const ScalarType & lhs, \
const Mat<ValType, NumRows, NumCols> & rhs) \
{ \
Mat<decltype(ValType() + ScalarType()), NumRows, NumCols> tmp{}; \
for (int i = 0; i < NumRows * NumCols; ++i) \
	* (begin(tmp) + i) = lhs Op * (begin(rhs) + i); \
return tmp; \
}

template<typename ValType1, typename ValType2, int NumRows, int NumCols>
bool operator==(
	const Mat<ValType1, NumRows, NumCols>& lhs,
	const Mat<ValType2, NumRows, NumCols>& rhs)
{
	auto lhs_iter = begin(lhs);
	auto rhs_iter = begin(rhs);
	while (lhs_iter != end(lhs))
		if (*lhs_iter != *rhs_iter)
			return false;
	return true;
}

template<typename ValType1, typename ValType2, int NumRows, int NumCols>
bool operator!=(
	const Mat<ValType1, NumRows, NumCols>& lhs,
	const Mat<ValType2, NumRows, NumCols>& rhs)
{
	return !(lhs == rhs);
}

ElementwiseOpsForMat(+);
ElementwiseOpsForMat(-);
ElementwiseOpsForMat(*);
ElementwiseOpsForMat(/);

template<typename ValType, int NumCols, int NumRows>
auto Abs(Mat<ValType, NumCols, NumRows> a)
{
	for (auto* p = begin(a); p != end(a); ++p)
		*p = std::abs(*p);
	return a;
}

template<typename ValType, int NumCols, int NumRows>
auto operator-(Mat<ValType, NumCols, NumRows> a)
{
	for (auto* p = begin(a); p != end(a); ++p)
		*p = -*p;
	return a;
}

////
// Matrix operations.
////

template<typename ValType, int NumRows>
std::enable_if_t<(NumRows>1), Mat<ValType, NumRows, NumRows>>
Transpose(Mat<ValType, NumRows, NumRows> m)
{
	return m.T();
}

inline auto Det(const Mat<float, 2, 2> &m)
{
	const auto* p = begin(m);
	return p[0] * p[3] - p[1] * p[2];
}

inline auto Det(const Mat<float, 3, 3> &m)
{
	const auto* p = begin(m);
	return p[0] * (p[4] * p[8] - p[5] * p[7])
		- p[3] * (p[1] * p[8] - p[2] * p[7])
		+ p[6] * (p[1] * p[5] - p[2] * p[4]);
}

inline auto Det(const Mat<float, 4, 4> &m)
{
	const auto* p = begin(m);
	return p[10] * p[13] * p[3] * p[4] + p[0] * p[10] * p[15] * p[5]
		- p[10] * p[12] * p[3] * p[5] + p[11] * (-p[13] * p[2] * p[4]
		- p[0] * p[14] * p[5] + p[12] * p[2] * p[5] + p[0] * p[13] * p[6])
		- p[0] * p[10] * p[13] * p[7] - p[15] * p[2] * p[5] * p[8]
		+ p[14] * p[3] * p[5] * p[8] - p[13] * p[3] * p[6] * p[8]
		+ p[13] * p[2] * p[7] * p[8] + p[1] * (p[11] * p[14] * p[4]
		- p[10] * p[15] * p[4] - p[11] * p[12] * p[6] + p[10] * p[12] * p[7]
		+ p[15] * p[6] * p[8] - p[14] * p[7] * p[8])
		+ p[15] * p[2] * p[4] * p[9] - p[14] * p[3] * p[4] * p[9]
		- p[0] * p[15] * p[6] * p[9] + p[12] * p[3] * p[6] * p[9]
		+ p[0] * p[14] * p[7] * p[9] - p[12] * p[2] * p[7] * p[9];
}

inline auto Inv(const Mat<float, 2, 2>& m)
{
	const auto* p = begin(m);
	return Mat<float, 2, 2>{
		{p[3], -p[1]},
		{ -p[2], p[0] }
	} /= Det(m);
}

inline auto Inv(const Mat<float, 3, 3>& m)
{
	const auto *p = begin(m);
	return Mat<float, 3, 3>{
	{
		-p[5] * p[7] + p[4] * p[8],
			p[2] * p[7] - p[1] * p[8],
			-p[2] * p[4] + p[1] * p[5]
	},
	{
		p[5] * p[6] - p[3] * p[8],
		-p[2] * p[6] + p[0] * p[8],
		p[2] * p[3] - p[0] * p[5]
	},
	{
		-p[4] * p[6] + p[3] * p[7],
		p[1] * p[6] - p[0] * p[7],
		-p[1] * p[3] + p[0] * p[4]
	}
	} /= Det(m);
}

inline auto Inv(const Mat<float, 4, 4>& m)
{
	const auto* p = begin(m);
	return Mat<float, 4, 4>{
	{
		-p[11] * p[14] * p[5] + p[10] * p[15] * p[5] + p[11] * p[13] * p[6]
			- p[10] * p[13] * p[7] - p[15] * p[6] * p[9] + p[14] * p[7] * p[9],
			p[1] * p[11] * p[14] - p[1] * p[10] * p[15] - p[11] * p[13] * p[2]
			+ p[10] * p[13] * p[3] + p[15] * p[2] * p[9] - p[14] * p[3] * p[9],
			-p[15] * p[2] * p[5] + p[14] * p[3] * p[5] + p[1] * p[15] * p[6]
			- p[13] * p[3] * p[6] - p[1] * p[14] * p[7] + p[13] * p[2] * p[7],
			p[11] * p[2] * p[5] - p[10] * p[3] * p[5] - p[1] * p[11] * p[6]
			+ p[1] * p[10] * p[7] + p[3] * p[6] * p[9] - p[2] * p[7] * p[9]
	},
	{
		p[11] * p[14] * p[4] - p[10] * p[15] * p[4] - p[11] * p[12] * p[6]
		+ p[10] * p[12] * p[7] + p[15] * p[6] * p[8] - p[14] * p[7] * p[8],
		-p[0] * p[11] * p[14] + p[0] * p[10] * p[15] + p[11] * p[12] * p[2]
		- p[10] * p[12] * p[3] - p[15] * p[2] * p[8] + p[14] * p[3] * p[8],
		p[15] * p[2] * p[4] - p[14] * p[3] * p[4] - p[0] * p[15] * p[6]
		+ p[12] * p[3] * p[6] + p[0] * p[14] * p[7] - p[12] * p[2] * p[7],
		-p[11] * p[2] * p[4] + p[10] * p[3] * p[4] + p[0] * p[11] * p[6]
		- p[0] * p[10] * p[7] - p[3] * p[6] * p[8] + p[2] * p[7] * p[8]
	},
	{
		-p[11] * p[13] * p[4] + p[11] * p[12] * p[5] - p[15] * p[5] * p[8]
		+ p[13] * p[7] * p[8] + p[15] * p[4] * p[9] - p[12] * p[7] * p[9],
		-p[1] * p[11] * p[12] + p[0] * p[11] * p[13] + p[1] * p[15] * p[8]
		- p[13] * p[3] * p[8] - p[0] * p[15] * p[9] + p[12] * p[3] * p[9],
		-p[1] * p[15] * p[4] + p[13] * p[3] * p[4] + p[0] * p[15] * p[5]
		- p[12] * p[3] * p[5] + p[1] * p[12] * p[7] - p[0] * p[13] * p[7],
		p[1] * p[11] * p[4] - p[0] * p[11] * p[5] + p[3] * p[5] * p[8]
		- p[1] * p[7] * p[8] - p[3] * p[4] * p[9] + p[0] * p[7] * p[9]
	},
	{
		p[10] * p[13] * p[4] - p[10] * p[12] * p[5] + p[14] * p[5] * p[8]
		- p[13] * p[6] * p[8] - p[14] * p[4] * p[9] + p[12] * p[6] * p[9],
		p[1] * p[10] * p[12] - p[0] * p[10] * p[13] - p[1] * p[14] * p[8]
		+ p[13] * p[2] * p[8] + p[0] * p[14] * p[9] - p[12] * p[2] * p[9],
		p[1] * p[14] * p[4] - p[13] * p[2] * p[4] - p[0] * p[14] * p[5]
		+ p[12] * p[2] * p[5] - p[1] * p[12] * p[6] + p[0] * p[13] * p[6],
		-p[1] * p[10] * p[4] + p[0] * p[10] * p[5] - p[2] * p[5] * p[8]
		+ p[1] * p[6] * p[8] + p[2] * p[4] * p[9] - p[0] * p[6] * p[9]
	}
	} /= Det(m);
}

template<typename ValType, int NumCols, int NumRows>
ValType ValueSum(const Mat<ValType, NumCols, NumRows>& a)
{
	ValType sum{ 0 };
	for (const auto* p = begin(a); p != end(a); ++p)
		sum += *p;
	return sum;
}

// Inner-product of two vecters.
template<int NumRows, int NumCols>
std::enable_if_t<NumCols == 1, float>
	Dot(const Mat<float, NumRows, NumCols>& p,
		const Mat<float, NumRows, NumCols>& q)
{
	auto result = p * q;
	return ValueSum(result);
}

// Inner-product of matrix A, with vector v (Av).
template<int MatSize, int NumCols>
std::enable_if_t<MatSize != 1 && NumCols == 1, Mat<float, MatSize, 1>>
	Dot(const Mat<float, MatSize, MatSize>& A,
		const Mat<float, MatSize, NumCols>& v)
{
	Mat<float, MatSize, 1> result{};
	for (int i = 0; i < MatSize; ++i)
		result += A[i]*v[i];
	return result;
}

// Inner-product of two matrices, A and B.
template<int MatSize>
std::enable_if_t<MatSize != 1, Mat<float, MatSize, MatSize>>
	Dot(const Mat<float, MatSize, MatSize>& A,
		const Mat<float, MatSize, MatSize>& B)
{
	Mat<float, MatSize, MatSize> result{};
	for (int i = 0; i < MatSize; ++i)
		result[i] = Dot(A, B[i]);
	return result;
}

template<int NumRows>
auto Length(const Mat<float,NumRows, 1>& v)
{
	return std::sqrtf(Dot(v,v));
}

template<int NumRows>
auto Normalize(const Mat<float, NumRows, 1>& v)
{
	return v / Length(v);
}

inline Vec3 Cross(const Vec3& p, const Vec3& q)
{
	return Vec3{
		p.y * q.z - q.y * p.z,
		p.z * q.x - q.z * p.x,
		p.x * q.y - q.x * p.y
	};
}

////
// Quaternion w+xi+yj+zk.
////

class Quat {
public:
	float w;
	float x;
	float y;
	float z;

	Quat()
		: w{1}, x{ 0 }, y{ 0 }, z{ 0 }
	{}

	Quat(float w, float x, float y, float z)
		: w{ w }, x{ x }, y{ y }, z{ z }
	{}

	Quat(float w, const Vec3& v)
		: w{ w }, x{ v.x }, y{ v.y }, z{ v.z }
	{}

	float Re() const { return w; }
	void Re(float w) { this->w = w; }
	Vec3 Im() const { return Vec3{ x, y, z }; }
	void Im(const Vec3& v) { x = v.x; y = v.y; z = v.z; }

	// Input axis should be a unit vector.
	static Quat AngleAxis(float angle, Vec3 axis)
	{
		return Quat{ cos(angle * .5f),
			axis * std::sinf(angle * .5f)};
	}
};

inline float Dot(const Quat& p, const Quat& q)
{
	return p.x*q.x + p.y*q.y + p.z*q.z + p.w*q.w;
}

inline Quat& operator+=(Quat lhs, const Quat& rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	lhs.w += rhs.w;
	return lhs;
}

inline Quat operator+(Quat lhs, const Quat& rhs)
{
	return lhs += rhs;
}

inline Quat& operator-=(Quat lhs, const Quat& rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	lhs.w -= rhs.w;
	return lhs;
}

inline Quat operator-(Quat lhs, const Quat& rhs)
{
	return lhs -= rhs;
}

inline Quat operator*(const Quat& p, float s)
{
	return Quat{p.w*s,p.x*s,p.y*s,p.z*s};
}

inline Quat operator*(float s, const Quat& p)
{
	return p*s;
}

inline Quat& operator*=(Quat & lhs, const Quat & rhs)
{
	const Quat lhs_{ lhs };
	lhs.w = lhs_.w * rhs.w - lhs_.x * rhs.x - lhs_.y * rhs.y - lhs_.z * rhs.z;
	lhs.x = lhs_.w * rhs.x + lhs_.x * rhs.w + lhs_.y * rhs.z - lhs_.z * rhs.y;
	lhs.y = lhs_.w * rhs.y + lhs_.y * rhs.w + lhs_.z * rhs.x - lhs_.x * rhs.z;
	lhs.z = lhs_.w * rhs.z + lhs_.z * rhs.w + lhs_.x * rhs.y - lhs_.y * rhs.x;
	return lhs;
}

inline Quat operator*(Quat lhs, const Quat & rhs)
{
	return lhs *= rhs;
}

inline Quat& operator/=(Quat& p, float s)
{
	p.x /= s;
	p.y /= s;
	p.z /= s;
	p.w /= s;
	return p;
}

inline Quat operator/(const Quat& p, float s)
{
	Quat result{ p };
	return result /= s;;
}

inline Quat operator/(float w, const Quat& p)
{
	return Quat{w/p.x,w/p.y,w/p.z,w/p.w};
}

inline Quat & operator/=(Quat & lhs, const Quat & rhs)
{
	const Quat lhs_{ lhs };

	lhs.w = lhs_.w * rhs.w + lhs_.x * rhs.x + lhs_.y * rhs.y + lhs_.z * rhs.z;
	lhs.x = lhs_.x * rhs.w - lhs_.w * rhs.x - lhs_.z * rhs.y + lhs_.y * rhs.z;
	lhs.y = lhs_.y * rhs.w + lhs_.z * rhs.x - lhs_.w * rhs.y - lhs_.x * rhs.z;
	lhs.z = lhs_.z * rhs.w - lhs_.y * rhs.x + lhs_.x * rhs.y - lhs_.w * rhs.z;

	lhs /= Dot(rhs, rhs);

	return lhs;
}

inline Quat operator/(Quat lhs, const Quat & rhs)
{
	return lhs /= rhs;
}

inline Quat Conj(const Quat& q)
{
	return Quat{q.w, -q.x,-q.y,-q.z};
}

inline Quat Inv(const Quat& q)
{
	return Conj(q) / Dot(q,q);
}

inline float Length(const Quat& q)
{
	return std::sqrtf(Dot(q,q));
}

inline Quat Normalize(Quat q)
{
	return q / Length(q);
}

// Input Quat should be a unit Quaternion.
inline Vec3 QuatRotate(const Quat& q, const Vec3& v)
{
	return 2.f*Dot(q.Im(), v) * q.Im()
		+ (q.w*q.w - Dot(q.Im(), q.Im())) * v
		+ 2.f*q.w * Cross(q.Im(), v);
}

// Input q should be normalized first.
inline Mat3 Quat2Mat3(const Quat& q)
{
	float xx = q.x*q.x, yy = q.y*q.y, zz = q.z*q.z, xz = q.x*q.z, \
		xy = q.x*q.y, yz = q.y*q.z, wx = q.w*q.x, wy = q.w*q.y, wz = q.w*q.z;

	return Mat3{
		{ 1.f - 2.f * (yy + zz), 2.f * (xy + wz), 2.f * (xz - wy)},
		{ 2.f * (xy - wz), 1.f - 2.f * (xx + zz), 2.f * (yz + wx)},
		{ 2.f * (xz + wy), 2.f * (yz - wx), 1.f - 2.f * (xx + yy)}};
}

inline float* begin(Quat& q) { return &(q.w); }
inline const float* begin(const Quat& q) { return &(q.w); }
inline float* end(Quat& q) {return begin(q) + 4; }
inline const float* end(const Quat& q) {return begin(q) + 4; }

// PCG, A Family of Better Random Number Generators.
class PCG {
public:

	using result_type = uint32_t;

	PCG(uint64_t seed)
		: state_{ seed }
	{}

	static uint32_t max() { return std::numeric_limits<uint32_t>::max(); }
	static uint32_t min() { return std::numeric_limits<uint32_t>::min(); }

	uint32_t operator()()
	{
		// Advance state.
		state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;

		// PCG randomize.
		auto xorshift = static_cast<uint32_t>(
			(state_ ^ (state_ >> 18u)) >> 27u);

		auto shift = state_ >> 59u;
		return (xorshift >> shift) | (xorshift << ((-shift) & 31));
	}

private:
	uint64_t state_;
};

////
// Utility functions.
////

template<typename ValType, int NumRows, typename ScalarType>
std::enable_if_t<NumRows==2||NumRows==3, Mat<ValType, NumRows+1, 1>>
Append(const Mat<ValType, NumRows, 1>& v, ScalarType s)
{
	Mat<ValType, NumRows+1, 1> result;
	for (int i = 0; i < NumRows; ++i)
		result[i] = v[i];
	result[NumRows] = static_cast<ValType>(s);
	return result;
}

template<typename ValType, int NumRows>
Mat<ValType, NumRows, NumRows> Diagonal(const Mat<ValType, NumRows, 1>& v)
{
	Mat<ValType, NumRows, NumRows> result{};
	for (int i = 0; i < NumRows; ++i)
			result[i][i] = v[i];
	return result;
}

template<typename ValType, int NumRows>
typename std::enable_if<(NumRows > 2), Mat<ValType, 2, 1>>::type
AsVec2(const Mat<ValType, NumRows, 1>& v)
{
	return {v.x, v.y};
}

template<typename ValType, int NumRows>
typename std::enable_if<(NumRows > 3), Mat<ValType, 3, 1>>::type
AsVec3(const Mat<ValType, NumRows, 1>& v)
{
	return {v.x, v.y, v.z};
}

template<typename ValType, int NumRows>
typename std::enable_if<(NumRows > 2), Mat<ValType, 2, 2>>::type
AsMat2(const Mat<ValType, NumRows, NumRows>& m)
{
	return {AsVec2(m[0]), AsVec2(m[1])};
}

template<typename ValType, int NumRows>
typename std::enable_if<(NumRows > 3), Mat<ValType, 3, 3>>::type
AsMat3(const Mat<ValType, NumRows, NumRows>& m)
{
	return {AsVec3(m[0]), AsVec3(m[1]), AsVec3(m[2])};
}

inline float Deg2Rad(const float deg)
{
	return deg * pi / 180.f;
}

template<typename T>
inline T Lerp(T v0, T v1, float t)
{
	return v0+(v1-v0)*t;
}

inline float Clamp(float s, float min, float max)
{
	return s>max?max:(s<min?min:s);
}

template<typename ValType, int NumRows, int NumCols>
auto ComponentWiseMin(
	Mat<ValType, NumRows, NumCols> v1,
	const Mat<ValType, NumRows, NumCols>& v2)
{
	auto v1_iter = begin(v1);
	auto v2_iter = begin(v2);
	while (v1_iter!=end(v1)) {
		*v1_iter = std::min(*v1_iter,*v2_iter);
		v1_iter++;
		v2_iter++;
	}
	return v1;
}

template<typename ValType, int NumRows, int NumCols>
auto ComponentWiseMax(
	Mat<ValType, NumRows, NumCols> v1,
	const Mat<ValType, NumRows, NumCols>& v2)
{
	auto v1_iter = begin(v1);
	auto v2_iter = begin(v2);
	while (v1_iter!=end(v1)) {
		*v1_iter = std::max(*v1_iter,*v2_iter);
		v1_iter++;
		v2_iter++;
	}
	return v1;
}

template<typename ValType1, typename ValType2, int NumRows, int NumCols>
bool ComponentWiseLessEqual(
	const Mat<ValType1, NumRows, NumCols>& lhs,
	const Mat<ValType2, NumRows, NumCols>& rhs)
{
	auto lhs_iter = begin(lhs);
	auto rhs_iter = begin(rhs);
	while (lhs_iter != end(lhs))
		if (*lhs_iter > *rhs_iter)
			return false;
	return true;
}

template<typename ValType1, typename ValType2, int NumRows, int NumCols>
bool ComponentWiseLessThan(
	const Mat<ValType1, NumRows, NumCols>& lhs,
	const Mat<ValType2, NumRows, NumCols>& rhs)
{
	auto lhs_iter = begin(lhs);
	auto rhs_iter = begin(rhs);
	while (lhs_iter != end(lhs))
		if (*lhs_iter >= *rhs_iter)
			return false;
	return true;
}

template<typename ValType1, typename ValType2, int NumRows, int NumCols>
bool ComponentWiseGreaterEqual(
	const Mat<ValType1, NumRows, NumCols>& lhs,
	const Mat<ValType2, NumRows, NumCols>& rhs)
{
	auto lhs_iter = begin(lhs);
	auto rhs_iter = begin(rhs);
	while (lhs_iter != end(lhs))
		if (*lhs_iter < *rhs_iter)
			return false;
	return true;
}

template<typename ValType1, typename ValType2, int NumRows, int NumCols>
bool ComponentWiseGreaterThan(
	const Mat<ValType1, NumRows, NumCols>& lhs,
	const Mat<ValType2, NumRows, NumCols>& rhs)
{
	auto lhs_iter = begin(lhs);
	auto rhs_iter = begin(rhs);
	while (lhs_iter != end(lhs))
		if (*lhs_iter <= *rhs_iter)
			return false;
	return true;
}

inline Vec3 UnProject(
	const Vec3& winpos, 
	const Mat4& view, 
	const Mat4& proj, 
	const Vec4& viewport)
{
	Vec4 tmp{
		(winpos.x - viewport.x) / viewport.z * 2.f - 1.f,
		(winpos.y - viewport.y) / viewport.w * 2.f - 1.f,
		winpos.z * 2.f - 1.f,
		1.f };

	Vec4 obj = Dot(Inv(Dot(proj, view)), tmp);
	obj /= obj.w;
	return AsVec3(obj);
}

inline Mat2 RotationTransform(float angle)
{
	return Mat2{
		{std::cosf(angle), std::sinf(angle)}, 
		{-std::sinf(angle), std::cosf(angle)}};
}

// Input axis should be a unit vector.
inline Mat3 RotationTransform(float angle, Vec3 axis)
{
	float cosw = std::cosf(angle), sin_w = std::sinf(angle);
	float _1cosw = 1-cosw;
	float x = axis.x, y = axis.y, z = axis.z;
	float zz = z*z, xx = x*x, yy = y*y, yz = y*z, xz = x*z, xy = x*y;
	float x_sin_w = x*sin_w, y_sin_w = y*sin_w, z_sin_w = z*sin_w;
	float xz_1cosw = xz*_1cosw;
	float xy_1cosw = xy*_1cosw;
	float yz_1cosw = yz*_1cosw;
	return Mat3{
		{ xx*_1cosw+cosw, xy_1cosw+z*sin_w, xz_1cosw-y_sin_w},
		{ xy_1cosw-z_sin_w, yy*_1cosw+cosw, yz_1cosw+x_sin_w},
		{ xz_1cosw+y_sin_w, yz_1cosw-x_sin_w, zz*_1cosw+cosw}};
}

template<int NumRows>
std::enable_if_t<NumRows==2||NumRows==3, Mat<float, NumRows+1,NumRows+1>>
AffineTransform(
	const Mat<float, NumRows, NumRows>& basis,
	const Mat<float, NumRows, 1>& translation)
{
	Mat<float, NumRows+1,NumRows+1> result{};
	for (int col = 0; col < NumRows; ++col)
		for (int row = 0; row < NumRows; ++row)
			result[col][row] = basis[col][row];
	for (int row = 0; row < NumRows; ++row)
		result[NumRows][row] = translation[row];
	result[NumRows][NumRows] = 1;
	return result;
}

// Solve for coordinates under new basis and origin.
inline Mat4 ViewTransform(
	Mat3 view_base, const Vec3& view_point)
{
	view_base.T();
	return AffineTransform(view_base, Dot(view_base, -view_point));
}

inline Mat4 LookAt(const Vec3 eye, const Vec3 spot, const Vec3 up)
{
	const Vec3 forward_ = Normalize(spot - eye);
	const Vec3 s = Normalize(Cross(forward_, up));
	const Vec3 up_ = Normalize(Cross(s, forward_));

	 return ViewTransform(Mat3{s,up_,-forward_}, eye);
}

inline Mat4 ProjectiveTransform(
	float fovy, float aspect, float znear, float zfar)
{
	float tan_half_fovy = std::tanf(fovy / 2.f);
	float tan_half_fovx = aspect * tan_half_fovy;

	Mat4 tmp{};
	tmp[0][0] = 1.f / (tan_half_fovx);
	tmp[1][1] = 1.f / (tan_half_fovy);
	tmp[2][2] = -(zfar + znear) / (zfar - znear);
	tmp[2][3] = -1.f;
	tmp[3][2] = -(2.f * zfar * znear) / (zfar - znear);
	return tmp;
}

inline Mat4 OrthographicTransform(
	float left, float right, float bottom, float top, 
	float near_plane, float far_plane)
{
	auto tmp = Mat4::Eye();
	tmp[0][0] = 2.f / (right - left);
	tmp[1][1] = 2.f / (top - bottom);
	tmp[2][2] = -2.f / (far_plane - near_plane);
	tmp[3][0] = -(right + left) / (right - left);
	tmp[3][1] = -(top + bottom) / (top - bottom);
	tmp[3][2] = -(far_plane + near_plane) / (far_plane - near_plane);
	tmp[3][3] = 1;
	return tmp;
}

////
// Ray tracing.
////

class Ray {
public:
	Vec3 o;
	Vec3 d;

	// Hit point: o+s*d.
	mutable float s;
};

template<typename VecType>
class Box {
public:
	VecType o;
	VecType size;

	Box() = default;

	Box(VecType o, VecType size)
		:o{o},size{size}
	{}

	VecType Sup() { return o+.5f*size; }
	VecType Inf() { return o-.5f*size; }

	static Box<VecType> BoundingBox(const std::vector<Box<VecType>>& boxes)
	{
		if (boxes.size() == VecType{})
			return Box<VecType>{};

		VecType inf = boxes[0].Inf(), sup = boxes[0].Sup();
		for (int i = 1; i < boxes.size(); ++i) {
			inf = ComponentWiseMin(inf,boxes[i].Inf());
			sup = ComponentWiseMax(sup,boxes[i].Sup());
		}
		auto o = (inf+sup)*.5f;
		auto size = sup - inf;
		return Box<VecType>{o, size};
	}
};

using Box2D = Box<Vec2>;
using Box3D = Box<Vec3>;

template<typename VecType>
bool Intersect(
	const Box<VecType>& box1, 
	const Box<VecType>& box2, 
	Box<VecType>& result)
{
	auto inf = ComponentWiseMax(box1.Inf(),box2.Inf());
	auto sup = ComponentWiseMin(box1.Sup(),box2.Sup());
	if (ComponentWiseLessEqual(inf,sup)) {
		result = Box<VecType>::BoundingBox({inf, sup});
		return true;
	}
	else
		return false;
}

template<typename VecType>
auto Union(
	const Box<VecType>& box1, 
	const Box<VecType>& box2)
{
	if (box1.size() == VecType{})
		return box2;
	if (box2.size() == VecType{})
		return box1;
	auto inf = ComponentWiseMin(box1.Inf(),box2.Inf());
	auto sup = ComponentWiseMax(box1.Sup(),box2.Sup());
	return Box<VecType>::BoundingBox({inf, sup});
}

class Sphere {
public:
	Vec3 o;
	float r;
};

inline bool Hit(const Sphere &sphere, const Ray &ray)
{
	auto b = 2.f*Dot(ray.o - sphere.o, ray.d);
	auto c = Dot(ray.o - sphere.o, ray.o - sphere.o) - sphere.r * sphere.r;
	auto delta = b * b - 4 * c;
	if (delta <= eps)
		return false;

	delta = std::sqrtf(delta);
	auto t1 = std::min(.5f*(-b - delta), .5f*(-b + delta));
	auto t2 = std::max(.5f*(-b - delta), .5f*(-b + delta));

	if (t2 < eps)
		return false;

	if (t1 >= eps)
		ray.s = t1;
	else
		ray.s = t2;

	return true;
}



}


#endif
