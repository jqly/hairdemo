#ifndef XY_RT
#define XY_RT


#include "./xy_calc.h"


namespace xy
{


struct Ray {

	Ray(vec3 o, vec3 d)
		:o{ o }, d{ xy::Normalize(d) }
	{}

	vec3 p(float s) const
	{
		return o + s * d;
	}

	vec3 p() const
	{
		return o + s * d;
	}

	vec3 o, d;
	mutable float s;

};

struct Sphere {
	xy::vec3 o;
	float r;
};

inline bool Hit(const Sphere &sphere, const Ray &ray)
{
	auto b = 2.f*xy::Dot(ray.o - sphere.o, ray.d);
	auto c = xy::Dot(ray.o - sphere.o, ray.o - sphere.o) - sphere.r * sphere.r;
	auto delta = b * b - 4 * c;
	if (delta <= big_eps<float>)
		return false;

	delta = sqrt(delta);
	auto t1 = xy::Min(.5f*(-b - delta), .5f*(-b + delta));
	auto t2 = xy::Max(.5f*(-b - delta), .5f*(-b + delta));

	if (t2 < big_eps<float>)
		return false;

	if (t1 >= big_eps<float>)
		ray.s = t1;
	else
		ray.s = t2;

	return true;
}


}


#endif