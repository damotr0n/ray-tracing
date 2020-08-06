#include "../Header Files/sphere.h"
#include "../Header Files/ray.h"
#include "../Header Files/framebuffer.h"
#include "../Header Files/vector.h"
#include "../Header Files/camera.h"
#include "../Header Files/polymesh.h"
#include "../Header Files/light.h"
#include "../Alglib/alglibmisc.h"
#include "../Alglib/stdafx.h"
#include "../Header Files/photon.h"

#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <random>

int object_array_length;

int lights_array_length;
float light_ratio;

float sf;

FrameBuffer *fb;

Object *objects[8];
Light *lights[25];
alglib::kdtree photon_map_global;
alglib::kdtree photon_map_caustic;

void find_closest_hit(Ray *ray, Hit &closest_hit);

float* calculate_colours(Hit *hit, Vector incident_dir, Vector light_dir, float shadow_coeff = 0);

float* cast_ray(Ray *ray, int curr_depth, Hit &hit);

float* cast_ray_PM(Ray *ray, int curr_depth, Hit &hit);

void generate_photon_maps();

void generate_rayTrace_image(int width, int height, Camera *c);

int main(int argc, char *argv[]);

Ray* get_refracted_ray(Ray *incident_ray, Hit *hit);

Ray* get_reflected_ray(Ray *ray, Hit *hit);

float get_gaussian_filter(float dist, float max_dist);
