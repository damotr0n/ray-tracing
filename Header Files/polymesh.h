#pragma once

#include "vertex.h"
#include "transform.h"
#include "ray.h"
#include "hit.h"
#include "triangle.h"
#include <vector>

class PolyMesh : public Object {
public:
	int vertex_count;
	int triangle_count;
    Vertex *vertex;
    Triangle *triangles;

	void do_construct(char *file, Transform *transform,
	        float ambient_ref, float diffuse_ref, float specular_ref,
	        float colours[3],
	        float int_a, float int_d, float int_s,
	        float refl_ind, float refr_ind, bool refr);
	
	PolyMesh(char *file);
	PolyMesh(char *file, Transform *transform,
	        float ambient_ref, float diffuse_ref, float specular_ref,
	        float colours[3],
	        float int_a, float int_d, float int_s,
	        float refl_ind, float refr_ind, bool refr = false);

	void intersection(Ray ray, Hit &hit);

	void compute_vertex_normals();

};
