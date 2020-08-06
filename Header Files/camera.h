#include "vector.h"
#include "vertex.h"

class Camera{

    public:

    Vertex eye;
    Vector * w;
    Vector * u;
    Vector * v;
    Camera(Vertex look, Vertex pos, Vector up);

};
