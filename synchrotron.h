#ifndef _SYNCHROTRON_
#define _SYNCHROTRON_
#include <vector>

#include "medium.h"
#include "mesh.h"
struct SynRad {
    double I_nu_peak{0};
    double nu_m{0};
    double nu_c{0};
    double nu_a{0};
    double nu_M{0};
    double pel{2.3};

    double I_nu(double nu) const;

   private:
    inline double I_nu_(double nu) const;
};
using SynRadArray = std::vector<SynRad>;
using SynRadMesh = std::vector<std::vector<SynRad>>;

SynRadMesh createSynRadGrid(size_t theta_size, size_t r_size, SynRad val = {0, 0, 0, 0, 0, 2.3});

SynRadMesh calc_syn_radiation(Coord const& coord, MeshGrid const& Gamma, MeshGrid const& t_com, Medium const& medium);
#endif