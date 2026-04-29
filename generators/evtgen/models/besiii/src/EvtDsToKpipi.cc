// Model: EvtDsToKpipi
// This file is an amplitude model for Ds+ -> K- pi+ pi+.
// The model is from JHEP08 (2022) 196. DOI: https://doi.org/10.1007/JHEP08(2022)196
//
// Permission to use these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#include <iostream>
#include <cmath>
#include <string>
#include <EvtGenBase/EvtCPUtil.hh>
#include <EvtGenBase/EvtTensor4C.hh>
#include <EvtGenBase/EvtPatches.hh>
#include <fstream>
#include <stdlib.h>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtGenKine.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtReport.hh>
#include <EvtGenBase/EvtResonance.hh>
#include <EvtGenBase/EvtResonance2.hh>
#include <string>
#include <EvtGenBase/EvtConst.hh>
#include <EvtGenBase/EvtDecayTable.hh>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/besiii/EvtDsToKpipi.h>

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtDsToKpipi);

  EvtDsToKpipi::~EvtDsToKpipi() {}

  std::string EvtDsToKpipi::getName()
  {
    return "DsToKpipi";
  }

  EvtDecayBase* EvtDsToKpipi::clone()
  {
    return new EvtDsToKpipi;
  }

  void EvtDsToKpipi::init()
  {
    checkNArg(0);
    checkNDaug(3);
    checkSpinParent(EvtSpinType::SCALAR);
    checkSpinDaughter(0, EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::SCALAR);
    checkSpinDaughter(2, EvtSpinType::SCALAR);

    phi[0] = 0;
    rho[0] = 1;
    phi[1] = 0;
    rho[1] = 0;
    phi[2] = 0;
    rho[2] = 0;
    phi[3] = 0;
    rho[3] = 0;
    phi[4] = 0;
    rho[4] = 0;
    phi[5] = 0;
    rho[5] = 0;
    phi[6] = 0;
    rho[6] = 0;
    phi[7] = 0;
    rho[7] = 0;

    phi[1] = -3.47995752;
    phi[2] = -1.249864467;
    phi[3] = -0.3067504308;
    phi[4] =  0.9229242379;
    phi[5] = -3.278567926;
    phi[6] = -0.6346408622;
    phi[7] =  1.762377475;

    rho[1] = 2.463898984;
    rho[2] = 0.7361782665;
    rho[3] = 1.90216812;
    rho[4] = 2.140687169;
    rho[5] = 0.914684056;
    rho[6] = 1.116206881;
    rho[7] = 1.483440555;

    modetype[0] = 23;
    modetype[1] = 23;
    modetype[2] = 23;
    modetype[3] = 23;
    modetype[4] = 23;
    modetype[5] = 13;
    modetype[6] = 13;
    modetype[7] = 13;

    width[0]  = 0.1478;
    width[1]  = 0.400;
    width[2]  = 0.100;
    width[3]  = 0.265;
    width[4]  = 0.270;
    width[5]  = 0.0473;
    width[6]  = 0.232;
    width[7]  = 0.270;

    mass[0]   = 0.77526;
    mass[1]   = 1.465;
    mass[2]   = 0.965;
    mass[3]   = 1.35;
    mass[4]   = 1.425;
    mass[5]   = 0.89555;
    mass[6]   = 1.414;
    mass[7]   = 1.432787726;

    mDsM = 1.9683;
    mD         = 1.86486;
    mDs        = 1.9683;
    rD         = 25.0;
    metap      = 0.95778;
    mkstr      = 0.89594;
    mk0        = 0.497614;
    mass_Kaon  = 0.49368;
    mass_Pion  = 0.13957;
    mass_Pi0   = 0.1349766;
    math_pi    = 3.1415926;
    mKa2 = 0.24371994;
    mPi2 = 0.01947978;
    mPi = 0.13957;
    mKa  = 0.49368;
    ma0 = 0.99;
    Ga0 = 0.0756;
    meta = 0.547862;

    GS1 = 0.636619783;
    GS2 = 0.01860182466;
    GS3 = 0.1591549458;
    GS4 = 0.00620060822;

    int GG[4][4] = { {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, -1} };
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        G[i][j] = GG[i][j];
      }
    }
  }

  void EvtDsToKpipi::initProbMax()
  {
    setProbMax(825.0);
  }

  void EvtDsToKpipi::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R D1 = p->getDaug(0)->getP4();
    EvtVector4R D2 = p->getDaug(1)->getP4();
    EvtVector4R D3 = p->getDaug(2)->getP4();

    double P1[4], P2[4], P3[4];
    P1[0] = D1.get(0); P1[1] = D1.get(1); P1[2] = D1.get(2); P1[3] = D1.get(3);
    P2[0] = D2.get(0); P2[1] = D2.get(1); P2[2] = D2.get(2); P2[3] = D2.get(3);
    P3[0] = D3.get(0); P3[1] = D3.get(1); P3[2] = D3.get(2); P3[3] = D3.get(3);

    double value;
    int g0[8] = {1, 1, 4, 2, 500, 1, 1, 2};
    int spin_param[8] = {1, 1, 0, 0, 0, 1, 1, 0};
    int nstates = 8;
    calEva(P1, P2, P3, mass, width, rho, phi, g0, spin_param, modetype, nstates, value);

    setProb(value);

    return ;
  }

  void EvtDsToKpipi::Com_Multi(double a1[2], double a2[2], double res[2])
  {
    res[0] = a1[0] * a2[0] - a1[1] * a2[1];
    res[1] = a1[1] * a2[0] + a1[0] * a2[1];
  }
  void EvtDsToKpipi::Com_Divide(double a1[2], double a2[2], double res[2])
  {
    double tmp = a2[0] * a2[0] + a2[1] * a2[1];
    res[0] = (a1[0] * a2[0] + a1[1] * a2[1]) / tmp;
    res[1] = (a1[1] * a2[0] - a1[0] * a2[1]) / tmp;
  }

  double EvtDsToKpipi::SCADot(double a1[4], double a2[4])
  {
    double _cal = a1[0] * a2[0] - a1[1] * a2[1] - a1[2] * a2[2] - a1[3] * a2[3];
    return _cal;
  }
  double EvtDsToKpipi::barrier(int l, double sa, double sb, double sc, double r, double mass_param)
  {
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (q < 0) q = 1e-16;
    double z;
    z = q * r * r;
    double sa0;
    sa0 = mass_param * mass_param;
    double q0 = (sa0 + sb - sc) * (sa0 + sb - sc) / (4 * sa0) - sb;
    if (q0 < 0) q0 = 1e-16;
    double z0 = q0 * r * r;
    double F = 0.0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + z0) / (1 + z));
    if (l == 2) F = sqrt((9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z));
    return F;
  }
  void EvtDsToKpipi::calt1(double daug1[4], double daug2[4], double t1[4])
  {
    double p, pq, tmp;
    double pa[4], qa[4];
    for (int i = 0; i < 4; i++) {
      pa[i] = daug1[i] + daug2[i];
      qa[i] = daug1[i] - daug2[i];
    }
    p = SCADot(pa, pa);
    pq = SCADot(pa, qa);
    tmp = pq / p;
    for (int i = 0; i < 4; i++) {
      t1[i] = qa[i] - tmp * pa[i];
    }
  }
  void EvtDsToKpipi::calt2(double daug1[4], double daug2[4], double t2[4][4])
  {
    double p, r;
    double pa[4], t1[4];
    calt1(daug1, daug2, t1);
    r = SCADot(t1, t1) / 3.0;
    for (int i = 0; i < 4; i++) {
      pa[i] = daug1[i] + daug2[i];
    }
    p = SCADot(pa, pa);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        t2[i][j] = t1[i] * t1[j] - r * (G[i][j] - pa[i] * pa[j] / p);
      }
    }
  }

//-------------------prop--------------------------------------------

  void EvtDsToKpipi::propagatorCBW(double mass_param, double width_param, double sx, double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass_param * mass_param - sx;
    b[1] = -mass_param * width_param;
    Com_Divide(a, b, prop);
  }
  double EvtDsToKpipi::wid(double mass2, double mass_param, double sa, double sb, double sc, double r2, int l)
  {
    double widm = 0.;
    double m = sqrt(sa);
    double tmp  = sb - sc;
    double tmp1 = sa + tmp;
    double q = 0.25 * tmp1 * tmp1 / sa - sb;
    if (q < 0) q = 1e-16;
    double tmp2 = mass2 + tmp;
    double q0 = 0.25 * tmp2 * tmp2 / mass2 - sb;
    if (q0 < 0) q0 = 1e-16;
    double z = q * r2;
    double z0 = q0 * r2;
    double t = q / q0;
    if (l == 0)      {widm = sqrt(t) * mass_param / m;}
    else if (l == 1) {widm = t * sqrt(t) * mass_param / m * (1 + z0) / (1 + z);}
    else if (l == 2) {widm = t * t * sqrt(t) * mass_param / m * (9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z);}
    return widm;
  }

  double EvtDsToKpipi::widl1(double mass2, double mass_param, double sa, double sb, double sc, double r2)
  {
    double widm = 0.;
    double m = sqrt(sa);
    double tmp  = sb - sc;
    double tmp1 = sa + tmp;
    double q = 0.25 * tmp1 * tmp1 / sa - sb;
    if (q < 0) q = 1e-16;
    double tmp2 = mass2 + tmp;
    double q0 = 0.25 * tmp2 * tmp2 / mass2 - sb;
    if (q0 < 0) q0 = 1e-16;
    double z = q * r2;
    double z0 = q0 * r2;
    double F = (1 + z0) / (1 + z);
    double t = q / q0;
    widm = t * sqrt(t) * mass_param / m * F;
    return widm;
  }
  void EvtDsToKpipi::propagatorRBW(double mass_param, double width_param, double sa, double sb, double sc, double r2, int l,
                                   double prop[2])
  {
    double a[2], b[2];
    double mass2 = mass_param * mass_param;

    a[0] = 1;
    a[1] = 0;
    b[0] = mass2 - sa;
    b[1] = -mass_param * width_param * wid(mass2, mass_param, sa, sb, sc, r2, l);
    Com_Divide(a, b, prop);
  }
  void EvtDsToKpipi::propagatorFlatte(double mass_param, double width_param __attribute__((unused)), double sa, double prop[2])
  {

    double q2_Pi, q2_Ka;
    double rhoPi[2] = {0, 0}, rhoKa[2] = {0, 0};

    q2_Pi = 0.25 * sa - mPi * mPi;
    q2_Ka = 0.25 * sa - mKa * mKa;

    if (q2_Pi > 0) {
      rhoPi[0] = 2.0 * sqrt(q2_Pi / sa);
      rhoPi[1] = 0.0;
    }
    if (q2_Pi <= 0) {
      rhoPi[0] = 0.0;
      rhoPi[1] = 2.0 * sqrt(-q2_Pi / sa);
    }

    if (q2_Ka > 0) {
      rhoKa[0] = 2.0 * sqrt(q2_Ka / sa);
      rhoKa[1] = 0.0;
    }
    if (q2_Ka <= 0) {
      rhoKa[0] = 0.0;
      rhoKa[1] = 2.0 * sqrt(-q2_Ka / sa);
    }

    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass_param * mass_param - sa + 0.165 * rhoPi[1] + 0.69465 * rhoKa[1];
    b[1] = - (0.165 * rhoPi[0] + 0.69465 * rhoKa[0]);
    Com_Divide(a, b, prop);
  }
  void EvtDsToKpipi::propagatorGS(double mass_param, double width_param, double sa, double sb, double sc, double r2, double prop[2])
  {
    double a[2], b[2];
    double mass2 = mass_param * mass_param;
    double tmp  = sb - sc;
    double tmp1 = sa + tmp;
    double q2 = 0.25 * tmp1 * tmp1 / sa - sb;
    if (q2 < 0) q2 = 1e-16;

    double tmp2 = mass2 + tmp;
    double q02 = 0.25 * tmp2 * tmp2 / mass2 - sb;
    if (q02 < 0) q02 = 1e-16;

    double q  = sqrt(q2);
    double q0 = sqrt(q02);
    double m  = sqrt(sa);
    double q03 = q0 * q02;
    double tmp3 = log(mass_param + 2 * q0) + 1.2760418309; // log(mass_2Pion) = 1.2760418309;

    double h  = GS1 * q / m * (log(m + 2 * q) + 1.2760418309);
    double h0 = GS1 * q0 / mass_param * tmp3;
    double dh = h0 * (0.125 / q02 - 0.5 / mass2) + GS3 / mass2;
    double d  = GS2 / q02 * tmp3 + GS3 * mass_param / q0 - GS4 * mass_param / q03;
    double f  = mass2 / q03 * (q2 * (h - h0) + (mass2 - sa) * q02 * dh);

    a[0] = 1.0 + d * width_param / mass_param;
    a[1] = 0.0;
    b[0] = mass2 - sa + width_param * f;
    b[1] = -mass_param * width_param * widl1(mass2, mass_param, sa, sb, sc, r2);
    Com_Divide(a, b, prop);
  }

  void EvtDsToKpipi::Flatte_rhoab(double sa, double sb, double sc, double rho_param[2])
  {
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (q > 0) {
      rho_param[0] = 2 * sqrt(q / sa);
      rho_param[1] = 0;
    } else if (q < 0) {
      rho_param[0] = 0;
      rho_param[1] = 2 * sqrt(-q / sa);
    }
  }


  void EvtDsToKpipi::propagatorKstr1430(double mass_param, double sx, double* sb, double* sc, double prop[2]) //K*1430 Flatte
  {
    double unit[2] = {1.0};
    double ci[2] = {0, 1};
    double rho1[2];
    Flatte_rhoab(sx, sb[0], sc[0], rho1);
    double rho2[2];
    Flatte_rhoab(sx, sb[1], sc[1], rho2);
    double gKPi_Kstr1430 = 0.2990, gEtaPK_Kstr1430 = 0.0529;
    double tmp1[2] = {gKPi_Kstr1430, 0};
    double tmp11[2];
    double tmp2[2] = {gEtaPK_Kstr1430, 0};
    double tmp22[2];
    Com_Multi(tmp1, rho1, tmp11);
    Com_Multi(tmp2, rho2, tmp22);
    double tmp3[2] = {tmp11[0] + tmp22[0], tmp11[1] + tmp22[1]};
    double tmp31[2];
    Com_Multi(tmp3, ci, tmp31);
    double tmp4[2] = {mass_param* mass_param - sx - tmp31[0], -1.0 * tmp31[1]};
    Com_Divide(unit, tmp4, prop);
  }

  double EvtDsToKpipi::DDalitz(double P1[4], double P2[4], double P3[4], int Ang, double mass_param)
  {
    double pR[4], pD[4];
    double temp_PDF, v_re;
    temp_PDF = 0.0;
    v_re = 0.0;
    double B[2], s1, s2, s3, sR, sD;
    for (int i = 0; i < 4; i++) {
      pR[i] = P1[i] + P2[i];
      pD[i] = pR[i] + P3[i];
    }
    s1 = SCADot(P1, P1);
    s2 = SCADot(P2, P2);
    s3 = SCADot(P3, P3);
    sR = SCADot(pR, pR);
    sD = SCADot(pD, pD);
    int GG[4][4];
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        if (i == j) {
          if (i == 0) GG[i][j] = 1;
          else GG[i][j] = -1;
        } else GG[i][j] = 0;
      }
    }
    if (Ang == 0) {
      B[0] = 1;
      B[1] = 1;
      temp_PDF = 1;
    }
    if (Ang == 1) {
      B[0] = barrier(1, sR, s1, s2, 3.0, mass_param);
      B[1] = barrier(1, sD, sR, s3, 5.0, mDsM);
      double t1[4], T1[4];
      calt1(P1, P2, t1);
      calt1(pR, P3, T1);
      temp_PDF = 0;
      for (int i = 0; i < 4; i++) {
        temp_PDF += t1[i] * T1[i] * GG[i][i];
      }
    }
    if (Ang == 2) {
      B[0] = barrier(2, sR, s1, s2, 3.0, mass_param);
      B[1] = barrier(2, sD, sR, s3, 5.0, mDsM);
      double t2[4][4], T2[4][4];
      calt2(P1, P2, t2);
      calt2(pR, P3, T2);
      temp_PDF = 0;
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          temp_PDF += t2[i][j] * T2[j][i] * GG[i][i] * GG[j][j];
        }
      }
    }
    v_re = temp_PDF * B[0] * B[1];
    return v_re;
  }


  void EvtDsToKpipi::rhoab(double sa, double sb, double sc, double res[2])
  {
    double tmp = sa + sb - sc;
    double q = 0.25 * tmp * tmp / sa - sb;
    if (q >= 0) {
      res[0] = 2.0 * sqrt(q / sa);
      res[1] = 0.0;
    } else {
      res[0] = 0.0;
      res[1] = 2.0 * sqrt(-q / sa);
    }
  }
  void EvtDsToKpipi::rho4Pi(double sa, double res[2])
  {
    double temp = 1.0 - 0.3116765584 / sa;
    if (temp >= 0) {
      res[0] = sqrt(temp) / (1.0 + exp(9.8 - 3.5 * sa));
      res[1] = 0.0;
    } else {
      res[0] = 0.0;
      res[1] = sqrt(-temp) / (1.0 + exp(9.8 - 3.5 * sa));
    }
  }

  void EvtDsToKpipi::propagatorsigma500(double sa, double sb, double sc, double prop[2])
  {
    double f = 0.5843 + 1.6663 * sa;
    const double M = 0.9264;
    const double mass2 = 0.85821696;
    const double mpi2d2 = 0.00973989245;
    double g1 = f * (sa - mpi2d2) / (mass2 - mpi2d2) * exp((mass2 - sa) / 1.082);
    double rho1s[2], rho1M[2], rho2s[2], rho2M[2], rho1[2], rho2[2];
    rhoab(sa, sb, sc, rho1s);
    rhoab(mass2, sb, sc, rho1M);
    rho4Pi(sa, rho2s);
    rho4Pi(mass2, rho2M);
    Com_Divide(rho1s, rho1M, rho1);
    Com_Divide(rho2s, rho2M, rho2);
    double a[2], b[2];
    a[0] = 1.0;
    a[1] = 0.0;
    b[0] = mass2 - sa + M * (g1 * rho1[1] + 0.0024 * rho2[1]);
    b[1] =         -M * (g1 * rho1[0] + 0.0024 * rho2[0]);
    Com_Divide(a, b, prop);
  }


  void EvtDsToKpipi::calEva(double* K, double* Pi1, double* Pi2, double* mass1, double* width1, double* amp, double* phase, int* g0,
                            int* spin_param, int* modetype_param, int nstates, double& Result)
  {
    double P23[4], P13[4];
    double cof[2], amp_PDF[2], PDF[2];
    double s13, s23;
    for (int i = 0; i < 4; i++) {
      P13[i] = K[i] + Pi2[i];
      P23[i] = Pi1[i] + Pi2[i];
    }
    s13  = SCADot(P13, P13);
    s23  = SCADot(P23, P23);
    double s1, s2, s3;
    s1 = SCADot(K, K);
    s2 = SCADot(Pi1, Pi1);
    s3 = SCADot(Pi2, Pi2);
    double pro[2], temp_PDF, amp_tmp[2];
    amp_PDF[0] = 0;
    amp_PDF[1] = 0;
    PDF[0] = 0;
    PDF[1] = 0;
    amp_tmp[0] = 0;
    amp_tmp[1] = 0;
    double rRess = 9.0;
    for (int i = 0; i < nstates; i++) {
      amp_tmp[0] = 0;
      amp_tmp[1] = 0;
      cof[0] = amp[i] * cos(phase[i]);
      cof[1] = amp[i] * sin(phase[i]);
      temp_PDF = 0;

      if (modetype_param[i] == 13) {
        temp_PDF = DDalitz(K, Pi2, Pi1, spin_param[i], mass1[i]);
        if (g0[i] == 1) propagatorRBW(mass1[i], width1[i], s13, mKa2, mPi2, rRess, spin_param[i], pro);
        if (g0[i] == 2) { // K*1430 Flatte
          double skm2[2] = {s1, 0.95778 * 0.95778};
          double spi2[2] = {s3, mass_Kaon * mass_Kaon};
          propagatorKstr1430(mass1[i], s13, skm2, spi2, pro);
        }
        if (g0[i] == 0) {
          pro[0] = 1;
          pro[1] = 0;
        }
        amp_tmp[0] =  temp_PDF * pro[0];
        amp_tmp[1] =  temp_PDF * pro[1];
      }

      if (modetype_param[i] == 23) {
        temp_PDF = DDalitz(Pi1, Pi2, K, spin_param[i], mass1[i]);
        if (g0[i] == 4) propagatorFlatte(mass1[i], width1[i], s23, pro); // Only for f0(980)
        if (g0[i] == 3) propagatorCBW(mass1[i], width1[i], s23, pro);
        if (g0[i] == 2) propagatorRBW(mass1[i], width1[i], s23, mPi2, mPi2, rRess, spin_param[i], pro);
        if (g0[i] == 1) propagatorGS(mass1[i], width1[i], s23, mPi2, mPi2, 9.0, pro);
        if (g0[i] == 500) propagatorsigma500(s23, s2, s3, pro);
        if (g0[i] == 0) {
          pro[0] = 1;
          pro[1] = 0;
        }
        amp_tmp[0] = temp_PDF * pro[0];
        amp_tmp[1] = temp_PDF * pro[1];
      }
      Com_Multi(amp_tmp, cof, amp_PDF);
      PDF[0] += amp_PDF[0];
      PDF[1] += amp_PDF[1];
    }

    double value = PDF[0] * PDF[0] + PDF[1] * PDF[1];
    if (value <= 0) value = 1e-20;
    Result = value;
  }

} // Belle2 namespace
