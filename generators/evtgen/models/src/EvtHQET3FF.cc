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

#include "generators/evtgen/models/EvtHQET3FF.h"


EvtHQET3FF::EvtHQET3FF(double hqetha1_1, double hqetrho2, double hqetr0_1, double hqetr1_1, double hqetr2_1)
{

  rho2 = hqetrho2;
  r0_1 = hqetr0_1;
  r1_1 = hqetr1_1;
  r2_1 = hqetr2_1;
  ha1_1 = hqetha1_1;

  return;
}

EvtHQET3FF::EvtHQET3FF(double hqetv1_1, double hqetrho2, double hqetdelta)
{

  v1_1 = hqetv1_1;
  rho2 = hqetrho2;
  d1_1 = hqetdelta;

  return;
}


void EvtHQET3FF::getscalarff(EvtId parent, EvtId,
                             double t, double mass, double* fp, double* f0)
{


  double mb = EvtPDL::getMeanMass(parent);
  double w = ((mb * mb) + (mass * mass) - t) / (2.0 * mb * mass);

  // Form factors have a general form, with parameters passed in
  // from the arguements.

  // Use dispersion relation parametrization from
  // I.Caprini, L.Lelluch, M.Neubert, Nucl. Phys. B 530,153(1998)
  const double z = (sqrt(w + 1) - sqrt(2.)) / (sqrt(w + 1) + sqrt(2.));
  double v1 = v1_1 * (1. - 8.*rho2 * z + (51.*rho2 - 10.) * z * z - (252.*rho2 - 84.) * z * z * z);

  *fp = 0.5 * (mass + mb) / sqrt(mass * mb) * v1;

  double s1 = (1 + d1_1 * (-0.019 + 0.041 * (w - 1) - 0.015 * pow(w - 1, 2.))) * v1;

  *f0 = sqrt(mass * mb) / (mass + mb) * (w + 1) * s1;

  return;
}

void EvtHQET3FF::getvectorff(EvtId parent, EvtId,
                             double t, double mass, double* a1f,
                             double* a2f, double* vf, double* a0f)
{


  double mb = EvtPDL::getMeanMass(parent);
  double w = ((mb * mb) + (mass * mass) - t) / (2.0 * mb * mass);

  // Form factors have a general form, with parameters passed in
  // from the arguements.

  double rstar = (2.0 * sqrt(mb * mass)) / (mb + mass);

  // Use disparsion relation parametrization from
  // I.Caprini, L.Lelluch, M.Neubert, Nucl. Phys. B 530,153(1998)
  const double z = (sqrt(w + 1) - sqrt(2.)) / (sqrt(w + 1) + sqrt(2.));
  double ha1 = ha1_1 * (1. - 8.*rho2 * z + (53.*rho2 - 15.) * z * z - (231.*rho2 - 91.) * z * z * z);
  double r0 = r0_1 - 0.11 * (w - 1) + 0.01 * (w - 1) * (w - 1);
  double r1 = r1_1 - 0.12 * (w - 1) + 0.05 * (w - 1) * (w - 1);
  double r2 = r2_1 + 0.11 * (w - 1) - 0.06 * (w - 1) * (w - 1);

  *a1f = (1.0 - (t / ((mb + mass) * (mb + mass)))) * ha1;
  *a1f = (*a1f) / rstar;
  *a2f = (r2 / rstar) * ha1;
  *vf  = (r1 / rstar) * ha1;
  *a0f = (r0 / rstar) * ha1;

  return;
}

void EvtHQET3FF::gettensorff(EvtId, EvtId, double, double, double*,
                             double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :gettensorff in EvtHQET3FF.\n";
  ::abort();

}



void EvtHQET3FF::getbaryonff(EvtId, EvtId, double, double, double*,
                             double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getbaryonff in EvtHQET3FF.\n";
  ::abort();

}

void EvtHQET3FF::getdiracff(EvtId, EvtId, double, double, double*, double*,
                            double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getdiracff in EvtHQET3FF.\n";
  ::abort();

}

void EvtHQET3FF::getraritaff(EvtId, EvtId, double, double, double*, double*,
                             double*, double*, double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getraritaff in EvtHQET3FF.\n";
  ::abort();

}
