
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

double SynElectrons::N(double gamma) const { return N_tot * gamma_spectrum_(gamma); }

double SynElectrons::n(double gamma) const { return n_tot * gamma_spectrum_(gamma); }

inline bool order(double a, double b, double c) { return a <= b && b <= c; };

size_t get_regime(double a, double c, double m) {
    if (order(a, m, c)) {
        return 1;
    } else if (order(m, a, c)) {
        return 2;
    } else if (order(a, c, m)) {
        return 3;
    } else if (order(c, a, m)) {
        return 4;
    } else if (order(m, c, a)) {
        return 5;
    } else if (order(c, m, a)) {
        return 6;
    }
    return 0;
}

inline double fast_pow(double a, double b) {
    /* union {
         double d;
         int x[2];
     } u = {a};
     u.x[1] = (int)(b * (u.x[1] - 1072632447) + 1072632447);
     u.x[0] = 0;
     return u.d;*/
    return pow(a, b);
}

double SynElectrons::gamma_spectrum_(double gamma) const {
    switch (regime) {
        case 1:
            if (gamma <= gamma_m) {
                return 0;
            } else if (gamma <= gamma_c) {
                return (p - 1) * fast_pow(gamma / gamma_m, -p) / gamma_m;
            } else {
                return (p - 1) * fast_pow(gamma / gamma_m, -p) * gamma_c / (gamma * gamma_m) * exp(-gamma / gamma_M);
            }
            break;
        case 2:
            if (gamma <= gamma_m) {
                return 0;
            } else if (gamma <= gamma_c) {
                return (p - 1) * fast_pow(gamma / gamma_m, -p) / gamma_m;
            } else {
                return (p - 1) * fast_pow(gamma / gamma_m, -p) * gamma_c / (gamma * gamma_m) * exp(-gamma / gamma_M);
            }
            break;
        case 3:
            if (gamma <= gamma_c) {
                return 0;
            } else if (gamma <= gamma_m) {
                return gamma_c / (gamma * gamma);
            } else {
                return gamma_c / (gamma * gamma_m) * fast_pow(gamma / gamma_m, -p) * exp(-gamma / gamma_M);
            }
            break;
        case 4:
            if (gamma <= gamma_a) {
                return 3 * gamma * gamma / (gamma_a * gamma_a * gamma_a);
            } else if (gamma <= gamma_m) {
                return gamma_c / (gamma * gamma);
            } else {
                return gamma_c / (gamma * gamma_m) * fast_pow(gamma / gamma_m, -p) * exp(-gamma / gamma_M);
            }
            break;
        case 5:
            if (gamma <= gamma_a) {
                return 3 * gamma * gamma / (gamma_a * gamma_a * gamma_a);
            } else {
                return (p - 1) * gamma_c / (gamma * gamma_m) * fast_pow(gamma / gamma_m, -p) * exp(-gamma / gamma_M);
            }
            break;
        case 6:
            if (gamma <= gamma_a) {
                return 3 * gamma * gamma / (gamma_a * gamma_a * gamma_a);
            } else {
                return fast_pow(gamma_m, p - 1) * gamma_c * fast_pow(gamma, -(p + 1)) * exp(-gamma / gamma_M);
            }
            break;
        default:
            return 0;
    }
}

double SynPhotons::L_nu(double nu) const { return L_nu_peak * spectrum_(nu); }

double SynPhotons::spectrum_(double nu) const {
    switch (regime) {
        case 1:
            if (nu <= nu_a) {
                return cbrt(nu_a / nu_m) * (nu / nu_a) * (nu / nu_a);
            } else if (nu <= nu_m) {
                return cbrt(nu / nu_m);
            } else if (nu <= nu_c) {
                return fast_pow(nu / nu_m, -(p - 1) / 2);
            } else {
                return sqrt(nu_c / nu_m) * fast_pow(nu / nu_m, -p / 2) * exp(-nu / nu_M);
            }
            break;
        case 2:
            if (nu <= nu_m) {
                return fast_pow(nu_m / nu_a, (p + 4) / 2) * (nu / nu_m) * (nu / nu_m);
            } else if (nu <= nu_a) {
                return fast_pow(nu_a / nu_m, -(p - 1) / 2) * fast_pow(nu / nu_a, 5. / 2);
            } else if (nu <= nu_c) {
                return fast_pow(nu / nu_m, -(p - 1) / 2);
            } else {
                return sqrt(nu_c / nu_m) * fast_pow(nu / nu_m, -p / 2) * exp(-nu / nu_M);
            }
            break;
        case 3:
            if (nu <= nu_a) {
                return cbrt(nu_a / nu_c) * (nu / nu_a) * (nu / nu_a);
            } else if (nu <= nu_c) {
                return cbrt(nu / nu_c);
            } else if (nu <= nu_m) {
                return sqrt(nu_c / nu);
            } else {
                return sqrt(nu_c / nu_m) * fast_pow(nu / nu_m, -p / 2) * exp(-nu / nu_M);
            }
            break;
        case 4:
            if (nu <= nu_a) {
                return (nu / nu_a) * (nu / nu_a);
            } else if (nu <= nu_m) {
                double R = sqrt(nu_c / nu_a) / 3;
                return R * sqrt(nu_a / nu);
            } else {
                double R = sqrt(nu_c / nu_a) / 3;
                return R * sqrt(nu_a / nu_m) * fast_pow(nu / nu_m, -p / 2) * exp(-nu / nu_M);
            }
            break;
        case 5:
            if (nu <= nu_a) {
                return (nu / nu_a) * (nu / nu_a);
            } else {
                double R = (p - 1) / 3 * sqrt(nu_c / nu_a) * fast_pow(nu_m / nu_a, (p - 1) / 2);
                return R * fast_pow(nu / nu_a, -p / 2) * exp(-nu / nu_M);
            }
            break;
        case 6:
            if (nu <= nu_a) {
                return (nu / nu_a) * (nu / nu_a);
            } else {
                double R = sqrt(nu_c / nu_a) * fast_pow(nu_m / nu_a, (p - 1) / 2) / 3;
                return R * fast_pow(nu / nu_a, -p / 2) * exp(-nu / nu_M);
            }
            break;

        default:
            break;
    }
}

// single electron power in the co-moving frame
double syn_p_nu_peak(double B, double p) { return (p - 1) / 2 * sqrt(3) * con::e3 * B / (con::me * con::c2); }

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

double syn_gamma_m(double Gamma, double gamma_M, double eps_e, double xi, double p) {
    double gamma_bar = 1 + eps_e * (Gamma - 1) * 1836 / xi;
    double gamma_m = 1;

    if (p > 2) {
        gamma_m = (p - 2) / (p - 1) * gamma_bar;
    } else if (p < 2) {
        // need to check in non-relativistic limit
        gamma_m = pow((2 - p) / (p - 1) * gamma_bar * pow(gamma_M, p - 1), 1 / (p - 1));
    } else {
        gamma_m = root_bisection(
            [=](double x) -> double { return (x * log(gamma_M) - (x + 1) * log(x) - gamma_bar - log(gamma_M)); }, 1,
            gamma_M);
    }

    if (gamma_m < 1) {
        gamma_m = 1;
    }
    return gamma_m;
}

double syn_gamma_c(double t_com, double B, double Y_tilt) {
    // t_com = (6*pi*gamma*me*c^2) /(gamma^2*beta^2*sigma_T*c*B^2*(1 + Y_tilt))
    double gamma_bar = 6 * con::pi * con::me * con::c / (con::sigmaT * B * B * (1 + Y_tilt) * t_com);
    double gamma_c = (gamma_bar + sqrt(gamma_bar * gamma_bar + 4)) / 2;

    return gamma_c;
}

double syn_gamma_a(double Gamma, double B, double I_syn_peak, double gamma_m, double gamma_c, double gamma_M) {
    double gamma_peak = std::min(gamma_m, gamma_c);
    double nu_peak = syn_nu(gamma_peak, B);
    double gamma_eos = (4 * Gamma + 1) / (3 * Gamma);  // adiabatic index

    double kT = (gamma_peak - 1) * con::me * con::c2 * (gamma_eos - 1);
    // 2kT(nv_a/c)^2 = I_peak*(nu_a/nu_peak)^(1/3)
    double nu_a = pow(I_syn_peak * con::c2 / cbrt(nu_peak) / kT / 2, 3. / 5);

    // the nu_peak is not the real peak, peak at nu_a; kT = (gamma_a-1) * me *c^2*(gamma_eos-1), I_syn = I_peak;
    if (fabs(gamma_peak - 1) < 1e-6 || nu_a > nu_peak) {
        /*nu_a = pow(I_syn_peak / con::me / 2 / (gamma_eos - 1) / sqrt(4 * con::pi / 3 * con::me * con::c / con::e /
         * B),2.0 / 5);*/ //this works only for gamma >> 1
        double nu_M = syn_nu(gamma_M, B);
        double A = sqrt(4 * con::pi / 3 * con::me * con::c / con::e / B);
        double B = I_syn_peak / (2 * con::me * (gamma_eos - 1));
        nu_a = root_bisection([=](double x) -> double { return A * x * x * x * x * x - x * x * x * x - B; },
                              sqrt(nu_peak), sqrt(nu_M));
        nu_a *= nu_a;
    }
    double gamma_a = syn_gamma(nu_a, B);
    if (gamma_a < 1) {
        gamma_a = 1;
    }
    return gamma_a;
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

SynElectronsMesh gen_syn_electrons(double p, Coord const& coord, Shock const& shock, MeshGrid const& Y_tilt) {
    SynElectronsMesh e = create_syn_electrons_grid(coord.theta.size(), coord.r.size());

    for (size_t j = 0; j < coord.theta.size(); ++j) {
        for (size_t k = 0; k < coord.r.size(); ++k) {
            double r = coord.r[k];
            double dcos = std::fabs(cos(coord.theta_b[j + 1]) - cos(coord.theta_b[j]));
            double dS = 2 * con::pi * r * r * dcos;
            double Gamma = shock.Gamma[j][k];
            double t_com = shock.t_com[j][k];
            double B = shock.B[j][k];
            double D = shock.width[j][k];
            double n_e = shock.n_p[j][k] * shock.xi;
            double Y = Y_tilt[j][k];

            e[j][k].n_tot = n_e;
            e[j][k].N_tot = n_e * dS * D;
            double I_nu_peak = syn_p_nu_peak(B, p) * e[j][k].N_tot / dS / (4 * con::pi);
            e[j][k].gamma_M = syn_gamma_M(B, shock.zeta, Y);
            e[j][k].gamma_m = syn_gamma_m(Gamma, e[j][k].gamma_M, shock.eps_e, shock.xi, p);
            e[j][k].gamma_c = syn_gamma_c(t_com, B, Y);
            e[j][k].gamma_a = syn_gamma_a(Gamma, B, I_nu_peak, e[j][k].gamma_m, e[j][k].gamma_c, e[j][k].gamma_M);
            e[j][k].regime = get_regime(e[j][k].gamma_a, e[j][k].gamma_c, e[j][k].gamma_m);
            e[j][k].p = p;
            e[j][k].gamma_N_peak = syn_gamma_N_peak(e[j][k].gamma_a, e[j][k].gamma_m, e[j][k].gamma_c);
        }
    }
    return e;
}

SynElectronsMesh gen_syn_electrons(double p, Coord const& coord, Shock const& shock) {
    SynElectronsMesh e = create_syn_electrons_grid(coord.theta.size(), coord.r.size());
    MeshGrid Y_tilt = create_grid_like(shock.Gamma, 0);
    return gen_syn_electrons(p, coord, shock, Y_tilt);
}

SynPhotonsMesh gen_syn_photons(SynElectronsMesh const& e, Coord const& coord, Shock const& shock) {
    SynPhotonsMesh ph = create_syn_photons_grid(coord.theta.size(), coord.r.size());

    for (size_t j = 0; j < coord.theta.size(); ++j) {
        for (size_t k = 0; k < coord.r.size(); ++k) {
            double B = shock.B[j][k];

            ph[j][k].L_nu_peak = syn_p_nu_peak(B, e[j][k].p) * e[j][k].N_tot;
            ph[j][k].nu_M = syn_nu(e[j][k].gamma_M, B);
            ph[j][k].nu_m = syn_nu(e[j][k].gamma_m, B);
            ph[j][k].nu_c = syn_nu(e[j][k].gamma_c, B);
            ph[j][k].nu_a = syn_nu(e[j][k].gamma_a, B);
            ph[j][k].regime = e[j][k].regime;
            ph[j][k].nu_E_peak = syn_nu_E_peak(ph[j][k]);
            ph[j][k].p = e[j][k].p;
        }
    }
    return ph;
}