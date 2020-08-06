#include "vertex.h"
#include "vector.h"
#include "object.h"

class Plane : public Object {
    Vector normal;
    Vector position;
public:
    Plane(Vector normal, Vector position, float colours[3]);
    void intersection(Ray ray, Hit &hit);
};
