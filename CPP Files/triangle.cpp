#include "../Header Files/triangle.h"
#include <math.h>
#include <iostream>

/**
 * Empty constructor, only exists to create array of triangles before they are initialized
 */
Triangle::Triangle() {}

/**
 * Triangle initializer, precomputes some often used values
 * @param vertices : vertices that make up the triangle
 */
void Triangle::initialize(Vertex &vec1, Vertex &vec2, Vertex &vec3){

    vertex[0] = &vec1;
    vertex[1] = &vec2;
    vertex[2] = &vec3;

    this->v1 = Vector(vertex[1]->x - vertex[0]->x, vertex[1]->y - vertex[0]->y, vertex[1]->z - vertex[0]->z);
    this->v2 = Vector(vertex[2]->x - vertex[0]->x, vertex[2]->y - vertex[0]->y, vertex[2]->z - vertex[0]->z);

    normal = Vector();
    // magnitude of this cross product as area of parallelogram created by reflecting this triangle, only works with not normalized plane normals
    // useful for barycentric coordinates
    this->v2.cross(this->v1, normal);

}

/**
 * Tests for the intersection with the triangle.
 * @param ray : ray that is intersecting the triangle
 * @param hit : hit where the information about the intersection is stored
 */
void Triangle::intersection(Ray ray, Hit &hit) {

    hit.flag = false;
    hit.what = this;

    float dot = normal.dot(ray.direction);

    if (dot == 0.0f)
    {
        // no intersection, ray parallel to plane
        return;
    }

    otov1 = Vector(vertex[0]->x, vertex[0]->y, vertex[0]->z);

    d = -normal.dot(otov1);

    dotv1v1 = v1.dot(v1);
    dotv1v2 = v1.dot(v2);
    dotv2v2 = v2.dot(v2);
    calcDenom = 1.0f / (dotv1v1 * dotv2v2 - dotv1v2 * dotv1v2);

    Vector other = Vector(ray.position.x, ray.position.y, ray.position.z);
    float ndotp = normal.dot(other);
    hit.t = -(ndotp + d) / dot;

    hit.position.x = ray.position.x + hit.t * ray.direction.x;
    hit.position.y = ray.position.y + hit.t * ray.direction.y;
    hit.position.z = ray.position.z + hit.t * ray.direction.z;

    // use barycentric coordinates to find out if ray intercept is inside the triangle
    // adapted from http://blackpawn.com/texts/pointinpoly/default.html?fbclid=IwAR3rHgXfmHYjLTgWIBFUQ4lhmWAMN-tTrHJNn2VcggD2tInAC7DOzMKhSAg

    Vector vhit = Vector (hit.position.x - vertex[0]->x, hit.position.y - vertex[0]->y, hit.position.z - vertex[0]->z);

    float dotv1hit = v1.dot(vhit);
    float dotv2hit = v2.dot(vhit);

    float u = (dotv2v2 * dotv1hit - dotv1v2 * dotv2hit) * calcDenom;
    float v = (dotv1v1 * dotv2hit - dotv1v2 * dotv1hit) * calcDenom;

    hit.flag = (u>=0) && (v>=0) && (u + v <= 1);

    // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
    // if the triangle is hit, compute normals for smooth shading
    // otherwise don't bother
    if(hit.flag){

        float w = 1 - u - v;

        hit.normal.x = w * this->vertex[0]->n.x + u * this->vertex[1]->n.x + v * this->vertex[2]->n.x;
        hit.normal.y = w * this->vertex[0]->n.y + u * this->vertex[1]->n.y + v * this->vertex[2]->n.y;
        hit.normal.z = w * this->vertex[0]->n.z + u * this->vertex[1]->n.z + v * this->vertex[2]->n.z;

        hit.normal.normalise();

    }

}

