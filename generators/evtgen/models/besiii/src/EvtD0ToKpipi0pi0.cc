// Model: EvtD0ToKpipi0pi0
// This file is an amplitude model for D0 -> K- pi+ pi0 pi0.
// The model is from the BESIII Collaboration in PRD 99, 092008 (2019). DOI:&nbsp; https://doi.org/10.1103/PhysRevD.99.092008
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
// Module: EvtD0ToKpipi0pi0.cc
//         the necessary file: EvtD0ToKpipi0pi0.hh
//
// Description: D0 -> K- pi+ pi0 pi0,
//              see PHYSICAL REVIEW D 99, 092008 (2019)
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
#include "generators/evtgen/models/besiii/EvtD0ToKpipi0pi0.h"

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtD0ToKpipi0pi0);

  EvtD0ToKpipi0pi0::~EvtD0ToKpipi0pi0() {}

  std::string EvtD0ToKpipi0pi0::getName()
  {
    return "D0ToKpipi0pi0";
  }

  EvtDecayBase* EvtD0ToKpipi0pi0::clone()
  {
    return new EvtD0ToKpipi0pi0;
  }

  void EvtD0ToKpipi0pi0::init()
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
    //std::cout << "Initializing EvtD0ToKpipi0pi0" << std::endl;

    mod[0] = 1; rho[0] = 2.02; phi[0] = -0.75;
    mod[1] = 1; rho[1] = 1.66; phi[1] = -2.90;
    mod[2] = 0; rho[2] = 0; phi[2] = 0;
    mod[3] = 0; rho[3] = 0; phi[3] = 0;
    mod[4] = 0; rho[4] = 0; phi[4] = 0;
    mod[5] = 0; rho[5] = 0; phi[5] = 0;
    mod[6] = 0; rho[6] = 0; phi[6] = 0;
    mod[7] = 1; rho[7] = 1; phi[7] = 0;
    mod[8] = 1; rho[8] = 0.842; phi[8] = -2.05;
    mod[9] = 1; rho[9] = 0.0218; phi[9] = 1.84;
    mod[10] = 0; rho[10] = 0; phi[10] = 0;
    mod[11] = 0; rho[11] = 0; phi[11] = 0;
    mod[12] = 0; rho[12] = 0; phi[12] = 0;
    mod[13] = 1; rho[13] = 0.0336; phi[13] = -1.55;
    mod[14] = 1; rho[14] = 0.109; phi[14] = -1.35;
    mod[15] = 1; rho[15] = 0.196; phi[15] = -2.07;
    mod[16] = 0; rho[16] = 0; phi[16] = 0;
    mod[17] = 0; rho[17] = 0; phi[17] = 0;
    mod[18] = 0; rho[18] = 0; phi[18] = 0;
    mod[19] = 1; rho[19] = 0.363; phi[19] = 1.93;
    mod[20] = 0; rho[20] = 0; phi[20] = 0;
    mod[21] = 0; rho[21] = 0; phi[21] = 0;
    mod[22] = 0; rho[22] = 0; phi[22] = 0;
    mod[23] = 1; rho[23] = 0.555; phi[23] = 0.44;
    mod[24] = 1; rho[24] = 0.526; phi[24] = -1.84;
    mod[25] = 0; rho[25] = 0; phi[25] = 0;
    mod[26] = 1; rho[26] = 1; phi[26] = 0.64;
    mod[27] = 0; rho[27] = 0; phi[27] = 0;
    mod[28] = 0; rho[28] = 0; phi[28] = 0;
    mod[29] = 1; rho[29] = 3.34; phi[29] = -0.02;
    mod[30] = 0; rho[30] = 0; phi[30] = 0;
    mod[31] = 0; rho[31] = 0; phi[31] = 0;
    mod[32] = 0; rho[32] = 0; phi[32] = 0;
    mod[33] = 0; rho[33] = 0; phi[33] = 0;
    mod[34] = 1; rho[34] = 1.76; phi[34] = -2.39;
    mod[35] = 1; rho[35] = 0.175; phi[35] = 1.59;
    mod[36] = 1; rho[36] = 0.397; phi[36] = 1.45;
    mod[37] = 0; rho[37] = 0; phi[37] = 0;
    mod[38] = 0; rho[38] = 0; phi[38] = 0;
    mod[39] = 0; rho[39] = 0; phi[39] = 0;
    mod[40] = 0; rho[40] = 0; phi[40] = 0;
    mod[41] = 1; rho[41] = 1.02; phi[41] = 0.52;
    mod[42] = 0; rho[42] = 0; phi[42] = 0;
    mod[43] = 0; rho[43] = 0; phi[43] = 0;
    mod[44] = 0; rho[44] = 0; phi[44] = 0;
    mod[45] = 0; rho[45] = 0; phi[45] = 0;
    mod[46] = 1; rho[46] = 0.146; phi[46] = 1.24;
    mod[47] = 1; rho[47] = 0.0978; phi[47] = -2.89;
    mod[48] = 1; rho[48] = 0.233; phi[48] = 2.41;
    mod[49] = 0; rho[49] = 0; phi[49] = 0;
    mod[50] = 1; rho[50] = 0.424; phi[50] = -0.94;
    mod[51] = 1; rho[51] = 1.03; phi[51] = -1.93;
    mod[52] = 0; rho[52] = 0; phi[52] = 0;
    mod[53] = 0; rho[53] = 0; phi[53] = 0;
    mod[54] = 1; rho[54] = 0.474; phi[54] = -1.17;
    mod[55] = 0; rho[55] = 0; phi[55] = 0;
    mod[56] = 0; rho[56] = 0; phi[56] = 0;
    mod[57] = 0; rho[57] = 0; phi[57] = 0;
    mod[58] = 0; rho[58] = 0; phi[58] = 0;
    mod[59] = 0; rho[59] = 0; phi[59] = 0;
    mod[60] = 0; rho[60] = 0; phi[60] = 0;
    mod[61] = 1; rho[61] = 6.74; phi[61] = -1.74;
    mod[62] = 0; rho[62] = 0; phi[62] = 0;
    mod[63] = 0; rho[63] = 0; phi[63] = 0;
    mod[64] = 0; rho[64] = 0; phi[64] = 0;
    mod[65] = 0; rho[65] = 0; phi[65] = 0;
    mod[66] = 1; rho[66] = 1.54; phi[66] = -2.93;
    mod[67] = 1; rho[67] = 1.36; phi[67] = 2.23;

    mass[0] = 1.23; width[0] = 0.50204;
    mass[1] = 1.2723; width[1] = 0.09;
    mass[2] = 0.89166; width[2] = 0.0508;
    mass[3] = 0.89581; width[3] = 0.0474;
    mass[4] = 0.77511; width[4] = 0.1491;

    //for (int i=0; i<68; i++) {
    //   cout << i << " rho,phi = " << rho[i] << ", "<< phi[i] << endl;
    //}
    mD = 1.86486;
    rRes = 3.0;
    rD = 5.0;
    metap = 0.95778;
    mk0 = 0.497614;
    mass_Kaon = 0.49368;
    mass_Pion = 0.13957;
    math_pi = 3.1415926;
    mass_Pi0  = 0.1349766;
    mkstrm = 0.89594;
    mkstr0 = 0.89594;

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

  void EvtD0ToKpipi0pi0::initProbMax()
  {
    setProbMax(8060.0);
  }

  void EvtD0ToKpipi0pi0::decay(EvtParticle* p)
  {
    /*
       double maxprob = 0.0;
       for(int ir=0;ir<=60000000;ir++){
          p->initializePhaseSpace(getNDaug(),getDaugs());
          EvtVector4R Km0 = p->getDaug(0)->getP4();
          EvtVector4R pi1 = p->getDaug(1)->getP4();
          EvtVector4R pi2 = p->getDaug(2)->getP4();
          EvtVector4R pi3 = p->getDaug(3)->getP4();
          double Km[4],Pip[4],Pi01[4],Pi02[4];
          Km[0] = Km0.get(0); Pip[0] = pi1.get(0); Pi01[0] = pi2.get(0); Pi02[0] = pi3.get(0);
          Km[1] = Km0.get(1); Pip[1] = pi1.get(1); Pi01[1] = pi2.get(1); Pi02[1] = pi3.get(1);
          Km[2] = Km0.get(2); Pip[2] = pi1.get(2); Pi01[2] = pi2.get(2); Pi02[2] = pi3.get(2);
          Km[3] = Km0.get(3); Pip[3] = pi1.get(3); Pi01[3] = pi2.get(3); Pi02[3] = pi3.get(3);
          double Prob = calPDF(Km, Pip, Pi01, Pi02);
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

    double Km[4], Pip[4], Pi01[4], Pi02[4];
    Km[0] = Km0.get(0); Pip[0] = pi1.get(0); Pi01[0] = pi2.get(0); Pi02[0] = pi3.get(0);
    Km[1] = Km0.get(1); Pip[1] = pi1.get(1); Pi01[1] = pi2.get(1); Pi02[1] = pi3.get(1);
    Km[2] = Km0.get(2); Pip[2] = pi1.get(2); Pi01[2] = pi2.get(2); Pi02[2] = pi3.get(2);
    Km[3] = Km0.get(3); Pip[3] = pi1.get(3); Pi01[3] = pi2.get(3); Pi02[3] = pi3.get(3);
    double prob = PDF(Km, Pip, Pi01, Pi02);
    setProb(prob);
    return;
  }

  double EvtD0ToKpipi0pi0::PDF(double* Km, double* Pip, double* Pi01, double* Pi02)
  {
    Km[0]   = sqrt(mass_Kaon * mass_Kaon + Km[1] * Km[1] + Km[2] * Km[2] + Km[3] * Km[3]);
    Pip[0] = sqrt(mass_Pion * mass_Pion + Pip[1] * Pip[1] + Pip[2] * Pip[2] + Pip[3] * Pip[3]);
    Pi01[0] = sqrt(mass_Pi0 * mass_Pi0 + Pi01[1] * Pi01[1] + Pi01[2] * Pi01[2] + Pi01[3] * Pi01[3]);
    Pi02[0]  = sqrt(mass_Pi0 * mass_Pi0 + Pi02[1] * Pi02[1] + Pi02[2] * Pi02[2] + Pi02[3] * Pi02[3]);

    EvtComplex PDF[68];
    int g[3];
    //-------PHSP---------
    PDF[0] = PHSP(Km, Pip) + PHSP(Km, Pip);
    PDF[1] = PHSP(Km, Pi01) + PHSP(Km, Pi02);
    //-------D2PP,P2VP------
//   PDF[2] = D2PP_P2VP(Pi01,Pi02,Km,Pip,0) + D2PP_P2VP(Pi02,Pi01,Km,Pip,0);
//   PDF[3] = D2PP_P2VP(Pi01,Pip,Km,Pi02,10) + D2PP_P2VP(Pi02,Pip,Km,Pi01,10);
//   PDF[4] = D2PP_P2VP(Pip,Pi01,Km,Pi02,20) + D2PP_P2VP(Pip,Pi02,Km,Pi01,20);
//   PDF[5] = D2PP_P2VP(Pi01,Km,Pip,Pi02,1) + D2PP_P2VP(Pi02,Km,Pip,Pi01,1);
//   PDF[6] = D2PP_P2VP(Km,Pi01,Pip,Pi02,11) + D2PP_P2VP(Km,Pi02,Pip,Pi01,11);
    //----------D2AP,A2VP--------------
    g[0] = 1; g[1] = 1; g[2] = 0;
    PDF[7] = D2AP_A2VP(Km, Pi01, Pip, Pi02, g, 0) + D2AP_A2VP(Km, Pi02, Pip, Pi01, g, 0);
    g[2] = 2;
    PDF[8] = D2AP_A2VP(Km, Pi01, Pip, Pi02, g, 0) + D2AP_A2VP(Km, Pi02, Pip, Pi01, g, 0);
    g[2] = 0;
    PDF[9] = D2AP_A2VP(Pip, Pi01, Km, Pi02, g, 1) + D2AP_A2VP(Pip, Pi02, Km, Pi01, g, 1);
//   g[2] = 2;
//   PDF[10] = D2AP_A2VP(Pip,Pi01,Km,Pi02,g,1) + D2AP_A2VP(Pip,Pi02,Km,Pi01,g,1);
//   g[2] = 0;
//   PDF[11] = D2AP_A2VP(Pi01,Pip,Km,Pi02,g,21) + D2AP_A2VP(Pi02,Pip,Km,Pi01,g,21);
//   g[2] = 2;
//   PDF[12] = D2AP_A2VP(Pi01,Pip,Km,Pi02,g,21) + D2AP_A2VP(Pi02,Pip,Km,Pi01,g,21);
    g[2] = 0;
    PDF[13] = D2AP_A2VP(Pi01, Pi02, Km, Pip, g, 31) + D2AP_A2VP(Pi02, Pi01, Km, Pip, g, 31);
    g[2] = 2;
    PDF[14] = D2AP_A2VP(Pi01, Pi02, Km, Pip, g, 31) + D2AP_A2VP(Pi02, Pi01, Km, Pip, g, 31);
    g[2] = 0;
    PDF[15] = D2AP_A2VP(Pi01, Km, Pip, Pi02, g, 3) + D2AP_A2VP(Pi02, Km, Pip, Pi01, g, 3);
//   g[2] = 2;
//   PDF[16] = D2AP_A2VP(Pi01,Km,Pip,Pi02,g,3) + D2AP_A2VP(Pi02,Km,Pip,Pi01,g,3);
    g[0] = 1; g[1] = 0; g[2] = 0;
//   PDF[17] = D2AP_A2VP(Km,Pi01,Pip,Pi02,g,0) + D2AP_A2VP(Km,Pi02,Pip,Pi01,g,0);
//   g[2] = 2;
//   PDF[18] = D2AP_A2VP(Km,Pi01,Pip,Pi02,g,0) + D2AP_A2VP(Km,Pi02,Pip,Pi01,g,0);
    g[2] = 0;
    PDF[19] = D2AP_A2VP(Pip, Pi01, Km, Pi02, g, 1) + D2AP_A2VP(Pip, Pi02, Km, Pi01, g, 1);
//   g[2] = 2;
//   PDF[20] = D2AP_A2VP(Pip,Pi01,Km,Pi02,g,1) + D2AP_A2VP(Pip,Pi02,Km,Pi01,g,1);
//   g[2] = 0;
//   PDF[21] = D2AP_A2VP(Pi01,Pip,Km,Pi02,g,21) + D2AP_A2VP(Pi02,Pip,Km,Pi01,g,21);
//   g[2] = 2;
//   PDF[22] = D2AP_A2VP(Pi01,Pip,Km,Pi02,g,21) + D2AP_A2VP(Pi02,Pip,Km,Pi01,g,21);
    g[2] = 0;
    PDF[23] = D2AP_A2VP(Pi01, Pi02, Km, Pip, g, 31) + D2AP_A2VP(Pi02, Pi01, Km, Pip, g, 31);
    g[2] = 2;
    PDF[24] = D2AP_A2VP(Pi01, Pi02, Km, Pip, g, 31) + D2AP_A2VP(Pi02, Pi01, Km, Pip, g, 31);
//   g[2] = 0;
//   PDF[25] = D2AP_A2VP(Pi01,Km,Pip,Pi02,g,3) + D2AP_A2VP(Pi02,Km,Pip,Pi01,g,3);
    g[2] = 2;
    PDF[26] = D2AP_A2VP(Pi01, Km, Pip, Pi02, g, 3) + D2AP_A2VP(Pi02, Km, Pip, Pi01, g, 3);
    //--------D2AP,A2SP-----------------------------------
//   PDF[27] = D2AP_A2SP(Km,Pi01,Pip,Pi02,0) + D2AP_A2SP(Km,Pi02,Pip,Pi01,0);
//   PDF[28] = D2AP_A2SP(Km,Pip,Pi01,Pi02,10) + D2AP_A2SP(Km,Pip,Pi02,Pi01,10);
    PDF[29] = D2AP_A2SP(Pi01, Pi02, Km, Pip, 1) + D2AP_A2SP(Pi02, Pi01, Km, Pip, 1);
//   PDF[30] = D2AP_A2SP(Pi01,Pip,Km,Pi02,11) + D2AP_A2SP(Pi02,Pip,Km,Pi01,11);
//   PDF[31] = D2AP_A2SP(Pip,Pi01,Km,Pi02,21) + D2AP_A2SP(Pip,Pi02,Km,Pi01,21);
//   PDF[32] = D2AP_A2SP(Pi01,Km,Pip,Pi02,31) + D2AP_A2SP(Pi02,Km,Pip,Pi01,31);
//   PDF[33] = D2AP_A2SP(Pip,Km,Pi01,Pi02,41) + D2AP_A2SP(Pip,Km,Pi02,Pi01,41);
    //--------D2VS----------------------------
    PDF[34] = D2VS(Pip, Pi01, Km, Pi02, 1, 0) + D2VS(Pip, Pi02, Km, Pi01, 1, 0);
    PDF[35] = D2VS(Km, Pi01, Pip, Pi02, 1, 1) + D2VS(Km, Pi02, Pip, Pi01, 1, 1);
    PDF[36] = D2VS(Km, Pip, Pi01, Pi02, 1, 11) + D2VS(Km, Pip, Pi02, Pi01, 1, 11);
//   PDF[37] = D2VS(Pi01,Pi02,Km,Pip,0,10) + D2VS(Pi02,Pi01,Km,Pip,0,10);
//   PDF[38] = D2VS(Pip,Pi01,Km,Pi02,0,0) + D2VS(Pip,Pi02,Km,Pi01,0,0);
//   PDF[39] = D2VS(Km,Pip,Pi01,Pi02,0,11) + D2VS(Km,Pip,Pi02,Pi01,0,11);
//   PDF[40] = D2VS(Km,Pi01,Pip,Pi02,0,1) + D2VS(Km,Pi02,Pip,Pi01,0,1);
    //---------D2VP,V2VP----------------------
    PDF[41] = D2VP_V2VP(Pi01, Pip, Km, Pi02, 0) + D2VP_V2VP(Pi02, Pip, Km, Pi01, 0);
//   PDF[42] = D2VP_V2VP(Pip,Pi01,Km,Pi02,10) + D2VP_V2VP(Pip,Pi02,Km,Pi01,10);
//   PDF[43] = D2VP_V2VP(Pi01,Pi02,Km,Pip,20) + D2VP_V2VP(Pi02,Pi01,Km,Pip,20);
//   PDF[44] = D2VP_V2VP(Pi01,Km,Pip,Pi02,1) + D2VP_V2VP(Pi02,Km,Pip,Pi01,1);
//   PDF[45] = D2VP_V2VP(Km,Pi01,Pip,Pi02,2) + D2VP_V2VP(Km,Pi02,Pip,Pi01,2);
    //-----------D2VV--------------------------
    g[0] = 1; g[1] = 1; g[2] = 0;
    PDF[46] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[2] = 1;
    PDF[47] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[2] = 2;
    PDF[48] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[0] = 0; g[1] = 1; g[2] = 0;
//   PDF[49] = D2VV(Km,Pi01,Pip,Pi02,g,0) + D2VV(Km,Pi02,Pip,Pi01,g,0);
    g[2] = 1;
    PDF[50] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[2] = 2;
    PDF[51] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[0] = 1; g[1] = 0; g[2] = 0;
//   PDF[52] = D2VV(Km,Pi01,Pip,Pi02,g,0) + D2VV(Km,Pi02,Pip,Pi01,g,0);
//   g[2] = 1;
//   PDF[53] = D2VV(Km,Pi01,Pip,Pi02,g,0) + D2VV(Km,Pi02,Pip,Pi01,g,0);
    g[2] = 2;
    PDF[54] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[0] = 1; g[1] = 0; g[2] = 0;
//   PDF[55] = D2VV(Km,Pip,Pi01,Pi01,g,1) + D2VV(Km,Pip,Pi02,Pi01,g,1);
//   g[2] = 1;
//   PDF[56] = D2VV(Km,Pip,Pi01,Pi01,g,1) + D2VV(Km,Pip,Pi02,Pi01,g,1);
//   g[2] = 2;
//   PDF[57] = D2VV(Km,Pip,Pi01,Pi01,g,1) + D2VV(Km,Pip,Pi02,Pi01,g,1);
    g[0] = 0; g[1] = 0; g[2] = 0;
//   PDF[58] = D2VV(Km,Pip,Pi01,Pi01,g,1) + D2VV(Km,Pip,Pi02,Pi01,g,1);
//   g[2] = 1;
//   PDF[59] = D2VV(Km,Pip,Pi01,Pi01,g,1) + D2VV(Km,Pip,Pi02,Pi01,g,1);
//   g[2] = 2;
//   PDF[60] = D2VV(Km,Pip,Pi01,Pi01,g,1) + D2VV(Km,Pip,Pi02,Pi01,g,1);
    g[0] = 0; g[1] = 0; g[2] = 0;
    PDF[61] = D2VV(Km, Pi01, Pip, Pi02, g, 0) + D2VV(Km, Pi02, Pip, Pi01, g, 0);
    g[2] = 1;
//   PDF[62] = D2VV(Km,Pi01,Pip,Pi02,g,0) + D2VV(Km,Pi02,Pip,Pi01,g,0);
    g[2] = 2;
//   PDF[63] = D2VV(Km,Pi01,Pip,Pi02,g,0) + D2VV(Km,Pi02,Pip,Pi01,g,0);
    //----------D2TS--------------------
//   PDF[64] = D2TS(Km,Pip,Pi01,Pi02,0) + D2TS(Km,Pip,Pi02,Pi01,0);
//   PDF[65] = D2TS(Km,Pi01,Pip,Pi02,10) + D2TS(Km,Pi02,Pip,Pi01,10);
    PDF[66] = D2TS(Pi02, Pi01, Km, Pip, 1) + D2TS(Pi01, Pi02, Km, Pip, 1);
    PDF[67] = D2TS(Pip, Pi01, Km, Pi02, 11) + D2TS(Pip, Pi02, Km, Pi01, 11);

//------------------------------------------
    EvtComplex cof;
    EvtComplex pdf, module;
    pdf = EvtComplex(0, 0);
    for (int i = 0; i < 68; i++) {
      if (mod[i] == 0) continue;
      cof = EvtComplex(rho[i] * cos(phi[i]), rho[i] * sin(phi[i]));
      pdf = pdf + cof * PDF[i];
    }
    module = conj(pdf) * pdf;
    double value;
    value = real(module);
    return (value <= 0) ? 1e-20 : value;
  }
  EvtComplex EvtD0ToKpipi0pi0::KPiSFormfactor(double sa, double sb, double sc, double r)
  {
    (void)r;
    double m1430 = 1.463;
    double sa0 = m1430 * m1430;
    double w1430 = 0.233;
    double q0 = (sa0 + sb - sc) * (sa0 + sb - sc) / (4 * sa0) - sb;
    if (q0 < 0) q0 = 1e-16;
    double qs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    double q = sqrt(qs);
    double Width = w1430 * q * m1430 / sqrt(sa * q0) * r / r;
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
  EvtComplex EvtD0ToKpipi0pi0::D2VV(const double* P1, const double* P2, const double* P3, const double* P4, int* g, const int flag)
  {
    double t1V1[4], t1V2[4], t1D[4], t2D[4][4];
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro[2];

    double sa[3], sb[3], sc[3], B[3];
    double pV1[4], pV2[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV1[i] = P1[i] + P2[i];
      pV2[i] = P3[i] + P4[i];
      pD[i] = pV1[i] + pV2[i];
    }
    sa[0] = LorentzDotProduct(pV1, pV1);
    sb[0] = LorentzDotProduct(P1, P1);
    sc[0] = LorentzDotProduct(P2, P2);
    sa[1] = LorentzDotProduct(pV2, pV2);
    sb[1] = LorentzDotProduct(P3, P3);
    sc[1] = LorentzDotProduct(P4, P4);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[0];
    sc[2] = sa[1];
    if (g[0] == 1) {
      if (flag == 0)pro[0] = propagatorRBW(mass[2], width[2], sa[0], sb[0], sc[0], rRes, 1);
      if (flag == 1)pro[0] = propagatorRBW(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    }
    if (g[1] == 1) {
      if (flag == 0) pro[1] = propagatorGS(mass[4], width[4], sa[1], sb[1], sc[1], rRes, 1); //rho
      if (flag == 1) pro[1] = 1;
    }
    if (g[0] == 0) pro[0] = 1;
    if (g[1] == 0) pro[1] = 1;
    B[0] = BWBarrierFactor(1, sa[0], sb[0], sc[0], rRes);
    B[1] = BWBarrierFactor(1, sa[1], sb[1], sc[1], rRes);
    covariantTensor1(P1, P2, t1V1);
    covariantTensor1(P3, P4, t1V2);
    if (g[2] == 0) {
      for (int i = 0; i != 4; i++) {
        temp_PDF += (G[i][i]) * t1V1[i] * t1V2[i];
      }
      B[2] = 1;
    }
    if (g[2] == 1) {
      covariantTensor1(pV1, pV2, t1D);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          for (int k = 0; k != 4; k++) {
            for (int l = 0; l != 4; l++) {
              temp_PDF += E[i][j][k][l] * pD[i] * t1D[j] * t1V1[k] * t1V2[l] * (G[i][i]) * (G[j][j]) * (G[l][l]) * (G[k][k]);
            }
          }
        }
      }
      B[2] = BWBarrierFactor(1, sa[2], sb[2], sc[2], rD);
    }
    if (g[2] == 2) {
      covariantTensor2(pV1, pV2, t2D);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t2D[i][j] * t1V1[i] * t1V2[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[2] = BWBarrierFactor(2, sa[2], sb[2], sc[2], rD);
    }
    amp_PDF = temp_PDF * B[0] * B[1] * B[2] * pro[0] * pro[1];
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipi0pi0::D2AP_A2VP(const double* P1, const double* P2, const double* P3, const double* P4, int* g,
                                         const int flag)
  {
    double temp_PDF = 0;
    EvtComplex amp_PDF(0, 0);
    EvtComplex pro[2];
    double t1V[4], t1D[4], t2A[4][4];
    double sa[3], sb[3], sc[3], B[3] = {0, 0, 0};
    double pV[4], pA[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV[i] = P3[i] + P4[i];
      pA[i] = pV[i] + P2[i];
      pD[i] = pA[i] + P1[i];
    }
    sa[0] = LorentzDotProduct(pV, pV);
    sb[0] = LorentzDotProduct(P3, P3);
    sc[0] = LorentzDotProduct(P4, P4);
    sa[1] = LorentzDotProduct(pA, pA);
    sb[1] = sa[0];
    sc[1] = LorentzDotProduct(P2, P2);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[1];
    sc[2] = LorentzDotProduct(P1, P1);
    if (g[0] == 1) {
      if (flag == 0 || flag == 3) pro[0] = propagatorGS(mass[4], width[4], sa[0], sb[0], sc[0], rRes, 1);
      else if (flag == 1 ||  flag == 21) pro[0] = propagatorRBW(mass[2], width[2], sa[0], sb[0], sc[0], rRes, 1);
      else if (flag == 31) pro[0] = propagatorRBW(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    } else if (g[0] == 0) pro[0] = 1;
    if (g[1] == 1) {
      if (flag == 0) pro[1] = propagatorRBW(mass[0], width[0], sa[1], sb[1], sc[1], rRes, g[2]);
      if (flag == 1 || flag == 21 || flag == 31 || flag == 3) pro[1] = propagatorRBW(mass[1], width[1], sa[1], sb[1], sc[1], rRes, g[2]);
    } else if (g[1] == 0) pro[1] = 1;
    B[0] = BWBarrierFactor(1, sa[0], sb[0], sc[0], rRes);
    B[2] = BWBarrierFactor(1, sa[2], sb[2], sc[2], rD);
    covariantTensor1(P3, P4, t1V);
    covariantTensor1(pA, P1, t1D);
    if (g[2] == 0) {
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t1D[i] * (G[i][j] - pA[i] * pA[j] / sa[1]) * t1V[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[1] = 1;
    } else if (g[2] == 2) {
      covariantTensor2(pV, P2, t2A);
      for (int i = 0; i != 4; i++) {
        for (int j = 0; j != 4; j++) {
          temp_PDF += t1D[i] * t2A[i][j] * t1V[j] * (G[i][i]) * (G[j][j]);
        }
      }
      B[1] = BWBarrierFactor(2, sa[1], sb[1], sc[1], rRes);
    }
    amp_PDF = temp_PDF * B[0] * B[1] * B[2] * pro[0] * pro[1];
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipi0pi0::D2AP_A2SP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    //flag = 0, S = rho; flag = 1, S = K*
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
    sa[0] = LorentzDotProduct(pS, pS);
    sb[0] = LorentzDotProduct(P3, P3);
    sc[0] = LorentzDotProduct(P4, P4);
    sa[1] = LorentzDotProduct(pA, pA);
    sb[1] = sa[0];
    sc[1] = LorentzDotProduct(P2, P2);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[1];
    sc[2] = LorentzDotProduct(P1, P1);
    B[1] = BWBarrierFactor(1, sa[1], sb[1], sc[1], rRes);
    B[2] = BWBarrierFactor(1, sa[2], sb[2], sc[2], rD);
    covariantTensor1(pA, P1, t1D);
    covariantTensor1(pS, P2, t1A);
    for (int i = 0; i != 4; i++) {
      temp_PDF += t1D[i] * t1A[i] * (G[i][i]);
    }
    amp_PDF = temp_PDF * B[1] * B[2];
    if (flag == 1 || flag == 11 || flag == 21) amp_PDF = amp_PDF * KPiSFormfactor(sa[0], sb[0], sc[0], rRes);
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipi0pi0::D2PP_P2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    //modeidx = 0 :(K*0 pi0)pi0
    //modeidx = 10:(K*- pi+)pi0
    //modeidx = 20:(K*- pi0)pi+
    //modeidx = 1 :(K- rho+)pi0
    //modeidx = 11:K-(rho+ pi0)
    double temp_PDF = 0;
    EvtComplex amp(0, 0);
    EvtComplex prop;
    double sa[3], sb[3], sc[3], B[3];
    double t1V[4];
    double pV[4], pP[4], pD[4];
    for (int i = 0; i != 4; i++) {
      pV[i] = P3[i] + P4[i];
      pP[i] = pV[i] + P2[i];
      pD[i] = pP[i] + P1[i];
    }
    sa[0] = LorentzDotProduct(pV, pV);
    sb[0] = LorentzDotProduct(P3, P3);
    sc[0] = LorentzDotProduct(P4, P4);
    sa[1] = LorentzDotProduct(pP, pP);
    sb[1] = sa[0];
    sc[1] = LorentzDotProduct(P2, P2);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[1];
    sc[2] = LorentzDotProduct(P1, P1);
    B[0] = BWBarrierFactor(1, sa[0], sb[0], sc[0], rRes);
    B[1] = BWBarrierFactor(1, sa[1], sb[1], sc[1], rRes);
    if (flag == 0) prop = propagatorRBW(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    else if (flag == 10 || 20) prop = propagatorRBW(mass[2], width[2], sa[0], sb[0], sc[0], rRes, 1);
    else if (flag == 1 || 11) prop = propagatorGS(mass[4], width[4], sa[0], sb[0], sc[0], rRes, 1);
    covariantTensor1(P3, P4, t1V);
    for (int i = 0; i != 4; i++) {
      temp_PDF += P2[i] * t1V[i] * (G[i][i]);
    }
    amp = temp_PDF * B[0] * B[1] * prop;
    return amp;
  }

  EvtComplex EvtD0ToKpipi0pi0::D2VP_V2VP(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
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
    sa[0] = LorentzDotProduct(pV2, pV2);
    sb[0] = LorentzDotProduct(P3, P3);
    sc[0] = LorentzDotProduct(P4, P4);
    sa[1] = LorentzDotProduct(pV1, pV1);
    sb[1] = sa[0];
    sc[1] = LorentzDotProduct(P2, P2);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[1];
    sc[2] = LorentzDotProduct(P1, P1);
    if (flag == 0 || flag == 10) pro = propagatorRBW(mass[2], width[2], sa[0], sb[0], sc[0], rRes, 1);
    else if (flag == 20) pro = propagatorRBW(mass[3], width[3], sa[0], sb[0], sc[0], rRes, 1);
    else if (flag == 1 || flag == 2) pro = propagatorGS(mass[4], width[4], sa[0], sb[0], sc[0], rRes, 1);
    B[0] = BWBarrierFactor(1, sa[0], sb[0], sc[0], rRes);
    B[1] = BWBarrierFactor(1, sa[1], sb[1], sc[1], rRes);
    B[2] = BWBarrierFactor(1, sa[2], sb[2], sc[2], rD);
    amp_PDF = temp_PDF * B[0] * B[1] * B[2] * pro;
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipi0pi0::D2VS(const double* P1, const double* P2, const double* P3, const double* P4, int g, const int flag)
  {
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
    sa[0] = LorentzDotProduct(pS, pS);
    sb[0] = LorentzDotProduct(P3, P3);
    sc[0] = LorentzDotProduct(P4, P4);
    sa[1] = LorentzDotProduct(pV, pV);
    sb[1] = LorentzDotProduct(P1, P1);
    sc[1] = LorentzDotProduct(P2, P2);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[0];
    sc[2] = sa[1];
    if (g == 1) {
      if (flag == 0) pro = propagatorGS(mass[4], width[4], sa[1], sb[1], sc[1], rRes, 1);
      else if (flag == 1) pro = propagatorRBW(mass[2], width[2], sa[1], sb[1], sc[1], rRes, 1);
      else if (flag == 11) pro = propagatorRBW(mass[3], width[3], sa[1], sb[1], sc[1], rRes, 1);
      else if (flag == 10) pro = 1;
    } else if (g == 0) pro = 1;
    B[1] = BWBarrierFactor(1, sa[1], sb[1], sc[1], rRes);
    B[2] = BWBarrierFactor(1, sa[2], sb[2], sc[2], rD);
    covariantTensor1(P1, P2, t1V);
    covariantTensor1(pS, pV, t1D);
    for (int i = 0; i != 4; i++) {
      temp_PDF += G[i][i] * t1D[i] * t1V[i];
    }
    amp_PDF = temp_PDF * B[1] * B[2] * pro;
    if (flag == 0 || flag == 10) amp_PDF *= KPiSFormfactor(sa[0], sb[0], sc[0], rRes);
    //  if(modeidx == 1 || modeidx == 11) amp_PDF *= -1.0; /// why ???????
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipi0pi0::D2TS(const double* P1, const double* P2, const double* P3, const double* P4, const int flag)
  {
    // flag == 0 KPiT. 1 PiPiT
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
    sa[0] = LorentzDotProduct(pT, pT);
    sb[0] = LorentzDotProduct(P1, P1);
    sc[0] = LorentzDotProduct(P2, P2);
    sa[1] = LorentzDotProduct(pS, pS);
    sb[1] = LorentzDotProduct(P3, P3);
    sc[1] = LorentzDotProduct(P4, P4);
    sa[2] = LorentzDotProduct(pD, pD);
    sb[2] = sa[0];
    sc[2] = sa[1];
    B[0] = BWBarrierFactor(2, sa[0], sb[0], sc[0], rRes);
    B[2] = BWBarrierFactor(2, sa[2], sb[2], sc[2], rD);
    covariantTensor2(P1, P2, t2T);
    covariantTensor2(pT, pS, t2D);
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        temp_PDF += t2D[i][j] * t2T[j][i] * (G[i][i]) * (G[j][j]);
      }
    }
    amp_PDF = temp_PDF * B[0] * B[2];
    if (flag == 1 || flag == 11) { amp_PDF = amp_PDF * KPiSFormfactor(sa[1], sb[1], sc[1], rRes);}
    return amp_PDF;
  }

  EvtComplex EvtD0ToKpipi0pi0::PHSP(double* P1, double* P2)
  {
    EvtComplex amp_PDF(0, 0);
    double sa, sb, sc;
    double KPi[4];
    for (int i = 0; i != 4; i++) {
      KPi[i] = P1[i] + P2[i];
    }
    sa = LorentzDotProduct(KPi, KPi);
    sb = LorentzDotProduct(P1, P1);
    sc = LorentzDotProduct(P2, P2);
    amp_PDF = KPiSFormfactor(sa, sb, sc, rRes);
    return amp_PDF;
  }

  double EvtD0ToKpipi0pi0::energyDependentWidth(const double Mass, double sa, double sb, double sc, double r, int l)const
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
  double EvtD0ToKpipi0pi0::h(double m, double q)const
  {
    double h(0.);
    h = 2 / pi * q / m * log((m + 2 * q) / (2 * mpi));
    return h;
  }
  double EvtD0ToKpipi0pi0::dh(double Mass, double q0)const
  {
    double dh = h(Mass, q0) * (1.0 / (8 * q0 * q0) - 1.0 / (2 * Mass * Mass)) + 1.0 / (2 * pi * Mass * Mass);
    return dh;
  }
  double EvtD0ToKpipi0pi0::f(double Mass, double sx, double q0, double q)const
  {
    double m = sqrt(sx);
    double f = Mass * Mass / (pow(q0, 3)) * (q * q * (h(m, q) - h(Mass, q0)) + (Mass * Mass - sx) * q0 * q0 * dh(Mass,
                                             q0));
    return f;
  }
  double EvtD0ToKpipi0pi0::d(double Mass, double q0)const
  {
    double d = 3.0 / pi * mpi * mpi / (q0 * q0) * log((Mass + 2 * q0) / (2 * mpi)) + Mass / (2 * pi * q0) - (mpi * mpi * Mass) /
               (pi * pow(q0, 3));
    return d;
  }
  EvtComplex EvtD0ToKpipi0pi0::propagatorRBW(double Mass, double Width, double sa, double sb, double sc, double r,
                                             int l)const
  {
    EvtComplex ci(0, 1);
    EvtComplex prop = 1.0 / (Mass * Mass - sa - ci * Mass * Width * energyDependentWidth(Mass, sa, sb, sc, r,
                             l));
    return prop;
  }
  EvtComplex EvtD0ToKpipi0pi0::propagatorGS(double Mass, double Width, double sa, double sb, double sc, double r,
                                            int l)const
  {
    EvtComplex ci(0, 1);
    double q = Qabcs(sa, sb, sc);
    double sa0 = Mass * Mass;
    double q0 = Qabcs(sa0, sb, sc);
    q = sqrt(q);
    q0 = sqrt(q0);
    EvtComplex prop = (1 + d(Mass, q0) * Width / Mass) / (Mass * Mass - sa + Width * f(Mass, sa, q0,
                                                          q) - ci * Mass * Width * energyDependentWidth(Mass,
                                                              sa, sb, sc, r, l));
    return prop;
  }
  double EvtD0ToKpipi0pi0::LorentzDotProduct(const double* a1, const double* a2)const
  {
    double dot = 0;
    for (int i = 0; i != 4; i++) {
      dot += a1[i] * a2[i] * G[i][i];
    }
    return dot;
  }
  double EvtD0ToKpipi0pi0::Qabcs(double sa, double sb, double sc)const
  {
    double Qabcs = (sa + sb - sc) * (sa + sb - sc) / (4 * sa) - sb;
    if (Qabcs < 0) Qabcs = 1e-16;
    return Qabcs;
  }
  double EvtD0ToKpipi0pi0::BWBarrierFactor(double l, double sa, double sb, double sc, double r)const
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
  void EvtD0ToKpipi0pi0::covariantTensor1(const double* daug1, const double* daug2, double* t1) const
  {
    double p, pq;
    double pa[4], qa[4];
    for (int i = 0; i != 4; i++) {
      pa[i] = daug1[i] + daug2[i];
      qa[i] = daug1[i] - daug2[i];
    }
    p = LorentzDotProduct(pa, pa);
    pq = LorentzDotProduct(pa, qa);
    for (int i = 0; i != 4; i++) {
      t1[i] = qa[i] - pq / p * pa[i];
    }
  }
  void EvtD0ToKpipi0pi0::covariantTensor2(const double* daug1, const double* daug2, double (*t2)[4]) const
  {
    double p, r;
    double pa[4], t1[4];
    covariantTensor1(daug1, daug2, t1);
    r = LorentzDotProduct(t1, t1);
    for (int i = 0; i != 4; i++) {
      pa[i] = daug1[i] + daug2[i];
    }
    p = LorentzDotProduct(pa, pa);
    for (int i = 0; i != 4; i++) {
      for (int j = 0; j != 4; j++) {
        t2[i][j] = t1[i] * t1[j] - 1.0 / 3 * r * (G[i][j] - pa[i] * pa[j] / p);
      }
    }
  }

} // Belle 2 namespace
