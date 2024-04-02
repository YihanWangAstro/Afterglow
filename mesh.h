#ifndef _MESHES_
#define _MESHES_

#include <functional>
#include <vector>

#include "mesh.h"

using MeshGrid = std::vector<std::vector<double>>;
using MeshGrid3d = std::vector<std::vector<std::vector<double>>>;

using Array = std::vector<double>;

using Profile = std::function<double(double)>;
using Profile2d = std::function<double(double, double)>;

class Coord {
   public:
    Coord(Array r_b, Array theta_b, Array phi_b) : r_b(r_b), theta_b(theta_b), phi_b(phi_b) {
        r = boundary2centerlog(r_b);
        theta = boundary2center(theta_b);
        phi = boundary2center(phi_b);
    }
    Coord() = delete;
    Array r_b;
    Array theta_b;
    Array phi_b;
    Array r;
    Array theta;
    Array phi;
};

Array linspace(double start, double end, size_t num);

Array logspace(double start, double end, size_t num);

Array zeros(size_t num);

Array ones(size_t num);

MeshGrid createGrid(size_t theta_size, size_t r_size, double val = 0);

MeshGrid3d createGrid3d(size_t phi_size, size_t theta_size, size_t r_size, double val = 0);

Array boundary2center(Array const& boundary);

Array boundary2centerlog(Array const& boundary);
#endif