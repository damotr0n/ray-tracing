// Object is the base class for objects.
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "ray.h"
#include "hit.h"

class Object {
public:

	Object *next;

	/**
	 * Light properties
	 */
	// ambient reflection coefficient
	float k_a;
	// diffuse reflection coefficient
	float k_d;
	// specular reflection coefficient
	float k_s;

	float I_a, I_d, I_s;

	float refl_ind;
	float refr_ind;
	bool refr;

	float colours [3];

	Object()
	{
		next = (Object *)0;
	}
	
	virtual void intersection(Ray ray, Hit &hit)
	{

	}
};

#endif
