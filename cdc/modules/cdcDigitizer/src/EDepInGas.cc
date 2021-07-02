/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/EDepInGas.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>
#include <TRandom.h>
//#include <iostream>

using namespace std;
using namespace Belle2;
using namespace CDC;

EDepInGas* EDepInGas::m_pntr = 0;

EDepInGas& EDepInGas::getInstance()
{
  if (!m_pntr) {
    m_pntr = new EDepInGas();
    m_pntr->initialize();
  }
  return *m_pntr;
}

EDepInGas::EDepInGas() {}

EDepInGas::~EDepInGas() {}

void EDepInGas::initialize()
{
// gas
  const double zHe = 2;
  const double aHe = 4.003;
  double fHe = 0.1108;

  const double zC = 6;
  const double aC = 12.011;
  double fC = 0.2223;

  const double zH = 1;
  const double aH = 1.008;
  double fH = 0.6669;

  double f = fHe + fC + fH;
  fHe /= f;
  fC  /= f;
  fH  /= f;
  m_z1 = fHe * zHe + fC * zC + fH * zH;
  m_a1 = fHe * aHe + fC * aC + fH * aH;
  m_i1 = 49.229;
  m_rho1 = 0.703e-3;
  //  B2DEBUG(m_dbg, m_z1 <<" "<< m_a1 <<" "<< m_i1 <<" "<< m_rho1*m_z1/m_a1);

  // wire
  const double zW = 74;
  const double aW = 183.842;
  double fW = 0.0198;

  const double zAl = 13;
  const double aAl = 26.982;
  double fAl = 0.9802;

  f = fW + fAl;
  fW  /= f;
  fAl /= f;
  m_z2 = fW * zW + fAl * zAl;
  m_a2 = fW * aW + fAl * aAl;
  m_i2 = 193.301;
  m_rho2 = 0.419e-3;
  //  B2DEBUG(m_dbg, m_z2 <<" "<< m_a2 <<" "<< m_i2 <<" "<< m_rho2*m_z2/m_a2);

  fHe = 0.01031;
  fC  = 0.02068;
  fH  = 0.06204;
  fW  = 0.0014;
  fAl = 0.0684;
  f = fHe + fC + fH + fW + fAl;
  fHe /= f;
  fC  /= f;
  fH  /= f;
  fW  /= f;
  fAl /= f;
  m_z0 = fHe * zHe + fC * zC + fH * zH + fW * zW + fAl * zAl;
  m_a0 = fHe * aHe + fC * aC + fH * aH + fW * aW + fAl * aAl;
  m_i0 = 76.693;
  //  m_rho0 = m_rho1 + m_rho2;
  m_rho0 = 1.121e-3;

  //Define coefficients of density correction term
  m_ak1[0][0] =  10.065;
  m_ak1[0][1] = -13.63;
  m_ak1[1][0] =   4.1334;
  m_ak1[1][1] = -12.4;
  m_ak1[2][0] =   0.8135;
  m_ak1[2][1] = -11.33;
  m_ak1[3][0] =   0.00402;
  m_ak1[3][1] = -10.36;
  m_ak1[4][0] = - 1.7564;
  m_ak1[4][1] = - 9.611;
  m_ak1[5][0] = - 1.5065;
  m_ak1[5][1] = - 8.6753;

  m_bk1[0][0] = - 8.5619;
  m_bk1[0][1] =   4.6;
  m_bk1[1][0] = - 4.5857;
  m_bk1[1][1] =   4.6;
  m_bk1[2][0] = - 2.1447;
  m_bk1[2][1] =   4.6;
  m_bk1[3][0] = - 1.1582;
  m_bk1[3][1] =   4.6;
  m_bk1[4][0] =   0.67256;
  m_bk1[4][1] =   4.6;
  m_bk1[5][0] =   1.0156;
  m_bk1[5][1] =   4.6;

  m_ck1[0][0] =   1.82804;
  m_ck1[0][1] =   0.0;
  m_ck1[1][0] =   1.27579;
  m_ck1[1][1] =   0.0;
  m_ck1[2][0] =   0.93676;
  m_ck1[2][1] =   0.0;
  m_ck1[3][0] =   0.79975;
  m_ck1[3][1] =   0.0;
  m_ck1[4][0] =   0.49093;
  m_ck1[4][1] =   0.0;
  m_ck1[5][0] =   0.44805;
  m_ck1[5][1] =   0.0;
}

double EDepInGas::getEDepInGas(int mode, int pdg, double mom, double dx, double e3) const
{
  double xi1 = m_rho1 * (m_z1 / m_a1);
  double xi2 = m_rho2 * (m_z2 / m_a2);
  //  B2DEBUG(m_dbg, (xi1+xi2)/xi1 <<" "<< e3);
  if (mode == 0) return e3 * xi1 / (xi1 + xi2);

  static TDatabasePDG* ptrTDatabasePDG = TDatabasePDG::Instance();
  const double mass = ptrTDatabasePDG->GetParticle(pdg)->Mass();
  const double chrg = fabs(ptrTDatabasePDG->GetParticle(pdg)->Charge() / 3);

  const double me1 = getMostProbabEDep(mom, mass, chrg, dx, m_z1, m_a1, m_i1, m_rho1);
  if (mode == 2) {
    const double me0 = getMostProbabEDep(mom, mass, chrg, dx, m_z0, m_a0, m_i0, m_rho0);
    return e3 * me1 / me0;
  }
  const double me2 = getMostProbabEDep(mom, mass, chrg, dx, m_z2, m_a2, m_i2, m_rho2);
  if (mode == 1) return e3 * me1 / (me1 + me2);

  // extract energy deposit (e1) based on probab., L(e1)*L(e3-e1),
  // where L is the analytic approx. of Landau dist.
  // find lm1 which maximizes probability
  const double mom2 = mom * mom;
  const double beta2 = mom2 / (mass * mass + mom2);
  const double buf = 0.1535e-3 * dx * chrg * chrg / beta2;
  xi1 *= buf;
  xi2 *= buf;
  const double xi1i = 1 / xi1;
  const double xi2i = 1 / xi2;
  const double lm1min =       -me1 * xi1i;
  const double lm1max = (e3 - me1) * xi1i;
  double lm1 = (e3 * me1 / (me1 + me2) - me1) * xi1i; // initial guess
  lm1 = min(lm1, lm1max);
  lm1 = max(lm1, lm1min);
  //  double edm = 1;
  int itry = 0;
  //  B2DEBUG(m_dbg, "e3,me1,me2,xi1i=" << e3 <<" "<< me1 <<" "<< me2 <<" "<< xi1i);
  //  B2DEBUG(m_dbg, "lm1min,lm1max,lm1=" << lm1min <<" "<< lm1max <<" "<< lm1);
  const double lm2b = (e3 - me1 - me2) * xi2i;
  const double r = xi1 * xi2i;
  double deltal = 0;
  double lm1old = lm1;
  while (itry <= 2 || fabs(lm1 - lm1old) > 0.001) {
    ++itry;
    lm1old = lm1;
    if (itry > 50) {
      //      B2DEBUG(m_dbg, "itry > 50" << "itry,lm1min,lm1max,lm1,edm=" << itry <<" "<< lm1min <<" "<< lm1max <<" "<< lm1);
      break;
    }
    lm1 -= deltal;
    lm1 = max(lm1min, lm1);
    lm1 = min(lm1max, lm1);
    double lm2 = lm2b - r * lm1;
    double emlm1 = exp(-lm1);
    double emlm2 = exp(-lm2);
    double dldlm1   = 1 - emlm1 - r + r * emlm2;
    double dl2dlm12 =     emlm1   + r * r * emlm2;
    deltal = dldlm1 / dl2dlm12;
    //    edm  = dldlm1 * deltal; //drop 0.5* for speed up
  }
  double lm2 = lm2b - r * lm1;
  double lmin = 0.5 * (lm1 + lm2 + exp(-lm1) + exp(-lm2));
  const double ymax = exp(-lmin);
  //  if (ymax < 1.e-10) return e3*me1/(me1+me2);

  // generate e1 according to probab.
  const double lm1w = lm1max - lm1min;
  double y = 10;
  double p =  1;
  int jtry = 0;
  double urndm[2];
  while (y > p) {
    ++jtry;
    if (jtry > 500) {
      //      B2DEBUG(m_dbg, "jtry > 500" <<"ymax" << ymax);
      lm1 = (e3 * me1 / (me1 + me2) - me1) * xi1i;
      break;
    }
    gRandom->RndmArray(2, urndm);
    lm1 = lm1min + lm1w * urndm[0];
    lm2 = lm2b - r * lm1;
    double l = 0.5 * (lm1 + lm2 + exp(-lm1) + exp(-lm2));
    p = exp(-l);
    y = ymax * urndm[1];
  }

  return xi1 * lm1 + me1;
}

double EDepInGas::getMostProbabEDep(double p, double mass, double zi, double dx, double Z,
                                    double A, double I, double rho) const
{
// Calculate most probable energy deposit in GeV.
// Ref. J.Va'vra et al., NIM 203 (1982) 109.
// ----------------------------------------------------------------------
// <Input>
//      p      : total momentum   (GeV)
//      mass   : mass of particle (GeV)
//      zi     : charge of particle (e)
//      dx     : thickness of gas   (cm)
//      Z      : atomic no. of gas
//      A      : atomic weight of gas
//      I      : mean ionization potential of gas (eV)
//      rho    : density              of gas (g/cm**3)
// ----------------------------------------------------------------------
  const double psq  = p * p;
  const double masq = mass * mass;
  const double tote = psq + masq;
  const double beta2 = psq / tote;
  const double bgam2 = psq / masq;
  const double gamma = sqrt(tote) / mass;
  const double alft = 0.1535e6 * rho * dx * Z / A;
  const double k = log(m_massE * alft / I / I) + 0.891;
  const double k1 = k - log(dx);
  const double alngam =   log(gamma);
  const double alggam = log10(gamma);
  // calculate density correction term
  int irgn = 0;
  double aa, bb, cc;
  if (k1 < 6.) {
    if (alggam > 3.6) irgn = 1;
    aa = (m_ak1[1][irgn] - m_ak1[0][irgn]) * (6. - k1) + m_ak1[0][irgn];
    bb = (m_bk1[1][irgn] - m_bk1[0][irgn]) * (6. - k1) + m_bk1[0][irgn];
    cc = (m_ck1[1][irgn] - m_ck1[0][irgn]) * (6. - k1) + m_ck1[0][irgn];
  } else if (k1 < 7.) {
    if (alggam > 3.6) irgn = 1;
    aa = (m_ak1[1][irgn] - m_ak1[0][irgn]) * (k1 - 6.) + m_ak1[0][irgn];
    bb = (m_bk1[1][irgn] - m_bk1[0][irgn]) * (k1 - 6.) + m_bk1[0][irgn];
    cc = (m_ck1[1][irgn] - m_ck1[0][irgn]) * (k1 - 6.) + m_ck1[0][irgn];
  } else if (k1 < 8.) {
    if (alggam > 3.6) irgn = 1;
    aa = (m_ak1[2][irgn] - m_ak1[1][irgn]) * (k1 - 7.) + m_ak1[1][irgn];
    bb = (m_bk1[2][irgn] - m_bk1[1][irgn]) * (k1 - 7.) + m_bk1[1][irgn];
    cc = (m_ck1[2][irgn] - m_ck1[1][irgn]) * (k1 - 7.) + m_ck1[1][irgn];
  } else if (k1 < 9.) {
    if (alggam > 3.6) irgn = 1;
    aa = (m_ak1[3][irgn] - m_ak1[2][irgn]) * (k1 - 8.) + m_ak1[2][irgn];
    bb = (m_bk1[3][irgn] - m_bk1[2][irgn]) * (k1 - 8.) + m_bk1[2][irgn];
    cc = (m_ck1[3][irgn] - m_ck1[2][irgn]) * (k1 - 8.) + m_ck1[2][irgn];
  } else if (k1 < 10.) {
    if (alggam > 3.6) irgn = 1;
    aa = (m_ak1[4][irgn] - m_ak1[3][irgn]) * (k1 - 9.) + m_ak1[3][irgn];
    bb = (m_bk1[4][irgn] - m_bk1[3][irgn]) * (k1 - 9.) + m_bk1[3][irgn];
    cc = (m_ck1[4][irgn] - m_ck1[3][irgn]) * (k1 - 9.) + m_ck1[3][irgn];
  } else if (k1 < 11.) {
    if (alggam > 4.0) irgn = 1;
    aa = (m_ak1[5][irgn] - m_ak1[4][irgn]) * (k1 - 10.) + m_ak1[4][irgn];
    bb = (m_bk1[5][irgn] - m_bk1[4][irgn]) * (k1 - 10.) + m_bk1[4][irgn];
    cc = (m_ck1[5][irgn] - m_ck1[4][irgn]) * (k1 - 10.) + m_ck1[4][irgn];
  } else {
    if (alggam > 4.0) irgn = 1;
    aa = (m_ak1[5][irgn] - m_ak1[4][irgn]) * (k1 - 11.) + m_ak1[5][irgn];
    bb = (m_bk1[5][irgn] - m_bk1[4][irgn]) * (k1 - 11.) + m_bk1[5][irgn];
    cc = (m_ck1[5][irgn] - m_ck1[4][irgn]) * (k1 - 11.) + m_ck1[5][irgn];
  }

  double delta = aa + alggam * (bb + cc * alggam);
  delta = max(delta, 0.);
  delta = min(delta, fabs(log(bgam2)));

  double val = zi * zi * alft * (k + 2 * alngam - beta2 - delta) / beta2;
  val = max(val, 0.);
  val *= Unit::eV;
  return val;
}
