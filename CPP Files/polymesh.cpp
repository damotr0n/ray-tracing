#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../Header Files/polymesh.h"

using namespace std;

PolyMesh::PolyMesh(char *file)
{
    Transform *transform = new Transform();

    float ambient_ref = 1.0f;
    float diffuse_ref = 0.0f;
    float specular_ref = 0.0f;
    float colours[3] = {255, 255, 255};
    float I_a = 1.0f;
    float I_d = 0.0f;
    float I_s = 0.0f;
    float refr_ind = 0.0f;
    float refl_ind = 0.0f;
    this->do_construct(file, transform, ambient_ref, diffuse_ref, specular_ref, colours, I_a, I_d, I_s, refl_ind, refr_ind, false);
}


PolyMesh::PolyMesh(char *file, Transform *transform,
        float ambient_ref, float diffuse_ref, float specular_ref,
        float colours[3],
        float int_a, float int_d, float int_s,
        float refl_ind, float refr_ind, bool refr)
{
    this->do_construct(file, transform, ambient_ref, diffuse_ref, specular_ref, colours, int_a, int_d, int_s, refl_ind, refr_ind, refr);
}

void PolyMesh::do_construct(
        char *file,
        Transform *transform,
        float ambient_ref, float diffuse_ref, float specular_ref,
        float colours[3],
        float int_a, float int_d, float int_s,
        float refl_ind, float refr_ind, bool refr)
{
    ifstream mesh (file);
    this->k_a = ambient_ref;
    this->k_d = diffuse_ref;
    this->k_s = specular_ref;
    this->colours[0] = colours[0];
    this->colours[1] = colours[1];
    this->colours[2] = colours[2];
    this->I_a = int_a;
    this->I_d = int_d;
    this->I_s = int_s;
    this->refl_ind = refl_ind;
    this->refr_ind = refr_ind;
    this->refr = refr;

    this->vertex_count = 0;
    this->triangle_count = 0;

    if (mesh.is_open())
    {
        string meshLine;

        while ( getline(mesh, meshLine) )
        {
            // finds the header elements in the file
            if (meshLine.find("element") != string::npos)
            {
                if (meshLine.find("vertex") != string::npos)
                {
                    vertex_count = std::stoi (meshLine.substr( meshLine.find_first_of("0123456789")));
                }
                else if (meshLine.find("face") != string::npos)
                {
                    triangle_count = std::stoi (meshLine.substr( meshLine.find_first_of("0123456789")));
                }
            }
            else if (vertex_count != 0 && triangle_count != 0)
            {

                int delim_pos, sec_delim_pos;

                // process vertices and put them in an array
                vertex = new Vertex[vertex_count];
                for (int line_number = 0; line_number < vertex_count ; line_number++) {


                    delim_pos = meshLine.find(" ");
                    sec_delim_pos = meshLine.rfind(" ");

                    try {
                        vertex[line_number].x = std::stof(meshLine.substr(0, delim_pos));
                        vertex[line_number].y = std::stof(meshLine.substr(delim_pos + 1, sec_delim_pos - delim_pos));
                        vertex[line_number].z = std::stof(meshLine.substr(sec_delim_pos + 1, string::npos));
                    } catch (std::invalid_argument)
                    {
                        cout << "No conversion" << endl;
                    }

                    getline(mesh, meshLine);

                    // applies transformation to vertex
                    transform->apply(vertex[line_number]);
                }

                // process triangles
                triangles = new Triangle[triangle_count]();
                Vertex *vertices = new Vertex[3];
                for (int line_number = 0; line_number < triangle_count; line_number++)
                {
                    string line = meshLine.substr(meshLine.find(" ") + 1, string::npos);

                    delim_pos = line.find(" ");
                    sec_delim_pos = line.rfind(" ");

                    int v1 = std::stoi(line.substr(0, delim_pos));
                    int v2 = std::stoi(line.substr(delim_pos + 1, sec_delim_pos - delim_pos));
                    int v3 = std::stoi(line.substr(sec_delim_pos + 1, string::npos));

                    vertex[v1].faces.push_back(line_number);
                    vertex[v2].faces.push_back(line_number);
                    vertex[v3].faces.push_back(line_number);

                    vertices[0] = vertex[v1];
                    vertices[1] = vertex[v2];
                    vertices[2] = vertex[v3];

                    triangles[line_number].initialize(vertex[v1], vertex[v2], vertex[v3]);
                    triangles[line_number].colours[0] = this->colours[0];
                    triangles[line_number].colours[1] = this->colours[1];
                    triangles[line_number].colours[2] = this->colours[2];
                    triangles[line_number].k_a = this->k_a;
                    triangles[line_number].k_d = this->k_d;
                    triangles[line_number].k_s = this->k_s;
                    triangles[line_number].refl_ind = this->refl_ind;
                    triangles[line_number].refr_ind = this->refr_ind;
                    triangles[line_number].refr = this->refr;

                    getline(mesh, meshLine);
                }

                this->compute_vertex_normals();

                for (int i = 0; i < triangle_count; i++)
                {
                    triangles[i].normal.normalise();
                }

            }

        }

        mesh.close();
    }
    else
    {
        cout << "File could not be opened, terminating!" << endl;
        exit(10);
    }
}

void PolyMesh::intersection(Ray ray, Hit &hit)
{
    Hit *intermediate_hit = new Hit();
    for (int tri = 0; tri < this->triangle_count; tri++) {
        this->triangles[tri].intersection(ray, *intermediate_hit);
        if (intermediate_hit->flag && intermediate_hit->t < hit.t) {
            hit = *intermediate_hit;
        }
    }
    free(intermediate_hit);
}

void PolyMesh::compute_vertex_normals(){

    for(int x = 0; x < this->vertex_count; x++){

        // adds all the normals of the faces adjacent to the vertex
        vector<int>::iterator it;
        int div = 0;
        for(it = this->vertex[x].faces.begin(); it != this->vertex[x].faces.end(); it++){
            this->vertex[x].n.x += this->triangles[*it].normal.x;
            this->vertex[x].n.y += this->triangles[*it].normal.y;
            this->vertex[x].n.z += this->triangles[*it].normal.z;
            div++;
        }

        // and divides them, finding the average
        this->vertex[x].n.x /= (float) div;
        this->vertex[x].n.y /= (float) div;
        this->vertex[x].n.z /= (float) div;

        this->vertex[x].n.normalise();

    }

}
