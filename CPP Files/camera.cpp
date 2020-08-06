#include "../Header Files/camera.h"

Camera::Camera(Vertex look, Vertex pos, Vector up)
{
    eye = pos;

    w = new Vector(eye.x - look.x, eye.y - look.y, eye.z - look.z);
    w->normalise();

    u = new Vector();
    w->cross(up, *u);
    u->normalise();

    v = new Vector();
    w->cross(*u, *v);

};
