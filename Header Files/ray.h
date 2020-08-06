// Ray is a class to store and maniplulate 3D rays.

#pragma once

#include "vertex.h"
#include "vector.h"

class Ray {
public:
	Vertex position;
	Vector direction;

	Ray()
	{
	}

	Ray(Vertex p, Vector d)
	{
		position = p;
		direction = d;
	}

	void add_bias(float bias, Vector dir){
	    this->position.x += bias * dir.x;
        this->position.y += bias * dir.y;
        this->position.z += bias * dir.z;

    }
};
