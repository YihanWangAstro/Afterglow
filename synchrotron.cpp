
#include "synchrotron.h"

#include <cmath>

#include "macros.h"
#include "utilities.h"
SynPhotonsMesh create_syn_photons_grid(size_t theta_size, size_t r_size) {
    return SynPhotonsMesh(theta_size, SynPhotonsArray(r_size));
}

SynElectronsMesh create_syn_electrons_grid(size_t theta_size, size_t r_size) {
    return SynElectronsMesh(theta_size, SynElectronsArray(r_size));
}

double SynPhotons::I_nu(double nu) const { return I_nu_peak * I_nu_(nu); }

inline bool order(double a, double b, double c) { return a <= b && b <= c; };

double SynPhotons::I_nu_(double nu) const {
    if (order(nu_a, nu_m, nu_c)) {
        if (nu <= nu_a) {
            return pow(nu_a / nu_m, 1. / 3) * pow(nu / nu_a, 2);
        } else if (nu <= nu_m) {
            return pow(nu / nu_m, 1. / 3);
        } else if (nu <= nu_c) {
            return pow(nu / nu_m, -(p - 1) / 2);
        } else if (nu <= nu_M) {
            return pow(nu_c / nu_m, -(p - 1) / 2) * pow(nu / nu_c, -p / 2);
        } else {
            return pow(nu_c / nu_m, -(p - 1) / 2) * pow(nu_M / nu_c, -p / 2) * sqrt(nu / nu_M) * exp(-nu / nu_M);
        }
    } else if (order(nu_m, nu_a, nu_c)) {
        if (nu <= nu_m) {
            return pow(nu_m / nu_a, (p + 4) / 2) * pow(nu / nu_m, 2);
        } else if (nu <= nu_a) {
            return pow(nu_a / nu_m, -(p - 1) / 2) * pow(nu / nu_a, 5. / 2);
        } else if (nu <= nu_c) {
            return pow(nu / nu_m, -(p - 1) / 2);
        } else if (nu <= nu_M) {
            return pow(nu_c / nu_m, -(p - 1) / 2) * pow(nu / nu_c, -p / 2);
        } else {
            return pow(nu_c / nu_m, -(p - 1) / 2) * pow(nu_M / nu_c, -p / 2) * sqrt(nu / nu_M) * exp(-nu / nu_M);
        }
    } else if (order(nu_a, nu_c, nu_m)) {
        if (nu <= nu_a) {
            return pow(nu_a / nu_c, 1. / 3) * pow(nu / nu_a, 2);
        } else if (nu <= nu_c) {
            return pow(nu / nu_c, 1. / 3);
        } else if (nu <= nu_m) {
            return pow(nu / nu_c, -1. / 2);
        } else if (nu <= nu_M) {
            return pow(nu_m / nu_c, -1. / 2) * pow(nu / nu_m, -p / 2);
        } else {
            return pow(nu_m / nu_c, -1. / 2) * pow(nu_M / nu_m, -p / 2) * sqrt(nu / nu_M) * exp(-nu / nu_M);
        }
    } else if (order(nu_c, nu_a, nu_m)) {
        if (nu <= nu_a) {
            return pow(nu / nu_a, 2);
        } else if (nu <= nu_m) {
            return pow(nu / nu_a, -1. / 2) * pow(nu_c / nu_a, 1. / 2) / 3;
        } else if (nu <= nu_M) {
            return pow(nu_m / nu_a, -1. / 2) * pow(nu / nu_m, -p / 2) * pow(nu_c / nu_a, 1. / 2) / 3;
        } else {
            return pow(nu_m / nu_a, -1. / 2) * pow(nu_M / nu_m, -p / 2) * pow(nu_c / nu_a, 1. / 2) / 3 *
                   sqrt(nu / nu_M) * exp(-nu / nu_M);
        }
    } else if (order(nu_m, nu_c, nu_a)) {
        if (nu <= nu_a) {
            return pow(nu / nu_a, 2);
        } else if (nu <= nu_M) {
            return (p - 1) / 3 * pow(nu / nu_a, -p / 2) * pow(nu_m / nu_a, (p - 1) / 2) * pow(nu_c / nu_a, 1. / 2);
        } else {
            return (p - 1) / 3 * pow(nu_M / nu_a, -p / 2) * pow(nu_m / nu_a, (p - 1) / 2) * pow(nu_c / nu_a, 1. / 2) *
                   sqrt(nu / nu_M) * exp(-nu / nu_M);
        }
    } else if (order(nu_c, nu_m, nu_a)) {
        if (nu <= nu_a) {
            return pow(nu / nu_a, 2);
        } else if (nu <= nu_M) {
            return pow(nu / nu_a, -p / 2) * pow(nu_m / nu_a, (p - 1) / 2) * pow(nu_c / nu_a, 1. / 2) / 3;
        } else {
            return pow(nu_M / nu_a, -p / 2) * pow(nu_m / nu_a, (p - 1) / 2) * pow(nu_c / nu_a, 1. / 2) / 3 *
                   sqrt(nu / nu_M) * exp(-nu / nu_M);
        }
    }
}

inline double syn_I_nu_peak(double r, double Gamma, double B, double Delta, double rho, double xi, double pel) {
    return (pel - 1) / 2 * sqrt(3) * con::e3 * B / (con::me * con::c2) * 4 * Gamma * (rho / con::mp) * xi * Delta;
}

double syn_nu(double gamma, double B) {
    double nu = 3 * con::e * B / (4 * con::pi * con::me * con::c) * gamma * gamma;
    return nu;
}
double syn_gamma(double nu, double B) {
    double gamma = sqrt(nu * 4 * con::pi * con::me * con::c / (3 * con::e * B));
    return gamma;
}

double syn_gamma_M(double B, double zeta, double Y_tilt) {
    return sqrt(6 * con::pi * con::e / (con::sigmaT * B * zeta * (1 + Y_tilt)));
}

double syn_gamma_m(double Gamma, double gamma_M, double eps_e, double xi, double pel) {
    double gamma_m = 1;
    if (pel > 2) {
        gamma_m = (pel - 2) / (pel - 1) * eps_e * (Gamma - 1) * 1836 / xi + 1;
    } else if (pel < 2) {
        gamma_m =
            pow((2 - pel) / (pel - 1) * eps_e * (Gamma - 1) * 1836 / xi * pow(gamma_M, pel - 1), 1 / (pel - 1)) + 1;
    } else {
        gamma_m = root_bisection(
            [=](double x) -> double {
                return (x * log(gamma_M) - (x + 1) * log(x) - eps_e * (Gamma - 1) * 1836 / xi - log(gamma_M));
            },
            1, gamma_M);
    }
    return gamma_m;
}

double syn_gamma_c(double Gamma, double t_com, double B, double Y_tilt) {
    double beta = sqrt(Gamma * Gamma - 1) / Gamma;
    double gamma_c = 6 * con::pi * con::me * con::c / (con::sigmaT * t_com * beta * beta * B * B * (1 + Y_tilt));

    // work around to avoid gamma_c < 1
    // double gamma_c = 6 * con::pi * con::me * con::c / (con::sigmaT * t_com * B * B * (1 + Y_tilt)) + 1;
    return gamma_c;
}

double syn_gamma_a(double Gamma, double B, double I_syn_peak, double gamma_peak, double gamma_M) {
    double nu_peak = syn_nu(gamma_peak, B);
    double gamma_eos = (4 * Gamma + 1) / (3 * Gamma);  // adiabatic index
    double kT = (gamma_peak - 1) * con::me * con::c2 * (gamma_eos - 1);
    // 2kT(nv_a/c)^2 = I_peak*(nu_a/nu_peak)^(1/3)
    double nu_a = pow(I_syn_peak * con::c2 / pow(nu_peak, 1. / 3) / kT / 2, 3. / 5);

    // the nu_peak is not the real peak, peak at nu_a; kT = (gamma_a-1) * me *c^2*(gamma_eos-1), I_syn = I_peak;
    if (nu_a > nu_peak) {
        /*nu_a = pow(I_syn_peak / con::me / 2 / (gamma_eos - 1) /
                       sqrt(4 * con::pi / 3 * con::me * con::c / con::e / B),
                   2.0 / 5);*/ //this works only for gamma >> 1
        double nu_M = syn_nu(gamma_M, B);
        double A = sqrt(4 * con::pi / 3 * con::me * con::c / con::e / B);
        double B = I_syn_peak / (2 * con::me * (gamma_eos - 1));
        nu_a = root_bisection([=](double x) -> double { return A * x * x * x * x * x - x * x * x * x - B; },
                              sqrt(nu_peak), sqrt(nu_M));
        nu_a *= nu_a;
    }
    return syn_gamma(nu_a, B);
}

double syn_nu_E_peak(double nu_a, double nu_m, double nu_c) {
    if (order(nu_a, nu_m, nu_c)) {
        return nu_c;
    } else if (order(nu_m, nu_a, nu_c)) {
        return nu_c;
    } else if (order(nu_a, nu_c, nu_m)) {
        return nu_m;
    } else if (order(nu_c, nu_a, nu_m)) {
        if (nu_a * nu_a < nu_m * nu_c) {
            return nu_m;
        } else {
            return nu_a;
        }
    } else if (order(nu_m, nu_c, nu_a)) {
        return nu_a;
    } else if (order(nu_c, nu_m, nu_a)) {
        return nu_a;
    }
}

double syn_nu_E_peak(SynPhotons const& ph) { return syn_nu_E_peak(ph.nu_a, ph.nu_m, ph.nu_c); }

double syn_gamma_N_peak(double gamma_a, double gamma_m, double gamma_c) {
    double gamma_peak = std::min(gamma_m, gamma_c);
    if (gamma_a > gamma_c) {
        return gamma_a;
    } else {
        return gamma_peak;
    }
}

double syn_gamma_N_peak(SynElectrons const& e) { return syn_gamma_N_peak(e.gamma_a, e.gamma_m, e.gamma_c); }

SynElectronsMesh gen_syn_electrons(Coord const& coord, Shock const& shock, Medium const& medium) {
    SynElectronsMesh e = create_syn_electrons_grid(coord.theta.size(), coord.r.size());

    for (size_t j = 0; j < coord.theta.size(); ++j) {
        for (size_t k = 0; k < coord.r.size(); ++k) {
            double r = coord.r[k];
            double rho = medium.rho(r);
            double Gamma = shock.Gamma[j][k];
            double t_com = shock.t_com[j][k];
            double B = shock.B[j][k];
            double D_com = shock.D_com[j][k];

            double I_nu_peak = syn_I_nu_peak(r, Gamma, B, D_com, rho, medium.xi, medium.pel);
            e[j][k].gamma_M = syn_gamma_M(B, medium.zeta, 0);
            e[j][k].gamma_m = syn_gamma_m(Gamma, e[j][k].gamma_M, medium.eps_e, medium.xi, medium.pel);
            e[j][k].gamma_c = syn_gamma_c(Gamma, t_com, B, 0);
            double gamma_peak = std::min(e[j][k].gamma_m, e[j][k].gamma_c);  // peak gamma before self-absorption
            e[j][k].gamma_a = syn_gamma_a(Gamma, B, I_nu_peak, gamma_peak, e[j][k].gamma_M);
            e[j][k].p = medium.pel;
        }
    }
    return e;
}

SynPhotonsMesh gen_syn_photons(Coord const& coord, SynElectronsMesh const& e, Shock const& shock,
                               Medium const& medium) {
    SynPhotonsMesh ph = create_syn_photons_grid(coord.theta.size(), coord.r.size());

    for (size_t j = 0; j < coord.theta.size(); ++j) {
        for (size_t k = 0; k < coord.r.size(); ++k) {
            double r = coord.r[k];
            double rho = medium.rho(r);
            double Gamma = shock.Gamma[j][k];
            double B = shock.B[j][k];
            double D_com = shock.D_com[j][k];

            ph[j][k].I_nu_peak = syn_I_nu_peak(r, Gamma, B, D_com, rho, medium.xi, e[j][k].p);
            ph[j][k].nu_M = syn_nu(e[j][k].gamma_M, B);
            ph[j][k].nu_m = syn_nu(e[j][k].gamma_m, B);
            ph[j][k].nu_c = syn_nu(e[j][k].gamma_c, B);
            ph[j][k].nu_a = syn_nu(e[j][k].gamma_a, B);
            ph[j][k].nu_E_peak = syn_nu_E_peak(ph[j][k]);
            ph[j][k].p = e[j][k].p;
        }
    }
    return ph;
}