// Model: EvtDToKSpipi0pi0
// This file is an amplitude model for D+ -> K_S0 pi- pi0 pi0.
// The model is from the BESIII Collaboration in JHEP09 (2023) 077. DOI:&nbsp; https://doi.org/10.1007/JHEP09(2023)077
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

#include <EvtGenBase/EvtPatches.hh>
#include <EvtGenBase/EvtParticle.hh>
#include <EvtGenBase/EvtGenKine.hh>
#include <EvtGenBase/EvtPDL.hh>
#include <EvtGenBase/EvtReport.hh>
#include <EvtGenBase/EvtComplex.hh>
#include <EvtGenBase/EvtDecayTable.hh>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>

#include <generators/evtgen/EvtGenModelRegister.h>
#include <generators/evtgen/models/besiii/EvtDToKSpipi0pi0.h>

using namespace std;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtDToKSpipi0pi0);

  EvtDToKSpipi0pi0::~EvtDToKSpipi0pi0() {}

  std::string EvtDToKSpipi0pi0::getName()
  {
    return "DToKSpipi0pi0";
  }

  EvtDecayBase* EvtDToKSpipi0pi0::clone()
  {
    return new EvtDToKSpipi0pi0;
  }

  void EvtDToKSpipi0pi0::init()
  {

    checkNArg(0);
    checkNDaug(4);
    checkSpinParent(EvtSpinType::SCALAR);

    mK1270 = 1.272;   mK1400 = 1.403;
    GK1270 = 0.09;    GK1400 = 0.174;
    mKstr0 = 0.89555; mrho   = 0.77511;
    GKstr0 = 0.0473;  Grho   = 0.1485;
    msigma = 0.526;   ma1    = 1.230;
    Gsigma = 0.535;   Ga1    = 0.420;
    mD = 1.86965;
    math_pi = 3.1415926;

    rho[0] =  3.0177e+00;//5
    phi[0] =  7.5497e-01;

    rho[1] =  1;//a1(1260)
    phi[1] =  0;

    rho[2] =  2.3527e-01;//7S_1400
    phi[2] = -2.9985e+00;

    rho[3] =  5.5731e-01;//7D_1400
    phi[3] =  4.2940e+00;

    rho[4] =  9.0803e-01;//11_sigma
    phi[4] =  4.7731e+00;

    rho[5] =  2.5345e-01;//23S
    phi[5] = -3.3205e+00;

    rho[6] =  6.0504e-02;//23P
    phi[6] = -1.6803e+00;

    rho[7] =  7.6978e-01;//25S
    phi[7] = -5.5937e+00;

    modetype[0] = 403;
    modetype[1] = 100;
    modetype[2] = 200;
    modetype[3] = 200;
    modetype[4] = 204;
    modetype[5] = 2;
    modetype[6] = 2;
    modetype[7] = 2;

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

  void EvtDToKSpipi0pi0::initProbMax()
  {
    setProbMax(2325.0);
  }

  void EvtDToKSpipi0pi0::decay(EvtParticle* p)
  {
    //-----------for max value------------------
    /*         double maxprob = 0.0;
           for(int ir=0;ir<=60000000;ir++){
           p->initializePhaseSpace(getNDaug(),getDaugs());
           EvtVector4R Ks0 = p->getDaug(0)->getP4();
           EvtVector4R pi1 = p->getDaug(1)->getP4();
           EvtVector4R pi2 = p->getDaug(2)->getP4();
           EvtVector4R pi3 = p->getDaug(3)->getP4();
           double value;
           double Ks[4],Pip[4],Pi01[4],Pi02[4];
           mother_c=EvtPDL::getStdHep(p->getId());
    if(mother_c==411){
    Ks[0] = Ks0.get(0); Pip[0] = pi1.get(0); Pi01[0] = pi2.get(0); Pi02[0] = pi3.get(0);
    Ks[1] = Ks0.get(1); Pip[1] = pi1.get(1); Pi01[1] = pi2.get(1); Pi02[1] = pi3.get(1);
    Ks[2] = Ks0.get(2); Pip[2] = pi1.get(2); Pi01[2] = pi2.get(2); Pi02[2] = pi3.get(2);
    Ks[3] = Ks0.get(3); Pip[3] = pi1.get(3); Pi01[3] = pi2.get(3); Pi02[3] = pi3.get(3);
    }else if(mother_c==-411){
    Ks[0] = Ks0.get(0); Pip[0] = pi1.get(0); Pi01[0] = pi2.get(0); Pi02[0] = pi3.get(0);
    Ks[1] = -1.0*Ks0.get(1); Pip[1] = -1.0*pi1.get(1); Pi01[1] = -1.0*pi2.get(1); Pi02[1] = -1.0*pi3.get(1);
    Ks[2] = -1.0*Ks0.get(2); Pip[2] = -1.0*pi1.get(2); Pi01[2] = -1.0*pi2.get(2); Pi02[2] = -1.0*pi3.get(2);
    Ks[3] = -1.0*Ks0.get(3); Pip[3] = -1.0*pi1.get(3); Pi01[3] = -1.0*pi2.get(3); Pi02[3] = -1.0*pi3.get(3);}
    calPDF(Ks, Pip, Pi01, Pi02, value);
    if(value>maxprob) {
    maxprob=value;
    }
    }
    return;*/
    //-----------------------------------------------
    p->initializePhaseSpace(getNDaug(), getDaugs());
    EvtVector4R Ks0 = p->getDaug(0)->getP4();
    EvtVector4R pip = p->getDaug(1)->getP4();
    EvtVector4R pi01 = p->getDaug(2)->getP4();
    EvtVector4R pi02 = p->getDaug(3)->getP4();

    mother_c = EvtPDL::getStdHep(p->getId());
    double Ks[4], Pip[4], Pi01[4], Pi02[4];
    if (mother_c == 411) {
      Ks[0] = Ks0.get(0); Pip[0] = pip.get(0); Pi01[0] = pi01.get(0); Pi02[0] = pi02.get(0);
      Ks[1] = Ks0.get(1); Pip[1] = pip.get(1); Pi01[1] = pi01.get(1); Pi02[1] = pi02.get(1);
      Ks[2] = Ks0.get(2); Pip[2] = pip.get(2); Pi01[2] = pi01.get(2); Pi02[2] = pi02.get(2);
      Ks[3] = Ks0.get(3); Pip[3] = pip.get(3); Pi01[3] = pi01.get(3); Pi02[3] = pi02.get(3);
    } else if (mother_c == -411) {
      Ks[0] = Ks0.get(0); Pip[0] = pip.get(0); Pi01[0] = pi01.get(0); Pi02[0] = pi02.get(0);
      Ks[1] = -1.0 * Ks0.get(1); Pip[1] = -1.0 * pip.get(1); Pi01[1] = -1.0 * pi01.get(1); Pi02[1] = -1.0 * pi02.get(1);
      Ks[2] = -1.0 * Ks0.get(2); Pip[2] = -1.0 * pip.get(2); Pi01[2] = -1.0 * pi01.get(2); Pi02[2] = -1.0 * pi02.get(2);
      Ks[3] = -1.0 * Ks0.get(3); Pip[3] = -1.0 * pip.get(3); Pi01[3] = -1.0 * pi01.get(3); Pi02[3] = -1.0 * pi02.get(3);
    }

    //Ks[0] = 0.656878382777544;  Pip[0] = 0.465468253141211;  Pi01[0] = 0.364737466814715;  Pi02[0] = 0.399546086976042;
    //Ks[1] = -0.426050514556759; Pip[1] = 0.440786345532845;  Pi01[1] = -0.195437195063332; Pi02[1] = 0.352799494203111;
    //Ks[2] = -0.015331547333183; Pip[2] = -0.003143271463748; Pi01[2] = 0.195716648451034;  Pi02[2] = -0.106603408016079;
    //Ks[3] = -0.046026135420350; Pip[3] = -0.053650975933667; Pi01[3] = 0.195739708428659;  Pi02[3] = 0.074743717862502;

    double value;
    calPDF(Ks, Pip, Pi01, Pi02, value);
    setProb(value);
    return;
  }

  double EvtDToKSpipi0pi0::calPDF(const double Ks[], const double Pip[], const double Pi01[], const double Pi02[], double& Result)
  {
    double cof[2], amp_tmp1[2], amp_tmp2[2], amp_tmp[2], amp_PDF[2], PDF[2];
    double flag[3], mass_R[2], width_R[2];
    double sa[3], sb[3], sc[3], B[3] = {};
    double t1D[4], t1V[4], t1A[4];
    double pS[4], pV[4], pD[4], pA[4];
    double pro1[2], pro2[2], proKPi_S[2], pro[2];

    double rD2 = 25.0;
    double rRes2 = 9.0;
    double mass1[8] = {mrho, mrho, mKstr0, mKstr0, msigma, mKstr0, mKstr0, mKstr0 };
    double mass2[8] = {mrho,  ma1, mK1400, mK1400,    ma1,  mrho,  mrho,   mrho };
    double width1[8] = {Grho, Grho, GKstr0, GKstr0, Gsigma, GKstr0, GKstr0, GKstr0 };
    double width2[8] = {Grho,  Ga1, GK1400, GK1400,    Ga1,  Grho,  Grho,   Grho };
    double g0[8]    = {   0,    1,      1,      1,      1,     1,     1,      1 };
    double g1[8]    = {   0,    1,      1,      1,      1,     1,     1,      0 };
    double g2[8]    = {   0,    0,      0,      2,      1,     0,     1,      0 };
    double temp_PDF = 0;
    PDF[0] = 0;
    PDF[1] = 0;

    for (int i = 0; i < 8; i++) { //0719 i=0, 7
      flag[0] = g0[i]; flag[1] = g1[i]; flag[2] = g2[i];
      mass_R[0] = mass1[i]; mass_R[1] = mass2[i];
      width_R[0] = width1[i]; width_R[1] = width2[i];

      amp_tmp[0] = 0;
      amp_tmp[1] = 0;
      amp_tmp1[0] = 0;
      amp_tmp1[1] = 0;
      amp_tmp2[0] = 0;
      amp_tmp2[1] = 0;

      cof[0] = rho[i] * cos(phi[i]);
      cof[1] = rho[i] * sin(phi[i]);

      if (modetype[i] == 100) { //D->Ks (rho+ pi0 ),D->Ks a1(1260)+

        temp_PDF = 0;
        double t2A[4][4];

        for (int ii = 0; ii != 4; ii++) {
          pV[ii] = Pip[ii] + Pi02[ii];
          pA[ii] = pV[ii] + Pi01[ii];
          pD[ii] = pA[ii] + Ks[ii];
        }
        sa[0] = SCADot(pV, pV);
        sb[0] = SCADot(Pip, Pip);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pA, pA);
        sb[1] = sa[0];
        sc[1] = SCADot(Pi01, Pi01);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Ks, Ks);
        if (flag[0] == 1) {
          propagatorGS(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        if (flag[1] == 1) {
          propagatorRBW_a1(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, flag[2], pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Pip, Pi02, t1V);
        calt1(pA, Ks, t1D);
        if (flag[2] == 0) {
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * (G[ii][j] - pA[ii] * pA[j] / sa[1]) * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = 1;
        } else if (flag[2] == 2) {
          calt2(pV, Pi01, t2A);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * t2A[ii][j] * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = Barrier(mass_R[1] * mass_R[1], 2, sa[1], sb[1], sc[1], rRes2);
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp1[0] = temp_PDF * B[0] * B[1] * B[2] * pro[0];
        amp_tmp1[1] = temp_PDF * B[0] * B[1] * B[2] * pro[1];


        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pV[ii] = Pip[ii] + Pi01[ii];
          pA[ii] = pV[ii] + Pi02[ii];
          pD[ii] = pA[ii] + Ks[ii];
        }
        sa[0] = SCADot(pV, pV);
        sb[0] = SCADot(Pip, Pip);
        sc[0] = SCADot(Pi01, Pi01);
        sa[1] = SCADot(pA, pA);
        sb[1] = sa[0];
        sc[1] = SCADot(Pi02, Pi02);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Ks, Ks);
        if (flag[0] == 1) {
          propagatorGS(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        if (flag[1] == 1) {
          propagatorRBW_a1(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, flag[2], pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Pip, Pi01, t1V);
        calt1(pA, Ks, t1D);
        if (flag[2] == 0) {
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * (G[ii][j] - pA[ii] * pA[j] / sa[1]) * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = 1;
        } else if (flag[2] == 2) {
          calt2(pV, Pi02, t2A);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * t2A[ii][j] * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = Barrier(mass_R[1] * mass_R[1], 2, sa[1], sb[1], sc[1], rRes2);
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp2[0] = temp_PDF * B[0] * B[1] * B[2] * pro[0];
        amp_tmp2[1] = temp_PDF * B[0] * B[1] * B[2] * pro[1];
      } else if (modetype[i] == 200) { //D->(K*0 pi0) pi+

        temp_PDF = 0;
        double t2A[4][4];
        for (int ii = 0; ii != 4; ii++) {
          pV[ii] = Ks[ii] + Pi02[ii];
          pA[ii] = pV[ii] + Pi01[ii];
          pD[ii] = pA[ii] + Pip[ii];
        }
        sa[0] = SCADot(pV, pV);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pA, pA);
        sb[1] = sa[0];
        sc[1] = SCADot(Pi01, Pi01);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Pip, Pip);
        if (flag[0] == 1) {
          propagatorRBW(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, 1, pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        if (flag[1] == 1) {
          propagatorRBW(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, flag[2], pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Ks, Pi02, t1V);
        calt1(pA, Pip, t1D);
        if (flag[2] == 0) {
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * (G[ii][j] - pA[ii] * pA[j] / sa[1]) * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = 1;
        } else if (flag[2] == 2) {
          calt2(pV, Pi01, t2A);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * t2A[ii][j] * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = Barrier(mass_R[1] * mass_R[1], 2, sa[1], sb[1], sc[1], rRes2);
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp1[0] = temp_PDF * B[0] * B[1] * B[2] * pro[0];
        amp_tmp1[1] = temp_PDF * B[0] * B[1] * B[2] * pro[1];
        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pV[ii] = Ks[ii] + Pi01[ii];
          pA[ii] = pV[ii] + Pi02[ii];
          pD[ii] = pA[ii] + Pip[ii];
        }
        sa[0] = SCADot(pV, pV);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi01, Pi01);
        sa[1] = SCADot(pA, pA);
        sb[1] = sa[0];
        sc[1] = SCADot(Pi02, Pi02);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Pip, Pip);
        if (flag[0] == 1) {
          propagatorRBW(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, 1, pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        if (flag[1] == 1) {
          propagatorRBW(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, flag[2], pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}

        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Ks, Pi01, t1V);
        calt1(pA, Pip, t1D);
        if (flag[2] == 0) {
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * (G[ii][j] - pA[ii] * pA[j] / sa[1]) * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = 1;
        } else if (flag[2] == 2) {
          calt2(pV, Pi02, t2A);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t1D[ii] * t2A[ii][j] * t1V[j] * G[ii][ii] * G[j][j];
            }
          }
          B[1] = Barrier(mass_R[1] * mass_R[1], 2, sa[1], sb[1], sc[1], rRes2);
        }

        Com_Multi(pro1, pro2, pro);
        amp_tmp2[0] = temp_PDF * B[0] * B[1] * B[2] * pro[0];
        amp_tmp2[1] = temp_PDF * B[0] * B[1] * B[2] * pro[1];
      } else if (modetype[i] == 204) { //D->Ks a1(1260)+, a1(1260)+->sigma pi+
        temp_PDF = 0;

        for (int ii = 0; ii != 4; ii++) {
          pS[ii] = Pi01[ii] + Pi02[ii];
          pA[ii] = pS[ii] + Pip[ii];
          pD[ii] = pA[ii] + Ks[ii];
        }
        sa[0] = SCADot(pS, pS);
        sb[0] = SCADot(Pi01, Pi01);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pA, pA);
        sb[1] = sa[0];
        sc[1] = SCADot(Pip, Pip);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Ks, Ks);
        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(pA, Ks, t1D);
        calt1(pS, Pip, t1A);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += t1D[ii] * t1A[ii] * (G[ii][ii]);
        }
        if (flag[0] == 1) { //sigma500
          propagatorsigma500(sa[0], sb[0], sc[0], pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        if (flag[1] == 1) { //a1
          propagatorRBW_a1(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, 1, pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}

        Com_Multi(pro1, pro2, pro);
        amp_tmp1[0] = temp_PDF * B[1] * B[2] * pro[0];
        amp_tmp1[1] = temp_PDF * B[1] * B[2] * pro[1];
        temp_PDF = 0;
        amp_tmp2[0] = amp_tmp1[0];
        amp_tmp2[1] = amp_tmp1[1];
      }     else if (modetype[i] == 120) { //D->(Ks pi0)S rho+
        temp_PDF = 0;

        for (int ii = 0; ii != 4; ii++) {
          pS[ii] = Ks[ii] + Pi02[ii];
          pV[ii] = Pip[ii] + Pi01[ii];
          pD[ii] = pS[ii] + pV[ii];
        }
        sa[0] = SCADot(pS, pS);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pV, pV);
        sb[1] = SCADot(Pip, Pip);
        sc[1] = SCADot(Pi01, Pi01);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[0];
        sc[2] = sa[1];
        if (flag[0] == 1) {
          propagatorGS(mass_R[1]*mass_R[1], mass_R[1], width_R[0], sa[1], sb[1], sc[1], rRes2, pro1);
        } else if (flag[0] == 0) { pro1[0] = 1; pro1[1] = 0; }
        KPiSLASS(sa[0], sb[0], sc[0], proKPi_S);
        if (flag[1] == 1) {
          pro2[0] = proKPi_S[0]; pro2[1] = proKPi_S[1];
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}

        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Pip, Pi01, t1V);
        calt1(pS, pV, t1D);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += G[ii][ii] * t1D[ii] * t1V[ii];
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp1[0] = temp_PDF * B[1] * B[2] * pro[0];
        amp_tmp1[1] = temp_PDF * B[1] * B[2] * pro[1];

        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pS[ii] = Ks[ii] + Pi01[ii];
          pV[ii] = Pip[ii] + Pi02[ii];
          pD[ii] = pS[ii] + pV[ii];
        }
        sa[0] = SCADot(pS, pS);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi01, Pi01);
        sa[1] = SCADot(pV, pV);
        sb[1] = SCADot(Pip, Pip);
        sc[1] = SCADot(Pi02, Pi02);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[0];
        sc[2] = sa[1];
        if (flag[0] == 1) {
          propagatorGS(mass_R[1]*mass_R[1], mass_R[1], width_R[0], sa[1], sb[1], sc[1], rRes2, pro1);
        } else if (flag[0] == 0) { pro1[0] = 1; pro1[1] = 0; }
        KPiSLASS(sa[0], sb[0], sc[0], proKPi_S);
        if (flag[1] == 1) {
          pro2[0] = proKPi_S[0]; pro2[1] = proKPi_S[1];
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}
        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Pip, Pi02, t1V);
        calt1(pS, pV, t1D);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += G[ii][ii] * t1D[ii] * t1V[ii];
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp2[0] = temp_PDF * B[1] * B[2] * pro[0];
        amp_tmp2[1] = temp_PDF * B[1] * B[2] * pro[1];
      } else if (modetype[i] == 2) {
        double t1V1[4], t1V2[4], t2D[4][4];
        temp_PDF = 0;

        double pV1[4], pV2[4];
        for (int ii = 0; ii != 4; ii++) {
          pV1[ii] = Ks[ii] + Pi01[ii];
          pV2[ii] = Pip[ii] + Pi02[ii];
          pD[ii] = pV1[ii] + pV2[ii];
        }
        sa[0] = SCADot(pV1, pV1);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi01, Pi01);
        sa[1] = SCADot(pV2, pV2);
        sb[1] = SCADot(Pip, Pip);
        sc[1] = SCADot(Pi02, Pi02);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[0];
        sc[2] = sa[1];
        if (flag[0] == 1) {
          propagatorRBW(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, 1, pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}
        if (flag[1] == 1) {
          propagatorGS(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        calt1(Ks, Pi01, t1V1);
        calt1(Pip, Pi02, t1V2);
        if (flag[2] == 0) {
          for (int ii = 0; ii != 4; ii++) {
            temp_PDF += (G[ii][ii]) * t1V1[ii] * t1V2[ii];
          }
          B[2] = 1;
        }
        if (flag[2] == 1) {
          calt1(pV1, pV2, t1D);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              for (int k = 0; k != 4; k++) {
                for (int l = 0; l != 4; l++) {
                  temp_PDF += E[ii][j][k][l] * pD[ii] * t1D[j] * t1V1[k] * t1V2[l] *
                              (G[ii][ii]) * (G[j][j]) * (G[l][l]) * (G[k][k]);
                }
              }
            }
          }
          B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        }
        if (flag[2] == 2) {
          calt2(pV1, pV2, t2D);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t2D[ii][j] * t1V1[ii] * t1V2[j] * (G[ii][ii]) * (G[j][j]);
            }
          }
          B[2] = Barrier(mD * mD, 2, sa[2], sb[2], sc[2], rD2);
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp1[0] = temp_PDF * B[0] * B[1] * B[2] * pro[0];
        amp_tmp1[1] = temp_PDF * B[0] * B[1] * B[2] * pro[1];
        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pV1[ii] = Ks[ii] + Pi02[ii];
          pV2[ii] = Pip[ii] + Pi01[ii];
          pD[ii] = pV1[ii] + pV2[ii];
        }
        sa[0] = SCADot(pV1, pV1);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pV2, pV2);
        sb[1] = SCADot(Pip, Pip);
        sc[1] = SCADot(Pi01, Pi01);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[0];
        sc[2] = sa[1];
        if (flag[0] == 1) {
          propagatorRBW(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, 1, pro1);
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}
        if (flag[1] == 1) {
          propagatorGS(mass_R[1]*mass_R[1], mass_R[1], width_R[1], sa[1], sb[1], sc[1], rRes2, pro2);
        } else if (flag[1] == 0) {pro2[0] = 1; pro2[1] = 0;}
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        calt1(Ks, Pi02, t1V1);
        calt1(Pip, Pi01, t1V2);
        if (flag[2] == 0) {
          for (int ii = 0; ii != 4; ii++) {
            temp_PDF += (G[ii][ii]) * t1V1[ii] * t1V2[ii];
          }
          B[2] = 1;
        }
        if (flag[2] == 1) {
          calt1(pV1, pV2, t1D);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              for (int k = 0; k != 4; k++) {
                for (int l = 0; l != 4; l++) {
                  temp_PDF += E[ii][j][k][l] * pD[ii] * t1D[j] * t1V1[k] * t1V2[l] *
                              (G[ii][ii]) * (G[j][j]) * (G[l][l]) * (G[k][k]);
                }
              }
            }
          }
          B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        }
        if (flag[2] == 2) {
          calt2(pV1, pV2, t2D);
          for (int ii = 0; ii != 4; ii++) {
            for (int j = 0; j != 4; j++) {
              temp_PDF += t2D[ii][j] * t1V1[ii] * t1V2[j] * (G[ii][ii]) * (G[j][j]);
            }
          }
          B[2] = Barrier(mD * mD, 2, sa[2], sb[2], sc[2], rD2);
        }
        Com_Multi(pro1, pro2, pro);
        amp_tmp2[0] = temp_PDF * B[0] * B[1] * B[2] * pro[0];
        amp_tmp2[1] = temp_PDF * B[0] * B[1] * B[2] * pro[1];
      } else if (modetype[i] == 43) {
        double KPi[4];
        for (int ii = 0; ii != 4; ii++) {
          KPi[ii] = Ks[ii] + Pi01[ii];
        }
        sa[0] = SCADot(KPi, KPi);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi01, Pi01);
        KPiSLASS(sa[0], sb[0], sc[0], proKPi_S);
        if (flag[0] == 1) {
          pro1[0] = proKPi_S[0]; pro1[1] = proKPi_S[1];
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        amp_tmp1[0] = pro1[0];
        amp_tmp1[1] = pro1[1];

        for (int ii = 0; ii != 4; ii++) {
          KPi[ii] = Ks[ii] + Pi02[ii];
        }
        sa[0] = SCADot(KPi, KPi);
        sb[0] = SCADot(Ks, Ks);
        sc[0] = SCADot(Pi02, Pi02);
        KPiSLASS(sa[0], sb[0], sc[0], proKPi_S);
        if (flag[0] == 1) {
          pro1[0] = proKPi_S[0]; pro1[1] = proKPi_S[1];
        } else if (flag[0] == 0) {pro1[0] = 1; pro1[1] = 0;}

        amp_tmp2[0] = pro1[0];
        amp_tmp2[1] = pro1[1];
      } else if (modetype[i] == 403) {
        temp_PDF = 0;
        double pP[4];
        for (int ii = 0; ii != 4; ii++) {
          pV[ii] = Pip[ii] + Pi02[ii];
          pP[ii] = pV[ii] + Pi01[ii];
          pD[ii] = pP[ii] + Ks[ii];
        }
        sa[0] = SCADot(pV, pV);
        sb[0] = SCADot(Pip, Pip);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pP, pP);
        sb[1] = sa[0];
        sc[1] = SCADot(Pi01, Pi01);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Ks, Ks);
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        propagatorGS(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, pro);
        calt1(Pip, Pi02, t1V);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += Pi01[ii] * t1V[ii] * (G[ii][ii]);
        }
        amp_tmp1[0] = temp_PDF * B[0] * B[1] * pro[0];
        amp_tmp1[1] = temp_PDF * B[0] * B[1] * pro[1];
        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pV[ii] = Pip[ii] + Pi01[ii];
          pP[ii] = pV[ii] + Pi02[ii];
          pD[ii] = pP[ii] + Ks[ii];
        }
        sa[0] = SCADot(pV, pV);
        sb[0] = SCADot(Pip, Pip);
        sc[0] = SCADot(Pi01, Pi01);
        sa[1] = SCADot(pP, pP);
        sb[1] = sa[0];
        sc[1] = SCADot(Pi02, Pi02);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[1];
        sc[2] = SCADot(Ks, Ks);
        B[0] = Barrier(mass_R[0] * mass_R[0], 1, sa[0], sb[0], sc[0], rRes2);
        B[1] = Barrier(mass_R[1] * mass_R[1], 1, sa[1], sb[1], sc[1], rRes2);
        propagatorGS(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[0], sb[0], sc[0], rRes2, pro);
        calt1(Pip, Pi01, t1V);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += Pi02[ii] * t1V[ii] * (G[ii][ii]);
        }
        amp_tmp2[0] = temp_PDF * B[0] * B[1] * pro[0];
        amp_tmp2[1] = temp_PDF * B[0] * B[1] * pro[1];
      } else if (modetype[i] == 220) {
        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pS[ii] = Pip[ii] + Pi02[ii];
          pV[ii] = Ks[ii] + Pi01[ii];
          pD[ii] = pS[ii] + pV[ii];
        }
        sa[0] = SCADot(pS, pS);
        sb[0] = SCADot(Pip, Pip);
        sc[0] = SCADot(Pi02, Pi02);
        sa[1] = SCADot(pV, pV);
        sb[1] = SCADot(Ks, Ks);
        sc[1] = SCADot(Pi01, Pi01);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[0];
        sc[2] = sa[1];
        if (flag[0] == 1) {
          propagatorRBW(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[1], sb[1], sc[1], rRes2, 1, pro);
        }
        if (flag[0] == 0) { pro[0] = 1; pro[1] = 0; }
        B[1] = Barrier(mass_R[0] * mass_R[0], 1, sa[1], sb[1], sc[1], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Ks, Pi01, t1V);
        calt1(pS, pV, t1D);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += G[ii][ii] * t1D[ii] * t1V[ii];
        }
        amp_tmp1[0] = temp_PDF * B[1] * B[2] * pro[0];
        amp_tmp1[1] = temp_PDF * B[1] * B[2] * pro[1];
        temp_PDF = 0;
        for (int ii = 0; ii != 4; ii++) {
          pS[ii] = Pip[ii] + Pi01[ii];
          pV[ii] = Ks[ii] + Pi02[ii];
          pD[ii] = pS[ii] + pV[ii];
        }
        sa[0] = SCADot(pS, pS);
        sb[0] = SCADot(Pip, Pip);
        sc[0] = SCADot(Pi01, Pi01);
        sa[1] = SCADot(pV, pV);
        sb[1] = SCADot(Ks, Ks);
        sc[1] = SCADot(Pi02, Pi02);
        sa[2] = SCADot(pD, pD);
        sb[2] = sa[0];
        sc[2] = sa[1];
        if (flag[0] == 1) {
          propagatorRBW(mass_R[0]*mass_R[0], mass_R[0], width_R[0], sa[1], sb[1], sc[1], rRes2, 1, pro);
        }
        if (flag[0] == 0) { pro[0] = 1; pro[1] = 0; }
        B[1] = Barrier(mass_R[0] * mass_R[0], 1, sa[1], sb[1], sc[1], rRes2);
        B[2] = Barrier(mD * mD, 1, sa[2], sb[2], sc[2], rD2);
        calt1(Ks, Pi02, t1V);
        calt1(pS, pV, t1D);
        for (int ii = 0; ii != 4; ii++) {
          temp_PDF += G[ii][ii] * t1D[ii] * t1V[ii];
        }
        amp_tmp2[0] = temp_PDF * B[1] * B[2] * pro[0];
        amp_tmp2[1] = temp_PDF * B[1] * B[2] * pro[1];
      }
      amp_tmp[0] = amp_tmp1[0] + amp_tmp2[0];
      amp_tmp[1] = amp_tmp1[1] + amp_tmp2[1];
      Com_Multi(amp_tmp, cof, amp_PDF);

      PDF[0] += amp_PDF[0];
      PDF[1] += amp_PDF[1];
    }
    double value = PDF[0] * PDF[0] + PDF[1] * PDF[1];

    Result = value;

    return value;
  }

  void EvtDToKSpipi0pi0::Com_Multi(const double a1[2], const double a2[2], double res[2])
  {
    res[0] = a1[0] * a2[0] - a1[1] * a2[1];
    res[1] = a1[1] * a2[0] + a1[0] * a2[1];
  }
  void EvtDToKSpipi0pi0::Com_Divide(const double a1[2], const double a2[2], double res[2])
  {
    double tmp = a2[0] * a2[0] + a2[1] * a2[1];
    res[0] = (a1[0] * a2[0] + a1[1] * a2[1]) / tmp;
    res[1] = (a1[1] * a2[0] - a1[0] * a2[1]) / tmp;
  }
  double EvtDToKSpipi0pi0::SCADot(const double a1[4], const double a2[4])
  {
    double _cal = a1[0] * a2[0] - a1[1] * a2[1] - a1[2] * a2[2] - a1[3] * a2[3];
    return _cal;
  }
  double EvtDToKSpipi0pi0::Barrier(const double mass2, const int l, const double sa, const double sb, const double sc,
                                   const double r2)
  {
    double F;
    double tmp = sa + sb - sc;
    double q = fabs(0.25 * tmp * tmp / sa - sb);
    double tmp2 = mass2 + sb - sc;
    double q0 = fabs(0.25 * tmp2 * tmp2 / mass2 - sb);
    double z = q * r2;
    double z0 = q0 * r2;
    if (l == 1) {
      F = sqrt((1.0 + z0) / (1.0 + z));
    } else if (l == 2) {
      double z2 = z * z; double z02 = z0 * z0;
      F = sqrt((9.0 + 3.0 * z0 + z02) / (9.0 + 3.0 * z + z2));
    } else {
      F = 1.0;
    }
    return F;
  }
  void EvtDToKSpipi0pi0::calt1(const double daug1[4], const double daug2[4], double t1[4])
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
  void EvtDToKSpipi0pi0::calt2(const double daug1[4], const double daug2[4], double t2[4][4])
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
  double EvtDToKSpipi0pi0::wid(const double mass2, const double mass, const double sa, const double sb, const double sc,
                               const double r2, const int l)
  {
    double widm = 0.;
    double m = sqrt(sa);
    double tmp  = sb - sc;
    double tmp1 = sa + tmp;
    double q = fabs(0.25 * tmp1 * tmp1 / sa - sb);
    double tmp2 = mass2 + tmp;
    double q0 = fabs(0.25 * tmp2 * tmp2 / mass2 - sb);
    double z = q * r2;
    double z0 = q0 * r2;
    double t = q / q0;
    if (l == 0)      {widm = sqrt(t) * mass / m;}
    else if (l == 1) {widm = t * sqrt(t) * mass / m * (1 + z0) / (1 + z);}
    else if (l == 2) {widm = t * t * sqrt(t) * mass / m * (9 + 3 * z0 + z0 * z0) / (9 + 3 * z + z * z);}
    return widm;
  }
  double EvtDToKSpipi0pi0::widl1(const double mass2, const double mass, const double sa, const double sb, const double sc,
                                 const double r2)
  {
    double widm = 0.;
    double m = sqrt(sa);
    double tmp  = sb - sc;
    double tmp1 = sa + tmp;
    double q = fabs(0.25 * tmp1 * tmp1 / sa - sb);
    double tmp2 = mass2 + tmp;
    double q0 = fabs(0.25 * tmp2 * tmp2 / mass2 - sb);
    double z = q * r2;
    double z0 = q0 * r2;
    double F = (1 + z0) / (1 + z);
    double t = q / q0;
    widm = t * sqrt(t) * mass / m * F;
    return widm;
  }
  void EvtDToKSpipi0pi0::propagatorRBW(const double mass2, const double mass, const double width, const double sa, const double sb,
                                       const double sc, const double r2, const int l, double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass2 - sa;
    b[1] = -mass * width * wid(mass2, mass, sa, sb, sc, r2, l);
    Com_Divide(a, b, prop);
  }
  void EvtDToKSpipi0pi0::propagatorRBWl1(const double mass2, const double mass, const double width, const double sa, const double sb,
                                         const double sc, const double r2, double prop[2])
  {
    double a[2], b[2];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass2 - sa;
    b[1] = -mass * width * widl1(mass2, mass, sa, sb, sc, r2);
    Com_Divide(a, b, prop);
  }
  void EvtDToKSpipi0pi0::propagatorRBW_a1(const double mass2, const double mass, const double width, const double sa, const double sb,
                                          const double sc, const double r2, const int l, double prop[2])
  {
    (void)width;
    (void)sb;
    (void)sc;
    (void)r2;
    (void)l;
    double a[2], b[2];
    int iii = int(sqrt(sa) * 1000) - 1;
    double a1width[3000] = {
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
      0.000000, 0.000001, 0.000001, 0.000001, 0.000002, 0.000002, 0.000002, 0.000003,
      0.000004, 0.000004, 0.000005, 0.000006, 0.000007, 0.000008, 0.000009, 0.000010,
      0.000011, 0.000012, 0.000014, 0.000015, 0.000017, 0.000019, 0.000021, 0.000023,
      0.000025, 0.000027, 0.000029, 0.000032, 0.000035, 0.000038, 0.000041, 0.000044,
      0.000047, 0.000050, 0.000054, 0.000058, 0.000062, 0.000066, 0.000070, 0.000075,
      0.000079, 0.000084, 0.000089, 0.000094, 0.000100, 0.000105, 0.000111, 0.000117,
      0.000124, 0.000130, 0.000137, 0.000143, 0.000151, 0.000158, 0.000165, 0.000173,
      0.000182, 0.000190, 0.000199, 0.000207, 0.000216, 0.000225, 0.000235, 0.000245,
      0.000256, 0.000266, 0.000277, 0.000288, 0.000300, 0.000311, 0.000322, 0.000335,
      0.000347, 0.000360, 0.000373, 0.000385, 0.000400, 0.000415, 0.000429, 0.000442,
      0.000457, 0.000473, 0.000488, 0.000504, 0.000520, 0.000539, 0.000555, 0.000572,
      0.000590, 0.000608, 0.000626, 0.000646, 0.000664, 0.000684, 0.000704, 0.000725,
      0.000745, 0.000766, 0.000787, 0.000809, 0.000828, 0.000854, 0.000878, 0.000901,
      0.000927, 0.000952, 0.000973, 0.001001, 0.001027, 0.001048, 0.001080, 0.001104,
      0.001132, 0.001159, 0.001189, 0.001219, 0.001245, 0.001277, 0.001308, 0.001338,
      0.001370, 0.001404, 0.001433, 0.001468, 0.001498, 0.001533, 0.001570, 0.001600,
      0.001638, 0.001678, 0.001711, 0.001745, 0.001780, 0.001825, 0.001857, 0.001898,
      0.001941, 0.001972, 0.002017, 0.002065, 0.002104, 0.002146, 0.002189, 0.002234,
      0.002277, 0.002319, 0.002369, 0.002410, 0.002461, 0.002511, 0.002557, 0.002605,
      0.002661, 0.002704, 0.002762, 0.002807, 0.002855, 0.002910, 0.002965, 0.003020,
      0.003074, 0.003127, 0.003178, 0.003228, 0.003288, 0.003351, 0.003409, 0.003471,
      0.003532, 0.003598, 0.003660, 0.003720, 0.003793, 0.003854, 0.003910, 0.003972,
      0.004050, 0.004108, 0.004181, 0.004254, 0.004309, 0.004380, 0.004464, 0.004533,
      0.004603, 0.004679, 0.004756, 0.004811, 0.004898, 0.004974, 0.005048, 0.005142,
      0.005215, 0.005279, 0.005363, 0.005449, 0.005533, 0.005604, 0.005695, 0.005783,
      0.005869, 0.005971, 0.006060, 0.006142, 0.006247, 0.006332, 0.006409, 0.006502,
      0.006594, 0.006713, 0.006784, 0.006889, 0.006995, 0.007079, 0.007190, 0.007303,
      0.007381, 0.007487, 0.007592, 0.007710, 0.007801, 0.007910, 0.008032, 0.008149,
      0.008247, 0.008378, 0.008462, 0.008559, 0.008706, 0.008843, 0.008943, 0.009091,
      0.009207, 0.009308, 0.009448, 0.009555, 0.009698, 0.009810, 0.009936, 0.010020,
      0.010186, 0.010320, 0.010474, 0.010611, 0.010742, 0.010840, 0.011011, 0.011167,
      0.011281, 0.011395, 0.011541, 0.011714, 0.011853, 0.012046, 0.012169, 0.012277,
      0.012460, 0.012617, 0.012805, 0.012922, 0.013072, 0.013234, 0.013389, 0.013561,
      0.013704, 0.013917, 0.014025, 0.014239, 0.014425, 0.014600, 0.014716, 0.014958,
      0.015114, 0.015325, 0.015488, 0.015630, 0.015797, 0.016035, 0.016206, 0.016404,
      0.016591, 0.016842, 0.016964, 0.017199, 0.017392, 0.017557, 0.017798, 0.017987,
      0.018178, 0.018337, 0.018631, 0.018829, 0.019008, 0.019221, 0.019467, 0.019698,
      0.019941, 0.020166, 0.020379, 0.020585, 0.020806, 0.021040, 0.021309, 0.021482,
      0.021764, 0.022046, 0.022306, 0.022478, 0.022736, 0.023049, 0.023222, 0.023536,
      0.023756, 0.024090, 0.024337, 0.024557, 0.024873, 0.025099, 0.025392, 0.025682,
      0.025995, 0.026291, 0.026498, 0.026927, 0.027119, 0.027377, 0.027804, 0.028135,
      0.028279, 0.028682, 0.028871, 0.029355, 0.029531, 0.029956, 0.030243, 0.030592,
      0.030873, 0.031246, 0.031494, 0.031771, 0.032167, 0.032515, 0.032881, 0.033211,
      0.033653, 0.033988, 0.034394, 0.034639, 0.035055, 0.035569, 0.035879, 0.036211,
      0.036611, 0.036932, 0.037489, 0.037779, 0.038284, 0.038723, 0.039001, 0.039574,
      0.039854, 0.040274, 0.040881, 0.041117, 0.041644, 0.042055, 0.042531, 0.043030,
      0.043354, 0.043832, 0.044277, 0.044956, 0.045284, 0.045828, 0.046440, 0.046800,
      0.047518, 0.047727, 0.048258, 0.048850, 0.049316, 0.049992, 0.050486, 0.050987,
      0.051410, 0.051928, 0.052613, 0.053110, 0.053824, 0.054351, 0.055078, 0.055654,
      0.056030, 0.056763, 0.057245, 0.057832, 0.058569, 0.059292, 0.060048, 0.060569,
      0.061056, 0.061869, 0.062612, 0.063186, 0.063886, 0.064655, 0.065198, 0.065815,
      0.066649, 0.067577, 0.068012, 0.068967, 0.069630, 0.070181, 0.070786, 0.071989,
      0.072764, 0.073466, 0.074461, 0.075093, 0.075994, 0.076834, 0.077455, 0.078709,
      0.079581, 0.080408, 0.080884, 0.081965, 0.082882, 0.083658, 0.084824, 0.085513,
      0.086662, 0.087602, 0.088678, 0.089492, 0.090641, 0.091369, 0.092494, 0.093484,
      0.094615, 0.095385, 0.096168, 0.097668, 0.098611, 0.099630, 0.100772, 0.102020,
      0.103145, 0.104110, 0.105071, 0.106604, 0.107791, 0.108451, 0.109509, 0.111356,
      0.112026, 0.113921, 0.114507, 0.116071, 0.117027, 0.118213, 0.120164, 0.120701,
      0.122121, 0.123894, 0.124937, 0.126134, 0.127391, 0.128882, 0.130056, 0.131649,
      0.133046, 0.134275, 0.135119, 0.137072, 0.138476, 0.139612, 0.140388, 0.142734,
      0.143576, 0.145445, 0.147414, 0.148856, 0.149891, 0.150963, 0.152477, 0.153717,
      0.155275, 0.156859, 0.158462, 0.159257, 0.161865, 0.163182, 0.164465, 0.165538,
      0.167003, 0.169257, 0.171211, 0.172093, 0.173261, 0.174639, 0.176510, 0.177684,
      0.179077, 0.181041, 0.182446, 0.184769, 0.184926, 0.186741, 0.188844, 0.190884,
      0.191714, 0.192254, 0.193921, 0.195917, 0.196766, 0.199052, 0.200603, 0.201808,
      0.202699, 0.204636, 0.205712, 0.206849, 0.208741, 0.209424, 0.211698, 0.212753,
      0.215516, 0.215857, 0.217790, 0.217774, 0.220454, 0.221821, 0.223466, 0.224494,
      0.225632, 0.227231, 0.229456, 0.229581, 0.231537, 0.232263, 0.233834, 0.234725,
      0.237079, 0.238015, 0.239400, 0.240193, 0.241693, 0.243787, 0.244317, 0.244971,
      0.246711, 0.248615, 0.249387, 0.250905, 0.252702, 0.253535, 0.254385, 0.255375,
      0.256671, 0.258405, 0.259741, 0.260875, 0.262131, 0.262920, 0.264860, 0.265893,
      0.266016, 0.267727, 0.270039, 0.270689, 0.271047, 0.272313, 0.272474, 0.274724,
      0.275813, 0.275937, 0.278793, 0.278783, 0.281407, 0.281351, 0.282481, 0.284226,
      0.284113, 0.284999, 0.285655, 0.288361, 0.287856, 0.288893, 0.290211, 0.291708,
      0.291985, 0.294298, 0.294849, 0.296796, 0.296197, 0.296851, 0.298011, 0.300368,
      0.299982, 0.302378, 0.304363, 0.303711, 0.304729, 0.306789, 0.306378, 0.307372,
      0.308720, 0.309509, 0.309712, 0.310782, 0.311699, 0.312668, 0.312755, 0.313675,
      0.315311, 0.316640, 0.317217, 0.317403, 0.318478, 0.319916, 0.321803, 0.322678,
      0.323237, 0.324343, 0.324433, 0.324493, 0.324969, 0.325894, 0.328563, 0.328721,
      0.328954, 0.330640, 0.328164, 0.331267, 0.331695, 0.333772, 0.333619, 0.334351,
      0.334605, 0.336434, 0.337510, 0.336535, 0.337362, 0.338799, 0.340732, 0.339896,
      0.342707, 0.343471, 0.342318, 0.342431, 0.344543, 0.345611, 0.345786, 0.346590,
      0.346610, 0.347761, 0.348914, 0.349558, 0.350577, 0.352128, 0.350982, 0.354134,
      0.352773, 0.353213, 0.352972, 0.354927, 0.355784, 0.355778, 0.355801, 0.357040,
      0.358013, 0.358432, 0.360045, 0.359743, 0.360238, 0.359850, 0.362184, 0.361580,
      0.363430, 0.362333, 0.364397, 0.364472, 0.364370, 0.365303, 0.366644, 0.367777,
      0.368604, 0.367631, 0.368324, 0.369782, 0.371121, 0.370653, 0.370040, 0.371649,
      0.370201, 0.373362, 0.373900, 0.374159, 0.374916, 0.374503, 0.376703, 0.372802,
      0.376191, 0.379596, 0.377325, 0.376363, 0.379369, 0.379791, 0.378703, 0.380177,
      0.381762, 0.381335, 0.381374, 0.384668, 0.381763, 0.382746, 0.384723, 0.385089,
      0.386229, 0.386702, 0.387749, 0.384423, 0.384714, 0.384181, 0.388489, 0.388618,
      0.388179, 0.390092, 0.389871, 0.390496, 0.391181, 0.390679, 0.392614, 0.392269,
      0.393899, 0.393466, 0.391421, 0.391090, 0.395586, 0.391776, 0.396882, 0.393254,
      0.394400, 0.395749, 0.398063, 0.397138, 0.397585, 0.397288, 0.397847, 0.395375,
      0.400170, 0.400007, 0.401191, 0.398513, 0.401922, 0.400477, 0.404257, 0.403271,
      0.400677, 0.403913, 0.403172, 0.404727, 0.403406, 0.404404, 0.405265, 0.406389,
      0.405738, 0.402173, 0.407831, 0.405895, 0.409172, 0.408934, 0.405915, 0.408486,
      0.407320, 0.407437, 0.405444, 0.408400, 0.410909, 0.412427, 0.409881, 0.411021,
      0.413001, 0.410369, 0.414702, 0.413372, 0.413095, 0.410972, 0.416346, 0.416095,
      0.414132, 0.414344, 0.416952, 0.415197, 0.417583, 0.416582, 0.416622, 0.416895,
      0.416576, 0.415551, 0.417925, 0.414838, 0.417051, 0.416831, 0.420000, 0.419132,
      0.418173, 0.417645, 0.419679, 0.419866, 0.419581, 0.421531, 0.420878, 0.422737,
      0.421872, 0.421304, 0.425486, 0.424434, 0.420842, 0.426753, 0.422761, 0.422178,
      0.422372, 0.424173, 0.425582, 0.425080, 0.425831, 0.423551, 0.422949, 0.425784,
      0.427977, 0.427948, 0.426368, 0.425138, 0.425351, 0.428643, 0.428148, 0.427488,
      0.431704, 0.430167, 0.429655, 0.429584, 0.425458, 0.430728, 0.429845, 0.431145,
      0.429180, 0.428874, 0.430720, 0.430024, 0.432034, 0.431359, 0.431535, 0.432995,
      0.432425, 0.432454, 0.433140, 0.432574, 0.433814, 0.433348, 0.432886, 0.435472,
      0.436517, 0.432681, 0.436999, 0.435182, 0.434834, 0.435478, 0.438255, 0.436650,
      0.434464, 0.438530, 0.434077, 0.436471, 0.434012, 0.436822, 0.437505, 0.440135,
      0.438322, 0.438032, 0.439001, 0.440270, 0.438661, 0.439233, 0.439274, 0.437945,
      0.443080, 0.439191, 0.438233, 0.440415, 0.441063, 0.440926, 0.440929, 0.439731,
      0.443584, 0.439729, 0.441597, 0.442615, 0.444637, 0.443180, 0.440789, 0.440261,
      0.442202, 0.445081, 0.445484, 0.445415, 0.445532, 0.442806, 0.444188, 0.441073,
      0.444299, 0.445897, 0.445279, 0.442830, 0.445506, 0.445272, 0.447267, 0.443522,
      0.445519, 0.446459, 0.446753, 0.446377, 0.446129, 0.446383, 0.448556, 0.446593,
      0.445293, 0.449199, 0.447590, 0.445968, 0.447482, 0.448474, 0.449890, 0.450004,
      0.447765, 0.449274, 0.450652, 0.448210, 0.449360, 0.449577, 0.448575, 0.452112,
      0.448780, 0.451393, 0.450200, 0.452018, 0.451182, 0.452050, 0.451748, 0.451377,
      0.451402, 0.448810, 0.452311, 0.452909, 0.452491, 0.452418, 0.454190, 0.454420,
      0.452121, 0.452307, 0.456857, 0.453506, 0.454058, 0.457203, 0.454394, 0.453596,
      0.452240, 0.453692, 0.456516, 0.453753, 0.455541, 0.452702, 0.456481, 0.452226,
      0.454280, 0.454855, 0.456297, 0.456482, 0.454154, 0.455387, 0.454748, 0.455764,
      0.457282, 0.455487, 0.454822, 0.454257, 0.457678, 0.454225, 0.458689, 0.456123,
      0.457011, 0.457386, 0.458351, 0.458638, 0.456164, 0.455884, 0.458525, 0.457575,
      0.458340, 0.458912, 0.457836, 0.461734, 0.457545, 0.460755, 0.460960, 0.459226,
      0.458613, 0.461078, 0.460958, 0.460337, 0.460237, 0.461190, 0.460760, 0.457911,
      0.461310, 0.459657, 0.461960, 0.461040, 0.459578, 0.461650, 0.461550, 0.461251,
      0.461054, 0.463082, 0.461732, 0.461324, 0.462547, 0.461261, 0.461629, 0.464067,
      0.462430, 0.462525, 0.464232, 0.462921, 0.463202, 0.465558, 0.462914, 0.461698,
      0.463963, 0.463040, 0.464275, 0.461940, 0.462913, 0.465261, 0.461500, 0.463679,
      0.463354, 0.465205, 0.464529, 0.462220, 0.464279, 0.463427, 0.465387, 0.465288,
      0.464839, 0.464926, 0.466100, 0.465531, 0.466187, 0.464647, 0.466285, 0.465461,
      0.464134, 0.466783, 0.466763, 0.466183, 0.467089, 0.464497, 0.466080, 0.466109,
      0.468166, 0.466984, 0.465335, 0.466721, 0.466856, 0.465113, 0.468377, 0.467904,
      0.464546, 0.468787, 0.465648, 0.469841, 0.469477, 0.466311, 0.468700, 0.465183,
      0.466559, 0.470433, 0.468563, 0.468109, 0.466980, 0.467567, 0.467670, 0.466991,
      0.467992, 0.468784, 0.469406, 0.469652, 0.468527, 0.470460, 0.467308, 0.470693,
      0.469539, 0.468000, 0.469295, 0.467038, 0.471908, 0.468829, 0.470663, 0.469266,
      0.468975, 0.470222, 0.468649, 0.469507, 0.472307, 0.471611, 0.470419, 0.471181,
      0.471140, 0.473187, 0.471086, 0.469801, 0.472234, 0.472131, 0.468996, 0.470229,
      0.471597, 0.469625, 0.472230, 0.470164, 0.468404, 0.472264, 0.471336, 0.471597,
      0.472280, 0.471256, 0.473151, 0.471863, 0.474458, 0.471956, 0.473099, 0.473956,
      0.471725, 0.472809, 0.473065, 0.473180, 0.470611, 0.473614, 0.474263, 0.472792,
      0.473543, 0.472656, 0.469728, 0.473431, 0.474538, 0.475322, 0.474962, 0.473598,
      0.474114, 0.473486, 0.472934, 0.473252, 0.477149, 0.471719, 0.476383, 0.473076,
      0.473952, 0.473104, 0.472459, 0.474433, 0.474494, 0.473588, 0.473839, 0.478113,
      0.472435, 0.475571, 0.475194, 0.475626, 0.474617, 0.474520, 0.474472, 0.476437,
      0.474512, 0.474497, 0.474628, 0.476203, 0.475698, 0.473907, 0.477144, 0.479000,
      0.475553, 0.477481, 0.473998, 0.476672, 0.477115, 0.477114, 0.476282, 0.476152,
      0.477009, 0.479854, 0.474354, 0.477645, 0.477517, 0.477111, 0.474843, 0.476173,
      0.477321, 0.477384, 0.477880, 0.475726, 0.476004, 0.478204, 0.475586, 0.477973,
      0.477935, 0.480640, 0.478234, 0.476349, 0.477493, 0.476994, 0.479815, 0.477771,
      0.476333, 0.476325, 0.478245, 0.477284, 0.479238, 0.478339, 0.478966, 0.478012,
      0.479304, 0.480148, 0.476125, 0.481267, 0.479801, 0.476720, 0.478898, 0.479284,
      0.479153, 0.480157, 0.478681, 0.479712, 0.478993, 0.479943, 0.478349, 0.478930,
      0.478052, 0.477173, 0.479244, 0.480454, 0.479128, 0.480530, 0.477843, 0.478369,
      0.478561, 0.478639, 0.479191, 0.481763, 0.481321, 0.480979, 0.479702, 0.479777,
      0.479384, 0.477571, 0.481880, 0.478615, 0.481303, 0.478783, 0.479384, 0.480517,
      0.481928, 0.481199, 0.479041, 0.479188, 0.481491, 0.482840, 0.478766, 0.481941,
      0.481298, 0.478105, 0.482933, 0.479744, 0.483361, 0.482332, 0.482556, 0.482057,
      0.483616, 0.480599, 0.482245, 0.481091, 0.480871, 0.481938, 0.480678, 0.481851,
      0.482902, 0.482158, 0.480187, 0.481772, 0.484967, 0.483094, 0.482133, 0.483929,
      0.483354, 0.483382, 0.483964, 0.479941, 0.481375, 0.480255, 0.482184, 0.482541,
      0.482032, 0.483484, 0.479492, 0.483305, 0.481070, 0.483573, 0.485689, 0.485767,
      0.484221, 0.481365, 0.482440, 0.481507, 0.483418, 0.480978
    };
    double width_a1 = a1width[iii];
    a[0] = 1;
    a[1] = 0;
    b[0] = mass2 - sa;
    b[1] = -mass * width_a1;
    Com_Divide(a, b, prop);
  }
  void EvtDToKSpipi0pi0::propagatorGS(const double mass2, const double mass, const double width, const double sa, const double sb,
                                      const double sc, const double r2, double prop[2])
  {

    double GS1 = 0.636619783;
    double GS2 = 0.01860182466;
    double GS3 = 0.1591549458;
    double GS4 = 0.00620060822;
    double a[2], b[2];
    double tmp  = sb - sc;
    double tmp1 = sa + tmp;
    double q2 = fabs(0.25 * tmp1 * tmp1 / sa - sb);
    double tmp2 = mass2 + tmp;
    double q02 = fabs(0.25 * tmp2 * tmp2 / mass2 - sb);

    double q  = sqrt(q2);
    double q0 = sqrt(q02);
    double m  = sqrt(sa);
    double q03 = q0 * q02;
    double tmp3 = log(mass + 2 * q0) + 1.2760418309; // log(mass_2Pion) = 1.2760418309;

    double h  = GS1 * q / m * (log(m + 2 * q) + 1.2760418309);
    double h0 = GS1 * q0 / mass * tmp3;
    double dh = h0 * (0.125 / q02 - 0.5 / mass2) + GS3 / mass2;
    double d  = GS2 / q02 * tmp3 + GS3 * mass / q0 - GS4 * mass / q03;
    double f  = mass2 / q03 * (q2 * (h - h0) + (mass2 - sa) * q02 * dh);

    a[0] = 1.0 + d * width / mass;
    a[1] = 0.0;
    b[0] = mass2 - sa + width * f;
    b[1] = -mass * width * widl1(mass2, mass, sa, sb, sc, r2);
    Com_Divide(a, b, prop);
  }
  void EvtDToKSpipi0pi0::rhoab(const double sa, const double sb, const double sc, double res[2])
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
  void EvtDToKSpipi0pi0::rho4Pi(const double sa, double res[2])
  {
    double temp = 1.0 - 0.3116765584 / sa; // 0.3116765584=0.13957*0.13957*16
    if (temp >= 0) {
      res[0] = sqrt(temp) / (1.0 + exp(9.8 - 3.5 * sa));
      res[1] = 0.0;
    } else {
      res[0] = 0.0;
      res[1] = sqrt(-temp) / (1.0 + exp(9.8 - 3.5 * sa));
    }
  }
  void EvtDToKSpipi0pi0::propagatorsigma500(const double sa, const double sb, const double sc, double prop[2])
  {
    double f = 0.5843 + 1.6663 * sa;
    const double M = 0.9264;
    const double mass2 = 0.85821696;  // M*M
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
  void EvtDToKSpipi0pi0::KPiSLASS(const double sa, const double sb, const double sc, double prop[2])
  {
    const double m1430 = 1.441;
    const double sa0   = 2.076481;   // m1430*m1430;
    const double w1430 = 0.193;
    const double Lass1 = 0.25 / sa0;
    double tmp = sb - sc;
    double tmp1 = sa0 + tmp;
    double q0 = fabs(Lass1 * tmp1 * tmp1 - sb);
    double tmp2 = sa + tmp;
    double qs = fabs(0.25 * tmp2 * tmp2 / sa - sb);
    double q = sqrt(qs);
    double width = w1430 * q * m1430 / sqrt(sa * q0);
    double temp_R = atan(m1430 * width / (sa0 - sa));
    if (temp_R < 0) temp_R += math_pi;
    double deltaR = -109.7 * math_pi / 180.0 + temp_R;
    double temp_F = atan(0.226 * q / (2.0 - 3.8194 * qs)); // 2.0*0.113 = 0.226; -33.8*0.113 = -3.8194
    if (temp_F < 0) temp_F += math_pi;
    double deltaF = 0.1 * math_pi / 180.0 + temp_F;
    double deltaS = deltaR + 2.0 * deltaF;
    double t1 = 0.96 * sin(deltaF);
    double t2 = sin(deltaR);
    double CF[2], CS[2];
    CF[0] = cos(deltaF);
    CF[1] = sin(deltaF);
    CS[0] = cos(deltaS);
    CS[1] = sin(deltaS);
    prop[0] = t1 * CF[0] + t2 * CS[0];
    prop[1] = t1 * CF[1] + t2 * CS[1];

  }

} // Belle 2 namespace
