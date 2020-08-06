#pragma once

#include "vertex.h"
#include "object.h"

class Sphere : public Object {
	Vertex center;
	float  radius;
public:
	Sphere(Vertex c, float r,
	        float ambient_ref, float diffuse_ref, float specular_ref,
	        float colours[3],
	        float int_a, float int_d, float int_s,
	        float refl_ind, float refr_ind, bool refr = false);
	void intersection(Ray ray, Hit &hit);
};
