
#include "forward-shock.h"

#include <boost/numeric/odeint.hpp>

#include "macros.h"

Shock::Shock(Coord const& coord)
    : t_com(create_grid(coord.theta.size(), coord.r.size(), 0)),
      Gamma(create_grid(coord.theta.size(), coord.r.size(), 1)),
      B(create_grid(coord.theta.size(), coord.r.size(), 0)) {}

MeshGrid FS_co_moving_B(Coord const& coord, Shock const& shock, Medium const& medium) {
    MeshGrid B = create_grid_like(shock.Gamma);
    for (size_t j = 0; j < B.size(); ++j) {
        for (size_t k = 0; k < B[j].size(); ++k) {
            double rho = medium.rho(coord.r[k]);
            double eps_B = medium.eps_B;
            double Gamma = shock.Gamma[j][k];
            B[j][k] = sqrt(8 * con::pi * eps_B * rho * 4 * Gamma * (Gamma - 1)) * con::c;
        }
    }
    return B;
}

MeshGrid FS_co_moving_shock_width(Coord const& coord, Shock const& shock) {
    MeshGrid Delta = create_grid_like(shock.Gamma);
    for (size_t j = 0; j < Delta.size(); ++j) {
        for (size_t k = 0; k < Delta[j].size(); ++k) {
            Delta[j][k] = coord.r[k] / shock.Gamma[j][k] / 12;
        }
    }
    return Delta;
}

ForwardShockEqn::ForwardShockEqn(Medium const& medium, Jet const& blast, double theta_lo, double theta_hi)
    : medium(medium),
      blast(blast),
      theta_lo(theta_lo),
      theta_hi(theta_hi),
      theta(0.5 * (theta_lo + theta_hi)),
      dOmega(4 * con::pi * std::fabs(std::cos(theta_hi) - std::cos(theta_lo))){};  // bipolar outflow

void ForwardShockEqn::operator()(Array const& y, Array& dydr, double r) {
    double Gamma = y[0];
    double u = y[1];
    double t_eng = y[2];  // engine time
    // double t_com= y[3];//comoving time
    dydr[0] = dGammadr(r, Gamma, u, t_eng);
    dydr[1] = dUdr(r, Gamma, u, t_eng);
    dydr[2] = dtdr_eng(Gamma);
    dydr[3] = dtdr_com(Gamma);
};

double ForwardShockEqn::dGammadr(double r, double Gamma, double u, double t_eng) {
    double gamma_eos = (4 * Gamma + 1) / (3 * Gamma);
    double dm = medium.mass(r) * dOmega / (4 * con::pi);
    double dM0 = blast.dEdOmega(theta, t_eng) * dOmega / (blast.Gamma0(theta) * con::c2);
    double Gamma2 = Gamma * Gamma;
    double a1 = dOmega * r * r * medium.rho(r) / Gamma * (Gamma2 - 1) * (gamma_eos * Gamma - gamma_eos + 1);
    double a2 = -(gamma_eos - 1) / Gamma * (gamma_eos * Gamma2 - gamma_eos + 1) * 3 * u / r;
    double b1 = (dM0 + dm) * con::c2;
    double b2 = (gamma_eos * gamma_eos * (Gamma2 - 1) + 3 * gamma_eos - 2) * u / Gamma2;
    return -(a1 + a2) / (b1 + b2);
};

double ForwardShockEqn::dUdr(double r, double Gamma, double u, double t_eng) {
    double gamma_eos = (4 * Gamma + 1) / (3 * Gamma);
    double E = dOmega * r * r * medium.rho(r) * con::c2;
    return (1 - medium.eps_e * medium.eta_rad) * (Gamma - 1) * E -
           (gamma_eos - 1) * (3 / r - dGammadr(r, Gamma, u, t_eng) / Gamma) * u;
};

double ForwardShockEqn::dtdr_eng(double Gamma) {
    double Gb = std::sqrt(Gamma * Gamma - 1);
    return (Gamma - Gb) / (Gb * con::c);
};

double ForwardShockEqn::dtdr_com(double Gamma) { return 1 / (sqrt(Gamma * Gamma - 1) * con::c); };  // co-moving time

void solve_single_shell(Array const& r, Array& Gamma, Array& t_com, double u0, ForwardShockEqn const& eqn) {
    using namespace boost::numeric::odeint;
    double atol = 0;     // integrator absolute tolerance
    double rtol = 1e-9;  // integrator relative tolerance
    auto stepper = bulirsch_stoer_dense_out<std::vector<double>>{atol, rtol};
    Array state{0, 0, 0, 0};

    double dr = (r[1] - r[0]) / 1000;
    double r0 = r[0];
    double Gamma0 = Gamma[0];
    double beta0 = sqrt(Gamma0 * Gamma0 - 1) / Gamma0;
    double t_com0 = t_com[0];

    // engine time used to calculate the energy injection
    double t_eng = r0 * (1 - beta0) / beta0 / con::c;

    // initialize the integrator
    stepper.initialize(Array{Gamma0, u0, t_eng, t_com0}, r0, dr);
    // integrate the shell over r
    for (int i = 0; stepper.current_time() <= r.back();) {
        stepper.do_step(eqn);

        for (; stepper.current_time() > r[i + 1] && i + 1 < r.size();) {
            i++;
            stepper.calc_state(r[i], state);
            Gamma[i] = state[0];
            // state[1]: u blast wave internal energy
            // state[2]: engine time
            t_com[i] = state[3];
        }
    }
}

Shock gen_forward_shock(Coord const& coord, Jet const& jet, Medium const& medium) {
    Shock shock(coord);

    for (size_t i = 0; i < coord.theta.size(); ++i) {
        auto eqn = ForwardShockEqn(medium, jet, coord.theta_b[i], coord.theta_b[i + 1]);
        double Gamma0 = jet.Gamma0(coord.theta[i]);

        shock.Gamma[i][0] = Gamma0;
        shock.t_com[i][0] = coord.r[0] / sqrt(Gamma0 * Gamma0 - 1) / con::c;

        // shell solid angle
        double dcos = std::fabs(std::cos(coord.theta_b[i + 1]) - std::cos(coord.theta_b[i]));
        double dphi = 2 * con::pi;
        double dOmega = 2 * dphi * dcos;  // bipolar outflow

        // initial internal energy
        double u0 = (Gamma0 - 1) * medium.mass(coord.r[0]) * dOmega / (4 * con::pi) * con::c2;
        solve_single_shell(coord.r, shock.Gamma[i], shock.t_com[i], u0, eqn);
    }
    shock.B = FS_co_moving_B(coord, shock, medium);
    shock.D_com = FS_co_moving_shock_width(coord, shock);
    return shock;
}
