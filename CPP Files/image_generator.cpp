#include "../Header Files/image_generator.h"

// set of macros that define the image
#define photon_map_global_size 275000
#define photon_map_caustic_size 25000
#define img_width 2048 / 8
#define max_depth 6
#define RT true
#define GPM true
#define CPM true
#define CPM_exposure 0.2

Photon *photon_map_global_array[photon_map_global_size];
Photon *photon_map_caustic_array[photon_map_caustic_size];

/**
 * Sets up the scene, generates photon maps, and renders the image
 */
int main(int argc, char *argv[])
{
    std::cout << "Setting up scene..." << std::endl;
    // start of setup //////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // define size of the image, defined as a square for simplicity
    int width = img_width;
    int height = width;

    fb = new FrameBuffer(width,height);

    // define and create camera
    Vertex camera_look = Vertex(0.000f, 0.0f, 15.0f);
    Vertex camera_pos = Vertex(0, 0, 0.0f);
    Vector up = Vector(0, 1, 0);
    Camera *c = new Camera(camera_look, camera_pos, up);

    // create objects to be rendered

    // teapot transform coefficients:
    // d: x axis, h: y axis, l: z axis
    // x offset by a minimal amount to avoid artifact down the middle of the teapot
    Transform *transform = new Transform (
            1,0,0,0.0001,
            0,0,1,-5,
            0,1,0,18,
            0,0,0,1
            );

    float *colours = new float[3];
    colours[0] = 66;
    colours[1] = 190;
    colours[2] = 212;
    PolyMesh *teapot = new PolyMesh((char *)"./ply_files/teapot.ply", transform,
            0.2f,0.5f,0.3f,
            colours,
            0.3f, 0.3f, 0.4f,
            0.0f, 0.0f
    );

    // model a glass sphere
    colours[0] = 66;
    colours[1] = 224;
    colours[2] = 245;
    Vertex centre = Vertex(3, -4, 13);
    Sphere *sphere = new Sphere(centre,1,
            0.2f, 0.5f, 0.3f,
            colours,
            0.3f, 0.3f, 0.4f,
            0.0f, 1.517f, true
    );

    colours[0] = 163;
    colours[1] = 36;
    colours[2] =36;
    Vertex centre2 = Vertex(-3.5, -3.5, 14);
    Sphere *sphere2 = new Sphere(centre2, 1.5,
             0.2f, 0.5f, 0.3f,
             colours,
             0.3f, 0.3f, 0.4f,
             0.9f, 0.0f
    );

    Transform *transform1 = new Transform();

    colours[0] = 255;
    colours[1] = 255;
    colours[2] = 255;
    PolyMesh *box_bot = new PolyMesh((char *)"./ply_files/box_bot.ply", transform1,
                                     0.8f, 0.1f, 0.1f,
                                     colours,
                                     0.8f, 0.1f, 0.1f,
                                     0.0f, 0.0f
    );

    PolyMesh *box_back = new PolyMesh((char *)"./ply_files/box_back.ply", transform1,
                                      0.8f, 0.1f, 0.1f,
                                      colours,
                                      0.8f, 0.1f, 0.1f,
                                      0.0f, 0.0f
    );

    PolyMesh *box_ceil = new PolyMesh((char *)"./ply_files/box_ceil.ply", transform1,
                                      0.8f, 0.1f, 0.1f,
                                      colours,
                                      0.8f, 0.1f, 0.1f,
                                      0.0f, 0.0f
    );

    colours[0] = 179;
    colours[1] = 0;
    colours[2] = 0;
    PolyMesh *box_rhs = new PolyMesh((char *)"./ply_files/box_rhs.ply", transform1,
                                     0.8f, 0.1f, 0.1f,
                                     colours,
                                     0.8f, 0.1f, 0.1f,
                                     0.0f, 0.0f
    );

    colours[0] = 74;
    colours[1] = 179;
    colours[2] = 0;
    PolyMesh *box_lhs = new PolyMesh((char *)"./ply_files/box_lhs.ply", transform1,
                                     0.8f, 0.1f, 0.1f,
                                     colours,
                                     0.8f, 0.1f, 0.1f,
                                     0.0f, 0.0f
    );


    // create list of objects
    objects[0] = box_bot;
    objects[1] = box_rhs;
    objects[2] = box_lhs;
    objects[3] = box_back;
    objects[4] = box_ceil;
    objects[5] = sphere2;
    objects[6] = sphere;
    objects[7] = teapot;
    object_array_length = sizeof(objects) / sizeof(objects[0]);

    // set up ray and other coefficients

    // aspect ratio is 1:1 therefore both scale factor values are the same
    sf = tan(M_PI/4) / width;

    // set up light
    // +/-
    // x: right/left, y: up/down, z: forward/back

    float x_l, z_l;
    int index = 0;
    int start = -2;
    int end = 3;
    int inc = 1;

    for (float i = start; i < end; i += inc){
        x_l = 0.0f + i;
        for (float j = start; j < end; j += inc){
            z_l = 15.0f + j;
            lights[index] = new Light();
            lights[index]->position = Vertex(x_l, 4.999, z_l);
            index++;
        }
    }

    lights_array_length = sizeof(lights) / sizeof(lights[0]);
    light_ratio = 1.0f / (float) lights_array_length;

    // end of setup ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << "Finished setting up scene..." << std::endl;

    generate_photon_maps();

    std::cout << "Generating image..." << std::endl;

    #pragma omp parallel for collapse(2) // parallelize the nested loop
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){

            float x_v = sf * (float)(x - (width/2));
            float y_v = sf * (float)(y - (width/2));

            float px = x_v * c->u->x + y_v * c->v->x - c->w->x;
            float py = x_v * c->u->y + y_v * c->v->y - c->w->y;
            float pz = x_v * c->u->z + y_v * c->v->z - c->w->z;

            Vector *ray_vector = new Vector();
            ray_vector->x = px; ray_vector->y = py; ray_vector->z = pz;

            Ray *ray = new Ray();
            ray->position = c->eye;
            ray->direction = *ray_vector;
            free(ray_vector);

            ray->direction.normalise();

            Hit hit = Hit();
            float *intensity = cast_ray_PM(ray, max_depth, hit);

            fb->plotPixel(x, y, intensity[0], intensity[1], intensity[2]);
        }
    }

    // write an image
    std::cout << "Writing file..." << std::endl;
    fb->writeRGBFile((char *)"test.ppm");

    std::cout << "Fin." << std::endl;

    return 0;

}

/**
 * Generates the two photon maps to be used in rendering
 */
void generate_photon_maps() {

    std::default_random_engine gen(static_cast<unsigned int>(time(nullptr)));
    std::normal_distribution<float> dist(0, 1);

    std::uniform_real_distribution<float> x_dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> y_dist(-1.0f, 0.0f);
    std::uniform_real_distribution<float> z_dist(-1.0f, 1.0f);

    if(GPM) {
        std::cout << "Building Global Photon Map..." << std::endl;
        int global_photon_max = photon_map_global_size;
        while (global_photon_max) {

            for (int light_index = 0; light_index < lights_array_length; light_index++) {

                std::srand(static_cast<unsigned int>(time(nullptr)));
                Hit *hit = new Hit();
                Ray *ray = new Ray();
                Photon *photon = new Photon();

                // finds a hit
                while (not hit->flag) {
                    ray->direction = Vector(x_dist(gen), y_dist(gen), z_dist(gen));

                    while (ray->direction.y > 0.0f) {
                        ray->direction.y = dist(gen);
                    }
                    ray->position = lights[light_index]->position;
                    find_closest_hit(ray, *hit);
                }
                photon->hit = true;
                photon->intensity = 1.0f / photon_map_global_size;

                bool survive = rand() % 2;
                bool store = false;
                Hit prev_hit;
                while (survive) {
                    prev_hit = *hit;
                    // only accept directions that will direct photon in diffuse direction
                    do {
                        ray->direction = Vector(dist(gen), dist(gen), dist(gen));
                    } while (ray->direction.dot(hit->normal) < 0);

                    ray->position = hit->position;
                    find_closest_hit(ray, *hit);

                    // only store second hit, storing first hit will add to direct illumination
                    if (hit->flag) {
                        photon->incident = ray->direction;
                        photon->incident.normalise();
                        photon->position = hit->position;
                        photon->colours[0] = prev_hit.what->colours[0];
                        photon->colours[1] = prev_hit.what->colours[1];
                        photon->colours[2] = prev_hit.what->colours[2];
                        store = true;
                    }

                    survive = rand() % 2;
                }

                if (store) {
                    photon_map_global_array[photon_map_global_size - global_photon_max] = photon;
                    global_photon_max--;
                    // while loop checks every 25, if it misses 0 it will continue below 0
                    if (global_photon_max == 0) break;
                }
            }
        }

        std::cout << "Creating Global KD Tree..." << std::endl;

        double photon_map_buffer_global[photon_map_global_size * 3];
        int photon_index_global = 0;
        while (photon_index_global < photon_map_global_size) {
            photon_map_buffer_global[photon_index_global *
                                     3] = photon_map_global_array[photon_index_global]->position.x;
            photon_map_buffer_global[photon_index_global * 3 +
                                     1] = photon_map_global_array[photon_index_global]->position.y;
            photon_map_buffer_global[photon_index_global * 3 +
                                     2] = photon_map_global_array[photon_index_global]->position.z;
            photon_index_global++;
        }
        alglib::integer_1d_array tag_array_global;
        tag_array_global.setlength(photon_map_global_size);
        for (int ti = 0; ti < photon_map_global_size; ti++) {
            tag_array_global[ti] = ti;
        }

        alglib::real_2d_array photon_map_alglib_array_global;
        photon_map_alglib_array_global.setcontent(photon_map_global_size, 3, photon_map_buffer_global);
        alglib::kdtreebuildtagged(photon_map_alglib_array_global, tag_array_global, photon_map_global_size, 3, 0, 2,
                                  photon_map_global);
    }

    if(CPM) {
        std::cout << "Building Caustic Photon Map..." << std::endl;

        std::default_random_engine gen_c(static_cast<unsigned int>(time(nullptr)));
        std::uniform_real_distribution<float> diff(-1, 1);

        float caustic_centre_x = 3.0f;
        float caustic_centre_y = -4.0f;
        float caustic_centre_z = 13.0f;

        Vertex caustic_centre;

        int caustic_photon_max = photon_map_caustic_size;
        while (caustic_photon_max) {

            for (int light_index = 0; light_index < lights_array_length; light_index++) {

                caustic_centre = Vertex(
                        caustic_centre_x,
                        caustic_centre_y,
                        caustic_centre_z
                );

                // finds a random end point within the glass sphere
                float end_x, end_y, end_z;
                do {
                    end_x = diff(gen_c);
                    end_y = diff(gen_c);
                    end_z = diff(gen_c);
                } while (end_x * end_x + end_y * end_y + end_z * end_z > 1.0f);

                caustic_centre.x += end_x;
                caustic_centre.y += end_y;
                caustic_centre.z += end_z;

                Vector ray_dir = Vector(
                        caustic_centre.x - lights[light_index]->position.x,
                        caustic_centre.y - lights[light_index]->position.y,
                        caustic_centre.z - lights[light_index]->position.z
                );
//            std::cout << "x:" << ray_dir.x << " y:" << ray_dir.y << " z:" << ray_dir.z << std::endl;
//            ray_dir.negate();
                ray_dir.normalise();

                Hit *hit = new Hit();

                Ray *ray = new Ray();
                ray->position = lights[light_index]->position;
                ray->direction = ray_dir;

                Photon *photon = new Photon();
                photon->intensity = 1.0f / photon_map_caustic_size;
                photon->incident = ray->direction;
                photon->colours[0] = 255;
                photon->colours[1] = 255;
                photon->colours[2] = 255;

                // finds a hit
                find_closest_hit(ray, *hit);
                if (not hit->flag) continue;
                if (not hit->what->refr) continue;

                bool store = false;

                // refract once inside the object
                Ray *refr_ray = get_refracted_ray(ray, hit);
                Hit refr_hit = Hit();
                find_closest_hit(refr_ray, refr_hit);
                if (not refr_hit.flag) continue;

                // refract again to leave object
                refr_ray = get_refracted_ray(refr_ray, &refr_hit);
                find_closest_hit(refr_ray, refr_hit);

                if (refr_hit.flag) {
                    store = true;
                    photon->position = refr_hit.position;
                }

                if (store) {
                    photon_map_caustic_array[photon_map_caustic_size - caustic_photon_max] = photon;
                    caustic_photon_max--;
                    // while loop checks every 25, if it misses 0 it will continue below 0
                    if (caustic_photon_max == 0) break;
                }
//            free(hit); free(ray);
            }
        }

        std::cout << "Creating Caustic KD Tree..." << std::endl;

        double photon_map_buffer_caustic[photon_map_caustic_size * 3];
        int photon_index_caustic = 0;
        while (photon_index_caustic < photon_map_caustic_size) {
            photon_map_buffer_caustic[photon_index_caustic *
                                      3] = photon_map_caustic_array[photon_index_caustic]->position.x;
            photon_map_buffer_caustic[photon_index_caustic * 3 +
                                      1] = photon_map_caustic_array[photon_index_caustic]->position.y;
            photon_map_buffer_caustic[photon_index_caustic * 3 +
                                      2] = photon_map_caustic_array[photon_index_caustic]->position.z;
            photon_index_caustic++;
        }
        alglib::integer_1d_array tag_array_caustic;
        tag_array_caustic.setlength(photon_map_caustic_size);
        for (int ti_c = 0; ti_c < photon_map_caustic_size; ti_c++) {
            tag_array_caustic[ti_c] = ti_c;
        }

        alglib::real_2d_array photon_map_alglib_array_caustic;
        photon_map_alglib_array_caustic.setcontent(photon_map_caustic_size, 3, photon_map_buffer_caustic);
        alglib::kdtreebuildtagged(photon_map_alglib_array_caustic, tag_array_caustic, photon_map_caustic_size, 3, 0, 2,
                                  photon_map_caustic);
    }
}

/**
 * Legacy function, used to render an image using only conventional ray tracing
 * @param width : of the image
 * @param height : of the image
 * @param c : camera
 */
void generate_rayTrace_image(int width, int height, Camera *c){
    // loop through the whole image, calculate rays, and figure out intersections and lighting
    std::cout << "Tracing rays..." << std::endl;
    #pragma omp parallel for collapse(2) // parallelize the nested loop
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){

            float x_v = sf * (float)(x - (width/2));
            float y_v = sf * (float)(y - (width/2));

            float px = x_v * c->u->x + y_v * c->v->x - c->w->x;
            float py = x_v * c->u->y + y_v * c->v->y - c->w->y;
            float pz = x_v * c->u->z + y_v * c->v->z - c->w->z;

            Vector *ray_vector = new Vector();
            ray_vector->x = px; ray_vector->y = py; ray_vector->z = pz;

            Ray *ray = new Ray();
            ray->position = c->eye;
            ray->direction = *ray_vector;
            free(ray_vector);

            ray->direction.normalise();

            Hit hit = Hit();
            // depth defined as extra rays, at least one ray is always cast
            float *intensity = cast_ray(ray, 3, hit);
            free(ray);

            fb->plotPixel(x, y, intensity[0], intensity[1], intensity[2]);

        }
    }
}

/**
 * Finds the nearest object that is hit by the ray
 * @param ray : ray to cast into the scene
 * @param closest_hit : hit to store the intersection in
 */
void find_closest_hit(Ray *ray, Hit &closest_hit){
    for (int object_no = 0; object_no < object_array_length; object_no++) {
        Hit *hit = new Hit();
        hit->t = MAXFLOAT;
        objects[object_no]->intersection(*ray, *hit);
        hit->what = objects[object_no];

        if (hit->flag && hit->t < closest_hit.t && hit->t > 0.0f) {
            closest_hit = *hit;
        }
        free(hit);
    }
}

/**
 * Computes Phong lighting of a hit
 * @param hit : the hit in the scene
 * @param incident_dir : vector of incoming ray
 * @param light_dir : direction of the incoming light, not necessarily direction towards actual light object
 * @param shadow_coeff : the decrease in luminosity of the image if the hit is in shadow
 * @return : pointer to an array of colours
 */
float* calculate_colours(Hit *hit, Vector incident_dir, Vector light_dir, float shadow_coeff){

    float *intensity = new float[3];

    // ambient component
    float ambient = (1 - shadow_coeff) * hit->what->I_a * hit->what->k_a;

    // diffuse component
    float diffuse_dot = hit->normal.dot(light_dir);
    float diffuse;
    (diffuse_dot < 0) ? diffuse = 0 : diffuse = (1 - shadow_coeff) * hit->what->I_d * hit->what->k_d * diffuse_dot;

    //specular component approximation, as defined by Phong
    Vector light_reflection = Vector();
    hit->normal.reflection(light_dir, light_reflection);
    light_reflection.normalise();
    float specular_dot = incident_dir.dot(light_reflection);
    float specular;
    (specular_dot < 0) ? specular = 0 : specular = (1 - shadow_coeff) * hit->what->I_s * hit->what->k_s * pow(specular_dot, 30);

    for (int i = 0; i < 3; i++)    intensity[i] = hit->what->colours[i] * (ambient + diffuse + specular);

    return intensity;

}

/**
 * Legacy function, used to cast a ray into the scene using conventional ray tracing
 * @param ray : ray to be cast
 * @param curr_depth : current depth of the image
 * @param hit : hit to store an intersection in
 * @return : pointer to an array of colours
 */
float* cast_ray(Ray *ray, int curr_depth, Hit &hit){
    hit.t = MAXFLOAT;

    float *curr_intensity = new float[3];
    for (int i = 0; i < 3; i++) curr_intensity[i] = 0;

    find_closest_hit(ray, hit);

    // https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
    if(hit.flag){

        for (int i = 0; i < lights_array_length; i++){
            Vector light_dir = Vector();
            lights[i]->compute_and_set_dir(hit.position, light_dir);

            // find if the point is in shadow
            Ray shadow_ray = Ray(hit.position, light_dir);
            shadow_ray.direction.normalise();
            shadow_ray.add_bias(0.0001f, hit.normal);

            float *intensity;

            Hit shadow_hit = Hit();
            shadow_hit.t = MAXFLOAT;
            float dist_to_light = hit.distance_from(lights[i]->position);
            find_closest_hit(&shadow_ray, shadow_hit);
            if (shadow_hit.flag && dist_to_light > shadow_hit.t) {
                intensity = calculate_colours(&hit, ray->direction, light_dir, 0.8f);
            } else {
                intensity = calculate_colours(&hit, ray->direction, light_dir);
            }

            curr_intensity[0] += light_ratio * intensity[0];
            curr_intensity[1] += light_ratio * intensity[1];
            curr_intensity[2] += light_ratio * intensity[2];

        }

        // refraction
        // implementation of Snell's law
        // based on https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
        if (hit.what->refr && curr_depth != 0) {

            Ray *refr = get_refracted_ray(ray, &hit);

            Hit refr_hit = Hit();
            float *refr_intensity = cast_ray(refr, curr_depth - 1, refr_hit);
            if (refr_hit.flag && refr_hit.t > 0){
                for (int i = 0; i < 3; i++) curr_intensity[i] = refr_intensity[i];
            }

        } else if (hit.what->refl_ind != 0.0f && curr_depth != 0) {

            Ray *refl = get_reflected_ray(ray, &hit);

            for (int i = 0; i < 3; i++) curr_intensity[i] = 0;

            Hit refl_hit = Hit();
            float *refl_intensity = cast_ray(refl, curr_depth - 1, refl_hit);
            if (refl_hit.flag && refl_hit.t > 0) {
                for (int i = 0; i < 3; i++) curr_intensity[i] = hit.what->refl_ind * refl_intensity[i];
            }

        }
    }

    return curr_intensity;

}

/**
 * As the above function, but includes the contribution of the global and caustic photon map in colour calculation
 * @param ray
 * @param curr_depth
 * @param hit
 * @return
 */
float* cast_ray_PM(Ray *ray, int curr_depth, Hit &hit){

    float *curr_intensity = new float[3];
    for (int i = 0; i < 3; i++) curr_intensity[i] = 0;

    find_closest_hit(ray, hit);

    if(hit.flag) {

        // compute direct component
        if(RT) {
            float exposure = 0.8f;
            for (int i = 0; i < lights_array_length; i++) {
                Vector light_dir = Vector();
                lights[i]->compute_and_set_dir(hit.position, light_dir);

                // find if the point is in shadow
                Ray shadow_ray = Ray(hit.position, light_dir);
                shadow_ray.direction.normalise();
                shadow_ray.add_bias(0.0001f, hit.normal);

                float *direct_intensity = new float[3];

                Hit shadow_hit = Hit();
                shadow_hit.t = MAXFLOAT;
                float dist_to_light = hit.distance_from(lights[i]->position);
                find_closest_hit(&shadow_ray, shadow_hit);
                if (shadow_hit.flag && dist_to_light > shadow_hit.t) {
                    direct_intensity[0] = 0.0f;
                    direct_intensity[1] = 0.0f;
                    direct_intensity[2] = 0.0f;
                } else {
                    direct_intensity = calculate_colours(&hit, ray->direction, light_dir);
                }

                curr_intensity[0] += light_ratio * direct_intensity[0] * exposure;
                curr_intensity[1] += light_ratio * direct_intensity[1] * exposure;
                curr_intensity[2] += light_ratio * direct_intensity[2] * exposure;
            }
        }

        // handle reflection and refraction

        // refraction
        // implementation of Snell's law
        // based on https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
        if (hit.what->refr && curr_depth != 0) {

            Ray *refr = get_refracted_ray(ray, &hit);

            Hit refr_hit = Hit();
            float *refr_intensity = cast_ray_PM(refr, curr_depth - 1, refr_hit);
            if (refr_hit.flag && refr_hit.t > 0){
                for (int i = 0; i < 3; i++) curr_intensity[i] += refr_intensity[i];
            }

        } else if (hit.what->refl_ind != 0.0f && curr_depth != 0) {

            Ray *refl = get_reflected_ray(ray, &hit);

            for (int i = 0; i < 3; i++) curr_intensity[i] = 0;

            Hit refl_hit = Hit();
            float *refl_intensity = cast_ray_PM(refl, curr_depth - 1, refl_hit);
            if (refl_hit.flag && refl_hit.t > 0) {
                for (int i = 0; i < 3; i++) curr_intensity[i] += hit.what->refl_ind * refl_intensity[i];
            }

        }

        if(GPM) {
            // fetch global photon map information
            alglib::kdtreerequestbuffer req_buf;
            alglib::kdtreecreaterequestbuffer(photon_map_global, req_buf);

            double point_buffer[3] = {hit.position.x, hit.position.y, hit.position.z};
            alglib::real_1d_array point_array;
            point_array.setcontent(3, point_buffer);

            float radius = 1.25f;
            alglib::kdtreetsqueryrnn(photon_map_global, req_buf, point_array, radius);

            alglib::integer_1d_array tags;
            alglib::kdtreetsqueryresultstags(photon_map_global, req_buf, tags);
            alglib::real_1d_array dist;
            alglib::kdtreetsqueryresultsdistances(photon_map_global, req_buf, dist);

            // compute global photon map contribution to the scene at that point
            Photon *curr_photon;
            float curr_dist;
            float spatial_scale = 1.0f / (M_PI * radius * radius);
            float exposure = 2.0f;
            for (int i = 0; i < tags.length(); i++) {
                curr_photon = photon_map_global_array[tags[i]];

                // if normal of photon incident and hit normal not within boundary discard it
                Vector hit_to_photon = Vector(
                        curr_photon->position.x - hit.position.x,
                        curr_photon->position.y - hit.position.y,
                        curr_photon->position.z - hit.position.z
                );
                float dot = hit.normal.dot(hit_to_photon);
                if (dot < -0.000001f || dot > 0.000001f) {
                    continue;
                }

                curr_dist = dist[i];
                float blur = get_gaussian_filter(curr_dist, radius);

                // specular component of BRDF approximation, as defined by Phong
                Vector light_reflection = Vector();
                hit.normal.reflection(curr_photon->incident, light_reflection);
                light_reflection.normalise();
                float specular_dot = ray->direction.dot(light_reflection);
                float specular;
                (specular_dot < 0) ? specular = 0 : specular = hit.what->I_s * hit.what->k_s * pow(specular_dot, 30);

                // diffuse component of BRDF
                curr_photon->incident.negate();
                float diffuse_dot = ray->direction.dot(curr_photon->incident);
                float diffuse;
                (diffuse_dot < 0) ? diffuse = 0 : diffuse = hit.what->I_d * hit.what->k_d * diffuse_dot;

                for (int j = 0; j < 3; j++) {
                    curr_intensity[j] += curr_photon->colours[j] * specular * blur * spatial_scale * exposure;
                    curr_intensity[j] += curr_photon->colours[j] * diffuse * blur * spatial_scale * exposure;
                }
            }
        }

        if(CPM) {
            // fetch caustic photon map information
            alglib::kdtreerequestbuffer req_buf_c;
            alglib::kdtreecreaterequestbuffer(photon_map_caustic, req_buf_c);

            double point_buffer_c[3] = {hit.position.x, hit.position.y, hit.position.z};
            alglib::real_1d_array point_array_c;
            point_array_c.setcontent(3, point_buffer_c);

            float radius_c = 1.0f;
            alglib::kdtreetsqueryrnn(photon_map_caustic, req_buf_c, point_array_c, radius_c);

            alglib::integer_1d_array tags_c;
            alglib::kdtreetsqueryresultstags(photon_map_caustic, req_buf_c, tags_c);
            alglib::real_1d_array dist_c;
            alglib::kdtreetsqueryresultsdistances(photon_map_caustic, req_buf_c, dist_c);

            // compute caustic photon map contribution to the scene at that point
            Photon *curr_photon_c;
            float curr_dist_c;
            float spatial_scale_c = 1.0f / (M_PI * radius_c * radius_c);
            float amount_scale_c = 1.0f;
            for (int i = 0; i < tags_c.length(); i++) {
                curr_photon_c = photon_map_caustic_array[tags_c[i]];

                // if normal of photon incident and hit normal != 0 discard it
                Vector hit_to_photon = Vector(
                        curr_photon_c->position.x - hit.position.x,
                        curr_photon_c->position.y - hit.position.y,
                        curr_photon_c->position.z - hit.position.z
                );
                float dot = hit.normal.dot(hit_to_photon);
                if(dot != 0.0f){
                    continue;
                }

                curr_dist_c = dist_c[i];
                float blur = get_gaussian_filter(curr_dist_c, radius_c);

                // specular component of BRDF approximation, as defined by Phong
                Vector light_reflection = Vector();
                hit.normal.reflection(curr_photon_c->incident, light_reflection);
                light_reflection.normalise();
                float specular_dot = ray->direction.dot(light_reflection);
                float specular;
                (specular_dot < 0) ? specular = 0 : specular = hit.what->I_s * hit.what->k_s * pow(specular_dot, 30);

                // diffuse component of BRDF
                curr_photon_c->incident.negate();
                float diffuse_dot = ray->direction.dot(curr_photon_c->incident);
                float diffuse;
                (diffuse_dot < 0) ? diffuse = 0 : diffuse = hit.what->I_d * hit.what->k_d * diffuse_dot;

                for (int j = 0; j < 3; j++) {
                    curr_intensity[j] += curr_photon_c->colours[j] * specular * blur * spatial_scale_c * amount_scale_c * CPM_exposure;
                    curr_intensity[j] += curr_photon_c->colours[j] * diffuse * blur * spatial_scale_c * amount_scale_c * CPM_exposure;
                }
            }
        }
    }
    return curr_intensity;
}

/**
 * Computes the refracted ray
 * @param incident_ray
 * @param hit : where the incident ray hit
 * @return : the refracted ray
 */
Ray* get_refracted_ray(Ray *incident_ray, Hit *hit){
    float n;
    float c1 = hit->normal.dot(incident_ray->direction);
    // account for incident_ray entering/exiting the object
    if(c1 < 0){
        n = 1.0f / hit->what->refr_ind;
        c1 = -c1;
    } else {
        hit->normal.negate();
        n = hit->what->refr_ind / 1.0f;
    }

    float c2_term = 1.0f - ((n * n) * (1.0f - c1 * c1));

    // test for total internal reflection
    Ray *refr = new Ray();
    refr->position = hit->position;

    if (c2_term < 0){
        hit->normal.reflection(incident_ray->direction, refr->direction);
    } else {
        float c2 = sqrt(c2_term);

        Vector refr_dir = Vector();
        refr_dir.x = n * incident_ray->direction.x + (n * c1 - c2) * hit->normal.x;
        refr_dir.y = n * incident_ray->direction.y + (n * c1 - c2) * hit->normal.y;
        refr_dir.z = n * incident_ray->direction.z + (n * c1 - c2) * hit->normal.z;

        refr->direction = refr_dir;
    }
    refr->direction.normalise();
    // add bias to avoid noise
    refr->add_bias(0.0001f, refr->direction);

    return refr;
}

/**
 * Computes the reflected ray
 * @param ray
 * @param hit : where the incident ray hit
 * @return : the refracted ray
 */
Ray* get_reflected_ray(Ray *ray, Hit *hit){
    Ray *refl = new Ray();
    refl->position = hit->position;
    hit->normal.reflection(ray->direction, refl->direction);
    refl->direction.normalise();
    // add bias to avoid noise
    refl->add_bias(0.0001f, hit->normal);

    return refl;
}

/**
 * Gets the gaussian filter value for a photon in a sample
 * @param dist : distance of photon from the sample centre
 * @param max_dist : the radius of the sample sphere
 * @return : the Gaussian filter value
 */
float get_gaussian_filter(float dist, float max_dist){
    float denom = 1 - exp(-1.953f);
    float power = -1.953f * (pow(dist, 2) / 2 * pow(max_dist, 2));
    float num = 1 - exp(power);

    return 0.918f * (1 - num/denom);
}
