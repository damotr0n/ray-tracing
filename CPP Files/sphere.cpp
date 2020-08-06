#include "../Header Files/sphere.h"
#include <math.h>


Sphere::Sphere(Vertex c, float r,
        float ambient_ref, float diffuse_ref, float specular_ref,
        float colours[3],
        float int_a, float int_d, float int_s,
        float refl_ind, float refr_ind, bool refr)
{
	center = c;
	radius = r;
    this->k_a = ambient_ref;
    this->k_d = diffuse_ref;
    this->k_s = specular_ref;
    this->colours[0] = colours[0];
    this->colours[1] = colours[1];
    this->colours[2] = colours[2];
    this->I_a = int_a;
    this->I_d = int_d;
    this->I_s = int_s;
    this->refl_ind = refl_ind;
    this->refr_ind = refr_ind;
    this->refr = refr;
}

void Sphere::intersection(Ray ray, Hit &hit)
{

	Vector ro;

	hit.flag = false;

	// offset ray by sphere position
	// equivalent to transforming ray into local sphere space

	ro.x = ray.position.x - center.x;
	ro.y = ray.position.y - center.y;
	ro.z = ray.position.z - center.z;

	float a = (float)ray.direction.dot(ray.direction);
	float b = (float)(2.0 * ray.direction.dot(ro));
	float c = (float)ro.dot(ro) - radius*radius;

	float disc = b*b - 4 * a*c;

	if (disc < 0.0)
	{
		return; // a negative value indicates no intersection.
	}

	float ds = sqrtf(disc);
	

	float t0 = (-b - ds) / 2.0f;
	float t1 = (-b + ds) / 2.0f;

	if (t1 < 0.0)
	{
		return;
	}

	// if an intersection has been found, record details in hit object

	hit.what = this;

	if (t0 > 0.0) //smallest root is positive.
	{
		hit.t = t0;

		hit.position.x = ray.position.x + t0 * ray.direction.x;
		hit.position.y = ray.position.y + t0 * ray.direction.y;
		hit.position.z = ray.position.z + t0 * ray.direction.z;
		hit.normal.x = hit.position.x - center.x;
		hit.normal.y = hit.position.y - center.y;
		hit.normal.z = hit.position.z - center.z;
		hit.normal.normalise();
		hit.flag = true;

		return;
	}

	hit.t = t1;

	hit.position.x = ray.position.x + t1 * ray.direction.x;
	hit.position.y = ray.position.y + t1 * ray.direction.y;
	hit.position.z = ray.position.z + t1 * ray.direction.z;
	hit.normal.x = hit.position.x - center.x;
	hit.normal.y = hit.position.y - center.y;
	hit.normal.z = hit.position.z - center.z;
	hit.normal.normalise();
	hit.flag = true;

	return;
}
