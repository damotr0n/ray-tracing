// Hit is a class to store and maniplulate information about an intersection
// between a ray and an object.

#pragma once

#include "vertex.h"
#include "vector.h"

class Object;

class Hit {
public:
	bool flag = false;
	float t = MAXFLOAT;
	Object *what;
	Vertex position;
	Vector normal;

	float distance_from(Vertex &to){
	    float dx = to.x - this->position.x;
        float dy = to.y - this->position.y;
        float dz = to.z - this->position.z;

        return sqrt(dx*dx + dy*dy + dz*dz);

	}

};
