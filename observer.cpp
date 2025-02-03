//              __     __                            _      __  _                     _
//              \ \   / /___   __ _   __ _  ___     / \    / _|| |_  ___  _ __  __ _ | |  ___ __      __
//               \ \ / // _ \ / _` | / _` |/ __|   / _ \  | |_ | __|/ _ \| '__|/ _` || | / _ \\ \ /\ / /
//                \ V /|  __/| (_| || (_| |\__ \  / ___ \ |  _|| |_|  __/| |  | (_| || || (_) |\ V  V /
//                 \_/  \___| \__, | \__,_||___/ /_/   \_\|_|   \__|\___||_|   \__, ||_| \___/  \_/\_/
//                            |___/                                            |___/

#include "observer.h"

#include <boost/numeric/odeint.hpp>
#include <cmath>

#include "macros.h"
#include "physics.h"
#include "utilities.h"

double LogScaleInterp::interpRadius(double log_t) const {
    return fastExp(log_r_lo + (log_r_hi - log_r_lo) * (log_t - log_t_lo) / (log_t_hi - log_t_lo));
}

double LogScaleInterp::interpIntensity(double log_t) const {
    return fastExp(log_I_lo + (log_I_hi - log_I_lo) * (log_t - log_t_lo) / (log_t_hi - log_t_lo));
}

double LogScaleInterp::interpDoppler(double log_t) const {
    return fastExp(log_d_lo + (log_d_hi - log_d_lo) * (log_t - log_t_lo) / (log_t_hi - log_t_lo));
}

void Observer::calcSolidAngle() {
    for (size_t i = 0; i < eff_phi_size; ++i) {
        for (size_t j = 0; j < coord.theta.size(); ++j) {
            dOmega[i][j] = coord.dcos[j] * (eff_phi_size == 1 ? 2 * con::pi : coord.dphi[i]);
        }
    }
}

Observer::Observer(Coord const& coord)
    : coord(coord),
      doppler(create3DGrid(coord.phi.size(), coord.theta.size(), coord.r.size())),
      t_obs_grid(create3DGrid(coord.phi.size(), coord.theta.size(), coord.r.size())),
      dOmega(createGrid(coord.phi.size(), coord.theta.size())),
      log_r(zeros(coord.r.size())) {
    for (size_t i = 0; i < coord.r.size(); ++i) {
        log_r[i] = fastLog(coord.r[i]);
    }

    calcSolidAngle();
}

void Observer::calcObsTimeGrid(MeshGrid3d const& Gamma, MeshGrid3d const& t_eng) {
    double cos_obs = std::cos(theta_obs);
    double sin_obs = std::sin(theta_obs);
    for (size_t i = 0; i < eff_phi_size; ++i) {
        double cos_phi = std::cos(coord.phi[i]);
        for (size_t j = 0; j < coord.theta.size(); ++j) {
            double cos_v = std::sin(coord.theta[j]) * cos_phi * sin_obs + std::cos(coord.theta[j]) * cos_obs;

            for (size_t k = 0; k < coord.r.size(); ++k) {
                double gamma_ = Gamma[i * interp.jet_3d][j][k];
                double t_eng_ = t_eng[i * interp.jet_3d][j][k];
                double beta = gammaTobeta(gamma_);
                doppler[i][j][k] = 1 / (gamma_ * (1 - beta * cos_v));
                if (gamma_ == 1) {
                    t_obs_grid[i][j][k] = std::numeric_limits<double>::infinity();
                } else {
                    t_obs_grid[i][j][k] = (t_eng_ + (1 - cos_v) * coord.r[k] / con::c) * (1 + z);
                }
            }
        }
    }
}
