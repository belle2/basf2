/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Lu Cao                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtId.hh"
#include <string>
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

void EvtBGLFF::getvectorff(EvtId, EvtId, double, double, double*,
                           double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getvectorff in EvtBGLFF.\n";
  ::abort();
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
