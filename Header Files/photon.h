#include "../Header Files/vector.h"
class Photon{

public:
    Vector incident;
    float colours[3] = {0.0f, 0.0f, 0.0f};
    float intensity;
    Vertex position;
    bool hit = false;

};
