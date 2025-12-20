#pragma once

#include "MyMath.h"

// Sphere
class BoundingSphere {
public:
	Vec3 centre;
	float radius;
};

// Ray
class Ray {
public:
	Vec3 o;
	Vec3 dir;
	Vec3 invdir;

	Ray() {}
	Ray(const Vec3 _o, const Vec3 _dir) { initialize(_o, _dir); }

	void initialize(const Vec3 _o, const Vec3 _dir) {
		o = _o;
		dir = _dir;
		invdir = Vec3(1.0f, 1.0f, 1.0f) / dir;
	}

	Vec3 at(const float t) { return (o + (dir * t)); }
};

// Axis Alligned Bounding Box
class AABB {
public:
	Vec3 max;
	Vec3 min;

	AABB() { reset(); }

	void reset() {
		max = Vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	}

	void extend(const Vec3& p) {
		max = Max(max, p);
		min = Min(min, p);
	}

	// Slab Test for Ray-AABB Collision
	bool rayAABB(const Ray& r, float& t) {
		Vec3 s = (min - r.o) * r.invdir;
		Vec3 l = (max - r.o) * r.invdir;
		Vec3 s1 = Min(s, l);
		Vec3 l1 = Max(s, l);
		float ts = std::max<float>(s1.x, std::max<float>(s1.y, s1.z));
		float tl = std::min<float>(l1.x, std::min<float>(l1.y, l1.z));
		t = std::min<float>(ts, tl);
		return (ts < tl);
	}
};

static bool collisionAabbAabb(const AABB& object1, const AABB& object2) {
	// Calculate Penetration Depths
	float pdx = std::min<float>(object1.max.x, object2.max.x) - std::max<float>(object1.min.x, object2.min.x);
	float pdy = std::min<float>(object1.max.y, object2.max.y) - std::max<float>(object1.min.y, object2.min.y);
	float pdz = std::min<float>(object1.max.z, object2.max.z) - std::max<float>(object1.min.z, object2.min.z);

	return pdx > 0 && pdy > 0 && pdz > 0;
}

static bool collisionSphereSphere(const BoundingSphere& object1, const BoundingSphere& object2) {
	return (object1.centre - object2.centre).lengthSquare() <= powf((object1.radius + object2.radius), 2.f);
}

static bool collisionSphereAabb(const BoundingSphere& sphere, const AABB& aabb) {
	// Find the closest point on AABB to Sphere centre
	float px = std::max<float>(aabb.min.x, std::min<float>(sphere.centre.x, aabb.max.x));
	float py = std::max<float>(aabb.min.y, std::min<float>(sphere.centre.y, aabb.max.y));
	float pz = std::max<float>(aabb.min.z, std::min<float>(sphere.centre.z, aabb.max.z));

	return powf((sphere.centre.x - px) + (sphere.centre.y - py) + (sphere.centre.z - pz), 2.f) <= powf(sphere.radius, 2.f);
}

static bool collisionRaySphere(const BoundingSphere& sphere, const Ray& ray) {
	float a = Dot(ray.dir, ray.dir);
	float b = 2.f * Dot(ray.dir, (ray.o - sphere.centre));
	float c = (ray.o - sphere.centre).lengthSquare() - powf(sphere.radius, 2.f);
	float quadraticEquation = powf(b, 2.f) * 4.f * a * c;

	if (quadraticEquation < 0) return false;					// No intersection
	float root1 = (-b + sqrtf(quadraticEquation)) / (2.f * a);  // Solution One
	float root2 = (-b - sqrtf(quadraticEquation)) / (2.f * a);  // Solution Two
	return true;
}