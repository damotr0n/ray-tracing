#include "../Header Files/light.h"

Light::Light() {};

Light::Light(Vector dir) {
    direction = dir;
}

/**
 * For modelling a point light that has the same intensity in all directions
 * Computes light direction from position of light to end
 * @param end : end of the light ray
 * @param dir : pointer where the direction should be stored
 */
void Light::compute_and_set_dir(Vertex end, Vector &dir) {
    // L->P = Vec L - Vec P
    dir.x = this->position.x - end.x;
    dir.y = this->position.y - end.y;
    dir.z = this->position.z - end.z;

}
