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

#include "generators/evtgen/models/EvtBGLFF.h"

// BGL (N=3) for scalar meson i.e. B->Dlv  (l=e,mu,tau)
EvtBGLFF::EvtBGLFF(double bglap_0, double bglap_1, double bglap_2, double bglap_3, double bgla0_0, double bgla0_1, double bgla0_2,
                   double bgla0_3)
{

  ap_0 = bglap_0;
  ap_1 = bglap_1;
  ap_2 = bglap_2;
  ap_3 = bglap_3;

  a0_0 = bgla0_0;
  a0_1 = bgla0_1;
  a0_2 = bgla0_2;
  a0_3 = bgla0_3;

  return;
}


// BGL for vector meson i.e. B->D*lv (l=e,mu), and should not be used to taus.
EvtBGLFF::EvtBGLFF(double bgla_0, double bgla_1, double bglb_0, double bglb_1, double bglc_1, double bglc_2)
{

  a_0 = bgla_0;
  a_1 = bgla_1;

  b_0 = bglb_0;
  b_1 = bglb_1;

  c_1 = bglc_1;
  c_2 = bglc_2;

  return;
}


// Use dispersion relation parametrization from
// C.G.Boyd, B.Grinstein, R.F.Lebed, Phys. Rev. Lett. 74,4603(1995)
// and
// R.Glattauer, etc. (Belle) Phys. Rev. D 93,032006 (2016)

void EvtBGLFF::getscalarff(EvtId parent, EvtId,
                           double t, double mass, double* fp, double* f0)
{

  double mb = EvtPDL::getMeanMass(parent);
  double r = mass / mb;
  double w = ((mb * mb) + (mass * mass) - t) / (2.0 * mb * mass);
  const double z = (sqrt(w + 1) - sqrt(2.)) / (sqrt(w + 1) + sqrt(2.));
  const double p_i = 1.0;

  double phi_sub = (1 + r) * (1 - z) + 2.0 * sqrt(r) * (1 + z);

  double g_sub = (4.0 * r) / pow(1 + r, 2);

  double phi_p = 1.1213 * pow(1 + z, 2.) * sqrt(1 - z) * pow(phi_sub, -5);

  double phi_0 = 0.5299 * (1 + z) * pow(1 - z, 1.5) * pow(phi_sub, -4);


  *fp = g_sub * (ap_0 + ap_1 * z + ap_2 * z * z + ap_3 * z * z * z) / (p_i * phi_p);

  *f0 = g_sub * (a0_0 + a0_1 * z + a0_2 * z * z + a0_3 * z * z * z) / (p_i * phi_0);

  return;
}

// Use z expansion parametrization from
// C.G.Boyd, B.Grinstein and R.F.Lebed, Phys. Rev. D 56,6895(1997)
// and
// B.Grinstein, A.Kobach, Phys. Lett. B 771(2017)359-364

void EvtBGLFF::getvectorff(EvtId parent, EvtId, double t, double mass, double* a1f,
                           double* a2f, double* vf, double* a0f)

{

  double mb = EvtPDL::getMeanMass(parent);
  double w = ((mb * mb) + (mass * mass) - t) / (2. * mb * mass);

  // Form factors have a general form, with parameters passed in
  // from the arguements.

  const double r = mass / mb;
  double z = (sqrt(w + 1.) - sqrt(2.)) / (sqrt(w + 1) + sqrt(2.));
  const double rstar = (2. * sqrt(mb * mass)) / (mb + mass);
  const double chiT_plus33 = 5.28e-4;
  const double chiT_minus33 = 3.07e-4;
  const double n_i = 2.6;
  const double axialvector_poles[4] = {6.730, 6.736, 7.135, 7.142};
  const double vector_poles[4] = {6.337, 6.899, 7.012, 7.280};

  const double c_0 = (mb - mass) / mb * sqrt(0.5) / (1 + r + 2. * sqrt(r)) * b_0;

  double phi_g = sqrt(256. * n_i / (3. * M_PI * chiT_plus33)) * r * r * (1. + z) * (1. + z) / sqrt(1. - z) / pow((1. + r) *
                 (1. - z) + 2. * sqrt(r) * (1. + z), 4.);
  double phi_f = 1. / (mb * mb) * sqrt(16. * n_i / (3. * M_PI * chiT_minus33)) * r * (1. + z) * pow(1. - z ,
                 1.5) / pow((1. + r) * (1. - z) + 2. * sqrt(r) * (1. + z), 4.);
  double phi_F1 = 1. / (mb * mb * mb) * sqrt(8. * n_i / (3. * M_PI * chiT_minus33)) * r * (1. + z) * pow(1. - z ,
                  2.5) / pow((1. + r) * (1. - z) + 2 * sqrt(r) * (1. + z), 5.);

  double p_g = 1.;
  double p_f = 1.;
  const double term3 = sqrt((mb + mass) * (mb + mass) - (mb - mass) * (mb - mass));
  for (int i = 0; i < 4; i++) {
    double term1 = sqrt((mb + mass) * (mb + mass) - vector_poles[i] * vector_poles[i]);
    double term2 = sqrt((mb + mass) * (mb + mass) - axialvector_poles[i] * axialvector_poles[i]);
    double z_p1 = (term1 - term3) / (term1 + term3);
    p_g = p_g * (z - z_p1) / (1 - z * z_p1);
    double z_p2 = (term2 - term3) / (term2 + term3);
    p_f = p_f * (z - z_p2) / (1 - z * z_p2);
  }

  double g = 1. / p_g / phi_g * (a_0 + a_1 * z);
  double f = 1. / p_f / phi_f * (b_0 + b_1 * z);
  double F1 = 1. / p_f / phi_F1 * (c_0 + c_1 * z + c_2 * z * z);

  double ha1 = f / sqrt(mb * mass) / (1. + w);
  double r1 = (w + 1.) * mb * mass * g / f;
  double r2 = (w - r) / (w - 1) - F1 / mb / (w - 1) / f;

  *a1f = (w + 1.) / 2. * rstar * ha1;
  *a2f = (r2 / rstar) * ha1;
  *vf  = (r1 / rstar) * ha1;
  *a0f = 0; // a0f is related to B->D* tau nu decay.
  // The class should not be used for taus, due to the lack of fitted parameters in a0f amplitude.

  return;
}

void EvtBGLFF::gettensorff(EvtId, EvtId, double, double, double*,
                           double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :gettensorff in EvtBGLFF.\n";
  ::abort();

}



void EvtBGLFF::getbaryonff(EvtId, EvtId, double, double, double*,
                           double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getbaryonff in EvtBGLFF.\n";
  ::abort();

}

void EvtBGLFF::getdiracff(EvtId, EvtId, double, double, double*, double*,
                          double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getdiracff in EvtBGLFF.\n";
  ::abort();

}

void EvtBGLFF::getraritaff(EvtId, EvtId, double, double, double*, double*,
                           double*, double*, double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getraritaff in EvtBGLFF.\n";
  ::abort();

}
