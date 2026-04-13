//--------------------------------------------------------------------------
//
// Model: EvtDTopipi0Eta
// This file is an amplitude model for D+ -> pi+ pi0 eta.
// The model is from Phys.Rev.D.110.L111102 (2024). DOI: https://doi.org/10.1103/PhysRevD.110.L111102
//
// Permission to use these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.
//
// Modified by : Jiyuan Zhang
//
//--------------------------------------------------------------------------

#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtVector4R.hh"
#include "EvtGenBase/EvtComplex.hh"
#include "generators/evtgen/models/besiii/EvtDTopipi0Eta.h"
#include "EvtGenBase/EvtDecayTable.hh"
#include <stdlib.h>

#include <generators/evtgen/EvtGenModelRegister.h>

using namespace std;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtDTopipi0Eta);

  EvtDTopipi0Eta::~EvtDTopipi0Eta() {}

  std::string EvtDTopipi0Eta::getName()
  {
    return "DTopipi0Eta";
  }

  EvtDecayBase* EvtDTopipi0Eta::clone()
  {
    return new EvtDTopipi0Eta;
  }

  void EvtDTopipi0Eta::init()
  {
    checkNArg(0);
    checkNDaug(3);
    checkSpinParent(EvtSpinType::SCALAR);
    checkSpinDaughter(0, EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::SCALAR);
    checkSpinDaughter(2, EvtSpinType::SCALAR);

    phi[0] = -3.3276; rho[0] =  0.31478; //rho eta
    phi[1] =  0.0;    rho[1] =  1.0;     //a0+ pi0
    phi[2] =  0.0;    rho[2] = -1.0;     //a00 pi+

    mrho = 0.77511;
    ma0 = 0.99;
    Grho = 0.1491;
    Ga0 = 0.0756;

    const double mk0 = 0.497614;
    const double mass_Kaon = 0.49368;
    const double mass_Pion = 0.13957;
    const double mass_Pi0 = 0.1349766;
    const double meta = 0.547862;
    mpi = 0.13957;
    mD = 1.86966;
    sD = mD * mD;
    spi = mpi * mpi;
    snk = mk0 * mk0;
    sck = mass_Kaon * mass_Kaon;
    scpi = mass_Pion * mass_Pion;
    snpi = mass_Pi0 * mass_Pi0;
    seta = meta * meta;

    pi = 3.1415926;

    ci  = EvtComplex(0.0, 1.0);
    one = EvtComplex(1.0, 0.0);

    int GG[4][4] = { {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, -1} };
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        G[i][j] = GG[i][j];
      }
    }

  }

  void EvtDTopipi0Eta::initProbMax()
  {
    setProbMax(20.0);
  }

  void EvtDTopipi0Eta::decay(EvtParticle* p)
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
    value = calDalEva(P1, P2, P3);
    setProb(value);
    return;

  }

  double EvtDTopipi0Eta::calDalEva(double P1[], double P2[], double P3[])
  {
    //pi+ pi0 eta
    //0: non-resonance
    //1: resonance, RBW
    //2: resonance, GS
    //3: resonance, Flatte
    //4: rho-omega mxing for omega
    EvtComplex PDF[3];
    EvtComplex cof, pdf, module;
    double value;
    PDF[0] = Spin_factor(P1, P2, P3, 1, 2, mrho, Grho); // rho+ eta
    PDF[1] = Spin_factor(P1, P3, P2, 0, 30, ma0,  Ga0);  // a0+ pi0
    PDF[2] = Spin_factor(P2, P3, P1, 0, 31, ma0,  Ga0);  // a00 pi+

    pdf = EvtComplex(0.0, 0.0);
    for (int i = 0; i < 3; i++) {
      cof = EvtComplex(rho[i] * cos(phi[i]), rho[i] * sin(phi[i]));
      pdf = pdf + cof * PDF[i];
    }
    module = conj(pdf) * pdf;
    value = real(module);
    return (value <= 0) ? 1e-20 : value;
  }

  EvtComplex EvtDTopipi0Eta::Spin_factor(double P1[], double P2[], double P3[], int spin, int flag, double mass_R, double width_R)
  {
    //D-> R P3, R->P1 P2, 0: non-resonance 1: resonance, RBW 2: resonance, GS 3: resonance, Flatte 4: rho-omega mxing for omega
    double R[4], s[3], sp2, B[2];
    double tmp;
    for (int i = 0; i < 4; i++) {
      R[i] = P1[i] + P2[i];
    }
    s[0] = dot(R, R);
    s[1] = dot(P1, P1);
    s[2] = dot(P2, P2);
    sp2 = dot(P3, P3);

    EvtComplex amp, prop, prop1, prop2;
    EvtComplex rhokk, rhopieta;
    if (spin == 0) {
      if (flag == 0) prop = one;
      if (flag == 1) prop = propagatorRBW(mass_R, width_R, s[0], s[1], s[2], 3.0, 0);
      if (flag == 30) {
        rhokk = Flatte_rhoab(s[0], snk, sck);
        rhopieta = Flatte_rhoab(s[0], scpi, seta);
        prop = 1.0 / (mass_R * mass_R - s[0] - ci * (0.341 * rhopieta + 0.341 * 0.892 * rhokk));
      }
      if (flag == 31) {
        double qKsK;
        qKsK = 0.25 * (s[0] + 3.899750596e-03) * (s[0] + 3.899750596e-03) / s[0] - 0.497614 * 0.497614;
        if (qKsK > 0) rhokk = 2.0 * sqrt(qKsK / s[0]) * one;
        if (qKsK < 0) rhokk = 2.0 * sqrt(qKsK / s[0]) * ci;
        rhopieta = Flatte_rhoab(s[0], snpi, seta);
        prop = 1.0 / (mass_R * mass_R - s[0] - ci * (0.341 * rhopieta + 0.341 * 0.892 * rhokk));
      }
      amp = prop;
    } else if (spin == 1) {
      if (flag == 0) {
        prop = EvtComplex(1.0, 0.0);
      }
      if (flag == 1) {
        prop = propagatorRBW(mass_R, width_R, s[0], s[1], s[2], 3.0, 1);
      }
      if (flag == 2) {
        prop = propagatorGS(mass_R, width_R, s[0], s[1], s[2], 3.0, 1);
      }
      if (flag == 4) {
        prop1 = propagatorGS(mass_R, width_R, s[0], s[1], s[2], 3.0, 1);
        prop2 = propagatorRBW(0.78266, 0.01358, s[0], s[1], s[2], 3.0, 1);
        prop = prop1 * prop2;
      }
      double T1[4], t1[4];
      calt1(R, P3, T1);
      calt1(P1, P2, t1);
      B[0] = barrier(1, s[0], s[1], s[2], 3.0, mass_R);
      B[1] = barrier(1, sD,  s[0], sp2, 5.0, mD);
      tmp = 0.0;
      for (int i = 0; i < 4; i++) {
        tmp += T1[i] * t1[i] * G[i][i];
      }
      amp = tmp * prop * B[0] * B[1];
    } else if (spin == 2) {
      double T2[4][4], t2[4][4];
      calt2(R, P3, T2);
      calt2(P1, P2, t2);
      B[0] = barrier(2, s[0], s[1], s[2], 3.0, mass_R);
      B[1] = barrier(2, sD,  s[0], sp2, 5.0, mD);
      tmp = 0.0;
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          tmp += T2[i][j] * t2[j][i] * G[j][j] * G[i][i];
        }
      }
      if (flag == 0) prop = one;
      if (flag == 1) prop = propagatorRBW(mass_R, width_R, s[0], s[1], s[2], 3.0, 2);
      amp = tmp * prop * B[0] * B[1];
    } else {
      cout << "Only S, P, D wave allowed" << endl;
    }
    return amp;
  }

  double EvtDTopipi0Eta::dot(double* a1, double* a2)
  {
    double Dot = 0;
    for (int i = 0; i != 4; i++) {
      Dot += a1[i] * a2[i] * G[i][i];
    }
    return Dot;
  }

  double EvtDTopipi0Eta::Qabcs(double sa, double sb, double sc)
  {
    double Qabcs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (Qabcs < 0) Qabcs = 1e-16;
    return Qabcs;
  }

  double EvtDTopipi0Eta::barrier(double l, double sa, double sb, double sc, double r, double mass)
  {
    double sa0 = mass * mass;
    double q0 = Qabcs(sa0, sb, sc);
    double z0 = q0 * r * r;
    double q = Qabcs(sa, sb, sc);
    q = sqrt(q);
    double z = q * r;
    z = z * z;
    double F = 1;
    if (l > 2) F = 0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + z0) / (1 + z));
    if (l == 2) F = sqrt((9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z));
    return F;
  }

  void EvtDTopipi0Eta::calt1(double daug1[], double daug2[], double t1[])
  {
    double p, pq;
    double pa[4], qa[4];
    for (int i = 0; i != 4; i++) {
      pa[i] = daug1[i] + daug2[i];
      qa[i] = daug1[i] - daug2[i];
    }
    p = dot(pa, pa);
    pq = dot(pa, qa);
    for (int i = 0; i != 4; i++) {
      t1[i] = qa[i] - pq / p * pa[i];
    }
  }

  void EvtDTopipi0Eta::calt2(double daug1[], double daug2[], double t2[][4])
  {
    double p, r;
    double pa[4], t1[4];
    calt1(daug1, daug2, t1);
    r = dot(t1, t1);
    for (int i = 0; i != 4; i++) {
      pa[i] = daug1[i] + daug2[i];
    }
    p = dot(pa, pa);
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        t2[i][j] = t1[i] * t1[j] - 1.0 / 3 * r * (G[i][j] - pa[i] * pa[j] / p);
      }
    }
  }

  double EvtDTopipi0Eta::wid(double mass, double sa, double sb, double sc, double r, int l)
  {
    double widm(0.), q(0.), q0(0.);
    double sa0 = mass * mass;
    double m = sqrt(sa);
    q = Qabcs(sa, sb, sc);
    q0 = Qabcs(sa0, sb, sc);
    double z, z0;
    z = q * r * r;
    z0 = q0 * r * r;
    double t = q / q0;
    double F(0.);
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + z0) / (1 + z));
    if (l == 2) F = sqrt((9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z));
    widm = pow(t, l + 0.5) * mass / m * F * F;
    return widm;
  }

  EvtComplex EvtDTopipi0Eta::propagatorRBW(double mass, double width, double sa, double sb, double sc, double r, int l)
  {
    EvtComplex prop = 1.0 / (mass * mass - sa - ci * mass * width * wid(mass, sa, sb, sc, r, l));
    return prop;
  }

  double EvtDTopipi0Eta::h(double m, double q)
  {
    double h = 2.0 / pi * q / m * log((m + 2 * q) / (0.13957 + 0.134976));
    return h;
  }

  double EvtDTopipi0Eta::dh(double mass, double q0)
  {
    double dh = h(mass, q0) * (1.0 / (8 * q0 * q0) - 1.0 / (2 * mass * mass)) + 1.0 / (2 * pi * mass * mass);
    return dh;
  }

  double EvtDTopipi0Eta::f(double mass, double sx, double q0, double q)
  {
    double m = sqrt(sx);
    double f = mass * mass / (pow(q0, 3)) * (q * q * (h(m, q) - h(mass, q0)) + (mass * mass - sx) * q0 * q0 * dh(mass, q0));
    return f;
  }

  double EvtDTopipi0Eta::d(double mass, double q0)
  {
    double cmpi = 0.5 * (0.13957 + 0.134976);
    double mpi2 = cmpi * cmpi;
    double d = 3.0 / pi * mpi2 / (q0 * q0) * log((mass + 2 * q0) / (2 * cmpi)) + mass / (2 * pi * q0) - (mpi2 * mass) / (pi * pow(q0,
               3));
    return d;
  }

  EvtComplex EvtDTopipi0Eta::propagatorGS(double mass, double width, double sa, double sb, double sc, double r, int l)
  {
    double q = Qabcs(sa, sb, sc);
    double sa0 = mass * mass;
    double q0 = Qabcs(sa0, sb, sc);
    q = sqrt(q);
    q0 = sqrt(q0);
    EvtComplex prop = (1 + d(mass, q0) * width / mass) / (mass * mass - sa + width * f(mass, sa, q0, q) - ci * mass * width * wid(mass,
                                                          sa, sb, sc, r, l));
    return prop;
  }

  EvtComplex EvtDTopipi0Eta::Flatte_rhoab(double sa, double sb, double sc)
  {
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    EvtComplex rho_val;
    if (q > 0) {
      rho_val = 2.0 * sqrt(q / sa) * one;
    }
    if (q < 0) {
      rho_val = 2.0 * sqrt(-q / sa) * ci;
    }
    return rho_val;
  }

  EvtComplex EvtDTopipi0Eta::propagatorFlatte(double mass, double width __attribute__((unused)), double sx, double* sb, double* sc)
  {
    const double g1sq = 0.5468 * 0.5468;
    const double g2sq = 0.23 * 0.23;
    EvtComplex rho1 = Flatte_rhoab(sx, sb[0], sc[0]);
    EvtComplex rho2 = Flatte_rhoab(sx, sb[1], sc[1]);
    EvtComplex prop = 1.0 / (mass * mass - sx - ci * (g1sq * rho1 + g2sq * rho2));
    return prop;
  }

} // Belle2 namespace
