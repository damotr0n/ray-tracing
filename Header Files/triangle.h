#include "object.h"
#include "vertex.h"
#include "vector.h"

class Triangle : public Object {
    Vertex *vertex[3];
    Vector v1, v2, otov1;
    public: Vector normal;
    float d, dotv1v1, dotv1v2, dotv2v2, calcDenom;

public:
    Triangle();
    void initialize(Vertex &v1, Vertex &v2, Vertex &v3);
    void intersection(Ray ray, Hit &hit);
};
