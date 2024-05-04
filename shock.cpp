
#include "shock.h"

#include <boost/numeric/odeint.hpp>

#include "macros.h"
#include "physics.h"
Shock::Shock(Coord const& coord, double eps_e, double eps_B, double xi, double zeta)
    : t_com(create_grid(coord.theta.size(), coord.r.size(), 0)),
      t_com_b(create_grid(coord.theta.size(), coord.r_b.size(), 0)),
      Gamma(create_grid(coord.theta.size(), coord.r.size(), 1)),
      e_th(create_grid(coord.theta.size(), coord.r.size(), 1)),
      B(create_grid(coord.theta.size(), coord.r.size(), 0)),
      width(create_grid(coord.theta.size(), coord.r.size(), 0)),
      n_p(create_grid(coord.theta.size(), coord.r.size(), 0)),
      eps_e{eps_e},
      eps_B{eps_B},
      xi{xi},
      zeta{zeta} {}

double co_moving_B(double eps_B, double e_thermal) { return sqrt(8 * con::pi * eps_B * e_thermal); }

double n_down_str(double Gamma_relative, double n_up_str, double cs_up_str) {
    double u4 = sqrt(Gamma_relative * Gamma_relative - 1);
    double us4 = 4 * u4 * sqrt((1 + u4 * u4) / (8 * u4 * u4 + 9));  // check this
    double M = us4 / cs_up_str;
    if (M > 10) {
        return 4 * Gamma_relative * n_up_str;
    } else if (M >= 1) {
        double ad_idx = adiabatic_index(Gamma_relative);
        return n_up_str * (ad_idx + 1) * M * M / ((ad_idx - 1) * M * M + 2);
    } else {
        return 0;  // shock cannot be formed
    }
}

inline double e_thermal_down_str(double Gamma_relative, double n_down_str) {
    return n_down_str * (Gamma_relative - 1) * con::mp * con::c2;
}

double sound_speed(double pressure, double ad_idx, double rho_rest) {
    return sqrt(ad_idx * pressure / (rho_rest * con::c2 + ad_idx / (ad_idx - 1) * pressure)) * con::c;
}

BlastWaveEqn::BlastWaveEqn(Medium const& medium, Jet const& blast, double theta_lo, double theta_hi)
    : medium(medium),
      jet(blast),
      theta_lo(theta_lo),
      theta_hi(theta_hi),
      theta(0.5 * (theta_lo + theta_hi)),
      dOmega(2 * con::pi * std::fabs(std::cos(theta_hi) - std::cos(theta_lo))){};

void BlastWaveEqn::operator()(Array const& y, Array& dydr, double r) {
    double Gamma = y[0];
    double u = y[1];
    double t_eng = y[2];  // engine time
    // double t_com = y[3];  // co-moving time
    // double D_RS = y[4];   // co-moving frame reverse shock width
    double D_FS = y[5];  // co-moving frame forward shock width

    dydr[0] = dGammadr(r, Gamma, u, t_eng);
    dydr[1] = dUdr(r, Gamma, u, t_eng);
    dydr[2] = dtdr_eng(Gamma);
    dydr[3] = dtdr_com(Gamma);
    dydr[4] = dDdr_RS(r, Gamma, D_FS, t_eng);
    dydr[5] = dDdr_FS(r, Gamma);
};

double BlastWaveEqn::dGammadr(double r, double Gamma, double u, double t_eng) {
    double ad_idx = adiabatic_index(Gamma);
    double dm = medium.mass(r) * dOmega / (4 * con::pi);
    double dM0 = jet.dEdOmega(theta, t_eng) * dOmega / (jet.Gamma0_profile(theta) * con::c2);
    double Gamma2 = Gamma * Gamma;
    double a1 = dOmega * r * r * medium.rho(r) / Gamma * (Gamma2 - 1) * (ad_idx * Gamma - ad_idx + 1);
    double a2 = -(ad_idx - 1) / Gamma * (ad_idx * Gamma2 - ad_idx + 1) * 3 * u / r;
    double b1 = (dM0 + dm) * con::c2;
    double b2 = (ad_idx * ad_idx * (Gamma2 - 1) + 3 * ad_idx - 2) * u / Gamma2;
    return -(a1 + a2) / (b1 + b2);
};

double BlastWaveEqn::dUdr(double r, double Gamma, double u, double t_eng) {
    double ad_idx = adiabatic_index(Gamma);
    double E = dOmega * r * r * medium.rho(r) * con::c2;
    return (1 - medium.eps_e) * (Gamma - 1) * E - (ad_idx - 1) * (3 / r - dGammadr(r, Gamma, u, t_eng) / Gamma) * u;
};

double BlastWaveEqn::dtdr_eng(double Gamma) {
    double Gb = std::sqrt(Gamma * Gamma - 1);
    return (Gamma - Gb) / (Gb * con::c);
};

double BlastWaveEqn::dtdr_com(double Gamma) { return 1 / (std::sqrt(Gamma * Gamma - 1) * con::c); };  // co-moving time

double BlastWaveEqn::dDdr_FS(double r, double Gamma) {
    double ad_idx = adiabatic_index(Gamma);
    double n1 = medium.rho(r) / con::mp;
    double n2 = n_down_str(Gamma, n1, medium.cs);
    double p2 = (ad_idx - 1) * e_thermal_down_str(Gamma, n2);
    double cs = sound_speed(p2, ad_idx, n2 * con::mp);
    return cs * dtdr_com(Gamma);
};

double BlastWaveEqn::dDdr_RS(double r, double Gamma, double D_com, double t_eng) {
    double Gamma0 = jet.Gamma0_profile(theta);
    double n4 = jet.dEdOmega(theta, t_eng) / (Gamma0 * con::mp * con::c2 * r * r * D_com);
    double n1 = medium.rho(r) / con::mp;
    // double gamma34 = sqrt(Gamma0 / (2 * sqrt(n4 / n1)));
    double Gamma34 = (Gamma0 / Gamma + Gamma / Gamma0) / 2;
    double ad_idx3 = adiabatic_index(Gamma34);
    double n3 = n4 * (ad_idx3 * Gamma34 + 1) / (ad_idx3 - 1);

    return Gamma / (Gamma0 * std::sqrt(n4 / n1) * (1 - Gamma0 * n4 / Gamma / n3));
};

void update_shock_state(size_t j, size_t k, Shock& shock, Array& state, double Gamma_up_str, double n_up_str,
                        double cs_up_str, double shock_width) {
    double Gamma = state[0];
    // state[1]: u blast wave internal energy
    double t_eng = state[2];
    double t_com = state[3];

    double Gamma_rel =
        fabs(Gamma_up_str - 1.) < 1e-6 ? Gamma : (Gamma / Gamma_up_str + Gamma_up_str / Gamma) / 2;  // can be improved

    shock.Gamma[j][k] = Gamma;
    shock.t_com[j][k] = t_com;
    shock.width[j][k] = shock_width;
    shock.n_p[j][k] = n_down_str(Gamma_rel, n_up_str, cs_up_str);
    shock.e_th[j][k] = e_thermal_down_str(Gamma_rel, shock.n_p[j][k]);
    shock.B[j][k] = co_moving_B(shock.eps_B, shock.e_th[j][k]);
}

void Blandford_McKee(size_t j, size_t k, Shock& shock, Array& state, double r, double N3, double E_th3) {
    double Gamma = state[0];
    // state[1]: u blast wave internal energy
    double t_eng = state[2];
    double t_com = state[3];
    double D_FS = state[5];

    shock.Gamma[j][k] = Gamma;
    shock.t_com[j][k] = t_com;
    shock.width[j][k] = D_FS;
    shock.n_p[j][k] = N3 / (D_FS * r * r);
    shock.e_th[j][k] = E_th3 / (D_FS * r * r);
    shock.B[j][k] = co_moving_B(shock.eps_B, shock.e_th[j][k]);
}

bool RS_cross_check(double D_RS, double D_FS, double r, double n_p, double e_th, double& N3, double& E_th3) {
    if (D_RS < D_FS) {
        return false;
    } else {
        N3 = n_p * D_RS * r * r;
        E_th3 = e_th * D_RS * r * r;
        return true;
    }
}

void solve_single_shell(size_t j, Array const& r_b, Array const& r, Shock& r_shock, Shock& f_shock,
                        BlastWaveEqn const& eqn) {
    using namespace boost::numeric::odeint;
    double atol = 0;     // integrator absolute tolerance
    double rtol = 1e-6;  // integrator relative tolerance
    // auto stepper = bulirsch_stoer_dense_out<std::vector<double>>{atol, rtol};
    auto stepper = make_dense_output(atol, rtol, runge_kutta_dopri5<std::vector<double>>());

    double r0 = r_b[0];
    double Gamma0 = eqn.jet.Gamma0_profile(eqn.theta);
    double u0 = (Gamma0 - 1) * eqn.medium.mass(r0) * eqn.dOmega / (4 * con::pi) * con::c2;
    double beta0 = gamma_to_beta(Gamma0);
    double t_eng0 = r0 * (1 - beta0) / beta0 / con::c;
    double t_com0 = r0 / sqrt(Gamma0 * Gamma0 - 1) / con::c;
    double D_RS0 = 0;
    double D_FS0 = con::c * eqn.jet.duration * Gamma0;

    Array state{Gamma0, u0, t_eng0, t_com0, D_RS0, D_FS0};

    double n1_0 = eqn.medium.rho(r0) / con::mp;
    double n4_0 = eqn.jet.dEdOmega(eqn.theta, t_eng0) / (Gamma0 * con::mp * con::c2 * r0 * r0 * D_FS0);

    // update_shock_state(j, 0, f_shock, state, 1., n1_0, eqn.medium.cs, D_FS0);
    // update_shock_state(j, 0, r_shock, state, Gamma0, n4_0, 1e-8, D_RS0);
    f_shock.t_com_b[j][0] = r_shock.t_com_b[j][0] = t_com0;
    // initialize the integrator
    double dr = (r[1] - r[0]) / 100;
    stepper.initialize(state, r0, dr);

    double N3 = 0;
    double E_th = 0;
    bool RS_crossed = false;

    // integrate the shell over r
    for (int k = 0, k1 = 0; stepper.current_time() <= r_b.back();) {
        stepper.do_step(eqn);

        for (; stepper.current_time() > r[k] && k < r.size(); k++) {
            stepper.calc_state(r[k], state);
            // state[1]: u blast wave internal energy
            double t_eng = state[2];
            double D_RS = state[4];
            // double D_FS = state[5];
            double D_FS = r[k] / state[0] / 12;

            double n1 = eqn.medium.rho(r[k]) / con::mp;
            update_shock_state(j, k, f_shock, state, 1., n1, eqn.medium.cs, D_FS);

            if (!RS_crossed) {
                double n4 = eqn.jet.dEdOmega(eqn.theta, t_eng) / (Gamma0 * con::mp * con::c2 * r[k] * r[k] * D_FS);
                update_shock_state(j, k, r_shock, state, Gamma0, n4, 1E-8, D_RS);
                RS_crossed = RS_cross_check(D_RS, D_FS, r[k], r_shock.n_p[j][k], r_shock.e_th[j][k], N3, E_th);
            } else {
                Blandford_McKee(j, k, r_shock, state, r[k], N3, E_th);
            }
        }

        for (; stepper.current_time() > r_b[k1 + 1] && k1 + 1 < r_b.size();) {
            k1++;
            stepper.calc_state(r_b[k1], state);
            f_shock.t_com_b[j][k1] = r_shock.t_com_b[j][k1] = state[3];
        }
    }
}

std::pair<Shock, Shock> gen_shocks(Coord const& coord, Jet const& jet, Medium const& medium) {
    Shock f_shock(coord, medium.eps_e, medium.eps_B, medium.xi, medium.zeta);  // forward shock
    Shock r_shock(coord, medium.eps_e, medium.eps_B, medium.xi, medium.zeta);  // reverse shock

    for (size_t j = 0; j < coord.theta.size(); ++j) {
        auto eqn = BlastWaveEqn(medium, jet, coord.theta_b[j], coord.theta_b[j + 1]);
        solve_single_shell(j, coord.r_b, coord.r, r_shock, f_shock, eqn);
    }
    return std::make_pair(r_shock, f_shock);
}
