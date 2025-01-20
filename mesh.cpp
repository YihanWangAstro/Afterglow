#include "mesh.h"

#include <cmath>

#include "macros.h"
Array linspace(double start, double end, size_t num) {
    Array result(boost::extents[num]);
    double step = (end - start) / (num - 1);
    for (size_t i = 0; i < num; i++) {
        result[i] = start + i * step;
    }
    return result;
}

Array logspace(double start, double end, size_t num) {
    Array result(boost::extents[num]);
    double log_start = std::log(start);
    double log_end = std::log(end);
    double step = (log_end - log_start) / (num - 1);
    for (size_t i = 0; i < num; i++) {
        result[i] = std::exp(log_start + i * step);
    }
    return result;
}

Array zeros(size_t num) { return Array(boost::extents[num]); }

Array ones(size_t num) {
    Array array(boost::extents[num]);
    std::fill(array.data(), array.data() + array.num_elements(), 1);
    return array;
}

MeshGrid create_grid(size_t theta_size, size_t r_size, double val) {
    MeshGrid grid(boost::extents[theta_size][r_size]);
    std::fill(grid.data(), grid.data() + grid.num_elements(), val);
    return grid;
}

bool is_linear_scale(Array const& arr, double tolerance) {
    if (arr.size() < 2) return false;  // Need at least two elements

    double diff = arr[1] - arr[0];
    for (size_t i = 2; i < arr.size(); ++i) {
        if (std::fabs((arr[i] - arr[i - 1] - diff) / diff) > tolerance) {
            return false;
        }
    }
    return true;
}

bool is_log_scale(Array const& arr, double tolerance) {
    if (arr.size() < 2) return false;  // Need at least two elements

    double ratio = arr[1] / arr[0];
    for (size_t i = 2; i < arr.size(); ++i) {
        if (std::fabs((arr[i] / arr[i - 1] - ratio) / ratio) > tolerance) {
            return false;
        }
    }
    return true;
}


Coord::Coord(Array const& r_b, Array const& theta_b, Array const& phi_b)
    : r_b(r_b),
      theta_b(theta_b),
      phi_b(phi_b),
      r(boundary2centerlog(r_b)),
      theta(boundary2center(theta_b)),
      phi(boundary2center(phi_b)) {}

Coord::Coord(double r_min, double r_max, double theta_max, size_t r_num, size_t theta_num, size_t phi_num)
    : r_b(logspace(r_min, r_max, r_num + 1)),
      theta_b(linspace(0, theta_max, theta_num + 1)),
      phi_b(linspace(0, 2 * con::pi, phi_num + 1)),
      r(boundary2centerlog(r_b)),
      theta(boundary2center(theta_b)),
      phi(boundary2center(phi_b)) {}

double min(MeshGrid const& grid) {
    double min = grid[0][0];
    for (size_t i = 0; i < grid.size(); ++i) {
        for (size_t j = 0; j < grid[i].size(); ++j) {
            if (grid[i][j] < min) {
                min = grid[i][j];
            }
        }
    }
    return min;
}

double max(MeshGrid const& grid) {
    double max = grid[0][0];
    for (size_t i = 0; i < grid.size(); ++i) {
        for (size_t j = 0; j < grid[i].size(); ++j) {
            if (grid[i][j] > max) {
                max = grid[i][j];
            }
        }
    }
    return max;
}

MeshGrid create_grid_like(MeshGrid const& grid_old, double val) {
    const size_t* shape = grid_old.shape();
    MeshGrid grid(boost::extents[shape[0]][shape[1]]);
    std::fill(grid.data(), grid.data() + grid.num_elements(), val);
    return grid;
}

MeshGrid3d create_3d_grid(size_t phi_size, size_t theta_size, size_t r_size, double val) {
    MeshGrid3d grid(boost::extents[phi_size][theta_size][r_size]);
    std::fill(grid.data(), grid.data() + grid.num_elements(), val);
    return grid;
}

MeshGrid3d create_3d_grid_like(MeshGrid3d const& grid_old, double val) {
    const size_t* shape = grid_old.shape();
    MeshGrid3d grid(boost::extents[shape[0]][shape[1]][shape[2]]);
    std::fill(grid.data(), grid.data() + grid.num_elements(), val);
    return grid;
}

Array boundary2center(Array const& boundary) {
    Array center(boost::extents[boundary.size() - 1]);
    for (size_t i = 0; i < center.size(); ++i) {
        center[i] = 0.5 * (boundary[i] + boundary[i + 1]);
    }
    return center;
}

Array boundary2centerlog(Array const& boundary) {
    Array center(boost::extents[boundary.size() - 1]);
    for (size_t i = 0; i < center.size(); ++i) {
        center[i] = std::sqrt(boundary[i] * boundary[i + 1]);
    }
    return center;
}