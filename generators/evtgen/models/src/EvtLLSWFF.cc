/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtId.hh"
#include <string>
#include "EvtGenBase/EvtPDL.hh"
#include <math.h>
#include <stdlib.h>

#include "generators/evtgen/models/EvtLLSWFF.h"


EvtLLSWFF::EvtLLSWFF(double _tau_w1, double _tau_wp, double _zeta_1)
{

  tau_w1 = _tau_w1;
  tau_wp = _tau_wp;

  zeta_1 = _zeta_1;

  mb = 4.2; mc = 1.4; L = 0.40; Lp = 0.80; Ls = 0.76;

  eta_1 = eta_2 = eta_3 = eta_b = 0.;
  chi_1 = chi_2 = chi_b = 0.;

  return;
}

EvtLLSWFF::EvtLLSWFF(double _tau_w1, double _tau_wp, double _tau_1, double _tau_2)
{

  tau_w1 = _tau_w1;
  tau_wp = _tau_wp;

  tau_1 = _tau_1;
  tau_2 = _tau_2;

  mb = 4.2; mc = 1.4; L = 0.40; Lp = 0.80; Ls = 0.76;

  eta_1 = eta_2 = eta_3 = eta_b = 0.;
  chi_1 = chi_2 = chi_b = 0.;

  return;
}


void EvtLLSWFF::getscalarff(EvtId parent, EvtId,
                            double t, double mass, double* fp, double* f0)
{

  // std::cerr << "Called EvtLLSWFF::getscalarff" << std::endl;


  double m = EvtPDL::getMeanMass(parent);
  double w = ((m * m) + (mass * mass) - t) / (2.0 * m * mass);

  *fp = ((m + mass) * gpD0(w) - (m - mass) * gmD0(w)) / (2 * sqrt(mass * m));
  *f0 = ((m - mass) * (pow(m + mass, 2.) - t) * gpD0(w) - (mass + m) * (pow(m - mass,
         2.) - t) * gmD0(w)) / (2 * (m - mass) * sqrt(m * mass) * (mass + m));

  return;
}

void EvtLLSWFF::getvectorff(EvtId parent, EvtId daughter,
                            double t, double mass, double* a1f,
                            double* a2f, double* vf, double* a0f)
{

  double m = EvtPDL::getMeanMass(parent);
  double w = ((m * m) + (mass * mass) - t) / (2.0 * m * mass);

  static EvtId D3P1P = EvtPDL::getId("D'_1+");
  static EvtId D3P1N = EvtPDL::getId("D'_1-");
  static EvtId D3P10 = EvtPDL::getId("D'_10");
  static EvtId D3P1B = EvtPDL::getId("anti-D'_10");
  static EvtId D3P1SP = EvtPDL::getId("D'_s1+");
  static EvtId D3P1SN = EvtPDL::getId("D'_s1-");

  // Form factors have a general form, with parameters passed in
  // from the arguements.

  if (daughter == D3P1P || daughter == D3P1N || daughter == D3P10 || daughter == D3P1B || daughter == D3P1SP || daughter == D3P1SN) {

    *a1f = sqrt(mass * m) / (mass + m) * gV1D1p(w);
    *a2f = -(mass + m) * (gV3D1p(w) + mass / m * gV2D1p(w)) / (2 * sqrt(mass * m));
    *vf = 0.5 * (mass + m) * 1. / sqrt(mass * m) * gAD1p(w);

    double a3f = (m + mass) / (2 * mass) * (*a1f) - (m - mass) / (2 * mass) * (*a2f);
    *a0f = -t * (gV3D1p(w) - mass / m * gV2D1p(w)) / (4.*mass * sqrt(m * mass)) + a3f;


  } else {

    *a1f = sqrt(mass * m) / (mass + m) * fV1D1(w);
    *a2f = -(mass + m) * (fV3D1(w) + mass / m * fV2D1(w)) / (2 * sqrt(mass * m));
    *vf = 0.5 * (mass + m) * 1. / sqrt(mass * m) * fAD1(w);

    double a3f = (m + mass) / (2 * mass) * (*a1f) - (m - mass) / (2 * mass) * (*a2f);
    *a0f = -t * (fV3D1(w) - mass / m * fV2D1(w)) / (4.*mass * sqrt(m * mass)) + a3f;

  }


  return;
}

void EvtLLSWFF::gettensorff(EvtId parent, EvtId,
                            double t, double mass, double* hf,
                            double* kf, double* bpf, double* bmf)
{

  double m = EvtPDL::getMeanMass(parent);
  double w = ((m * m) + (mass * mass) - t) / (2.0 * m * mass);

  *hf = -sqrt(mass * m) / (mass * m * m) / 2.*kVD2(w);
  *kf = sqrt(mass * m) / m * kA1D2(w);

  *bpf = sqrt(mass * m) * (kA3D2(w) * m + kA2D2(w) * mass) / (2 * pow(m, 3.) * mass);
  *bmf = kA2D2(w) * sqrt(mass * m) / (2 * pow(m, 3.)) - kA3D2(w) * sqrt(mass * m) / (2 * pow(m, 2.) * mass);

  return;

}


void EvtLLSWFF::getbaryonff(EvtId, EvtId, double, double, double*,
                            double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getbaryonff in EvtLLSWFF.\n";
  ::abort();

}

void EvtLLSWFF::getdiracff(EvtId, EvtId, double, double, double*, double*,
                           double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getdiracff in EvtLLSWFF.\n";
  ::abort();

}

void EvtLLSWFF::getraritaff(EvtId, EvtId, double, double, double*, double*,
                            double*, double*, double*, double*, double*, double*)
{

  EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Not implemented :getraritaff in EvtLLSWFF.\n";
  ::abort();

}

// Isgur-Wise Function
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::IsgurWiseFunction(double w)
{

  double value = 0;

  value += tau_w1;
  value += tau_w1 * (w - 1.) * tau_wp;

  return value;

}

// Form Factors for D0
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::gpD0(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gp = 0;

  gp += ec * (2.*(w - 1.) * zeta_1 * z12 - 3.*z12 * (w * Ls - L) / (w + 1.));
  gp += -eb * ((Ls * (2.*w + 1.) - L * (w + 2.)) / (w + 1.) * z12 - 2.*(w - 1.) * zeta_1 * z12);

  return gp;

}

double EvtLLSWFF::gmD0(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gm = 0;

  gm += z12;
  gm += ec * (6 * chi_1 - 2 * (w + 1) * chi_2);
  gm += eb * chi_b;

  return gm;
}

// Form Factors for D1*
//-------------------------------------------------------------------------------------------


double EvtLLSWFF::gV1D1p(double w)
{

  double ec = 1 / (2 * mc), eb = 1 / (2 * mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gv1 = 0;

  gv1 += (w - 1.) * z12;
  gv1 += ec * ((w * Ls - L) * z12 + (w - 1.) * (-2 * chi_1));
  gv1 -= eb * ((Ls * (2.*w + 1) - L * (w + 2.)) * z12 - 2 * (pow(w, 2.) - 1.) * zeta_1 * z12 - (w - 1.) * chi_b);

  return gv1;

};

double EvtLLSWFF::gV2D1p(double w)
{

  double ec = 1 / (2 * mc);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gv2 = 0;

  gv2 += 2 * ec * (zeta_1 * z12 - chi_2);

  return gv2;

};

double EvtLLSWFF::gV3D1p(double w)
{

  double ec = 1 / (2 * mc), eb = 1 / (2 * mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double gv3 = 0;

  gv3 += eb * ((Ls * (2.*w + 1) - L * (w + 2.)) / (w + 1.) * z12 - 2 * (w - 1.) * zeta_1 * z12 - chi_b);
  gv3 -= z12;
  gv3 -= ec * ((w * Ls - L) / (w + 1) * z12 + 2.*zeta_1 * z12 - 2 * chi_1 + 2 * chi_2);

  return gv3;


};

double EvtLLSWFF::gAD1p(double w)
{

  double ec = 1 / (2 * mc), eb = 1 / (2 * mb);

  // Leading IW function values
  double z12 = IsgurWiseFunction(w);

  double ga = 0;

  ga += eb * (2.*(w - 1.) * zeta_1 * z12 - (Ls * (2.*w + 1.) - L * (w + 2.)) / (w + 1.) * z12 + chi_b);
  ga += z12;
  ga += ec * ((w * Ls - L) / (w + 1.) * z12 - 2 * chi_1);

  return ga;

};

// Form Factors for D1
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::fV1D1(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fv1 = 0;

  fv1 += (1. - pow(w, 2.)) * t32;
  fv1 -= eb * (pow(w, 2.) - 1.) * ((Lp + L) * t32 - (2.*w + 1) * tau_1 * t32 - tau_2 * t32 + eta_b);
  fv1 -= ec * (4.*(w + 1.) * (w * Lp - L) * t32 - (pow(w, 2.) - 1) * (3.*tau_1 * t32 - 3.*tau_2 * t32 + 2 * eta_1 + 3 * eta_3));

  fv1 /= sqrt(6);

  return fv1;

};

double EvtLLSWFF::fV2D1(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fv2 = 0;

  fv2 -= 3.*t32;
  fv2 -= 3 * eb * ((Lp + L) * t32 - (2 * w + 1) * tau_1 * t32 - tau_2 * t32 + eta_b);
  fv2 -= ec * ((4.*w - 1) * tau_1 * t32 + 5 * tau_2 * t32 + 10 * eta_1 + 4 * (w - 1.) * eta_2 - 5 * eta_3);

  fv2 /= sqrt(6);

  return fv2;

};

double EvtLLSWFF::fV3D1(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fv3 = 0;

  fv3 += (w - 2.) * t32;
  fv3 += eb * ((2. + w) * ((Lp + L) * t32 - (2.*w + 1.) * tau_1 * t32 - tau_2 * t32) - (2. - w) * eta_b);
  fv3 += ec * (4.*(w * Lp - L) * t32 + (2. + w) * tau_1 * t32 + (2. + 3.*w) * tau_2 * t32 - 2.*(6. + w) * eta_1 - 4.*
               (w - 1) * eta_2 - (3.*w - 2.) * eta_3);

  fv3 /= sqrt(6);

  return fv3;


};

double EvtLLSWFF::fAD1(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double fa = 0;

  fa += -(w + 1.) * t32;
  fa -= eb * ((w - 1.) * ((Lp + L) * t32 - (2.*w + 1.) * tau_1 * t32 - tau_2 * t32) + (w + 1.) * eta_b);
  fa -= ec * (4.*(w * Lp - L) * t32 - 3.*(w - 1.) * (tau_1 * t32 - tau_2 * t32) + (w + 1.) * (-2 * eta_1 - 3 * eta_3));

  fa /= sqrt(6);

  return fa;

};

// Form Factors for D2
//-------------------------------------------------------------------------------------------

double EvtLLSWFF::kA1D2(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double ka1 = 0;

  ka1 -= (1. + w) * t32;
  ka1 -= eb * ((w - 1.) * ((Lp + L) * t32 - (2.*w + 1.) * tau_1 * t32 - tau_2 * t32) + (1. + w) * eta_b);
  ka1 -= ec * ((w - 1.) * (tau_1 * t32 - tau_2 * t32) + (w + 1.) * (-2 * eta_1 + eta_3));

  return ka1;

};

double EvtLLSWFF::kA2D2(double w)
{

  double ec = 1. / (2.*mc);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double ka2 = 0;

  ka2 -=  2.*ec * (tau_1 * t32 + eta_2);

  return ka2;

};

double EvtLLSWFF::kA3D2(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double ka3 = 0;

  ka3 += t32;
  ka3 += eb * ((Lp + L) * t32 - (2.*w + 1.) * tau_1 * t32 - tau_2 * t32 + eta_b);
  ka3 -= ec * (tau_1 * t32 + tau_2 * t32 + 2 * eta_1 - 2 * eta_2 - eta_3);

  return ka3;

};

double EvtLLSWFF::kVD2(double w)
{

  double ec = 1. / (2.*mc), eb = 1. / (2.*mb);

  // Leading IW function values
  double t32 = IsgurWiseFunction(w);

  double kv = 0;

  kv -= t32;
  kv -= eb * ((Lp + L) * t32 - (2.*w + 1) * tau_1 * t32 - tau_2 * t32 + eta_b);
  kv -= ec * (tau_1 * t32 - tau_2 * t32 - 2 * eta_1 + eta_3);

  return kv;

};
