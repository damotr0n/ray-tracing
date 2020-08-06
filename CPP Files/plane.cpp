#include "../Header Files/plane.h"

Plane::Plane(Vector normal, Vector position, float colours[3]) {

    this->normal = normal;
    normal.normalise();
    this->position = position;
    this->colours[0] = colours[0];
    this->colours[1] = colours[1];
    this->colours[2] = colours[2];

}

void Plane::intersection(Ray ray, Hit &hit) {
    hit.flag = false;
    hit.normal = this->normal;
    hit.what = this;

    float dot = normal.dot(ray.direction);

    if (dot == 0.0f)
    {
        // no intersection, ray parallel to plane
        return;
    }

    // ray intersection, as adapted from: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
    Vector diff = Vector(
            position.x - ray.position.x,
            position.y - ray.position.y,
            position.z - ray.position.z
            );

    hit.t = diff.dot(normal) / ray.direction.dot(normal);

//    Vector other = Vector(ray.position.x, ray.position.y, ray.position.z);
//    float ndotp = normal.dot(other);
//    hit.t = -(ndotp + d) / dot;

    hit.position.x = hit.t * ray.direction.x;
    hit.position.y = hit.t * ray.direction.y;
    hit.position.z = hit.t * ray.direction.z;
}
