// Model: EvtD0ToKpipipi
// This file is an amplitude model for D0 -> K- pi+ pi+ pi-.
// The model is from the BESIII Collaboration in PRD 95, 072010 (2017). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.95.072010
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

//--------------------------------------------------------------------------
// Environment:
//      This software is part of models developed at BES collaboration
//      based on the EvtGen framework.  If you use all or part
//      of it, please give an appropriate acknowledgement.
//
// Copyright Information: See EvtGen/BesCopyright
//      Copyright (A) 2006      Ping Rong-Gang @IHEP
//
// Module: EvtD0ToKpipipi.cc
//         the necessary file: EvtD0ToKpipipi.hh
//
// Description: D0 -> K- pi+ pi+ pi-,
//              see PHYSICAL REVIEW D 95, 072010 (2017)
//
// Modification history:
//
//    Liaoyuan Dong    Jan.15, 2020       Module created
//
//------------------------------------------------------------------------
#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtComplex.hh"
#include "EvtGenBase/EvtDecayTable.hh"
#include <stdlib.h>

#include <generators/evtgen/EvtGenModelRegister.h>
#include "generators/evtgen/models/besiii/EvtD0ToKpipipi.h"

namespace Belle2 {

  B2_EVTGEN_REGISTER_MODEL(EvtD0ToKpipipi);

  EvtD0ToKpipipi::~EvtD0ToKpipipi() {}

  std::string EvtD0ToKpipipi::getName()
  {
    return "D0ToKpipipi";
  }

  EvtDecayBase* EvtD0ToKpipipi::clone()
  {
    return new EvtD0ToKpipipi;
  }

  void EvtD0ToKpipipi::init()
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
    //std::cout << "Initializing EvtD0ToKpipipi" << std::endl;

    width[0] = 0.09;
    width[1] = 0.044183653178315;
    width[2] = 0.541879469380012;
    width[3] = 0.148423336450619;
    mass[0] =  1.272;
    mass[1] =  0.894781734682169;
    mass[2] =  1.3622013558915;
    mass[3] =  0.779143408171384;

    phi[0] =   2.34794687054858;
    rho[0] =   0.0759345115620669;
    phi[1] =  -2.24641399153466;
    rho[1] =   0.0383327604903577;
    phi[2] =   2.48955684856045;
    rho[2] =   0.0931445480476023;

    phi[3] =   0;
    rho[3] =   1;

    phi[4] =  -2.10558220063012;
    rho[4] =   0.347041869435286;
    phi[5] =   1.47445088061872;
    phi[6] =   3.00243265559304;
    rho[5] =   0.00965088341753795;
    rho[6] =   0.120536507325731;
    phi[7] =  -2.45477499325158;
    rho[7] =   0.101419048440676;
    phi[8] =  -1.35809992343491;
    rho[8] =   4.28149643321317;
    phi[9] =  -2.45149221243198;
    rho[9] =   0.339492272598394;
    phi[10] = -0.17419389225461;
    rho[10] = -0.143619437541254;

    phi[11] = -2.08744386934208;
    rho[11] =  0.296286583716349;
    phi[12] =  0.;
    rho[12] =  0.;
    phi[13] = -0.432190571560873;
    rho[13] =  0.657344690733276;
    phi[14] = -1.39790294886865;
    rho[14] =  1.71208007006123;
    phi[15] =  1.58945300476228;
    rho[15] =  3.58248347683687;
    phi[16] =  2.58249107256307;
    rho[16] = -1.10728829503506;
    phi[17] = -0.163623135170955;
    rho[17] =  1.70863070178363;
    phi[18] = -0.134699023080211;
    rho[18] =  0.567531283682344;
    phi[19] = -2.12670610368279;
    rho[19] =  0.276571752504914;
    phi[20] = -1.3352622107357;
    rho[20] =  0.416634203151278;

    phi[21] = -2.91571684221842;
    rho[21] =  0.423062298489176;
    phi[22] =  2.4544220004327;
    rho[22] =  1.4017194038459;
    phi[23] = -2.23388390670423;
    rho[23] =  4.11110400629068;

    //for (int i=0; i<24; i++) {
    //   cout << i << " rho,phi = " << rho[i] << ", "<< phi[i] << endl;
    //}

    mD = 1.86486;
    rRes = 3.0;
    rD = 5.0;
    metap = 0.95778;
    mkstr = 0.89594;
    mk0 = 0.497614;
    mass_Kaon = 0.49368;
    mass_Pion = 0.13957;
    mass_Pi0 = 0.1349766;
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

  void EvtD0ToKpipipi::initProbMax()
  {
    setProbMax(720.0);
  }

  void EvtD0ToKpipipi::decay(EvtParticle* p)
  {
    /*
       double maxprob = 0.0;
       for(int ir=0;ir<=60000000;ir++){
          p->initializePhaseSpace(getNDaug(),getDaugs());
          EvtVector4R Km0 = p->getDaug(0)->getP4();
          EvtVector4R pi1 = p->getDaug(1)->getP4();
          EvtVector4R pi2 = p->getDaug(2)->getP4();
          EvtVector4R pi3 = p->getDaug(3)->getP4();
          double Km[4],Pip1[4],Pip2[4],Pim[4];
          Km[0] = Km0.get(0); Pip1[0] = pi1.get(0); Pip2[0] = pi2.get(0); Pim[0] = pi3.get(0);
          Km[1] = Km0.get(1); Pip1[1] = pi1.get(1); Pip2[1] = pi2.get(1); Pim[1] = pi3.get(1);
          Km[2] = Km0.get(2); Pip1[2] = pi1.get(2); Pip2[2] = pi2.get(2); Pim[2] = pi3.get(2);
          Km[3] = Km0.get(3); Pip1[3] = pi1.get(3); Pip2[3] = pi2.get(3); Pim[3] = pi3.get(3);
          double Prob = calPDF(Km, Pip1, Pip2, Pim);
          if(Prob>maxprob) {
             maxprob=Prob;
             std::cout << "Max PDF = " << ir << " prob= " << Prob << std::endl;
          }
       }
       std::cout << "Max!!!!!!!!!!! " << maxprob<< std::endl;
    */
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R Km0 = p->getDaug(0)->getP4();
    EvtVector4R pi1 = p->getDaug(1)->getP4();
    EvtVector4R pi2 = p->getDaug(2)->getP4();
    EvtVector4R pi3 = p->getDaug(3)->getP4();

    double Km[4], Pip1[4], Pip2[4], Pim[4];
    Km[0] = Km0.get(0); Pip1[0] = pi1.get(0); Pip2[0] = pi2.get(0); Pim[0] = pi3.get(0);
    Km[1] = Km0.get(1); Pip1[1] = pi1.get(1); Pip2[1] = pi2.get(1); Pim[1] = pi3.get(1);
    Km[2] = Km0.get(2); Pip1[2] = pi1.get(2); Pip2[2] = pi2.get(2); Pim[2] = pi3.get(2);
    Km[3] = Km0.get(3); Pip1[3] = pi1.get(3); Pip2[3] = pi2.get(3); Pim[3] = pi3.get(3);
    double prob = calPDF(Km, Pip1, Pip2, Pim);
    setProb(prob);
    return;
  }

  double EvtD0ToKpipipi::calPDF(double* Km, double* Pip1, double* Pip2, double* Pim)
  {
    Km[0]   = sqrt(mass_Kaon * mass_Kaon + Km[1] * Km[1] + Km[2] * Km[2] + Km[3] * Km[3]);
    Pip1[0] = sqrt(mass_Pion * mass_Pion + Pip1[1] * Pip1[1] + Pip1[2] * Pip1[2] + Pip1[3] * Pip1[3]);
    Pip2[0] = sqrt(mass_Pion * mass_Pion + Pip2[1] * Pip2[1] + Pip2[2] * Pip2[2] + Pip2[3] * Pip2[3]);
    Pim[0]  = sqrt(mass_Pion * mass_Pion + Pim[1] * Pim[1] + Pim[2] * Pim[2] + Pim[3] * Pim[3]);

    EvtComplex PDF[24];
    int g[3];
    g[0] = 1; g[1] = 1;

    //-----------D->K*rho--------
    g[2] = 0;
    PDF[0] = D2VV(Km, Pip1, Pip2, Pim, g) + D2VV(Km, Pip2, Pip1, Pim, g);
    g[2] = 1;
    PDF[1] = D2VV(Km, Pip1, Pip2, Pim, g) + D2VV(Km, Pip2, Pip1, Pim, g);
    g[2] = 2;
    PDF[2] = D2VV(Km, Pip1, Pip2, Pim, g) + D2VV(Km, Pip2, Pip1, Pim, g);
    //-----------D->K*rho finish--
    //----------D->a1K------------
    g[2] = 0;
    PDF[3] = D2AP_A2VP(Km, Pip2, Pip1, Pim, g, 2) + D2AP_A2VP(Km, Pip1, Pip2, Pim, g, 2);
    g[2] = 2;
    PDF[4] = D2AP_A2VP(Km, Pip2, Pip1, Pim, g, 2) + D2AP_A2VP(Km, Pip1, Pip2, Pim, g, 2);
    //----------D->a1K finish-----
    //--D->K1Pi--K1->K*Pi---------
    g[2] = 0;
    PDF[5] = D2AP_A2VP(Pip2, Pim, Km, Pip1, g, 1) + D2AP_A2VP(Pip1, Pim, Km, Pip2, g, 1);
    g[2] = 2;
    PDF[6] = D2AP_A2VP(Pip2, Pim, Km, Pip1, g, 1) + D2AP_A2VP(Pip1, Pim, Km, Pip2, g, 1);
    //--D->K1Pi--K1->K*Pi-finish--
    //--D->K1Pi--K1->rhoK---------
    g[2] = 0;
    PDF[7] = D2AP_A2VP(Pip2, Km, Pip1, Pim, g, 3) + D2AP_A2VP(Pip1, Km, Pip2, Pim, g, 3);
    //--D->K1Pi--K1->rhoK-finish--
    //--D->K1Pi--K1->K*0(1430)Pi--
    PDF[8] = D2AP_A2SP(Pip2, Pim, Km, Pip1, 1) + D2AP_A2SP(Pip1, Pim, Km, Pip2, 1);
    //--------res finish----------
    //----------------non-res------------------
    //--------KPiSrho------------------
    PDF[9] = D2VS(Pip1, Pim, Km, Pip2, 1, 2) + D2VS(Pip2, Pim, Km, Pip1, 1, 2);
    //--------K*PiPiS-----------------
    PDF[10] = D2VS(Km, Pip1, Pip2, Pim, 1, 1) + D2VS(Km, Pip2, Pip1, Pim, 1, 1);
    //--------K*PiP-------------------
    PDF[11] = D2PP_P2VP(Pip2, Pim, Km, Pip1, 1) + D2PP_P2VP(Pip1, Pim, Km, Pip2, 1);
    //--------rhoKA--------------------
    g[0] = 1; g[1] = 0;
    g[2] = 0;
    PDF[12] = 0;
    g[2] = 2;
    PDF[13] = D2AP_A2VP(Pip2, Km, Pip1, Pim, g, 3) + D2AP_A2VP(Pip1, Km, Pip2, Pim, g, 3);
    //-------PHSP-----------------------
    PDF[14] = PHSP(Km, Pip1) + PHSP(Km, Pip2);
    //------KPiVPiPiV-----------------------
    g[0] = 0; g[1] = 0; g[2] = 0;
    PDF[15] = D2VV(Km, Pip1, Pip2, Pim, g) + D2VV(Km, Pip2, Pip1, Pim, g);
    //------KPiVPiPiS----------------------
    PDF[16] = D2VS(Km, Pip1, Pip2, Pim, 0, 1) + D2VS(Km, Pip2, Pip1, Pim, 0, 1);
    //------KPiSPiPiV----------------------
    PDF[17] = D2VS(Pip1, Pim, Km, Pip2, 0, 2) + D2VS(Pip2, Pim, Km, Pip1, 0, 2);
    //-------------------------------------
    PDF[18] = D2PP_P2VP(Pip2, Km, Pip1, Pim, 2) + D2PP_P2VP(Pip1, Km, Pip2, Pim, 2);
    //-------------------------------------
    PDF[19] = D2VP_V2VP(Pip2, Pim, Km, Pip1, 1) + D2VP_V2VP(Pip1, Pim, Km, Pip2, 1);
    //-------------------------------------
    PDF[20] = D2VP_V2VP(Pip2, Km, Pip1, Pim, 2) + D2VP_V2VP(Pip1, Km, Pip2, Pim, 2);
    //-------------------------------------
    PDF[21] = D2TS(Km, Pip1, Pip2, Pim, 1) + D2TS(Km, Pip2, Pip1, Pim, 1);
    PDF[22] = D2TS(Pip1, Pim, Km, Pip2, 2) + D2TS(Pip2, Pim, Km, Pip1, 2);
    PDF[23] = D2AP_A2SP(Km, Pip2, Pip1, Pim, 2) + D2AP_A2SP(Km, Pip1, Pip2, Pim, 2);
//------------------------------------------
    EvtComplex cof;
    EvtComplex pdf, module;
    pdf = EvtComplex(0, 0);
    for (int i = 0; i != 24; i++) {
      cof = EvtComplex(rho[i] * cos(phi[i]), rho[i] * sin(phi[i]));
      //cout<<i << " " << (cof*PDF[i])<<" : "<<cof<<" "<<PDF[i]<<endl;
      pdf = pdf + cof * PDF[i];
    }
    module = conj(pdf) * pdf;
    double value;
    value = real(module);
    return (value <= 0) ? 1e-20 : value;
  }

  EvtComplex EvtD0ToKpipipi::KPiSFormfactor(double sa, double sb, double sc, double r)
  {
    (void)r;
    double m1430 = 1.463;
    double sa0 = m1430 * m1430;
    double w1430 = 0.233;
    double q0 = (sa0 + sb - sc) * (sa0 + sb - sc) / (4 * sa0) - sb;
    if (q0 < 0) q0 = 1e-16;
    double qs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    double q = sqrt(qs);
    double Width = w1430 * q * m1430 / sqrt(sa * q0);
    double temp_R = atan(m1430 * Width / (sa0 - sa));
    if (temp_R < 0) temp_R += math_pi;
    double deltaR = -5.31 + temp_R;
    double temp_F = atan(2 * 1.07 * q / (2 + (-1.8) * 1.07 * qs));
    if (temp_F < 0) temp_F += math_pi;
    double deltaF = 2.33 + temp_F;
    EvtComplex cR(cos(deltaR), sin(deltaR));
    EvtComplex cF(cos(deltaF), sin(deltaF));
    EvtComplex amp = 0.8 * sin(deltaF) * cF + sin(deltaR) * cR * cF * cF;
    return amp;
  }

  EvtComplex EvtD0ToKpipipi::D2VV(const double* P1, const double* P2, const double* P3, const double* P4, int* g)
  {
    double t1V1[4], t1V2[4], t1D[4], t2D[4][4];
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro[2];
    //---------use g[0],g[1] to define res or non-res, g[2] represent S, P or D wave
    double sa[3], sb[3], sc[3], B[3];
    double pV1[4], pV2[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV1[i] = P1[i] + P2[i];
      pV2[i] = P3[i] + P4[i];
      pD[i] = pV1[i] + pV2[i];
    }
    sa[0] = dot(pV1, pV1);
    sb[0] = dot(P1, P1);
    sc[0] = dot(P2, P2);
    sa[1] = dot(pV2, pV2);
    sb[1] = dot(P3, P3);
    sc[1] = dot(P4, P4);
    sa[2] = dot(pD, pD);
    sb[2] = sa[0];
    sc[2] = sa[1];
    if (g[1] == 1) {
      pro[1] = propagatorGS(mass[3], width[3], sa[1], sb[1], sc[1], rRes, 1);
    }
    if (g[0] == 1) {
      pro[0] = propagatorRBW(mass[1], width[1], sa[0], sb[0], sc[0], rRes, 1);
    }
    if (g[0] == 0) pro[0] = 1;
    if (g[1] == 0) pro[1] = 1;
    B[0] = barrier(1, sa[0], sb[0], sc[0], rRes);
    B[1] = barrier(1, sa[1], sb[1], sc[1], rRes);
    calt1(P1, P2, t1V1);
    calt1(P3, P4, t1V2);
    if (g[2] == 0) {
      for (int i = 0; i != 4; i++) {
        temp_PDF += (G[i][i]) * t1V1[i] * t1V2[i];
      }
      B[2] = 1;
    }
    if (g[2] == 1) {
      calt1(pV1, pV2, t1D);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          for (int k = 0; k != 4; k++) {
            for (int l = 0; l != 4; l++) {
              temp_PDF += E[i][j][k][l] * pD[i] * t1D[j] * t1V1[k] * t1V2[l] * (G[i][i]) * (G[j][j]) * (G[l][l]) * (G[k][k]);
            }
          }
        }
      }
      B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    }
    if (g[2] == 2) {
      calt2(pV1, pV2, t2D);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t2D[i][j] * t1V1[i] * t1V2[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[2] = barrier(2, sa[2], sb[2], sc[2], rD);
    }
    amp_PDF = temp_PDF * B[0] * B[1] * B[2] * pro[0] * pro[1];
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipipi::D2AP_A2VP(const double* P1, const double* P2, const double* P3, const double* P4, int* g, const int flag)
  {
    //flag = 1, V = K*, A = K1(1270); flag = 2, V = rho, A = a1(1260);
    //flag = 3, V = rho, A = K1(1270);
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro[2];
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
    if (g[0] == 1) {
      if (flag == 1) pro[0] = propagatorRBW(mass[1], width[1], sa[0], sb[0], sc[0], rRes, 1);
      if (flag == 2 || flag == 3) pro[0] = propagatorGS(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    }
    if (g[1] == 1) {
      if (flag == 1) pro[1] = propogator(mass[0], width[0], sa[1]);
      if (flag == 2) pro[1] = propagatorRBW(mass[2], width[2], sa[1], sb[1], sc[1], rRes, g[2]);
      if (flag == 3) pro[1] = propogator(mass[0], width[0], sa[1]);
    }
    if (g[0] == 0) pro[0] = 1;
    if (g[1] == 0) pro[1] = 1;
    B[0] = barrier(1, sa[0], sb[0], sc[0], rRes);
    B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    calt1(P3, P4, t1V);
    calt1(pA, P1, t1D);
    if (g[2] == 0) {
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t1D[i] * (G[i][j] - pA[i] * pA[j] / sa[1]) * t1V[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[1] = 1;
    }
    if (g[2] == 2) {
      calt2(pV, P2, t2A);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t1D[i] * t2A[i][j] * t1V[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[1] = barrier(2, sa[1], sb[1], sc[1], rRes);
    }
    amp_PDF = temp_PDF * B[0] * B[1] * B[2] * pro[0] * pro[1];
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::D2AP_A2SP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    //flag = 1, S = K*; flag = 2, S = rho
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro;
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
    B[1] = barrier(1, sa[1], sb[1], sc[1], rRes);
    B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    calt1(pA, P1, t1D);
    calt1(pS, P2, t1A);
    for (int i = 0; i != 4; i++) {
      temp_PDF += t1D[i] * t1A[i] * (G[i][i]);
    }
    amp_PDF = temp_PDF * B[1] * B[2];
    if (flag == 1) amp_PDF *= KPiSFormfactor(sa[0], sb[0], sc[0], rRes);
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::D2PP_P2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    //flag = 1, V = K*; flag = 2, V = rho
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
    B[0] = barrier(1, sa[0], sb[0], sc[0], rRes);
    B[1] = barrier(1, sa[1], sb[1], sc[1], rRes);
    if (flag == 1) pro = propagatorRBW(mass[1], width[1], sa[0], sb[0], sc[0], rRes, 1);
    if (flag == 2) pro = propagatorGS(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    calt1(P3, P4, t1V);
    for (int i = 0; i != 4; i++) {
      temp_PDF += P2[i] * t1V[i] * (G[i][i]);
    }
    amp_PDF = temp_PDF * B[0] * B[1] * pro;
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::D2VP_V2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    //flag = 1, (K*Pi)V; flag = 2, (rhoK)V
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro;
    double sa[3], sb[3], sc[3], B[3];
    double pV1[4], pV2[4], qV1[4], qV2[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV2[i] = P3[i] + P4[i];
      qV2[i] = P3[i] - P4[i];
      pV1[i] = pV2[i] + P2[i];
      qV1[i] = pV2[i] - P2[i];
      pD[i] = pV1[i] + P1[i];
    }
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        for (int k = 0; k != 4; k++) {
          for (int l = 0; l != 4; l++) {
            temp_PDF += E[i][j][k][l] * pV1[i] * qV1[j] * P1[k] * qV2[l] * (G[i][i]) * (G[j][j]) * (G[k][k]) * (G[l][l]);
          }
        }
      }
    }
    sa[0] = dot(pV2, pV2);
    sb[0] = dot(P3, P3);
    sc[0] = dot(P4, P4);
    sa[1] = dot(pV1, pV1);
    sb[1] = sa[0];
    sc[1] = dot(P2, P2);
    sa[2] = dot(pD, pD);
    sb[2] = sa[1];
    sc[2] = dot(P1, P1);
    if (flag == 1) pro = propagatorRBW(mass[1], width[1], sa[0], sb[0], sc[0], rRes, 1);
    if (flag == 2) pro = propagatorGS(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    B[0] = barrier(1, sa[0], sb[0], sc[0], rRes);
    B[1] = barrier(1, sa[1], sb[1], sc[1], rRes);
    B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    amp_PDF = temp_PDF * B[0] * B[1] * B[2] * pro;
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::D2VS(const double* P1, const double* P2, const double* P3, const double* P4, int g, const int flag)
  {
    //flag = 1, V = K*; flag = 2, V = rho
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro;
    double sa[3], sb[3], sc[3], B[3];
    double t1D[4], t1V[4];
    double pS[4], pV[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pS[i] = P3[i] + P4[i];
      pV[i] = P1[i] + P2[i];
      pD[i] = pS[i] + pV[i];
    }
    sa[0] = dot(pS, pS);
    sb[0] = dot(P3, P3);
    sc[0] = dot(P4, P4);
    sa[1] = dot(pV, pV);
    sb[1] = dot(P1, P1);
    sc[1] = dot(P2, P2);
    sa[2] = dot(pD, pD);
    sb[2] = sa[0];
    sc[2] = sa[1];
    if (g == 1) {
      if (flag == 2) pro = propagatorGS(mass[3], width[3], sa[1], sb[1], sc[1], rRes, 1);
      if (flag == 1) pro = propagatorRBW(mass[1], width[1], sa[1], sb[1], sc[1], rRes, 1);
    }
    if (g == 0) pro = 1;
    B[1] = barrier(1, sa[1], sb[1], sc[1], rRes);
    B[2] = barrier(1, sa[2], sb[2], sc[2], rD);
    calt1(P1, P2, t1V);
    calt1(pS, pV, t1D);
    for (int i = 0; i != 4; i++) {
      temp_PDF += G[i][i] * t1D[i] * t1V[i];
    }
    amp_PDF = temp_PDF * B[1] * B[2] * pro;
    if (flag == 2) amp_PDF *= KPiSFormfactor(sa[0], sb[0], sc[0], rRes);
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::D2TS(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    //flag = 1, T = K*; flag = 2, T = rho
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    double sa[3], sb[3], sc[3], B[3];
    double t2D[4][4], t2T[4][4];
    double pS[4], pT[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pS[i] = P3[i] + P4[i];
      pT[i] = P1[i] + P2[i];
      pD[i] = pT[i] + pS[i];
    }
    sa[0] = dot(pT, pT);
    sb[0] = dot(P1, P1);
    sc[0] = dot(P2, P2);
    sa[1] = dot(pS, pS);
    sb[1] = dot(P3, P3);
    sc[1] = dot(P4, P4);
    sa[2] = dot(pD, pD);
    sb[2] = sa[0];
    sc[2] = sa[1];
    B[0] = barrier(2, sa[0], sb[0], sc[0], rRes);
    B[2] = barrier(2, sa[2], sb[2], sc[2], rD);
    calt2(P1, P2, t2T);
    calt2(pT, pS, t2D);
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        temp_PDF += t2D[i][j] * t2T[j][i] * (G[i][i]) * (G[j][j]);
      }
    }
    amp_PDF = temp_PDF * B[0] * B[2];
    if (flag == 2) amp_PDF *= KPiSFormfactor(sa[1], sb[1], sc[1], rRes);
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::PHSP(double* Km, double* Pip)
  {
    EvtComplex amp_PDF(0, 0);
    double sa, sb, sc;
    double KPi[4];
    for (int i = 0; i != 4; i++) {
      KPi[i] = Km[i] + Pip[i];
    }
    sa = dot(KPi, KPi);
    sb = dot(Km, Km);
    sc = dot(Pip, Pip);
    amp_PDF = KPiSFormfactor(sa, sb, sc, rRes);
    return amp_PDF;
  }
  EvtComplex EvtD0ToKpipipi::propogator(double Mass, double Width, double sx)const
  {
    EvtComplex ci(0, 1);
    EvtComplex prop = 1.0 / (Mass * Mass - sx - ci * Mass * Width);
    return prop;
  }
  double EvtD0ToKpipipi::wid(double Mass, double sa, double sb, double sc, double r, int l)const
  {
    double widm(0.), q(0.), q0(0.);
    double sa0 = Mass * Mass;
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
    widm = pow(t, l + 0.5) * Mass / m * F * F;
    return widm;
  }
  double EvtD0ToKpipipi::h(double m, double q)const
  {
    double h(0.);
    h = 2 / pi * q / m * log((m + 2 * q) / (2 * mpi));
    return h;
  }
  double EvtD0ToKpipipi::dh(double Mass, double q0)const
  {
    double dh = h(Mass, q0) * (1.0 / (8 * q0 * q0) - 1.0 / (2 * Mass * Mass)) + 1.0 / (2 * pi * Mass * Mass);
    return dh;
  }
  double EvtD0ToKpipipi::f(double Mass, double sx, double q0, double q)const
  {
    double m = sqrt(sx);
    double f = Mass * Mass / (pow(q0, 3)) * (q * q * (h(m, q) - h(Mass, q0)) + (Mass * Mass - sx) * q0 * q0 * dh(Mass, q0));
    return f;
  }
  double EvtD0ToKpipipi::d(double Mass, double q0)const
  {
    double d = 3.0 / pi * mpi * mpi / (q0 * q0) * log((Mass + 2 * q0) / (2 * mpi)) + Mass / (2 * pi * q0) - (mpi * mpi * Mass) /
               (pi * pow(q0, 3));
    return d;
  }
  EvtComplex EvtD0ToKpipipi::propagatorRBW(double Mass, double Width, double sa, double sb, double sc, double r, int l)const
  {
    EvtComplex ci(0, 1);
    EvtComplex prop = 1.0 / (Mass * Mass - sa - ci * Mass * Width * wid(Mass, sa, sb, sc, r, l));
    return prop;
  }
  EvtComplex EvtD0ToKpipipi::propagatorGS(double Mass, double Width, double sa, double sb, double sc, double r, int l)const
  {
    EvtComplex ci(0, 1);
    double q = Qabcs(sa, sb, sc);
    double sa0 = Mass * Mass;
    double q0 = Qabcs(sa0, sb, sc);
    q = sqrt(q);
    q0 = sqrt(q0);
    EvtComplex prop = (1 + d(Mass, q0) * Width / Mass) / (Mass * Mass - sa + Width * f(Mass, sa, q0, q) - ci * Mass * Width * wid(Mass,
                                                          sa, sb, sc, r, l));
    return prop;
  }
  double EvtD0ToKpipipi::dot(const double* a1, const double* a2)const
  {
    double dot = 0;
    for (int i = 0; i != 4; i++) {
      dot += a1[i] * a2[i] * G[i][i];
    }
    return dot;
  }
  double EvtD0ToKpipipi::Qabcs(double sa, double sb, double sc)const
  {
    double Qabcs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (Qabcs < 0) Qabcs = 1e-16;
    return Qabcs;
  }
  double EvtD0ToKpipipi::barrier(double l, double sa, double sb, double sc, double r)const
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
  void EvtD0ToKpipipi::calt1(const double* daug1, const double* daug2, double* t1) const
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
  void EvtD0ToKpipipi::calt2(const double* daug1, const double* daug2, double (*t2)[4]) const
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
