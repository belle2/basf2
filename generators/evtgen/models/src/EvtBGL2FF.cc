/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtPDL.hh"
#include <math.h>
#include <stdlib.h>
#include <boost/math/constants/constants.hpp>
#include <cmath>
#include <iostream>
#include <vector>

#include "generators/evtgen/models/EvtBGL2FF.h"

// BGL (N=3) for scalar meson i.e. B->Dlv  (l=e,mu,tau)
EvtBGL2FF::EvtBGL2FF(double bglap_0, double bglap_1, double bglap_2, double bglap_3, double bgla0_0, double bgla0_1, double bgla0_2,
                     double bgla0_3)
{

  ap_0 = bglap_0;
  ap_1 = bglap_1;
  ap_2 = bglap_2;
  ap_3 = bglap_3; // set to 0 for N = 2 expansion

  a0_0 = bgla0_0;
  a0_1 = bgla0_1;
  a0_2 = bgla0_2;
  a0_3 = bgla0_3; // set to 0 for N = 2 expansion

  return;
}

// Use dispersion relation parametrization from
// C.G.Boyd, B.Grinstein, R.F.Lebed, Phys. Rev. Lett. 74,4603 (1995)
// and
// Bigi, Dante and Gambino, Paolo, Phys. Rev. D. 94.094008 (2016)
// a+0, a+1, a+2, a+3 : 0.01565, -0.0353, -0.043, 0.194
// a00, a01, a02, a03 : 0.07932, -0.214, 0.17, -0.958

// NOTE : This is an alternative BGL model. Use it if you understand the difference with the
// default one called 'EvtBGLFF' in basf2.

void EvtBGL2FF::getscalarff(EvtId parent, EvtId,
                            double t, double mass, double* fp, double* f0)
{

  static constexpr double pi = boost::math::constants::pi<double>();
  double mb = EvtPDL::getMeanMass(parent);
  double r = mass / mb; // mass = mass of the D meson
  double w = ((mb * mb) + (mass * mass) - t) / (2.0 * mb * mass);
  const double z = (sqrt(w + 1) - sqrt(2.)) / (sqrt(w + 1) + sqrt(2.));
  const double chiT = 5.131e-4;
  const double chiL = 6.332e-3;
  const double tp = (mass + mb) * (mass + mb);
  const double tm = (mass - mb) * (mass - mb);
  const double sqrttpt0 = sqrt(tp - tm); // t0 = tm (see Bigi, Gambino paper)
  std::vector<double> BcStatesp = {6.329, 6.920, 7.020}; //GeV
  std::vector<double> BcStates0 = {6.716, 7.121}; //GeV

  double Pp = 1.;
  for (size_t n = 0; n < BcStatesp.size(); ++n) {
    double sqrttpmp = sqrt(tp - pow(BcStatesp[n], 2));
    double zPp = (sqrttpmp - sqrttpt0) / (sqrttpmp + sqrttpt0);
    Pp *= (z - zPp) / (1 - z * zPp);
  }

  double P0 = 1.;
  for (size_t n = 0; n < BcStates0.size(); ++n) {
    double sqrttpmp = sqrt(tp - pow(BcStates0[n], 2));
    double zP0 = (sqrttpmp - sqrttpt0) / (sqrttpmp + sqrttpt0);
    P0 *= (z - zP0) / (1 - z * zP0);
  }

  double ni = 2.6;
  double kp = (8 * pow(r, 2) / mb) * sqrt(8 * ni / (3 * pi * chiT));
  double k0 = r * (1 - pow(r, 2)) * sqrt(8 * ni / (pi * chiL));

  double phi_sub = (1 + r) * (1 - z) + 2.0 * sqrt(r) * (1 + z);

  double phi_p = kp * pow(1 + z, 2.) * sqrt(1 - z) * pow(phi_sub, -5);

  double phi_0 = k0 * (1 - z * z) * sqrt(1 - z) * pow(phi_sub, -4);


  *fp = (ap_0 + ap_1 * z + ap_2 * z * z + ap_3 * z * z * z) / (Pp * phi_p);

  *f0 = (a0_0 + a0_1 * z + a0_2 * z * z + a0_3 * z * z * z) / (P0 * phi_0);

  return;
}

// NOTE : This class should not be used for vector mesons (D*). Use the EvtBGL class instead.

void EvtBGL2FF::getvectorff(EvtId parent, EvtId, double t, double mass, double* a1f,
                            double* a2f, double* vf, double* a0f)

{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getvectorff in EvtBGL2FF. Use EvtBGLFF instead.\n";
  ::abort();

}

void EvtBGL2FF::gettensorff(EvtId, EvtId, double, double, double*,
                            double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :gettensorff in EvtBGL2FF.\n";
  ::abort();

}



void EvtBGL2FF::getbaryonff(EvtId, EvtId, double, double, double*,
                            double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getbaryonff in EvtBGL2FF.\n";
  ::abort();

}

void EvtBGL2FF::getdiracff(EvtId, EvtId, double, double, double*, double*,
                           double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getdiracff in EvtBGL2FF.\n";
  ::abort();

}

void EvtBGL2FF::getraritaff(EvtId, EvtId, double, double, double*, double*,
                            double*, double*, double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getraritaff in EvtBGL2FF.\n";
  ::abort();

}
