#include "vector.h"
#include "vertex.h"

class Light {

public:

    Vertex position;
    Vector direction;

    Light();

    Light(Vector dir);

    void compute_and_set_dir(Vertex end, Vector &dir);

};
