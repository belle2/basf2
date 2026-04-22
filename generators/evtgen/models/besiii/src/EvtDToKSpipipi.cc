#include <EvtGenBase/EvtPatches.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtGenKine.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtReport.hh>
#include <EvtGenBase/EvtComplex.hh>
#include <EvtGenBase/EvtDecayTable.hh>
#include <stdlib.h>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/besiii/EvtDToKSpipipi.h>

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtDToKSpipipi);

  EvtDToKSpipipi::~EvtDToKSpipipi() {}

  std::string EvtDToKSpipipi::getName()
  {
    return "DToKSpipipi";
  }

  EvtDecayBase* EvtDToKSpipipi::clone()
  {
    return new EvtDToKSpipipi;
  }

  void EvtDToKSpipipi::init()
  {
    checkNArg(0);
    checkNDaug(4);
    checkSpinParent(EvtSpinType::SCALAR);
    /*
       checkSpinDaughter(0,EvtSpinType::SCALAR);
       checkSpinDaughter(1,EvtSpinType::SCALAR);
       checkSpinDaughter(3,EvtSpinType::SCALAR);
       checkSpinDaughter(4,EvtSpinType::SCALAR);
    */

    mK1270 = 1.272; mK1400 = 1.403;
    GK1270 = 0.09;  GK1400 = 0.174;
    mKstr = 0.89166; mrho = 0.77549;
    GKstr = 0.0462;  Grho = 0.1491;
    msigma = 0.472;
    Gsigma = 0.542;
    phi_omega = -0.02;
    mK1650 = 1.65;
    GK1650 = 0.158;
    rho[0] = 1.0;
    phi[0] = 0.0;

    ma1 = 1.22;
    Ga1 = 0.4282;
    mK1460 = 1.4152;
    GK1460 = 0.2485;
    rho_omega = 0.00294;

    phi[1] =  -1.55;
    rho[1] =   0.5843;

    phi[2] =  -1.8223;
    rho[2] =   2.0974;

    phi[3] =  -2.6751;
    rho[3] =   0.46642;

    phi[4] =  -2.2429;
    rho[4] =   0.33334;

    phi[5] =  -0.55888;
    rho[5] =   0.15549;

    phi[6] =  -1.8778;
    rho[6] =   0.94452;

    phi[7] =   2.7724;
    rho[7] =   0.99411;

    phi[8] =  -2.6461;
    rho[8] =   0.21231;

    phi[9] =  -0.95137;
    rho[9] =   0.29895;

    phi[10] = -3.0843;
    rho[10] =  3.6361;

    phi[11] =  2.0954;
    rho[11] =  0.96472;

    phi[12] = -2.4965;
    rho[12] =  0.48470;

    mD = 1.86486;
    rD = 5;
    metap = 0.95778;
    mkstr = 0.89594;
    mk0 = 0.497614;
    mass_Kaon = 0.49368;
    mass_Pion = 0.13957;
    math_pi = 3.1415926;

    pi = 3.1415926;
    mpi = 0.13957;
    g1 = 0.5468;
    g2 = 0.23;

    int GG[4][4] = { {1, 0, 0, 0}, {0, -1, 0, 0}, {0, 0, -1, 0}, {0, 0, 0, -1} };
    int EE[4][4][4][4] = {
      { {{0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0} },
        {{0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 1},  {0, 0, -1, 0}},
        {{0, 0, 0, 0},  {0, 0, 0, -1}, {0, 0, 0, 0},  {0, 1, 0, 0} },
        {{0, 0, 0, 0},  {0, 0, 1, 0},  {0, -1, 0, 0}, {0, 0, 0, 0} }
      },
      { {{0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, -1}, {0, 0, 1, 0} },
        {{0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0} },
        {{0, 0, 0, 1},  {0, 0, 0, 0},  {0, 0, 0, 0},  {-1, 0, 0, 0}},
        {{0, 0, -1, 0}, {0, 0, 0, 0},  {1, 0, 0, 0},  {0, 0, 0, 0} }
      },
      { {{0, 0, 0, 0},  {0, 0, 0, 1},  {0, 0, 0, 0},  {0, -1, 0, 0}},
        {{0, 0, 0, -1}, {0, 0, 0, 0},  {0, 0, 0, 0},  {1, 0, 0, 0} },
        {{0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0} },
        {{0, 1, 0, 0},  {-1, 0, 0, 0}, {0, 0, 0, 0},  {0, 0, 0, 0} }
      },
      { {{0, 0, 0, 0},  {0, 0, -1, 0}, {0, 1, 0, 0},  {0, 0, 0, 0} },
        {{0, 0, 1, 0},  {0, 0, 0, 0},  {-1, 0, 0, 0}, {0, 0, 0, 0} },
        {{0, -1, 0, 0}, {1, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0} },
        {{0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0},  {0, 0, 0, 0} }
      }
    };
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        G[i][j] = GG[i][j];
        for (int k = 0; k < 4; k++) {
          for (int l = 0; l < 4; l++) {
            E[i][j][k][l] = EE[i][j][k][l];
          }
        }
      }
    }
  }

  void EvtDToKSpipipi::initProbMax()
  {
    setProbMax(3700.0);
  }

  void EvtDToKSpipipi::decay(EvtParticle* p)
  {
    /*
       double maxprob = 0.0;
       for(int ir=0;ir<=60000000;ir++){
          p->initializePhaseSpace(getNDaug(),getDaugs());
          EvtVector4R Ks0 = p->getDaug(0)->getP4();
          EvtVector4R pi1 = p->getDaug(1)->getP4();
          EvtVector4R pi2 = p->getDaug(2)->getP4();
          EvtVector4R pi3 = p->getDaug(3)->getP4();
          double Ks[4],Pip1[4],Pip2[4],Pim[4];
          Ks[0] = Ks0.get(0); Pip1[0] = pi1.get(0); Pip2[0] = pi2.get(0); Pim[0] = pi3.get(0);
          Ks[1] = Ks0.get(1); Pip1[1] = pi1.get(1); Pip2[1] = pi2.get(1); Pim[1] = pi3.get(1);
          Ks[2] = Ks0.get(2); Pip1[2] = pi1.get(2); Pip2[2] = pi2.get(2); Pim[2] = pi3.get(2);
          Ks[3] = Ks0.get(3); Pip1[3] = pi1.get(3); Pip2[3] = pi2.get(3); Pim[3] = pi3.get(3);
          double Prob = calPDF(Ks, Pip1, Pip2, Pim);
          if(Prob>maxprob) {
              maxprob=Prob;
          }
       }
    */
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R Ks0 = p->getDaug(0)->getP4();
    EvtVector4R pi1 = p->getDaug(1)->getP4();
    EvtVector4R pi2 = p->getDaug(2)->getP4();
    EvtVector4R pi3 = p->getDaug(3)->getP4();

    double Ks[4], Pip1[4], Pip2[4], Pim[4];
    Ks[0] = Ks0.get(0); Pip1[0] = pi1.get(0); Pip2[0] = pi2.get(0); Pim[0] = pi3.get(0);
    Ks[1] = Ks0.get(1); Pip1[1] = pi1.get(1); Pip2[1] = pi2.get(1); Pim[1] = pi3.get(1);
    Ks[2] = Ks0.get(2); Pip1[2] = pi1.get(2); Pip2[2] = pi2.get(2); Pim[2] = pi3.get(2);
    Ks[3] = Ks0.get(3); Pip1[3] = pi1.get(3); Pip2[3] = pi2.get(3); Pim[3] = pi3.get(3);
    double prob = calPDF(Ks, Pip1, Pip2, Pim);
    setProb(prob);
    return;
  }

  double EvtDToKSpipipi::calPDF(double Ks[], double Pip1[], double Pip2[], double Pim[])
  {
    EvtComplex PDF[100];
    double P14[4], P24[4], P34[4];
    for (int i = 0; i < 4; i++) {
      P14[i] = Ks[i] + Pim[i];
      P24[i] = Pip1[i] + Pim[i];
      P34[i] = Pip2[i] + Pim[i];
    }
    //----------D->a1Ks--------------
    //----------a1->rhoPi------------
    PDF[0] = D2AP_A2VP(Ks, Pip2, Pip1, Pim, 0) * getprop(P24, Pip2, ma1, Ga1, 1, 0) *
             getprop(Pip1, Pim, mrho, Grho, 2, 1) +
             D2AP_A2VP(Ks, Pip1, Pip2, Pim, 0) * getprop(P34, Pip1, ma1, Ga1, 1, 0) *
             getprop(Pip2, Pim, mrho, Grho, 2, 1);
    PDF[1] = D2AP_A2VP(Ks, Pip2, Pip1, Pim, 2) * getprop(P24, Pip2, ma1, Ga1, 1, 2) *
             getprop(Pip1, Pim, mrho, Grho, 2, 1) +
             D2AP_A2VP(Ks, Pip1, Pip2, Pim, 2) * getprop(P34, Pip1, ma1, Ga1, 1, 2) *
             getprop(Pip2, Pim, mrho, Grho, 2, 1);
    //----------a1->sigma pi---------
    PDF[2] = D2AP_A2SP(Ks, Pip2, Pip1, Pim) * getprop(P24, Pip2, ma1, Ga1, 1, 1) *
             getprop(Pip1, Pim, msigma, Gsigma, 4, 0) +
             D2AP_A2SP(Ks, Pip1, Pip2, Pim) * getprop(P34, Pip1, ma1, Ga1, 1, 1) *
             getprop(Pip2, Pim, msigma, Gsigma, 4, 0);
    //----------D->a1K finish-----
    //---------D->K1(1400) pi-----
    //K1400[S]->K* pi
    PDF[3] = D2AP_A2VP(Pip2, Pip1, Ks, Pim, 0) * getprop(P14, Pip1, mK1400, GK1400, 1, 0) *
             getprop(Ks, Pim, mKstr, GKstr, 1, 1) +
             D2AP_A2VP(Pip1, Pip2, Ks, Pim, 0) * getprop(P14, Pip2, mK1400, GK1400, 1, 0) *
             getprop(Ks, Pim, mKstr, GKstr, 1, 1);
    //K1400[D]->K* pi
    PDF[4] = D2AP_A2VP(Pip2, Pip1, Ks, Pim, 2) * getprop(P14, Pip1, mK1400, GK1400, 1, 2) *
             getprop(Ks, Pim, mKstr, GKstr, 1, 1) +
             D2AP_A2VP(Pip1, Pip2, Ks, Pim, 2) * getprop(P14, Pip2, mK1400, GK1400, 1, 2) *
             getprop(Ks, Pim, mKstr, GKstr, 1, 1);
    //-----------------------------
    //-------K1270[S]->Ksrho-------
    PDF[5] = D2AP_A2VP(Pip2, Ks, Pip1, Pim, 0) * getprop(P24, Ks, mK1270, GK1270, 0, 0) *
             getprop(Pip1, Pim, mrho, Grho, 2, 1) +
             D2AP_A2VP(Pip1, Ks, Pip2, Pim, 0) * getprop(P34, Ks, mK1270, GK1270, 0, 0) *
             getprop(Pip2, Pim, mrho, Grho, 2, 1);
    //-------D->rhoKAD------------
    PDF[6] = D2AP_A2VP(Pip2, Ks, Pip1, Pim, 0) * getprop(Pip1, Pim, mrho, Grho, 2, 1) +
             D2AP_A2VP(Pip1, Ks, Pip2, Pim, 0) * getprop(Pip2, Pim, mrho, Grho, 2, 1);
    PDF[7] = D2AP_A2VP(Pip2, Ks, Pip1, Pim, 2) * getprop(Pip1, Pim, mrho, Grho, 2, 1) +
             D2AP_A2VP(Pip1, Ks, Pip2, Pim, 2) * getprop(Pip2, Pim, mrho, Grho, 2, 1);
    //-------D->K1460, K1460->Ks rho---------
    PDF[8] = D2PP_P2VP(Pip2, Ks, Pip1, Pim) * getprop(P24, Ks, mK1460, GK1460, 1, 1) *
             getprop(Pip1, Pim, mrho, Grho, 2, 1) +
             D2PP_P2VP(Pip1, Ks, Pip2, Pim) * getprop(P34, Ks, mK1460, GK1460, 1, 1) *
             getprop(Pip2, Pim, mrho, Grho, 2, 1);
    //--------K*PiA (K1650)---------------------
    PDF[9] = D2AP_A2VP(Pip2, Pip1, Ks, Pim, 0) * getprop(P14, Pip1, mK1650, GK1650, 1, 0) *
             getprop(Ks, Pim, mKstr, GKstr, 1, 1) +
             D2AP_A2VP(Pip1, Pip2, Ks, Pim, 0) * getprop(P14, Pip2, mK1650, GK1650, 1, 0) *
             getprop(Ks, Pim, mKstr, GKstr, 1, 1);
    //-------KsPiPiSPiA-----------------
    PDF[10] = D2AP_A2SP(Pip2, Ks, Pip1, Pim) + D2AP_A2SP(Pip1, Ks, Pip2, Pim);
    //-------KPiS wave------------------
    EvtComplex  corr(2, 0);
    PDF[11] = corr * PHSP(Ks, Pim);
    //-------D->K1460pi, K1460->K*-pi+--------
    PDF[12] = D2PP_P2VP(Pip2, Pip1, Ks, Pim) * getprop(P14, Pip1, mK1460, GK1460, 1, 1) *
              getprop(Ks, Pim, mKstr, GKstr, 1, 1) +
              D2PP_P2VP(Pip1, Pip2, Ks, Pim) * getprop(P14, Pip2, mK1460, GK1460, 1, 1) *
              getprop(Ks, Pim, mKstr, GKstr, 1, 1);
    //-------------------------------------------
    EvtComplex  cof;
    EvtComplex  pdf, module;
    pdf = EvtComplex(0, 0);
    for (int i = 0; i < 13; i++) {
      cof = EvtComplex(rho[i] * cos(phi[i]), rho[i] * sin(phi[i]));
      pdf = pdf + cof * PDF[i];
    }
    module = conj(pdf) * pdf;
    double value;
    value = real(module);
    return (value <= 0) ? 1e-20 : value;
  }

  EvtComplex EvtDToKSpipipi::KPiSFormfactor(const double sa, const double sb, const double sc, const double r)
  {
    (void)r;
    double m1430 = 1.463;
    double sa0 = m1430 * m1430;
    double w1430 = 0.233;
    double q0 = (sa0 + sb - sc) * (sa0 + sb - sc) / (4 * sa0) - sb;
    if (q0 < 0) q0 = 1e-16;
    double qs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    double q = sqrt(qs);
    double width_ = w1430 * q * m1430 / sqrt(sa * q0);
    double temp_R = atan(m1430 * width_ / (sa0 - sa));
    if (temp_R < 0) temp_R += math_pi;
    double deltaR = -5.31 + temp_R;
    double temp_F = atan(2 * 1.07 * q / (2 + (-1.8) * 1.07 * qs));
    if (temp_F < 0) temp_F += math_pi;
    double deltaF = 2.33 + temp_F;
    EvtComplex  cR(cos(deltaR), sin(deltaR));
    EvtComplex  cF(cos(deltaF), sin(deltaF));
    EvtComplex  amp = 0.8 * sin(deltaF) * cF + sin(deltaR) * cR * cF * cF;
    return amp;
  }
  EvtComplex EvtDToKSpipipi::D2AP_A2VP(double P1[], double P2[], double P3[], double P4[], int L)
  {
    double temp_PDF = 0;
    EvtComplex  amp_PDF(0, 0);
    double t1V[4], t1D[4], t2A[4][4];
    double sa[3], sb[3], sc[3], B[3];
    double pV[4], pA[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV[i] = P3[i] + P4[i];
      pA[i] = pV[i] + P2[i];
      pD[i] = pA[i] + P1[i];
    }
    sa[0] = dot(pV, pV);
    sb[0] = dot(P3, P3);
    sc[0] = dot(P4, P4);
    sa[1] = dot(pA, pA);
    sb[1] = sa[0];
    sc[1] = dot(P2, P2);
    sa[2] = dot(pD, pD);
    sb[2] = sa[1];
    sc[2] = dot(P1, P1);
    B[0] = barrier(1, sa[0], sb[0], sc[0], 3);
    B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    calt1(P3, P4, t1V);
    calt1(pA, P1, t1D);
    if (L == 0) {
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t1D[i] * (G[i][j] - pA[i] * pA[j] / sa[1]) * t1V[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[1] = 1;
    }
    if (L == 2) {
      calt2(pV, P2, t2A);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t1D[i] * t2A[i][j] * t1V[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[1] = barrier(2, sa[1], sb[1], sc[1], 3);
    }
    amp_PDF = temp_PDF * B[0] * B[1] * B[2];
    return amp_PDF;
  }
  EvtComplex EvtDToKSpipipi::D2AP_A2SP(double P1[], double P2[], double P3[], double P4[])
  {
    double temp_PDF = 0;
    EvtComplex  amp_PDF(0, 0);
    EvtComplex  pro;
    double sa[3], sb[3], sc[3], B[3];
    double t1D[4], t1A[4];
    double pS[4], pA[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pS[i] = P3[i] + P4[i];
      pA[i] = pS[i] + P2[i];
      pD[i] = pA[i] + P1[i];
    }
    sa[0] = dot(pS, pS);
    sb[0] = dot(P3, P3);
    sc[0] = dot(P4, P4);
    sa[1] = dot(pA, pA);
    sb[1] = sa[0];
    sc[1] = dot(P2, P2);
    sa[2] = dot(pD, pD);
    sb[2] = sa[1];
    sc[2] = dot(P1, P1);
    B[1] = barrier(1, sa[1], sb[1], sc[1], 3);
    B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    calt1(pA, P1, t1D);
    calt1(pS, P2, t1A);
    for (int i = 0; i != 4; i++) {
      temp_PDF += t1D[i] * t1A[i] * (G[i][i]);
    }
    amp_PDF = temp_PDF * B[1] * B[2];
    return amp_PDF;
  }
  EvtComplex EvtDToKSpipipi::D2PP_P2VP(double P1[], double P2[], double P3[], double P4[])
  {
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro;
    double sa[3], sb[3], sc[3], B[3];
    double t1V[4];
    double pV[4], pP[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV[i] = P3[i] + P4[i];
      pP[i] = pV[i] + P2[i];
      pD[i] = pP[i] + P1[i];
    }
    sa[0] = dot(pV, pV);
    sb[0] = dot(P3, P3);
    sc[0] = dot(P4, P4);
    sa[1] = dot(pP, pP);
    sb[1] = sa[0];
    sc[1] = dot(P2, P2);
    sa[2] = dot(pD, pD);
    sb[2] = sa[1];
    sc[2] = dot(P1, P1);
    B[0] = barrier(1, sa[0], sb[0], sc[0], 3);
    B[1] = barrier(1, sa[1], sb[1], sc[1], 3);
    calt1(P3, P4, t1V);
    for (int i = 0; i != 4; i++) {
      temp_PDF += P2[i] * t1V[i] * (G[i][i]);
    }
    amp_PDF = temp_PDF * B[0] * B[1];
    return amp_PDF;
  }
  EvtComplex  EvtDToKSpipipi::PHSP(double Km[], double Pip[])
  {
    EvtComplex  amp_PDF(0, 0);
    double sa, sb, sc;
    double KPi[4];
    for (int i = 0; i != 4; i++) {
      KPi[i] = Km[i] + Pip[i];
    }
    sa = dot(KPi, KPi);
    sb = dot(Km, Km);
    sc = dot(Pip, Pip);
    amp_PDF = KPiSFormfactor(sa, sb, sc, 3.0);
    return amp_PDF;
  }
  EvtComplex EvtDToKSpipipi::getprop(double daug1[], double daug2[], double mass_, double width_, int flag, int L)
  {
    //flag = 0, RBW with constant width
    //flag = 1, RBW with width depends on momentum
    //flag = 2, GS convolute with RBW_omega
    //flag = 3, flatte for f_0(980)/a_0(980)
    //flag = 4, Bugg formula for sigma;
    //effect radii == 3.0 GeV^-1
    EvtComplex  prop1, prop2, prop;
    EvtComplex one(1, 0);
    double pR[4];
    for (int i = 0; i < 4; i++) {
      pR[i] = daug1[i] + daug2[i];
    }
    double sa, sb, sc;
    sa = dot(pR, pR);
    sb = dot(daug1, daug1);
    sc = dot(daug2, daug2);
    if (flag == 0) return propogator(mass_, width_, sa);
    if (flag == 1) return propagatorRBW(mass_, width_, sa, sb, sc, 3.0, L);
    if (flag == 2) {
      prop1 = propagatorGS(mass_, width_, sa, sb, sc, 3.0, L);
      prop2 = propagatorRBW(0.783, 0.008, sa, sb, sc, 3.0, L);
      EvtComplex  coef_omega(rho_omega * cos(phi_omega), rho_omega * sin(phi_omega));
      prop = prop1 * (one + 0.783 * 0.783 * coef_omega * prop2);
      return prop;
    }
    if (flag == 3) {
      //Not need for D+ -> Ks 3pi
    }
    if (flag == 4) {
      EvtComplex  ci(0, 1);
      double f = 0.5843 + 1.6663 * sa;
      double M = 0.9264;
      double mpi2 = mass_Pion * mass_Pion;
      double mass2 = M * M;
      double g1_ = f * (sa - mpi2 / 2) / (mass2 - mpi2 / 2) * exp((mass2 - sa) / 1.082);
      EvtComplex  rho1s = rhoab(sa, sb, sc);
      EvtComplex  rho1M = rhoab(mass2, sb, sc);
      EvtComplex  rho2s = rho4Pi(sa);
      EvtComplex  rho2M = rho4Pi(mass2);
      prop = 1.0 / (M * M - sa - ci * M * (g1_ * rho1s / rho1M + 0.0024 * rho2s / rho2M));
      return prop;
    }
    return one;
  }
  EvtComplex  EvtDToKSpipipi::rhoab(const double sa, const double sb, const double sc)
  {
    EvtComplex one(1, 0);
    double q = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    EvtComplex rho_;
    EvtComplex ci(0, 1);
    if (q > 0) rho_ = one * sqrt(q / sa);
    if (q < 0) rho_ = ci * sqrt(-q / sa);
    rho_ = 2.0 * rho_;
    return rho_;
  }
  EvtComplex EvtDToKSpipipi::rho4Pi(const double sa)
  {
    double mpi_ = 0.13957;
    EvtComplex one(1, 0);
    EvtComplex  rho_(0, 0);
    EvtComplex  ci(0, 1);
    double temp = 1 - 16 * mpi_ * mpi_ / sa;
    if (temp > 0) rho_ = one * sqrt(temp) / (1 + exp(9.8 - 3.5 * sa));
    if (temp < 0) rho_ = ci * sqrt(-temp) / (1 + exp(9.8 - 3.5 * sa));
    return rho_;
  }

  EvtComplex  EvtDToKSpipipi::propogator(const double mass_, const double width_, const double sx)const
  {
    EvtComplex  ci(0, 1);
    EvtComplex  prop = 1.0 / (mass_ * mass_ - sx - ci * mass_ * width_);
    return prop;
  }
  double EvtDToKSpipipi::wid(double mass_, double sa, double sb, double sc, double r, int l)const
  {
    double widm(0.), q(0.), q0(0.);
    double sa0 = mass_ * mass_;
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
    widm = pow(t, l + 0.5) * mass_ / m * F * F;
    return widm;
  }
  double EvtDToKSpipipi::h(const double m, const double q)const
  {
    double h(0.);
    h = 2 / pi * q / m * log((m + 2 * q) / (2 * mpi));
    return h;
  }
  double EvtDToKSpipipi::dh(const double mass_, const double q0)const
  {
    double dh = h(mass_, q0) * (1.0 / (8 * q0 * q0) - 1.0 / (2 * mass_ * mass_)) + 1.0 / (2 * pi * mass_ * mass_);
    return dh;
  }
  double EvtDToKSpipipi::f(const double mass_, const double sx, const double q0, const double q)const
  {
    double m = sqrt(sx);
    double f = mass_ * mass_ / (pow(q0, 3)) * (q * q * (h(m, q) - h(mass_, q0)) + (mass_ * mass_ - sx) * q0 * q0 * dh(mass_, q0));
    return f;
  }
  double EvtDToKSpipipi::d(const double mass_, const double q0)const
  {
    double d = 3.0 / pi * mpi * mpi / (q0 * q0) * log((mass_ + 2 * q0) / (2 * mpi)) + mass_ / (2 * pi * q0) - (mpi * mpi * mass_) /
               (pi * pow(q0, 3));
    return d;
  }
  EvtComplex  EvtDToKSpipipi::propagatorRBW(const double mass_, const double width_, const double sa, const double sb,
                                            const double sc, const double r, const int l)const
  {
    EvtComplex  ci(0, 1);
    EvtComplex  prop = 1.0 / (mass_ * mass_ - sa - ci * mass_ * width_ * wid(mass_, sa, sb, sc, r, l));
    return prop;
  }
  EvtComplex  EvtDToKSpipipi::propagatorGS(const double mass_, const double width_, const double sa, const double sb, const double sc,
                                           const double r, const int l)const
  {
    EvtComplex  ci(0, 1);
    double q = Qabcs(sa, sb, sc);
    double sa0 = mass_ * mass_;
    double q0 = Qabcs(sa0, sb, sc);
    q = sqrt(q);
    q0 = sqrt(q0);
    EvtComplex  prop = (1 + d(mass_, q0) * width_ / mass_) / (mass_ * mass_ - sa + width_ * f(mass_, sa, q0,
                                                              q) - ci * mass_ * width_ * wid(mass_, sa, sb, sc, r, l));
    return prop;
  }
  double EvtDToKSpipipi::dot(double* a1, double* a2)const
  {
    double dot = 0;
    for (int i = 0; i != 4; i++) {
      dot += a1[i] * a2[i] * G[i][i];
    }
    return dot;
  }
  double EvtDToKSpipipi::Qabcs(const double sa, const double sb, const double sc)const
  {
    double Qabcs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (Qabcs < 0) Qabcs = 1e-16;
    return Qabcs;
  }
  double EvtDToKSpipipi::barrier(const double l, const double sa, const double sb, const double sc, const double r)const
  {
    double q = Qabcs(sa, sb, sc);
    q = sqrt(q);
    double z = q * r;
    z = z * z;
    double F = 1;
    if (l > 2) F = 0;
    if (l == 0)
      F = 1;
    if (l == 1) {
      F = sqrt((2 * z) / (1 + z));
    }
    if (l == 2) {
      F = sqrt((13 * z * z) / (9 + 3 * z + z * z));
    }
    return F;
  }
  void EvtDToKSpipipi::calt1(double daug1[], double daug2[], double t1[]) const
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
  void EvtDToKSpipipi::calt2(double daug1[], double daug2[], double t2[][4]) const
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

} // Belle 2 namespace
