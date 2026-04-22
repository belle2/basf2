#include <iomanip>
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
#include <EvtGenBase/EvtFlatte.hh>
#include <EvtGenBase/EvtDecayTable.hh>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/besiii/EvtDsToKKpi.h>

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtDsToKKpi);

  EvtDsToKKpi::~EvtDsToKKpi() {}

  std::string EvtDsToKKpi::getName()
  {
    return "DsToKKpi";
  }

  EvtDecayBase* EvtDsToKKpi::clone()
  {
    return new EvtDsToKKpi;
  }

  void EvtDsToKKpi::init()
  {
    checkNArg(0);
    checkNDaug(3);
    checkSpinParent(EvtSpinType::SCALAR);
    checkSpinDaughter(0, EvtSpinType::SCALAR);
    checkSpinDaughter(1, EvtSpinType::SCALAR);
    checkSpinDaughter(2, EvtSpinType::SCALAR);


    phi[0] = 0;
    phi[1] = 6.1944e+00;
    phi[2] = 4.7398e+00;
    phi[3] = 2.9047e+00;
    phi[4] = 1.0068e+00;
    phi[5] = 5.8035e-01;

    rho[0] = 1;
    rho[1] = 1.0963e+00;
    rho[2] = 2.7818e+00;
    rho[3] = 1.2570e+00;
    rho[4] = 7.7351e-01;
    rho[5] = 5.6277e-01;

    mass[0] = 8.9581e-01;
    mass[1] = 1.019461e+00;
    mass[2] = 0.919;
    mass[3] = 1.4712e+00;
    mass[4] = 1.7220e+00;
    mass[5] = 1.3500e+00;

    width[0] = 4.7400e-02;
    width[1] = 4.2660e-03;
    width[2] = 0.272;
    width[3] = 2.7000e-01;
    width[4] = 1.3500e-01;
    width[5] = 2.6500e-01;

    modetype[0] = 0;
    modetype[1] = 1;
    modetype[2] = 13;
    modetype[3] = 3;
    modetype[4] = 4;
    modetype[5] = 5;


    mD = 1.86484;
    mDs = 1.96835;
    rRes = 1.5;
    rD = 3.0;
    mkstr = 0.89555;
    mk0 = 0.497611;
    mass_Kaon = 0.49368;
    mass_Pion = 0.13957;
    mass_Pi0 = 0.1349768;
    mass_EtaP = 0.95778;
    mass_Eta = 0.547862;
    math_pi = 3.1415926;
    afRatio = 2.04835;   //BABAR // afRatio= 1.23202;  //BES2
    int GG[4][4] = { {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, -1} };
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        G[i][j] = GG[i][j];
      }
    }
  }

  void EvtDsToKKpi::initProbMax()
  {
    setProbMax(35638.0);
  }

  void EvtDsToKKpi::decay(EvtParticle* p)
  {
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R D1 = p->getDaug(0)->getP4();
    EvtVector4R D2 = p->getDaug(1)->getP4();
    EvtVector4R D3 = p->getDaug(2)->getP4();
    double P1[4], P2[4], P3[4];
    P1[1] = D1.get(1); P1[2] = D1.get(2); P1[3] = D1.get(3);
    P2[1] = D2.get(1); P2[2] = D2.get(2); P2[3] = D2.get(3);
    P3[1] = D3.get(1); P3[2] = D3.get(2); P3[3] = D3.get(3);

    double value;
    int g0[6] = {1, 1, 1, 1, 1, 1};
    int nstates = 6;
    calEvaMy(P1, P2, P3, mass, width, rho, phi, g0, modetype, nstates, value);
    setProb(value);
    return ;
  }

  void EvtDsToKKpi::MIP_LineShape(double sa, double pro[2])
  {
    double m0 = mass[2], cKK = width[2];
    pro[0] = sqrt(1 / (((m0 * m0) - sa) * ((m0 * m0) - sa) + (cKK * m0 * sqrt(1 - 4 * mass_Kaon * mass_Kaon / sa)) * (cKK * m0 * sqrt(
                         1 - 4 * mass_Kaon * mass_Kaon / sa))));
    pro[1] = 0;
  }
  void EvtDsToKKpi::calEvaMy(double* pKm, double* pKp, double* pPi, double* mass1, double* width1, double* amp, double* phase,
                             int* g0, int* modetype_param, int nstates, double& Result)  // Renamed modetype to modetype_param to avoid shadowing member variable
  {
    double pF0_980[4], pPhi1020[4], pKstr[4], pD[4], p23[4];
    pKp[0] = sqrt(mass_Kaon * mass_Kaon + pKp[1] * pKp[1] + pKp[2] * pKp[2] + pKp[3] * pKp[3]);
    pKm[0] = sqrt(mass_Kaon * mass_Kaon + pKm[1] * pKm[1] + pKm[2] * pKm[2] + pKm[3] * pKm[3]);
    pPi[0] = sqrt(mass_Pion * mass_Pion + pPi[1] * pPi[1] + pPi[2] * pPi[2] + pPi[3] * pPi[3]);
    for (int u = 0; u < 4; u++) {
      pF0_980[u] = pKp[u] + pKm[u];
      pPhi1020[u] = pKp[u] + pKm[u];
      pKstr[u] = pKm[u] + pPi[u];
      p23[u] = pKp[u] + pPi[u];
      pD[u] = pKp[u] + pKm[u] + pPi[u];
    }
    double cof[2], amp_tmp1[2], amp_tmp[2], amp_PDF[2], PDF[2];
    amp_PDF[0] = 0;
    amp_PDF[1] = 0;
    PDF[0] = 0;
    PDF[1] = 0;
    double temp_PDF, tmp1;
    double pro[2], B[3];
    double t1kstr1[4], t1phi1020[4], t1D1[4], t1D2[4];

    double sD, sF0_980, sF0_1710, sF0_1370, sKp, sKm, sPi, sKstr, sPhi1020, sKstr1430;
    sF0_980 = SCADot(pF0_980, pF0_980);
    sF0_1710 = sF0_980;
    sF0_1370 = sF0_980;
    sKstr = SCADot(pKstr, pKstr);
    sD = SCADot(pD, pD);
    sKstr1430 = sKstr;
    sPhi1020 = SCADot(pPhi1020, pPhi1020);
    sKp = SCADot(pKp, pKp);
    sKm = SCADot(pKm, pKm);
    sPi = SCADot(pPi, pPi);

    calt1(pKm, pPi, t1kstr1);
    calt1(pKm, pKp, t1phi1020);
    calt1(pKstr, pKp, t1D1);
    calt1(pPhi1020, pPi, t1D2);

    for (int i = 0; i < nstates; i++) {
      amp_tmp[0] = 0;
      amp_tmp[1] = 0;
      amp_tmp1[0] = 0;
      amp_tmp1[1] = 0;
      tmp1 = 0;
      temp_PDF = 0;
      cof[0] = amp[i] * cos(phase[i]);
      cof[1] = amp[i] * sin(phase[i]);
      if (modetype_param[i] == 13) {
        //a0(980) and f0(980) mixture
        double amp_a0[2] = {0};
        double sa0_980 = sF0_980;
        MIP_LineShape(sa0_980, pro);
        B[0] = barrier(0, sa0_980, sKp, sKm, rRes);
        temp_PDF = 1;
        tmp1 = temp_PDF * B[0];
        amp_a0[0] = tmp1 * pro[0];
        amp_a0[1] = tmp1 * pro[1];
        amp_tmp1[0] = amp_a0[0] ;
        amp_tmp1[1] = amp_a0[1] ;
      } else if (modetype_param[i] == 0) {
        //K*(892) K+
        if (g0[i] == 0) {
          pro[0] = 1;
          pro[1] = 0;
        }
        bool neo = true;
        if (neo) {
          double sBC = SCADot(p23, p23);
          if (g0[i] == 1) propagatorRBWNeoKstr892(mass1[i], width1[i], sKstr, sPi, sKm, rRes, 1, pro);
          B[0] = barrierNeo(1, sKstr, sPi, sKm, rRes, mass1[i]);
          B[1] = barrierNeoDs(1, sD, sKstr, sKp, rD, mDs, mass1[i]);
          temp_PDF = (sBC - sPhi1020 + ((sD - sKp) * (sKm - sPi) / (sKstr)));
        } else {
          if (g0[i] == 1) propagatorRBW(mass1[i], width1[i], sKstr, sKm, sPi, rRes, 1, pro);
          B[0] = barrier(1, sKstr, sKm, sPi, rRes);
          B[1] = barrier(1, sD, sKstr, sKp, rD);
          for (int m = 0; m < 4; m++) {
            for (int j = 0; j < 4; j++) {
              temp_PDF += t1D1[m] * t1kstr1[j] * G[m][j];
            }
          }
        }
        tmp1 = temp_PDF * B[0] * B[1];
        amp_tmp1[0] = tmp1 * pro[0];
        amp_tmp1[1] = tmp1 * pro[1];
      } else if (modetype_param[i] == 1) {
        if (g0[i] == 0) {
          pro[0] = 1;
          pro[1] = 0;
        }
        bool neo = true;
        if (neo) {
          if (g0[i] == 1) propagatorRBWNeo(mass1[i], width1[i], sPhi1020, sKm, sKp, rRes, 1, pro);
          B[0] = barrierNeo(1, sPhi1020, sKp, sKm, rRes, mass1[i]);
          B[1] = barrierNeoDs(1, sD, sPhi1020, sPi, rD, mDs, mass1[i]);
          double sBC = SCADot(p23, p23);
          temp_PDF = (sKstr - sBC + ((sD - sPi) * (sKp - sKm) / (sKstr)));
        } else {
          if (g0[i] == 1) propagatorRBW(mass1[i], width1[i], sPhi1020, sKm, sKp, rRes, 1, pro);
          B[0] = barrier(1, sPhi1020, sKp, sKm, rRes);
          B[1] = barrier(1, sD, sPhi1020, sPi, rD);
          for (int m = 0; m < 4; m++) {
            for (int j = 0; j < 4; j++) {
              temp_PDF += t1D2[m] * t1phi1020[j] * G[m][j];
            }
          }
        }

        tmp1 = temp_PDF * B[0] * B[1];
        amp_tmp1[0] = tmp1 * pro[0];
        amp_tmp1[1] = tmp1 * pro[1];
      } else if (modetype_param[i] == 3) {
        //Kstr1430 K S
        double sKm2[2] = {sKm, mass_EtaP * mass_EtaP};
        double sPi2[2] = {sPi, mass_Kaon * mass_Kaon};
        propagatorKstr1430(mass1[i], sKstr1430, sKm2, sPi2, pro);
        B[0] = barrier(0, sPhi1020, sKp, sKm, rRes);
        tmp1 = 1 * B[0];
        amp_tmp1[0] = tmp1 * pro[0];
        amp_tmp1[1] = tmp1 * pro[1];

      } else if (modetype_param[i] == 4) {
        if (g0[i] == 1) propagatorRBWNeo(mass1[i], width1[i], sF0_1710, sKp, sKm, rRes, 0, pro);
        if (g0[i] == 0) {
          pro[0] = 1;
          pro[1] = 0;
        }
        B[0] = barrier(0, sF0_1710, sKp, sKm, rRes);
        temp_PDF = 1;
        tmp1 = temp_PDF * B[0];
        amp_tmp1[0] = tmp1 * pro[0];
        amp_tmp1[1] = tmp1 * pro[1];
      } else if (modetype_param[i] == 5) {
        //f0(1370) Pi+  S
        if (g0[i] == 1) propagatorRBWNeo(mass1[i], width1[i], sF0_1370, sKp, sKm, rRes, 0, pro);
        if (g0[i] == 0) {
          pro[0] = 1;
          pro[1] = 0;
        }
        B[0] = barrier(0, sF0_1370, sKp, sKm, rRes);
        tmp1 = 1 * B[0];
        amp_tmp1[0] = tmp1 * pro[0];
        amp_tmp1[1] = tmp1 * pro[1];
      }
      amp_tmp[0] = amp_tmp1[0];
      amp_tmp[1] = amp_tmp1[1];
      Com_Multi(amp_tmp, cof, amp_PDF);
      PDF[0] += amp_PDF[0];
      PDF[1] += amp_PDF[1];
    }

    double value = PDF[0] * PDF[0] + PDF[1] * PDF[1];
    if (value <= 0) {
      value = 1e-20;
    }
    Result = value;
  }

  void EvtDsToKKpi::Com_Multi(double a1[2], double a2[2], double res[2])
  {
    res[0] = a1[0] * a2[0] - a1[1] * a2[1];
    res[1] = a1[1] * a2[0] + a1[0] * a2[1];
  }
  void EvtDsToKKpi::Com_Divide(double a1[2], double a2[2], double res[2])
  {
    res[0] = (a1[0] * a2[0] + a1[1] * a2[1]) / (a2[0] * a2[0] + a2[1] * a2[1]);
    res[1] = (a1[1] * a2[0] - a1[0] * a2[1]) / (a2[0] * a2[0] + a2[1] * a2[1]);
  }

  double EvtDsToKKpi::SCADot(double a1[4], double a2[4])
  {
    double _cal = 0.;
    _cal = a1[0] * a2[0] - a1[1] * a2[1] - a1[2] * a2[2] - a1[3] * a2[3];
    return _cal;
  }
  double EvtDsToKKpi::barrier(int l, double sa, double sb, double sc, double r)
  {
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (q < 0) q = 1e-16;
    double z;
    z = q * r * r;
    double F = 0.0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt(2 * z / (1 + z));
    if (l == 2) F = sqrt(13 * z * z / (9 + 3 * z + z * z));
    return F;
  }
  double EvtDsToKKpi::barrierNeo(int l, double sa, double sb, double sc, double r, double mR)
  {
    double pAB = ((sa - sb - sc) * (sa - sb - sc) / 4.0 - (sb * sc)) / sa;
    double pR = ((mR * mR - sb - sc) * (mR * mR - sb - sc) / 4.0 - (sb * sc)) / (mR * mR);
    double zAB = pAB * r * r;
    double zR = pR * r * r;
    double F = 0.0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + zR) / (1 + zAB));
    if (l == 2) F = sqrt((9 + 3 * zAB + zAB * zAB) / (9 + 3 * zAB + zAB * zAB));
    return F;
  }
  double EvtDsToKKpi::barrierNeoDs(int l, double sa, double sb, double sc, double r, double mR, double mb)
  {
    double pAB = ((sa - sb - sc) * (sa - sb - sc) / 4.0 - (sb * sc)) / sa;
    double pR = ((sa - mb * mb - sc) * (sa - mb * mb - sc) / 4.0 - (mb * mb * sc)) / (mR * mR);
    double zAB = pAB * r * r;
    double zR = pR * r * r;
    double F = 0.0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + zR) / (1 + zAB));
    if (l == 2) F = sqrt((9 + 3 * zAB + zAB * zAB) / (9 + 3 * zAB + zAB * zAB));
    return F;
  }
//------------------spin-------------------------------------------
  void EvtDsToKKpi::calt1(double daug1[4], double daug2[4], double t1[4])
  {
    double p, pq;
    double pa[4], qa[4];
    for (int i = 0; i < 4; i++) {
      pa[i] = daug1[i] + daug2[i];
      qa[i] = daug1[i] - daug2[i];
    }
    p = SCADot(pa, pa);
    pq = SCADot(pa, qa);
    for (int i = 0; i < 4; i++) {
      t1[i] = qa[i] - pq / p * pa[i];
    }
  }
  void EvtDsToKKpi::calt2(double daug1[4], double daug2[4], double t2[4][4])
  {
    double p, r;
    double pa[4], t1[4];
    calt1(daug1, daug2, t1);
    r = SCADot(t1, t1);
    for (int i = 0; i < 4; i++) {
      pa[i] = daug1[i] + daug2[i];
    }
    p = SCADot(pa, pa);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        t2[i][j] = t1[i] * t1[j] - 1.0 / 3 * r * (G[i][j] - pa[i] * pa[j] / p);
      }
    }
  }
//-------------------prop--------------------------------------------
  void EvtDsToKKpi::propagator(double mass_param, double width_param, double sx, double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass_param * mass_param - sx;
    b[1] = -mass_param * width_param;
    Com_Divide(a, b, prop);
  }
  double EvtDsToKKpi::wid(double mass_param, double sa, double sb, double sc, double r, int l)
  {
    double widm = 0.;
    double q = 0.;
    double q0 = 0.;
    double sa0 = mass_param * mass_param;
    double m = sqrt(sa);
    q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (q < 0) q = 1e-16;
    q0 = (sa0 + sb - sc) * (sa0 + sb - sc) / (4 * sa0) - sb;
    if (q0 < 0) q0 = 1e-16;
    double z = q * r * r;
    double z0 = q0 * r * r;
    double F = 0;
    if (l == 0) F = 1;
    if (l == 1) F = sqrt((1 + z0) / (1 + z));
    if (l == 2) F = sqrt((9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z));
    double t = q / q0;
    widm = pow(t, l + 0.5) * mass_param / m * F * F;
    return widm;
  }

  void  EvtDsToKKpi::Flatte_rhoab(double sa, double sb, double sc, double rho_param[2])
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

  void EvtDsToKKpi::propagatorFlatte(double mass_param, double width_param __attribute__((unused)), double sx, double* sb, double* sc,
                                     double prop[2])
  {
    double unit[2] = {1.0};
    double ci[2] = {0, 1};
    double rho1[2];
    Flatte_rhoab(sx, sb[0], sc[0], rho1);
    double rho2[2];
    Flatte_rhoab(sx, sb[1], sc[1], rho2);
    double g1_f980 = 0.165, g2_f980 = 0.69465;
    double tmp1[2] = {g1_f980, 0};
    double tmp11[2];
    double tmp2[2] = {g2_f980, 0};
    double tmp22[2];
    Com_Multi(tmp1, rho1, tmp11);
    Com_Multi(tmp2, rho2, tmp22);
    double tmp3[2] = {tmp11[0] + tmp22[0], tmp11[1] + tmp22[1]};
    double tmp31[2];
    Com_Multi(tmp3, ci, tmp31);
    double tmp4[2] = {mass_param* mass_param - sx - tmp31[0], -1.0 * tmp3[1]};
    Com_Divide(unit, tmp4, prop);
  }
  void EvtDsToKKpi::propagator980(double mass_param, double sx, double* sb, double* sc, double prop[2])
  {
    double unit[2] = {1.0};
    double ci[2] = {0, 1};
    double rho1[2];
    Flatte_rhoab(sx, sb[0], sc[0], rho1);
    double rho2[2];
    Flatte_rhoab(sx, sb[1], sc[1], rho2);
    double gK_f980 = 0.69466, gPi_f980 = 0.165;
    double tmp1[2] = {gK_f980, 0};
    double tmp11[2];
    double tmp2[2] = {gPi_f980, 0};
    double tmp22[2];
    Com_Multi(tmp1, rho1, tmp11);
    Com_Multi(tmp2, rho2, tmp22);
    double tmp3[2] = {tmp11[0] + tmp22[0], tmp11[1] + tmp22[1]};
    double tmp31[2];
    Com_Multi(tmp3, ci, tmp31);
    double tmp4[2] = {mass_param* mass_param - sx - tmp31[0], -1.0 * tmp31[1]};
    Com_Divide(unit, tmp4, prop);
  }

  void EvtDsToKKpi::propagatora0980(double mass_param, double sx, double* sb, double* sc, double prop[2])
  {
    double unit[2] = {1.0};
    double ci[2] = {0, 1};
    double rho1[2];
    Flatte_rhoab(sx, sb[0], sc[0], rho1);
    double rho2[2];
    Flatte_rhoab(sx, sb[1], sc[1], rho2);
    double gKK_a980 = 0.892 * 0.341, gPiEta_a980 = 0.341;
    double tmp1[2] = {gKK_a980, 0};
    double tmp11[2];
    double tmp2[2] = {gPiEta_a980, 0};
    double tmp22[2];
    Com_Multi(tmp1, rho1, tmp11);
    Com_Multi(tmp2, rho2, tmp22);
    double tmp3[2] = {tmp11[0] + tmp22[0], tmp11[1] + tmp22[1]};
    double tmp31[2];
    Com_Multi(tmp3, ci, tmp31);
    double tmp4[2] = {mass_param* mass_param - sx - tmp31[0], -1.0 * tmp31[1]};
    Com_Divide(unit, tmp4, prop);
  }

  void EvtDsToKKpi::propagatorKstr1430(double mass_param, double sx, double* sb, double* sc, double prop[2])
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

  void EvtDsToKKpi::propagatorRBW(double mass_param, double width_param, double sa, double sb, double sc, double r, int l,
                                  double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass_param * mass_param - sa;
    b[1] = -mass_param * width_param * wid(mass_param, sa, sb, sc, r, l);
    Com_Divide(a, b, prop);
  }

  void EvtDsToKKpi::propagatorRBWNeo(double mass_param, double width_param, double sa, double sb, double sc,
                                     double r __attribute__((unused)), int l, double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass_param * mass_param - sa;
    double tmp1 = (sa - sb - sc);
    double tmp2 = sb * sc;
    double pAB = sqrt((tmp1 * tmp1 / 4.0 - tmp2) / sa);
    double pR = sqrt(((mass_param * mass_param - sb - sc) * (mass_param * mass_param - sb - sc) / 4.0 - (sb * sc)) /
                     (mass_param * mass_param));
    double fR = sqrt(1.0 + 1.5 * 1.5 * pR * pR) / sqrt(1.0 + 1.5 * 1.5 * pAB * pAB);
    double power = 1;
    if (!l) {
      power = 1;
      fR = 1;
    } else if (l == 1) {
      power = 3;
    }
    double gammaAB = width_param * pow(pAB / pR, power) * (mass_param / sqrt(sa)) * fR * fR;
    b[1] = -mass_param * gammaAB;
    Com_Divide(a, b, prop);
  }

  void EvtDsToKKpi::propagatorRBWNeoKstr892(double mass_param, double width_param, double sa, double sb, double sc,
                                            double r __attribute__((unused)), int l, double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass_param * mass_param - sa;
    double tmp1 = (sa - sb - sc);
    double tmp2 = sb * sc;
    double pAB = sqrt((tmp1 * tmp1 / 4.0 - tmp2) / sa);
    double pR = sqrt(((mass_param * mass_param - sb - sc) * (mass_param * mass_param - sb - sc) / 4.0 - (sb * sc)) /
                     (mass_param * mass_param));
    double fR = sqrt(1.0 + 1.5 * 1.5 * pR * pR) / sqrt(1.0 + 1.5 * 1.5 * pAB * pAB);
    double power = 1;
    if (!l) {
      power = 1;
    } else if (l == 1) {
      power = 3;
    }
    double gammaAB = width_param * pow(pAB / pR, power) * (mass_param / sqrt(sa)) * fR * fR;
    b[1] = -mass_param * gammaAB;
    Com_Divide(a, b, prop);
  }

//------------GS---used by rho----------------------------
  double EvtDsToKKpi::h(double m, double q)
  {
    double h = 2 / math_pi * q / m * log((m + 2 * q) / (2 * mass_Pion));
    return h;
  }
  double EvtDsToKKpi::dh(double mass_param, double q0)
  {
    double dh = h(mass_param, q0) * (1.0 / (8 * q0 * q0) - 1.0 / (2 * mass_param * mass_param)) + 1.0 /
                (2 * math_pi * mass_param * mass_param);
    return dh;
  }
  double EvtDsToKKpi::f(double mass_param, double sx, double q0, double q)
  {
    double m = sqrt(sx);
    double f = mass_param * mass_param / (pow(q0, 3)) * (q * q * (h(m, q) - h(mass_param,
                                                         q0)) + (mass_param * mass_param - sx) * q0 * q0 * dh(mass_param, q0));
    return f;
  }
  double EvtDsToKKpi::d(double mass_param, double q0)
  {
    double d = 3.0 / math_pi * mass_Pion * mass_Pion / (q0 * q0) * log((mass_param + 2 * q0) / (2 * mass_Pion)) + mass_param /
               (2 * math_pi * q0)
               - (mass_Pion * mass_Pion * mass_param) / (math_pi * pow(q0, 3));
    return d;
  }

//rho
  void EvtDsToKKpi::propagatorGS(double mass_param, double width_param, double sa, double sb, double sc, double r, int l,
                                 double prop[2])
  {
    double a[2], b[2];
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    double sa0 = mass_param * mass_param;
    double q0 = (sa0 + sb - sc) * (sa0 + sb - sc) / (4 * sa0) - sb;
    if (q < 0) q = 1e-16;
    if (q0 < 0) q0 = 1e-16;
    q = sqrt(q);
    q0 = sqrt(q0);
    a[0] = 1 + d(mass_param, q0) * width_param / mass_param;
    a[1] = 0;
    b[0] = mass_param * mass_param - sa + width_param * f(mass_param, sa, q0, q);
    b[1] = -mass_param * width_param * wid(mass_param, sa, sb, sc, r, l);
    Com_Divide(a, b, prop);
  }



} // Belle2 namespace
