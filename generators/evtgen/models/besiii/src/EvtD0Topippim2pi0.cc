// Model: EvtD0Topippim2pi0
// This file is an amplitude model for D0 -> pi+ pi- pi0 pi0.
// The model is from the BESIII Collaboration in Chin. Phys. C 48, 083001 (2024). DOI:&nbsp; https://doi.org/10.1088/1674-1137/ad3d4d
//
// Permission to include these files in basf2 was generously granted by the BESIII Collaboration.
//
// Please cite the original reference for any public/published results where this model was used.

//--------------------------------------------------------------------------
//
// Environment:
//      This software is part of models developed at BES collaboration
//      based on the EvtGen framework.  If you use all or part
//      of it, please give an appropriate acknowledgement.
//
// Copyright Information: See EvtGen/BesCopyright
//      Copyright (A) 2006      Ping Rong-Gang @IHEP
//
// Module:  EvtD0Topippim2pi0.cc
//
// Description: Chin. Phys. C 48, 083001 (2024)
//
// Modification history:
//
//    Liaoyuan Dong   Aug. 22, 2024 Module updated
//------------------------------------------------------------------------
#include "EvtGenBase/EvtPatches.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtVector4R.hh"
#include "EvtGenBase/EvtVector3R.hh"
#include "EvtGenBase/EvtReport.hh"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <complex>
#include <vector>
#include <math.h>
#include "TMath.h"

#include <generators/evtgen/EvtGenModelRegister.h>
#include "generators/evtgen/models/besiii/EvtD0Topippim2pi0.h"

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtD0Topippim2pi0);

  EvtD0Topippim2pi0::~EvtD0Topippim2pi0() {}

  std::string EvtD0Topippim2pi0::getName()
  {
    return "D0Topippim2pi0";
  }

  EvtDecayBase* EvtD0Topippim2pi0::clone()
  {
    return new EvtD0Topippim2pi0;
  }

  void EvtD0Topippim2pi0::init()
  {
    checkNArg(2);
    checkNDaug(4);
    charm   = getArg(0);
    tagmode = getArg(1);
    //std::cout<<"Initializing EvtD0Topippim2pi0: charm="<<charm<<" tagmode= "<<tagmode<<std::endl;

    double mag[36], pha[36];
    mag[0] = 100; pha[0] = 0;
    mag[1] = 6.03691; pha[1] = 0.0111121;
    mag[2] = 31.0556; pha[2] = -1.58228;
    mag[3] = 82.5439; pha[3] = -2.17864;
    mag[4] = 246.483; pha[4] = 0.337741;
    mag[5] = 659.439; pha[5] = 1.76185;
    mag[6] = 0.353305; pha[6] = 0.22473;
    mag[7] = 0.508529; pha[7] = -2.99122;
    mag[8] = 19.0362; pha[8] = 2.70136;
    mag[9] = 20.1349; pha[9] = -2.06863;
    mag[10] = 10.5129; pha[10] = -1.26235;
    mag[11] = 0.23207; pha[11] = -2.91839;
    mag[12] = 0.295583; pha[12] = -0.47002;
    mag[13] = 9.71285; pha[13] = -0.58869;
    mag[14] = 65.1187; pha[14] = -2.63145;
    mag[15] = 5.04613; pha[15] = -0.642076;
    mag[16] = 43.8422; pha[16] = 0.339301;
    mag[17] = 76.3005; pha[17] = -2.32496;
    mag[18] = 61.1311; pha[18] = 0.609366;
    mag[19] = 12.2241; pha[19] = -1.12858;
    mag[20] = 13.6557; pha[20] = 3.02972;
    mag[21] = 7.09973; pha[21] = -1.69019;
    mag[22] = 4.5858; pha[22] = 0.0637721;
    mag[23] = 8.0728; pha[23] = -1.01323;
    mag[24] = 8.40155; pha[24] = -1.68028;
    mag[25] = 40.748; pha[25] = -0.503741;
    mag[26] = 120.464; pha[26] = 1.72667;
    mag[27] = 2224.09; pha[27] = -1.04373;
    mag[28] = 7286.76; pha[28] = 1.72657;
    mag[29] = 8815.51; pha[29] = -1.10724;
    mag[30] = 2433.26; pha[30] = 1.79639;
    mag[31] = 5417.04; pha[31] = 2.67794;
    mag[32] = 18.3282; pha[32] = -1.3945;
    mag[33] = 55.535; pha[33] = 2.29339;
    mag[34] = 1.57835; pha[34] = -0.497796;
    mag[35] = 0.439629; pha[35] = 2.50596;


    fitpara.clear();
    for (int i = 0; i < 36; i++) {
      std::complex<double> ctemp(mag[i]*cos(pha[i]), mag[i]*sin(pha[i]));
      fitpara.push_back(ctemp);
    }

    g_uv.clear();
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        if (i != j) {
          g_uv.push_back(0.0);
        } else if (i < 3) {
          g_uv.push_back(-1.0);
        } else if (i == 3) {
          g_uv.push_back(1.0);
        }
      }
    }

    epsilon_uvmn.clear();
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
          for (int l = 0; l < 4; l++) {
            if (i == j || i == k || i == l || j == k || j == l || k == l) {
              epsilon_uvmn.push_back(0.0);
            } else {
              if (i == 0 && j == 1 && k == 2 && l == 3) epsilon_uvmn.push_back(1.0);
              if (i == 0 && j == 1 && k == 3 && l == 2) epsilon_uvmn.push_back(-1.0);
              if (i == 0 && j == 2 && k == 1 && l == 3) epsilon_uvmn.push_back(-1.0);
              if (i == 0 && j == 2 && k == 3 && l == 1) epsilon_uvmn.push_back(1.0);
              if (i == 0 && j == 3 && k == 1 && l == 2) epsilon_uvmn.push_back(1.0);
              if (i == 0 && j == 3 && k == 2 && l == 1) epsilon_uvmn.push_back(-1.0);

              if (i == 1 && j == 0 && k == 2 && l == 3) epsilon_uvmn.push_back(-1.0);
              if (i == 1 && j == 0 && k == 3 && l == 2) epsilon_uvmn.push_back(1.0);
              if (i == 1 && j == 2 && k == 0 && l == 3) epsilon_uvmn.push_back(1.0);
              if (i == 1 && j == 2 && k == 3 && l == 0) epsilon_uvmn.push_back(-1.0);
              if (i == 1 && j == 3 && k == 0 && l == 2) epsilon_uvmn.push_back(-1.0);
              if (i == 1 && j == 3 && k == 2 && l == 0) epsilon_uvmn.push_back(1.0);

              if (i == 2 && j == 0 && k == 1 && l == 3) epsilon_uvmn.push_back(1.0);
              if (i == 2 && j == 0 && k == 3 && l == 1) epsilon_uvmn.push_back(-1.0);
              if (i == 2 && j == 1 && k == 0 && l == 3) epsilon_uvmn.push_back(-1.0);
              if (i == 2 && j == 1 && k == 3 && l == 0) epsilon_uvmn.push_back(1.0);
              if (i == 2 && j == 3 && k == 0 && l == 1) epsilon_uvmn.push_back(1.0);
              if (i == 2 && j == 3 && k == 1 && l == 0) epsilon_uvmn.push_back(-1.0);

              if (i == 3 && j == 0 && k == 1 && l == 2) epsilon_uvmn.push_back(-1.0);
              if (i == 3 && j == 0 && k == 2 && l == 1) epsilon_uvmn.push_back(1.0);
              if (i == 3 && j == 1 && k == 0 && l == 2) epsilon_uvmn.push_back(1.0);
              if (i == 3 && j == 1 && k == 2 && l == 0) epsilon_uvmn.push_back(-1.0);
              if (i == 3 && j == 2 && k == 0 && l == 1) epsilon_uvmn.push_back(-1.0);
              if (i == 3 && j == 2 && k == 1 && l == 0) epsilon_uvmn.push_back(1.0);

            }
          }
        }
      }
    }

    _nd = 4;
    math_pi = 3.1415926f;
    mass_Pion = 0.13957f;

    rRes = 3.0 * 0.197321;
    rD   = 5.0 * 0.197321;
    m_Pi  = 0.139570;
    m_Pi0  = 0.134977;
    m2_Pi = m_Pi * m_Pi;
    m2_Pi0 = m_Pi0 * m_Pi0;
    m_Ka  = 0.493677;
    m2_Ka = m_Ka * m_Ka;

    m0_f0980  = 0.965;
    g1_f0980  = 0.165;
    g2_f0980  = 4.210;

    m0_rho7700 = 0.77526;
    w0_rho7700 = 0.1478;

    m0_rho770p = 0.77511;
    w0_rho770p = 0.1491;

    m0_rho1450 = 1.465;
    w0_rho1450 = 0.400;

    m0_f21270 = 1.2755;
    w0_f21270 = 0.1867;

    m0_a11260 = 1.1927;
    g1_a11260 = 0.003777;
    g2_a11260 = 0.0;

    m0_pi1300 = 1.534;
    w0_pi1300 = 0.610;

    m0_a11420 = 1.411;
    w0_a11420 = 0.161;

    m0_a11640 = 1.655;
    w0_a11640 = 0.254;

    m0_a21320 = 1.3186;
    w0_a21320 = 0.105;

    m0_pi21670 = 1.6706;
    w0_pi21670 = 0.258;

    m0_h11170  = 1.166;
    w0_h11170  = 0.375;

    m0_omega   = 0.78265;
    w0_omega   = 0.00849;

    m0_phi     = 1.019461;
    w0_phi     = 0.004249;

    s0_prod = -5.0;

  }

  void EvtD0Topippim2pi0::initProbMax()
  {
    setProbMax(1098009205);
  }

  void EvtD0Topippim2pi0::decay(EvtParticle* p)
  {
    /*
       double maxprob = 0.0;
       for(int ir=0;ir<=60000000;ir++){
       p->initializePhaseSpace(getNDaug(),getDaugs());
       for(int i=0; i<_nd; i++){
       _p4Lab[i]=p->getDaug(i)->getP4Lab();
       _p4CM[i]=p->getDaug(i)->getP4();
       }
       double Prob = AmplitudeSquare(charm, tagmode);
       if(Prob>maxprob) {
       maxprob=Prob;
       std::cout << "Max PDF = " << ir << " prob= " << Prob << std::endl;
       }
       }
       std::cout << "Max!!!!!!!!!!! " << maxprob<< std::endl;
       */
    p->initializePhaseSpace(getNDaug(), getDaugs());
    for (int i = 0; i < _nd; i++) {
      _p4Lab[i] = p->getDaug(i)->getP4Lab();
      _p4CM[i] = p->getDaug(i)->getP4();
    }
    double prob = AmplitudeSquare(charm, tagmode);
//  std::cout<<"Amplitude in EvtGen: "<<prob<<std::endl;
    setProb(prob);
    return;
  }

  void EvtD0Topippim2pi0::setInput(double* pip, double* pim, double* pi01, double* pi02)
  {
    m_Pip.clear(); m_Pim.clear(); m_Pi01.clear(); m_Pi02.clear();
    m_Pip.push_back(pip[0]); m_Pim.push_back(pim[0]); m_Pi01.push_back(pi01[0]); m_Pi02.push_back(pi02[0]);
    m_Pip.push_back(pip[1]); m_Pim.push_back(pim[1]); m_Pi01.push_back(pi01[1]); m_Pi02.push_back(pi02[1]);
    m_Pip.push_back(pip[2]); m_Pim.push_back(pim[2]); m_Pi01.push_back(pi01[2]); m_Pi02.push_back(pi02[2]);
    m_Pip.push_back(pip[3]); m_Pim.push_back(pim[3]); m_Pi01.push_back(pi01[3]); m_Pi02.push_back(pi02[3]);
  }

  std::vector<double> EvtD0Topippim2pi0::sum_tensor(std::vector<double> pa, std::vector<double> pb)
  {
    //if (pa.size() != pb.size()) {
    //  std::cout << "error sum tensor" << std::endl;
    //  exit(0);
    //}

    std::vector<double> temp; temp.clear();
    for (size_t i = 0; i < pa.size(); i++) {
      double sum = pa[i] + pb[i];
      temp.push_back(sum);
    }
    return temp;
  }

  double EvtD0Topippim2pi0::contract_11_0(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != pb.size() || pa.size() != 4) {
      std::cout << "error contract 11->0" << std::endl;
      exit(0);
    }
    double temp = pa[3] * pb[3] - pa[0] * pb[0] - pa[1] * pb[1] - pa[2] * pb[2];
    return temp;

  }

  std::vector<double> EvtD0Topippim2pi0::contract_21_1(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != 16 || pb.size() != 4) {
      std::cout << "error contract 21->1" << std::endl;
      exit(0);
    }
    std::vector<double> temp; temp.clear();
    for (int i = 0; i < 4; i++) {
      double sum = 0;
      for (int j = 0; j < 4; j++) {
        int idx   = i * 4 + j;
        sum += pa[idx] * pb[j] * g_uv[4 * j + j];
      }
      temp.push_back(sum);
    }
    return temp;

  }

  double EvtD0Topippim2pi0::contract_22_0(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != pb.size() || pa.size() != 16) {
      std::cout << "error contract 22->0" << std::endl;
      exit(0);
    }
    double temp = 0;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        int idx    = i * 4 + j;
        temp += pa[idx] * pb[idx] * g_uv[4 * i + i] * g_uv[4 * j + j];
      }
    }
    return temp;

  }

  std::vector<double> EvtD0Topippim2pi0::contract_31_2(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != 64 || pb.size() != 4) {
      std::cout << "error contract 31->2" << std::endl;
      exit(0);
    }
    std::vector<double> temp; temp.clear();
    for (int i = 0; i < 16; i++) {
      double sum = 0;
      for (int j = 0; j < 4; j++) {
        int idx   = i * 4 + j;
        sum += pa[idx] * pb[j] * g_uv[4 * j + j];
      }
      temp.push_back(sum);
    }
    return temp;

  }

  std::vector<double> EvtD0Topippim2pi0::contract_41_3(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != 256 || pb.size() != 4) {
      std::cout << "error contract 41->3" << std::endl;
      exit(0);
    }
    std::vector<double> temp; temp.clear();
    for (int i = 0; i < 64; i++) {
      double sum = 0;
      for (int j = 0; j < 4; j++) {
        int idx   = i * 4 + j;
        sum += pa[idx] * pb[j] * g_uv[4 * j + j];
      }
      temp.push_back(sum);
    }
    return temp;

  }

  std::vector<double> EvtD0Topippim2pi0::contract_42_2(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != 256 || pb.size() != 16) {
      std::cout << "error contract 42->2" << std::endl;
      exit(0);
    }
    std::vector<double> temp; temp.clear();
    for (int i = 0; i < 16; i++) {
      double sum = 0;
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
          int idxa = i * 16 + j * 4 + k;
          int idxb = j * 4 + k;
          sum += pa[idxa] * pb[idxb] * g_uv[4 * j + j] * g_uv[4 * k + k];
        }
      }
      temp.push_back(sum);
    }

    return temp;

  }
  std::vector<double> EvtD0Topippim2pi0::contract_22_2(std::vector<double> pa, std::vector<double> pb)
  {
    if (pa.size() != 16 || pb.size() != 16) {
      std::cout << "error contract 42->2" << std::endl;
      exit(0);
    }
    std::vector<double> temp; temp.clear();
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        double sum = 0;
        for (int k = 0; k < 4; k++) {
          int idxa = i * 4 + k;
          int idxb = j * 4 + k;
          sum += pa[idxa] * pb[idxb] * g_uv[4 * k + k];
        }
        temp.push_back(sum);
      }
    }

    return temp;

  }

//OrbitalTensors
  std::vector<double> EvtD0Topippim2pi0::OrbitalTensors(std::vector<double> pa, std::vector<double> pb, std::vector<double> pc,
                                                        double r, int rank)
  {
    if (pa.size() != 4 || pb.size() != 4 || pc.size() != 4) {
      std::cout << "Error: pa, pb, pc" << std::endl;
      exit(0);
    }
    if (rank < 0) {
      std::cout << "Error: L<0 !!!" << std::endl;
      exit(0);
    }

    // relative momentum
    std::vector<double> mr; mr.clear();

    for (int i = 0; i < 4; i++) {
      double temp = pb[i] - pc[i];
      mr.push_back(temp);
    }

    // "Masses" of particles
    double msa = contract_11_0(pa, pa);
    double msb = contract_11_0(pb, pb);
    double msc = contract_11_0(pc, pc);

    // Q^2_abc
    double top = msa + msb - msc;
    double Q2abc = top * top / (4.0 * msa) - msb;

    // Barrier factors
    double Q_0 = 0.197321f / r;
    double Q_02 = Q_0 * Q_0;
    double Q_04 = Q_02 * Q_02;
    //  double Q_06 = Q_04*Q_02;
    //  double Q_08 = Q_04*Q_04;

    double Q4abc = Q2abc * Q2abc;
    //  double Q6abc = Q4abc*Q2abc;
    //  double Q8abc = Q4abc*Q4abc;

    double mB1 = sqrt(2.0f / (Q2abc + Q_02));
    double mB2 = sqrt(13.0f / (Q4abc + (3.0f * Q_02) * Q2abc + 9.0f * Q_04));
    //  mB3 = &sqrt(277.0f/(Q6abc + (6.0f*Q_02)*Q4abc + (45.0f*Q_04)*Q2abc + 225.0f*Q_06));
    //  mB4 = &sqrt(12746.0f/(Q8abc + (10.0f*Q_02)*Q6abc + (135.0f*Q_04)*Q4abc + (1575.0f*Q_06)*Q2abc + 11025.0f*Q_08));

    // Projection Operator 2-rank
    std::vector<double> proj_uv; proj_uv.clear();
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        int idx = i * 4 + j;
        double temp = -g_uv[idx] + pa[i] * pa[j] / msa;
        proj_uv.push_back(temp);
      }
    }

    // Orbital Tensors
    if (rank == 0) {
      std::vector<double> t; t.clear();
      t.push_back(1.0);
      return t;

    } else if (rank < 3) {
      std::vector<double> t_u; t_u.clear();
      std::vector<double> Bt_u; Bt_u.clear();
      for (int i = 0; i < 4; i++) {
        double temp = 0;
        for (int j = 0; j < 4; j++) {
          int idx = i * 4 + j;
          temp += -proj_uv[idx] * mr[j] * g_uv[j * 4 + j];
        }
        t_u.push_back(temp);
        Bt_u.push_back(temp * mB1);
      }
      if (rank == 1) return Bt_u;

      double t_u2 = contract_11_0(t_u, t_u);

      std::vector<double> Bt_uv; Bt_uv.clear();
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          int idx = 4 * i + j;
          double temp = t_u[i] * t_u[j] + (1.0 / 3.0) * proj_uv[idx] * t_u2;
          Bt_uv.push_back(temp * mB2);
        }
      }
      if (rank == 2) return Bt_uv;

    } else {
      std::cout << "rank>2: please add it by yourself!!!" << std::endl;
      exit(0);
    }
    return std::vector<double>();
  }

// projection Tensor
  std::vector<double> EvtD0Topippim2pi0::ProjectionTensors(std::vector<double> pa, int rank)
  {
    if (pa.size() != 4) {
      std::cout << "Error: pa" << std::endl;
      exit(0);
    }
    if (rank < 0) {
      std::cout << "Error: L<0 !!!" << std::endl;
      exit(0);
    }

    double msa = contract_11_0(pa, pa);

    // Projection Operator 2-rank
    std::vector<double> proj_uv; proj_uv.clear();
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        int idx = i * 4 + j;
        double temp = -g_uv[idx] + pa[i] * pa[j] / msa;
        proj_uv.push_back(temp);
      }
    }

    // Orbital Tensors
    if (rank == 0) {
      std::vector<double> t; t.clear();
      t.push_back(1.0);
      return t;

    } else if (rank == 1) {
      return  proj_uv;
    } else if (rank == 2) {
      std::vector<double> proj_uvmn; proj_uvmn.clear();
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          for (int k = 0; k < 4; k++) {
            for (int l = 0; l < 4; l++) {

              int idx1_1 = 4 * i + k;
              int idx1_2 = 4 * i + l;
              int idx1_3 = 4 * i + j;

              int idx2_1 = 4 * j + l;
              int idx2_2 = 4 * j + k;
              int idx2_3 = 4 * k + l;

              double temp = (1.0 / 2.0) * (proj_uv[idx1_1] * proj_uv[idx2_1] + proj_uv[idx1_2] * proj_uv[idx2_2]) -
                            (1.0 / 3.0) * proj_uv[idx1_3] * proj_uv[idx2_3];
              proj_uvmn.push_back(temp);
            }
          }
        }
      }
      return proj_uvmn;

    } else {
      std::cout << "rank>2: please add it by yourself!!!" << std::endl;
      exit(0);
    }
  }
  double EvtD0Topippim2pi0::fundecaymomentum(double mr2, double m1_2, double m2_2)
  {
    double mr = sqrt(mr2);
    double poly = mr2 * mr2 + m1_2 * m1_2 + m2_2 * m2_2 - 2 * m1_2 * mr2 - 2 * m2_2 * mr2 - 2 * m1_2 * m2_2;
    double ret = sqrt(poly) / (2 * mr);
    if (poly < 0)
      //if(sqrt(m1_2) +sqrt(m2_2) > mr)
      ret = 0.0f;
    return ret;
  }

  std::complex<double> EvtD0Topippim2pi0::breitwigner(double mx2, double mr, double wr)
  {
    double output_x = 0;
    double output_y = 0;

    double mr2 = mr * mr;
    double diff = mr2 - mx2;
    double denom = diff * diff + wr * wr * mr2;
    if (wr < 0) {
      output_x = 0;
      output_y = 0;
    } else if (wr < 10) {
      output_x = diff / denom;
      output_y = wr * mr / denom;
    } else { /* phase space */
      output_x = 1;
      output_y = 0;
    }
    std::complex<double>  output(output_x, output_y);
    return output;
  }

  /* GS propagator */
  double EvtD0Topippim2pi0::h(double m, double q)
  {
    double h = 2.0 / math_pi * q / m * log((m + 2.0 * q) / (2.0 * mass_Pion));
    return h;
  }

  double EvtD0Topippim2pi0::dh(double m0, double q0)
  {
    double dh = h(m0, q0) * (1.0 / (8.0 * q0 * q0) - 1.0 / (2.0 * m0 * m0)) + 1.0 / (2.0 * math_pi * m0 * m0);
    return dh;
  }

  double EvtD0Topippim2pi0::f(double m0, double sx, double q0, double q)
  {
    double m = sqrt(sx);
    double f = m0 * m0 / (q0 * q0 * q0) * (q * q * (h(m, q) - h(m0, q0)) + (m0 * m0 - sx) * q0 * q0 * dh(m0, q0));
    return f;
  }

  double EvtD0Topippim2pi0::d(double m0, double q0)
  {
    double d = 3.0 / math_pi * mass_Pion * mass_Pion / (q0 * q0) * log((m0 + 2.0 * q0) / (2.0 * mass_Pion)) + m0 /
               (2.0 * math_pi * q0) - (mass_Pion * mass_Pion * m0) / (math_pi * q0 * q0 * q0);
    return d;
  }

  double EvtD0Topippim2pi0::fundecaymomentum2(double mr2, double m1_2, double m2_2)
  {
    double poly = mr2 * mr2 + m1_2 * m1_2 + m2_2 * m2_2 - 2 * m1_2 * mr2 - 2 * m2_2 * mr2 - 2 * m1_2 * m2_2;
    double ret = poly / (4.0f * mr2);
    if (poly < 0)
      ret = 0.0f;
    return ret;
  }

  double EvtD0Topippim2pi0::wid(double mass, double sa, double sb, double sc, double r, int l)
  {
    double widm = 1.0;
    double sa0 = mass * mass;
    double m = sqrt(sa);
    double q = fundecaymomentum2(sa, sb, sc);
    double q0 = fundecaymomentum2(sa0, sb, sc);
    r = r / 0.197321;
    double z = q * r * r;
    double z0 = q0 * r * r;
    double F = 0.0;
    if (l == 0) F = 1.0;
    if (l == 1) F = sqrt((1.0 + z0) / (1.0 + z));
    if (l == 2) F = sqrt((9.0 + 3.0 * z0 + z0 * z0) / (9.0 + 3.0 * z + z * z));
    if (l == 3) F = sqrt((225.0 + 45.0 * z0 + 6.0 * z0 * z0 + z0 * z0 * z0) / (225.0 + 45.0 * z + 6.0 * z * z + z * z * z));
    if (l == 4) F = sqrt((11025.0 + 1575.0 * z0 + 135.0 * z0 * z0 + 10.0 * z0 * z0 * z0 + z0 * z0 * z0 * z0) /
                           (11025.0 + 1575.0 * z + 135.0 * z * z + 10.0 * z * z * z + z * z * z * z));
    double t = sqrt(q / q0);
    //printf("sa0 = %f, sb = %f, sc = %f, q = %f, q0 = %0.15f, qq0 = %f, t = %f \n",sa0,sb,sc,q,q0,q/q0,t);
    uint i = 0;
    for (i = 0; i < static_cast<uint>(2 * l + 1); i++) {
      widm *= t;
    }
    widm *= (mass / m * F * F);
    return widm;
  }

  /* for rho0, use GS, rho0->pi+ pi-, only angular momentum 1*/
  std::complex<double> EvtD0Topippim2pi0::GS(double mx2, double mr, double wr, double m1_2, double m2_2, double r, int l)
  {

    double mr2 = mr * mr;
    double q = fundecaymomentum(mx2, m1_2, m2_2);
    double q0 = fundecaymomentum(mr2, m1_2, m2_2);
    double numer = 1.0 + d(mr, q0) * wr / mr;
    double denom_real = mr2 - mx2 + wr * f(mr, mx2, q0, q);
    double denom_imag = mr * wr * wid(mr, mx2, m1_2, m2_2, r, l); //real-i*imag;

    double denom = denom_real * denom_real + denom_imag * denom_imag;
    double output_x = denom_real * numer / denom;
    double output_y = denom_imag * numer / denom;

    std::complex<double>  output(output_x, output_y);
    return output;
  }

  std::complex<double> EvtD0Topippim2pi0::irho(double mr2, double m1_2, double m2_2)
  {
    double poly = mr2 * mr2 + m1_2 * m1_2 + m2_2 * m2_2 - 2 * m1_2 * mr2 - 2 * m2_2 * mr2 - 2 * m1_2 * m2_2;
    double ret_real = 0;
    double ret_imag = 0;
    if (poly >= 0) {
      ret_real = 2.0f * sqrt(poly) / (2.0f * mr2);
      ret_imag = 0;
    } else {
      ret_real = 0;
      ret_imag = 2.0f * sqrt(-1.0f * poly) / (2.0f * mr2);
    }
    std::complex<double>  ret(ret_real, ret_imag);
    return ret;
  }

  std::complex<double> EvtD0Topippim2pi0::Flatte(double mx2, double mr, double g1, double g2, double m1a, double m1b, double m2a,
                                                 double m2b)
  {

    double mr2 = mr * mr;
    double diff = mr2 - mx2;
    double g22 = g2 * g1;
    std::complex<double> ps1 = irho(mx2, m1a * m1a, m1b * m1b);
    std::complex<double> ps2 = irho(mx2, m2a * m2a, m2b * m2b);
    std::complex<double> iws = g1 * ps1 + g22 * ps2; /*mass dependent width */

    double denom_real = diff + iws.imag();
    double denom_imag = iws.real();
    double denom = denom_real * denom_real + denom_imag * denom_imag;

    double output_x = denom_real / denom;
    double output_y = denom_imag / denom;

    std::complex<double>  output(output_x, output_y);
    return output;

  }

  std::complex<double> EvtD0Topippim2pi0::RBW(double mx2, double mr, double wr, double m1_2, double m2_2, double r, int l)
  {
    double mr2 = mr * mr;
    double denom_real = mr2 - mx2;
    double denom_imag = 0;
    if (m1_2 > 0 && m2_2 > 0) {
      denom_imag = mr * wr * wid(mr, mx2, m1_2, m2_2, r, l); //real-i*imag;
    } else {
      denom_imag = mr * wr;
    }

    double denom = denom_real * denom_real + denom_imag * denom_imag;
    double output_x = denom_real / denom;
    double output_y = denom_imag / denom;

    std::complex<double>  output(output_x, output_y);
    return output;
  }

  /* build a1260 propagator */
  double EvtD0Topippim2pi0::widT1260(int i, double g1, double g2)
  {
    double wid1[300]  = { 0.000765966, 0.00526534, 0.0168824, 0.0381541, 0.0709798, 0.116716, 0.176348, 0.250811, 0.340516, 0.44606,
                          0.567244, 0.704921, 0.859082, 1.02965, 1.21689, 1.42109, 1.64283, 1.88082, 2.13774, 2.41039,
                          2.70334, 3.0139, 3.34224, 3.69257, 4.05705, 4.44768, 4.85524, 5.28944, 5.73832, 6.21287,
                          6.71567, 7.2421, 7.79776, 8.37191, 8.98624, 9.63238, 10.3101, 11.0061, 11.7641, 12.5458,
                          13.3764, 14.2363, 15.1628, 16.1227, 17.1638, 18.2475, 19.3925, 20.603, 21.8923, 23.2399,
                          24.6837, 26.2041, 27.8182, 29.5295, 31.3134, 33.246, 35.249, 37.3791, 39.6217, 41.9618,
                          44.3717, 46.8669, 49.4843, 52.1752, 54.9009, 57.5853, 60.3722, 63.1059, 65.7868, 68.4831,
                          71.0904, 73.6104, 76.1854, 78.5089, 80.9245, 83.0936, 85.2363, 87.3891, 89.3678, 91.2757,
                          93.049, 94.7878, 96.4351, 98.0146, 99.4775, 100.97, 102.344, 103.571, 104.91, 106.134,
                          107.27, 108.404, 109.517, 110.378, 111.45, 112.438, 113.096, 114.086, 114.935, 115.693,
                          116.439, 117.249, 117.822, 118.369, 119.062, 119.631, 120.217, 120.995, 121.537, 121.963,
                          122.856, 123.129, 123.642, 124.165, 124.729, 124.969, 125.572, 125.878, 126.373, 126.837,
                          127.287, 127.575, 128.226, 128.539, 128.808, 129.144, 129.702, 129.988, 130.595, 130.759,
                          131.101, 131.251, 131.637, 131.951, 132.552, 132.804, 132.991, 133.455, 133.674, 133.95,
                          134.351, 134.674, 134.97, 135.174, 135.429, 135.94, 136.292, 136.595, 136.786, 137.014,
                          137.454, 137.782, 138.096, 138.396, 138.629, 139.002, 139.213, 139.419, 139.819, 140.165,
                          140.352, 140.579, 140.992, 141.404, 141.577, 141.786, 142.224, 142.476, 142.807, 143.216,
                          143.697, 144.059, 144.326, 144.735, 144.951, 145.342, 145.887, 145.88, 146.382, 146.848,
                          147.207, 147.553, 147.962, 148.267, 148.676, 149.175, 149.612, 149.996, 150.341, 150.7,
                          151.071, 151.478, 152.081, 152.566, 152.899, 153.305, 153.923, 154.102, 154.68, 155.234,
                          155.518, 156.153, 156.547, 157.044, 157.414, 158.057, 158.427, 158.869, 159.4, 159.95,
                          160.321, 160.917, 161.523, 161.92, 162.626, 162.708, 163.281, 163.842, 164.594, 164.975,
                          165.366, 166.104, 166.274, 166.837, 167.451, 168.012, 168.635, 169.078, 169.339, 170.036,
                          170.683, 171.155, 171.515, 172.134, 172.733, 173.207, 173.756, 174.176, 174.769, 175.036,
                          175.702, 176.163, 176.59, 177.36, 177.654, 178.347, 178.708, 179.455, 180.003, 180.334,
                          180.716, 181.446, 181.945, 182.401, 182.778, 183.317, 184.106, 184.271, 184.896, 185.414,
                          185.818, 186.491, 186.966, 187.533, 188.148, 188.61, 189.022, 189.321, 189.907, 190.607,
                          191.104, 191.341, 191.888, 192.648, 193.027, 193.363, 194.166, 194.692, 194.995, 195.407,
                          195.898, 196.352, 197.005, 197.378, 198.08, 198.479, 198.95, 199.224, 199.647, 200.37,
                          201.205, 201.251, 201.629, 202.244, 202.762, 203.404, 203.65, 204.172, 204.751, 204.817
                        };

    double wid2[300]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          1.87136e-06, 1.50063e-05, 5.10425e-05, 0.000122121, 0.000240853, 0.000420318, 0.000675161, 0.0010173, 0.00146434, 0.00203321,
                          0.00273489, 0.0035927, 0.00462579, 0.00584255, 0.00727372, 0.00895462, 0.0108831, 0.013085, 0.0156197, 0.0184865,
                          0.0217078, 0.0253423, 0.0294103, 0.0339191, 0.0389837, 0.0446351, 0.0508312, 0.0577268, 0.0653189, 0.0737049,
                          0.0829819, 0.0930611, 0.104328, 0.116663, 0.130105, 0.144922, 0.16122, 0.179091, 0.198759, 0.220133,
                          0.243916, 0.269803, 0.298861, 0.330061, 0.365741, 0.40437, 0.447191, 0.49501, 0.548576, 0.606445,
                          0.674414, 0.748353, 0.831686, 0.929938, 1.03771, 1.16187, 1.30387, 1.47341, 1.65629, 1.88318,
                          2.14353, 2.44169, 2.79831, 3.2009, 3.65522, 4.16317, 4.69597, 5.2585, 5.85965, 6.44984,
                          7.04202, 7.60113, 8.14571, 8.73195, 9.24537, 9.75717, 10.2093, 10.6731, 11.1487, 11.5819,
                          12.0158, 12.4253, 12.8113, 13.2073, 13.5995, 13.9317, 14.312, 14.6595, 14.9511, 15.2668,
                          15.6092, 15.9349, 16.1873, 16.5049, 16.819, 17.0743, 17.3621, 17.6094, 17.8418, 18.0681,
                          18.3141, 18.5914, 18.8187, 19.0562, 19.2282, 19.4918, 19.7326, 19.9112, 20.134, 20.3386,
                          20.511, 20.6865, 20.8958, 21.0518, 21.2967, 21.44, 21.6361, 21.8012, 21.9523, 22.1736,
                          22.2615, 22.4207, 22.6056, 22.7198, 22.9299, 23.0605, 23.2959, 23.3808, 23.4961, 23.6793,
                          23.7843, 23.9697, 24.0689, 24.1919, 24.405, 24.3898, 24.6018, 24.7294, 24.789, 24.9978,
                          25.0626, 25.1728, 25.2809, 25.3579, 25.5444, 25.5995, 25.7644, 25.8397, 25.9229, 26.095,
                          26.1495, 26.2899, 26.3871, 26.54, 26.6603, 26.7008, 26.7836, 26.907, 26.9653, 26.9969,
                          27.1226, 27.226, 27.3543, 27.4686, 27.4887, 27.6163, 27.6986, 27.7506, 27.7884, 27.8662,
                          27.9886, 28.0573, 28.1238, 28.2612, 28.3209, 28.3457, 28.4392, 28.5086, 28.6399, 28.7603,
                          28.788, 28.8502, 28.9038, 28.9667, 28.975, 29.0032, 29.2681, 29.2392, 29.2572, 29.3364
                        };

    return wid1[i] * g1 + wid2[i] * g2;
  }

  double EvtD0Topippim2pi0::anywid1260(double sc, double g1, double g2)
  {

    double smin = (0.13957 * 3) * (0.13957 * 3);
    double dh = 0.01;
    int od  = (sc - 0.18) / dh;
    double sc_m = 0.18 + od * dh;
    double widuse = 0;
    if (sc >= 0.18 && sc <= 3.17) {
      widuse = ((sc - sc_m) / dh) * (widT1260(od + 1, g1, g2) - widT1260(od, g1, g2)) + widT1260(od, g1, g2);
    } else if (sc < 0.18 && sc > smin) {
      widuse = ((sc - smin) / (0.18 - smin)) * widT1260(0, g1, g2);
    } else if (sc > 3.17) {
      widuse = widT1260(299, g1, g2);
    } else {
      widuse = 0;
    }
    return widuse;

  }

  std::complex<double> EvtD0Topippim2pi0::RBWa1260(double mx2, double mr, double g1, double g2)
  {

    double mx  = sqrt(mx2);
    double mr2 = mr * mr;
    double wid0 = anywid1260(mx2, g1, g2);

    double denom_real = mr2 - mx2;
    double denom_imag = mx * wid0; //real-i*imag;

    double denom = denom_real * denom_real + denom_imag * denom_imag;
    double output_x = denom_real / denom;
    double output_y = denom_imag / denom;

    std::complex<double>  output(output_x, output_y);
    return output;

  }

  /* build pi1300 propagator */
  double EvtD0Topippim2pi0::widT1300(int i)
  {
    double wid1[300]  = { 0.032058, 0.181916, 0.451826, 0.828744, 1.30099, 1.85751, 2.48953, 3.18916, 3.9474, 4.75752,
                          5.61567, 6.5142, 7.4487, 8.41743, 9.41268, 10.4375, 11.4806, 12.5426, 13.6259, 14.7324,
                          15.8316, 16.9667, 18.1034, 19.2511, 20.4, 21.5745, 22.7364, 23.9313, 25.1231, 26.331,
                          27.5222, 28.7648, 29.9995, 31.2184, 32.4692, 33.7247, 34.9821, 36.2849, 37.5859, 38.9141,
                          40.208, 41.5699, 42.8807, 44.2708, 45.6339, 47.0794, 48.5124, 49.9607, 51.4382, 52.9726,
                          54.5431, 56.0448, 57.755, 59.4508, 61.0778, 62.8293, 64.7519, 66.559, 68.5168, 70.4768,
                          72.5278, 74.6728, 76.8742, 79.0858, 81.4848, 83.8468, 86.3982, 88.9172, 91.645, 94.4245,
                          97.2676, 100.073, 103.181, 106.146, 109.37, 112.451, 115.574, 118.859, 122.118, 125.663,
                          128.847, 132.328, 135.778, 139.325, 142.708, 146.613, 150.058, 153.466, 157.281, 160.692,
                          164.484, 168.1, 171.52, 175.195, 178.72, 182.098, 185.828, 189.775, 192.768, 196.145,
                          199.837, 203.808, 206.594, 209.711, 213.22, 216.724, 219.613, 223.289, 226.325, 228.679,
                          232.516, 234.97, 237.984, 241.407, 244.237, 247.025, 250.195, 252.545, 255.669, 258.907,
                          262.137, 264.45, 267.707, 270.006, 273.093, 275.135, 278.181, 281.148, 284.513, 286.352,
                          288.876, 291.133, 293.855, 296.082, 298.943, 301.294, 303.401, 305.955, 308.498, 310.318,
                          312.861, 314.59, 317.381, 319.36, 321.528, 323.834, 326.465, 328.516, 330.043, 331.913,
                          334.528, 336.647, 338.083, 340.873, 342.284, 344.527, 346.21, 347.898, 349.829, 351.885,
                          353.297, 355.228, 357.187, 359.115, 360.499, 362.05, 363.388, 365.451, 367.202, 369.099,
                          371.197, 372.755, 374.302, 375.418, 377.114, 378.595, 380.72, 381.193, 383.21, 385.243,
                          386.41, 387.791, 388.97, 390.617, 391.731, 393.692, 394.627, 396.209, 397.339, 398.582,
                          399.836, 401.752, 402.704, 404.86, 405.287, 406.561, 408.001, 408.361, 410.374, 411.672,
                          412.688, 414.259, 415.406, 416.22, 417.659, 418.652, 419.82, 420.586, 421.835, 423.323,
                          423.714, 425.449, 426.524, 427.196, 428.974, 429.086, 430.614, 431.642, 432.744, 434.121,
                          434.083, 436.251, 436.024, 437.267, 438.584, 439.499, 440.402, 441.609, 441.785, 443.319,
                          444.053, 445.303, 445.359, 446.906, 447.629, 448.931, 449.379, 450.143, 451.303, 452.034,
                          453.026, 453.73, 454.424, 455.852, 456.574, 458.04, 457.593, 459.097, 459.977, 461.041,
                          461.527, 462.486, 463.506, 464.275, 465.849, 465.581, 467.364, 467.648, 468.615, 469.74,
                          470.1, 471.406, 472.099, 472.723, 474.476, 474.81, 475.291, 476.741, 477.751, 478.678,
                          479.833, 479.724, 480.47, 481.717, 482.226, 483.068, 484.311, 485.806, 485.987, 487.431,
                          487.108, 488.798, 488.758, 489.889, 491.406, 491.455, 492.192, 493.215, 493.758, 494.763,
                          496.16, 496.003, 496.981, 497.871, 498.674, 499.319, 500.16, 500.455, 501.069, 502.515
                        };

    return wid1[i];
  }

  double EvtD0Topippim2pi0::anywid1300(double sc)
  {

    double smin = (0.13957 * 3) * (0.13957 * 3);
    double dh = 0.01;
    int od  = (sc - 0.18) / dh;
    double sc_m = 0.18 + od * dh;
    double widuse = 0;
    if (sc >= 0.18 && sc <= 3.17) {
      widuse = ((sc - sc_m) / dh) * (widT1300(od + 1) - widT1300(od)) + widT1300(od);
    } else if (sc < 0.18 && sc > smin) {
      widuse = ((sc - smin) / (0.18 - smin)) * widT1300(0);
    } else if (sc > 3.17) {
      widuse = widT1300(299);
    } else {
      widuse = 0;
    }
    return widuse;
  }

  std::complex<double> EvtD0Topippim2pi0::RBWpi1300(double mx2, double mr, double wr)
  {

    double mx  = sqrt(mx2);
    double mr2 = mr * mr;
    double g1 = wr / anywid1300(mr2);
    double wid0 = anywid1300(mx2) * g1;

    double denom_real = mr2 - mx2;
    double denom_imag = mx * wid0; //real-i*imag;

    double denom = denom_real * denom_real + denom_imag * denom_imag;
    double output_x = denom_real / denom;
    double output_y = denom_imag / denom;

    std::complex<double>  output(output_x, output_y);
    return output;

  }

  /* build a1640 propagator */
  double EvtD0Topippim2pi0::widT1640(int i)
  {
    double wid1[300]  = { 0.000266799, 0.0018334, 0.00594952, 0.0136464, 0.0257838, 0.0430639, 0.0660891, 0.0954588, 0.131591, 0.175007,
                          0.225896, 0.284916, 0.352348, 0.42844, 0.513613, 0.608422, 0.713236, 0.827844, 0.953841, 1.09028,
                          1.23896, 1.3999, 1.57261, 1.75997, 1.95828, 2.17416, 2.40274, 2.65003, 2.91017, 3.18896,
                          3.4876, 3.80491, 4.14473, 4.50064, 4.88498, 5.29342, 5.72815, 6.18169, 6.67661, 7.19577,
                          7.75069, 8.33372, 8.96242, 9.62548, 10.3429, 11.1008, 11.9068, 12.7649, 13.684, 14.6561,
                          15.7, 16.8052, 17.9946, 19.2573, 20.5822, 22.02, 23.5335, 25.1379, 26.8401, 28.6285,
                          30.4874, 32.4225, 34.4622, 36.5651, 38.7177, 40.8728, 43.1028, 45.3167, 47.5153, 49.7404,
                          51.913, 54.0368, 56.2129, 58.2269, 60.3061, 62.2158, 64.109, 66.007, 67.7917, 69.5455,
                          71.1901, 72.8026, 74.3817, 75.8687, 77.2941, 78.7569, 80.1295, 81.3587, 82.6853, 83.9199,
                          85.0982, 86.2791, 87.4225, 88.3923, 89.5057, 90.5393, 91.3378, 92.4117, 93.3536, 94.2291,
                          95.0644, 95.9397, 96.6709, 97.3455, 98.1876, 98.8578, 99.5411, 100.383, 101.071, 101.612,
                          102.565, 102.933, 103.563, 104.141, 104.805, 105.172, 105.82, 106.22, 106.798, 107.369,
                          107.889, 108.279, 108.856, 109.296, 109.633, 110.055, 110.639, 110.974, 111.553, 111.775,
                          112.189, 112.364, 112.693, 113.063, 113.737, 113.872, 114.177, 114.604, 114.814, 115.086,
                          115.475, 115.797, 116.066, 116.276, 116.453, 116.959, 117.212, 117.536, 117.714, 117.784,
                          118.206, 118.435, 118.77, 118.937, 119.078, 119.443, 119.54, 119.649, 119.903, 120.219,
                          120.308, 120.402, 120.661, 121.042, 121.051, 121.058, 121.445, 121.57, 121.593, 121.896,
                          122.146, 122.339, 122.519, 122.694, 122.665, 122.943, 123.242, 123.137, 123.276, 123.516,
                          123.635, 123.746, 124.053, 123.942, 124.071, 124.211, 124.4, 124.546, 124.776, 124.811,
                          124.872, 124.814, 125.187, 125.28, 125.261, 125.33, 125.61, 125.509, 125.683, 125.777,
                          125.698, 126.017, 126.035, 126.269, 126.338, 126.377, 126.655, 126.603, 126.602, 126.721,
                          126.917, 126.91, 127.064, 127.141, 127.193, 127.105, 127.27, 127.377, 127.566, 127.461,
                          127.571, 127.747, 127.719, 127.763, 127.834, 128.103, 128.173, 128.217, 128.021, 128.291,
                          128.348, 128.376, 128.528, 128.489, 128.74, 128.767, 128.858, 128.845, 128.925, 128.965,
                          128.947, 129.009, 129.118, 129.165, 129.219, 129.277, 129.356, 129.26, 129.595, 129.474,
                          129.422, 129.679, 129.526, 129.699, 129.702, 129.64, 129.879, 129.773, 129.798, 130.091,
                          130.023, 130.004, 130.118, 129.995, 130.182, 130.233, 130.262, 130.584, 130.462, 130.547,
                          130.551, 130.513, 130.501, 130.627, 130.632, 130.679, 130.908, 130.924, 130.894, 130.986,
                          131.024, 131.075, 130.995, 130.902, 131.228, 131.115, 131.17, 131.257, 131.16, 131.366,
                          131.439, 131.397, 131.386, 131.472, 131.48, 131.564, 131.505, 131.614, 131.573, 131.455
                        };
    return wid1[i];
  }

  double EvtD0Topippim2pi0::anywid1640(double sc)
  {

    double smin = (0.13957 * 3) * (0.13957 * 3);
    double dh = 0.01;
    int od  = (sc - 0.18) / dh;
    double sc_m = 0.18 + od * dh;
    double widuse = 0;
    if (sc >= 0.18 && sc <= 3.17) {
      widuse = ((sc - sc_m) / dh) * (widT1640(od + 1) - widT1640(od)) + widT1640(od);
    } else if (sc < 0.18 && sc > smin) {
      widuse = ((sc - smin) / (0.18 - smin)) * widT1640(0);
    } else if (sc > 3.17) {
      widuse = widT1640(299);
    } else {
      widuse = 0;
    }
    return widuse;
  }

  std::complex<double> EvtD0Topippim2pi0::RBWa1640(double mx2, double mr, double wr)
  {

    double mx  = sqrt(mx2);
    double mr2 = mr * mr;
    double g1 = wr / anywid1640(mr2);
    double wid0 = anywid1640(mx2) * g1;

    double denom_real = mr2 - mx2;
    double denom_imag = mx * wid0; //real-i*imag;

    double denom = denom_real * denom_real + denom_imag * denom_imag;
    double output_x = denom_real / denom;
    double output_y = denom_imag / denom;

    std::complex<double>  output(output_x, output_y);
    return output;

  }
  /* build h11170 propagator */
  double EvtD0Topippim2pi0::widT1170(int i)
  {
    double wid1[300]  = { 0.000166175, 0.00161921, 0.00561815, 0.0131971, 0.0252114, 0.0422987, 0.0651321, 0.0943468, 0.130226, 0.173219,
                          0.22388, 0.28243, 0.349257, 0.424971, 0.50948, 0.603832, 0.707638, 0.821261, 0.946341, 1.08278,
                          1.22851, 1.38899, 1.56053, 1.74557, 1.94153, 2.15585, 2.38028, 2.62579, 2.88376, 3.16032,
                          3.452, 3.77036, 4.10553, 4.45572, 4.8352, 5.23722, 5.66246, 6.11599, 6.60404, 7.12086,
                          7.658, 8.23991, 8.85135, 9.50382, 10.2039, 10.9498, 11.7427, 12.5806, 13.4853, 14.4446,
                          15.4604, 16.5243, 17.7062, 18.9394, 20.219, 21.6307, 23.1328, 24.6832, 26.3629, 28.1076,
                          29.9211, 31.8286, 33.8171, 35.8791, 38.0244, 40.1354, 42.376, 44.5605, 46.8186, 48.9738,
                          51.1693, 53.2751, 55.4668, 57.5333, 59.6529, 61.5273, 63.4519, 65.3347, 67.1338, 68.9518,
                          70.5915, 72.1639, 73.7419, 75.2814, 76.7236, 78.3086, 79.6492, 80.8699, 82.228, 83.3878,
                          84.6403, 85.8205, 86.8573, 87.9557, 88.9784, 89.9703, 90.8957, 92.0648, 92.8484, 93.755,
                          94.603, 95.5404, 96.2543, 96.8307, 97.8023, 98.5217, 99.1155, 100.047, 100.693, 101.165,
                          102.153, 102.449, 103.126, 103.777, 104.336, 104.807, 105.412, 105.714, 106.345, 107.009,
                          107.679, 107.941, 108.403, 108.962, 109.354, 109.726, 110.228, 110.788, 111.304, 111.496,
                          111.885, 112.06, 112.414, 112.681, 113.347, 113.465, 113.829, 114.221, 114.53, 114.718,
                          115.217, 115.391, 115.661, 115.903, 116.177, 116.576, 116.951, 117.321, 117.423, 117.454,
                          117.903, 118.201, 118.416, 118.627, 118.788, 119.237, 119.307, 119.397, 119.601, 119.892,
                          119.853, 120.044, 120.214, 120.734, 120.762, 120.81, 121.148, 121.317, 121.209, 121.544,
                          121.786, 121.945, 122.121, 122.401, 122.272, 122.593, 122.968, 122.78, 122.868, 123.262,
                          123.316, 123.424, 123.822, 123.683, 123.577, 123.93, 124.017, 124.27, 124.515, 124.476,
                          124.516, 124.54, 124.82, 124.951, 124.829, 124.911, 125.281, 125.126, 125.511, 125.315,
                          125.354, 125.749, 125.629, 125.968, 126.13, 126.15, 126.372, 126.217, 126.39, 126.451,
                          126.566, 126.618, 126.639, 126.94, 126.874, 126.738, 126.929, 126.999, 127.267, 127.057,
                          127.188, 127.427, 127.424, 127.414, 127.407, 127.749, 127.938, 127.776, 127.687, 128.017,
                          128.04, 128.019, 128.096, 128.156, 128.281, 128.354, 128.496, 128.675, 128.553, 128.666,
                          128.536, 128.63, 128.866, 128.867, 128.908, 128.975, 129.05, 128.894, 129.29, 129.062,
                          129.156, 129.361, 129.105, 129.346, 129.276, 129.368, 129.612, 129.517, 129.494, 129.864,
                          129.677, 129.68, 129.696, 129.629, 129.826, 129.898, 129.984, 130.271, 130.179, 130.301,
                          130.193, 130.208, 130.212, 130.354, 130.244, 130.318, 130.549, 130.554, 130.477, 130.529,
                          130.765, 130.753, 130.709, 130.598, 130.809, 130.67, 130.792, 130.87, 130.857, 131.148,
                          131.145, 130.977, 131.056, 131.149, 131.09, 131.106, 131.156, 131.28, 131.184, 131.122
                        };
    return wid1[i];
  }

  double EvtD0Topippim2pi0::anywid1170(double sc)
  {

    double smin = (0.13957 * 3) * (0.13957 * 3);
    double dh = 0.01;
    int od  = (sc - 0.18) / dh;
    double sc_m = 0.18 + od * dh;
    double widuse = 0;
    if (sc >= 0.18 && sc <= 3.17) {
      widuse = ((sc - sc_m) / dh) * (widT1170(od + 1) - widT1170(od)) + widT1170(od);
    } else if (sc < 0.18 && sc > smin) {
      widuse = ((sc - smin) / (0.18 - smin)) * widT1170(0);
    } else if (sc > 3.17) {
      widuse = widT1170(299);
    } else {
      widuse = 0;
    }
    return widuse;
  }

  std::complex<double> EvtD0Topippim2pi0::RBWh11170(double mx2, double mr, double wr)
  {

    double mx  = sqrt(mx2);
    double mr2 = mr * mr;
    double g1 = wr / anywid1170(mr2);
    double wid0 = anywid1170(mx2) * g1;

    double denom_real = mr2 - mx2;
    double denom_imag = mx * wid0; //real-i*imag;

    double denom = denom_real * denom_real + denom_imag * denom_imag;
    double output_x = denom_real / denom;
    double output_y = denom_imag / denom;

    std::complex<double>  output(output_x, output_y);
    return output;
  }

// PiPi-S wave K-Matrix
  double EvtD0Topippim2pi0::rho22(double sc)
  {
    double rho[689] = { 3.70024e-18, 8.52763e-15, 1.87159e-13, 1.3311e-12, 5.61842e-12, 1.75224e-11, 4.48597e-11, 9.99162e-11, 2.00641e-10, 3.71995e-10,
                        6.47093e-10, 1.06886e-09, 1.69124e-09, 2.58031e-09, 3.8168e-09, 5.49601e-09, 7.72996e-09, 1.06509e-08, 1.44078e-08, 1.91741e-08,
                        2.51445e-08, 3.25345e-08, 4.15946e-08, 5.25949e-08, 6.58316e-08, 8.16443e-08, 1.00389e-07, 1.22455e-07, 1.48291e-07, 1.78348e-07,
                        2.1313e-07, 2.53192e-07, 2.99086e-07, 3.51462e-07, 4.10993e-07, 4.78349e-07, 5.54327e-07, 6.3972e-07, 7.35316e-07, 8.42099e-07,
                        9.61004e-07, 1.09295e-06, 1.2391e-06, 1.40051e-06, 1.57824e-06, 1.77367e-06, 1.98805e-06, 2.22257e-06, 2.47877e-06, 2.7581e-06,
                        3.06186e-06, 3.39182e-06, 3.74971e-06, 4.137e-06, 4.5555e-06, 5.00725e-06, 5.4939e-06, 6.01725e-06, 6.57992e-06, 7.18371e-06,
                        7.83044e-06, 8.52301e-06, 9.26342e-06, 1.00535e-05, 1.08967e-05, 1.17953e-05, 1.27514e-05, 1.37679e-05, 1.48482e-05, 1.59943e-05,
                        1.72088e-05, 1.84961e-05, 1.98586e-05, 2.12987e-05, 2.28207e-05, 2.44279e-05, 2.61228e-05, 2.79084e-05, 2.97906e-05, 3.17718e-05,
                        3.38544e-05, 3.60443e-05, 3.8345e-05, 4.07591e-05, 4.32903e-05, 4.59459e-05, 4.87285e-05, 5.16403e-05, 5.46887e-05, 5.7878e-05,
                        6.12111e-05, 6.46908e-05, 6.83274e-05, 7.21231e-05, 7.60817e-05, 8.0208e-05, 8.45102e-05, 8.89919e-05, 9.36544e-05, 9.85082e-05,
                        0.000103559, 0.000108812, 0.000114267, 0.000119938, 0.000125827, 0.00013194, 0.000138278, 0.000144857, 0.000151681, 0.000158752,
                        0.000166074, 0.000173663, 0.000181521, 0.000189652, 0.000198059, 0.000206761, 0.000215761, 0.000225063, 0.00023467, 0.000244599,
                        0.000254855, 0.00026544, 0.000276357, 0.000287629, 0.00029926, 0.000311253, 0.000323609, 0.000336351, 0.000349483, 0.000363009,
                        0.000376926, 0.000391264, 0.000406029, 0.000421225, 0.000436848, 0.000452921, 0.000469458, 0.000486461, 0.00050393, 0.00052187,
                        0.000540322, 0.000559278, 0.000578746, 0.00059872, 0.000619236, 0.0006403, 0.000661911, 0.000684074, 0.000706799, 0.000730127,
                        0.00075405, 0.000778569, 0.000803686, 0.000829443, 0.000855839, 0.000882879, 0.000910561, 0.000938898, 0.000967939, 0.000997674,
                        0.00102811, 0.00105923, 0.0010911, 0.0011237, 0.00115706, 0.00119117, 0.00122601, 0.00126168, 0.00129815, 0.00133543,
                        0.00137351, 0.00141242, 0.00145219, 0.00149283, 0.00153434, 0.0015767, 0.00161995, 0.00166415, 0.00170928, 0.00175534,
                        0.00180232, 0.00185028, 0.00189924, 0.00194919, 0.00200014, 0.00205207, 0.00210503, 0.0021591, 0.00221421, 0.0022704,
                        0.00232766, 0.00238602, 0.00244554, 0.00250619, 0.00256799, 0.0026309, 0.002695, 0.00276033, 0.00282689, 0.00289467,
                        0.00296367, 0.00303389, 0.00310543, 0.0031783, 0.00325244, 0.0033279, 0.0034046, 0.00348275, 0.00356229, 0.00364322,
                        0.00372555, 0.00380924, 0.00389438, 0.00398104, 0.00406914, 0.00415877, 0.00424985, 0.00434235, 0.00443651, 0.00453224,
                        0.00462954, 0.00472848, 0.00482894, 0.00493102, 0.00503483, 0.00514029, 0.00524749, 0.0053563, 0.00546675, 0.00557905,
                        0.0056931, 0.00580901, 0.0059267, 0.00604613, 0.00616735, 0.00629049, 0.00641557, 0.00654254, 0.00667142, 0.00680216,
                        0.00693472, 0.00706946, 0.00720621, 0.00734497, 0.0074858, 0.00762855, 0.00777338, 0.00792036, 0.00806957, 0.00822087,
                        0.00837426, 0.00852982, 0.0086875, 0.00884756, 0.00900991, 0.00917447, 0.00934137, 0.00951052, 0.00968194, 0.0098558,
                        0.010032, 0.0102108, 0.0103919, 0.0105754, 0.0107612, 0.0109496, 0.0111406, 0.0113343, 0.0115305, 0.0117293,
                        0.0119303, 0.0121343, 0.0123409, 0.0125502, 0.0127623, 0.0129771, 0.0131944, 0.0134145, 0.0136376, 0.0138636,
                        0.0140924, 0.0143241, 0.0145587, 0.0147959, 0.0150363, 0.0152797, 0.0155262, 0.0157758, 0.0160283, 0.0162838,
                        0.0165421, 0.016804, 0.0170691, 0.0173374, 0.0176087, 0.0178835, 0.0181612, 0.0184423, 0.0187269, 0.0190149,
                        0.0193063, 0.0196009, 0.0198991, 0.0202003, 0.0205052, 0.0208137, 0.0211259, 0.0214418, 0.0217611, 0.0220841,
                        0.0224105, 0.0227406, 0.0230746, 0.0234125, 0.0237542, 0.0240996, 0.0244486, 0.0248012, 0.025158, 0.0255188,
                        0.0258837, 0.0262527, 0.0266256, 0.0270025, 0.0273833, 0.027768, 0.0281572, 0.0285505, 0.0289483, 0.0293503,
                        0.0297564, 0.0301665, 0.0305808, 0.0309997, 0.0314231, 0.0318511, 0.0322835, 0.0327205, 0.0331616, 0.0336073,
                        0.0340576, 0.0345128, 0.0349727, 0.0354373, 0.0359066, 0.0363807, 0.0368589, 0.0373419, 0.0378302, 0.0383234,
                        0.0388218, 0.0393252, 0.0398336, 0.040347, 0.0408652, 0.041388, 0.0419165, 0.0424502, 0.0429893, 0.0435338,
                        0.0440833, 0.044638, 0.0451976, 0.0457627, 0.0463338, 0.0469103, 0.047492, 0.0480797, 0.0486729, 0.0492716,
                        0.0498757, 0.0504852, 0.0511009, 0.0517229, 0.0523503, 0.0529838, 0.0536231, 0.0542678, 0.054918, 0.0555743,
                        0.0562372, 0.0569065, 0.0575818, 0.0582634, 0.0589511, 0.0596454, 0.0603451, 0.061051, 0.0617635, 0.0624826,
                        0.0632084, 0.0639409, 0.06468, 0.0654254, 0.0661772, 0.0669346, 0.0676994, 0.0684714, 0.0692503, 0.0700354,
                        0.0708285, 0.0716277, 0.0724347, 0.0732479, 0.0740671, 0.0748947, 0.0757299, 0.0765715, 0.0774207, 0.0782771,
                        0.0791407, 0.0800119, 0.0808897, 0.0817743, 0.0826672, 0.0835684, 0.0844769, 0.0853938, 0.0863179, 0.0872493,
                        0.0881882, 0.0891349, 0.090089, 0.0910523, 0.0920236, 0.093002, 0.0939894, 0.094985, 0.0959887, 0.0970003,
                        0.0980191, 0.0990454, 0.100081, 0.101126, 0.10218, 0.103242, 0.104312, 0.105392, 0.10648, 0.107576,
                        0.10868, 0.109793, 0.110916, 0.112048, 0.113188, 0.114339, 0.115498, 0.116666, 0.117843, 0.119028,
                        0.120223, 0.121427, 0.122641, 0.123865, 0.125098, 0.126342, 0.127595, 0.128857, 0.130128, 0.131409,
                        0.132701, 0.134002, 0.135314, 0.136635, 0.137966, 0.139308, 0.14066, 0.142022, 0.143394, 0.144774,
                        0.146166, 0.14757, 0.148985, 0.15041, 0.151845, 0.153291, 0.154749, 0.156215, 0.157694, 0.159182,
                        0.160682, 0.162194, 0.163718, 0.165251, 0.166797, 0.168354, 0.169921, 0.1715, 0.17309, 0.17469,
                        0.176304, 0.177929, 0.179566, 0.181216, 0.182878, 0.184553, 0.186238, 0.187934, 0.189642, 0.191362,
                        0.193096, 0.194842, 0.196602, 0.198374, 0.200158, 0.201954, 0.203764, 0.205586, 0.207421, 0.209266,
                        0.211124, 0.212997, 0.214882, 0.216783, 0.218697, 0.220624, 0.222565, 0.224518, 0.226486, 0.228466,
                        0.230458, 0.232463, 0.234484, 0.23652, 0.238569, 0.240633, 0.242711, 0.244803, 0.246909, 0.249031,
                        0.251165, 0.253313, 0.255475, 0.257649, 0.259841, 0.262051, 0.264274, 0.266514, 0.268768, 0.271036,
                        0.273319, 0.275618, 0.277932, 0.280259, 0.282602, 0.28496, 0.287338, 0.28973, 0.292138, 0.294563,
                        0.297003, 0.299458, 0.30193, 0.304417, 0.306919, 0.309437, 0.311972, 0.314526, 0.317095, 0.319684,
                        0.322289, 0.324911, 0.327551, 0.330205, 0.332876, 0.335567, 0.338271, 0.340993, 0.343736, 0.346496,
                        0.349272, 0.352065, 0.354878, 0.35771, 0.360561, 0.363426, 0.366311, 0.369212, 0.372128, 0.375067,
                        0.378027, 0.381006, 0.384001, 0.387014, 0.39005, 0.393106, 0.396181, 0.399271, 0.402384, 0.405513,
                        0.408661, 0.41183, 0.41502, 0.418233, 0.421462, 0.424709, 0.42798, 0.43127, 0.434583, 0.437914,
                        0.441267, 0.444637, 0.448022, 0.451434, 0.454868, 0.458328, 0.461805, 0.465302, 0.468821, 0.472364,
                        0.475928, 0.47951, 0.483119, 0.486748, 0.490397, 0.494066, 0.497758, 0.501477, 0.505217, 0.508977,
                        0.512762, 0.516567, 0.520394, 0.524247, 0.528125, 0.532027, 0.535947, 0.53989, 0.543852, 0.547844,
                        0.551863, 0.555904, 0.559966, 0.56406, 0.568177, 0.572312, 0.576471, 0.580662, 0.584875, 0.58911,
                        0.593373, 0.597653, 0.601965, 0.606301, 0.610663, 0.615051, 0.619465, 0.623907, 0.62837, 0.632863,
                        0.637383, 0.641924, 0.646494, 0.651091, 0.655708, 0.660356, 0.665027, 0.669732, 0.674464, 0.679227,
                        0.684016, 0.688827, 0.693664, 0.698532, 0.703428, 0.708353, 0.713307, 0.718283, 0.72329, 0.728322,
                        0.733387, 0.738479, 0.743605, 0.748763, 0.753949, 0.759163, 0.764407, 0.769674, 0.774973, 0.780311,
                        0.78567, 0.791057, 0.796476, 0.801922, 0.8074, 0.812919, 0.818466, 0.824044
                      };

    double m2 = 0.13957 * 0.13957;
    double smin = (0.13957 * 4) * (0.13957 * 4);
    double dh = 0.001;
    int od  = (sc - 0.312) / dh;
    double sc_m = 0.312 + od * dh;
    double rhouse = 0;
    if (sc >= 0.312 && sc < 1) {
      rhouse = ((sc - sc_m) / dh) * (rho[od + 1] - rho[od]) + rho[od];
    } else if (sc < 0.312 && sc >= smin) {
      rhouse = ((sc - smin) / (0.312 - smin)) * rho[0];
    } else if (sc >= 1) {
      //    rhouse = (1-16*m2/sc)*(1-16*m2/sc)*(1-16*m2/sc)*(1-16*m2/sc)*(1-16*m2/sc);
      rhouse = sqrt(1 - 16 * m2 / sc);
    } else {
      rhouse = 0;
    }
    return rhouse;
  }

  std::complex<double> EvtD0Topippim2pi0::rhoMTX(int i, int j, double s)
  {

    double rhoijx;
    double rhoijy = 0.0;
    if (i == j && i == 0) {
      double m2 = 0.13957 * 0.13957;
      if ((1 - (4 * m2) / s) > 0) {
        rhoijx = sqrt(1.0f - (4 * m2) / s);
        rhoijy = 0;
      } else {
        rhoijy = sqrt((4 * m2) / s - 1.0f);
        rhoijx = 0;
      }
    }
    if (i == j && i == 1) {
      double m2 = 0.493677 * 0.493677;
      if ((1 - (4 * m2) / s) > 0) {
        rhoijx = sqrt(1.0f - (4 * m2) / s);
        rhoijy = 0;
      } else {
        rhoijy = sqrt((4 * m2) / s - 1.0f);
        rhoijx = 0;
      }
    }
    if (i == j && i == 2) {
      rhoijx = rho22(s);
      rhoijy = 0;
    }
    if (i == j && i == 3) {
      double m2 = 0.547862 * 0.547862;
      if ((1 - (4 * m2) / s) > 0) {
        rhoijx = sqrt(1.0f - (4 * m2) / s);
        rhoijy = 0;
      } else {
        rhoijy = sqrt((4 * m2) / s - 1.0f);
        rhoijx = 0;
      }
    }
    if (i == j && i == 4) {
      double m_1 = 0.547862;
      double m_2 = 0.95778;
      double mp2 = (m_1 + m_2) * (m_1 + m_2);
      if ((1 - mp2 / s) > 0) {
        rhoijx = sqrt(1.0f - mp2 / s);
        rhoijy = 0;
      } else {
        rhoijy = sqrt(mp2 / s - 1.0f);
        rhoijx = 0;
      }
    }

    if (i != j) {
      rhoijx = 0;
      rhoijy = 0;
    }
    std::complex<double> rhoij(rhoijx, rhoijy);
    return rhoij;

  }
  /* Kij = (sum_a gigj/(ma^2-s) + fij(1-s0)/(s-s0))((s-0.5sampi2)(1-sa0)/(s-sa0))  */
  std::complex<double> EvtD0Topippim2pi0::KMTX(int i, int j, double s)
  {

    double Kijx;
    double Kijy;
    double mpi = 0.13957;
    double m[5]  = { 0.65100, 1.20360, 1.55817, 1.21000, 1.82206};

    double g1[5] = { 0.22889, -0.55377, 0.00000, -0.39899, -0.34639};
    double g2[5] = { 0.94128, 0.55095, 0.00000, 0.39065, 0.31503};
    double g3[5] = { 0.36856, 0.23888, 0.55639, 0.18340, 0.18681};
    double g4[5] = { 0.33650, 0.40907, 0.85679, 0.19906, -0.00984};
    double g5[5] = { 0.18171, -0.17558, -0.79658, -0.00355, 0.22358};

    double f1[5] = { 0.23399, 0.15044, -0.20545, 0.32825, 0.35412};

    double eps   = 1e-11;

    double upreal[5] = { 0, 0, 0, 0, 0};
    double upimag[5] = { 0, 0, 0, 0, 0};

    for (int k = 0; k < 5; k++) {

      /*    down[k] = (m[k]*m[k]-s)*(m[k]*m[k]-s)+eps*eps;
          upreal[k] = (m[k]*m[k]-s)/down[k];
          upimag[k] = -1.0f * eps/down[k];  */

      double dm2 = m[k] * m[k] - s;
      if (fabs(dm2) < eps && dm2 <= 0) dm2 = -eps;
      if (fabs(dm2) < eps && dm2 > 0)  dm2 = eps;
      upreal[k] = 1.0f / dm2;
      upimag[k] = 0;
    }

    double tmp1x = g1[i] * g1[j] * upreal[0] + g2[i] * g2[j] * upreal[1] + g3[i] * g3[j] * upreal[2] + g4[i] * g4[j] * upreal[3] + g5[i]
                   * g5[j] * upreal[4];
    double tmp1y = g1[i] * g1[j] * upimag[0] + g2[i] * g2[j] * upimag[1] + g3[i] * g3[j] * upimag[2] + g4[i] * g4[j] * upimag[3] + g5[i]
                   * g5[j] * upimag[4];

    double tmp2  = 0;
    if (i == 0) {
      tmp2 = f1[j] * (1 + 3.92637) / (s + 3.92637);
    }
    if (j == 0) {
      tmp2 = f1[i] * (1 + 3.92637) / (s + 3.92637);
    }
    double tmp3 = (s - 0.5 * mpi * mpi) * (1 + 0.15) / (s + 0.15);

    Kijx = (tmp1x + tmp2) * tmp3;
    Kijy = (tmp1y) * tmp3;

    std::complex<double> Kij(Kijx, Kijy);
    return Kij;
  }

  std::complex<double> EvtD0Topippim2pi0::IMTX(int i, int j)
  {

    double Iijx;
    double Iijy;
    if (i == j) {
      Iijx = 1;
      Iijy = 0;
    } else {
      Iijx = 0;
      Iijy = 0;
    }
    std::complex<double> Iij(Iijx, Iijy);
    return Iij;

  }

  /* F = I - i*K*rho */
  std::complex<double> EvtD0Topippim2pi0::FMTX(double Kijx, double Kijy, double rhojjx, double rhojjy, int i, int j)
  {

    double Fijx;
    double Fijy;

    double tmpx = rhojjx * Kijx - rhojjy * Kijy;
    double tmpy = rhojjx * Kijy + rhojjy * Kijx;

    Fijx = IMTX(i, j).real() + tmpy;
    Fijy = -tmpx;

    std::complex<double> Fij(Fijx, Fijy);
    return Fij;
  }

  /* inverse for Matrix (I - i*rho*K) using LUP */
  double EvtD0Topippim2pi0::FINVMTX(double s, double* FINVx, double* FINVy)
  {

    int P[5] = { 0, 1, 2, 3, 4};

    double Fx[5][5];
    double Fy[5][5];

    double Ux[5][5];
    double Uy[5][5];
    double Lx[5][5];
    double Ly[5][5];

    double UIx[5][5];
    double UIy[5][5];
    double LIx[5][5];
    double LIy[5][5];

    for (int k = 0; k < 5; k++) {
      double rhokkx = rhoMTX(k, k, s).real();
      double rhokky = rhoMTX(k, k, s).imag();
      Ux[k][k] = rhokkx;
      Uy[k][k] = rhokky;
      for (int l = k; l < 5; l++) {
        double Kklx = KMTX(k, l, s).real();
        double Kkly = KMTX(k, l, s).imag();
        Lx[k][l] = Kklx;
        Ly[k][l] = Kkly;
        Lx[l][k] = Lx[k][l];
        Ly[l][k] = Ly[k][l];
      }
    }

    for (int k = 0; k < 5; k++) {
      for (int l = 0; l < 5; l++) {
        double Fklx = FMTX(Lx[k][l], Ly[k][l], Ux[l][l], Uy[l][l], k, l).real();
        double Fkly = FMTX(Lx[k][l], Ly[k][l], Ux[l][l], Uy[l][l], k, l).imag();
        Fx[k][l] = Fklx;
        Fy[k][l] = Fkly;
      }
    }

    for (int k = 0; k < 5; k++) {
      double tmprM = (Fx[k][k] * Fx[k][k] + Fy[k][k] * Fy[k][k]);
      int tmpID = 0;
      for (int l = k; l < 5; l++) {
        double tmprF = (Fx[l][k] * Fx[l][k] + Fy[l][k] * Fy[l][k]);
        if (tmprM <= tmprF) {
          tmprM = tmprF;
          tmpID = l;
        }
      }

      int tmpP = P[k];
      P[k]     = P[tmpID];
      P[tmpID] = tmpP;

      for (int l = 0; l < 5; l++) {

        double tmpFx = Fx[k][l];
        double tmpFy = Fy[k][l];

        Fx[k][l] = Fx[tmpID][l];
        Fy[k][l] = Fy[tmpID][l];

        Fx[tmpID][l] = tmpFx;
        Fy[tmpID][l] = tmpFy;

      }

      for (int l = k + 1; l < 5; l++) {
        double rFkk = Fx[k][k] * Fx[k][k] + Fy[k][k] * Fy[k][k];
        double Fxlk = Fx[l][k];
        double Fylk = Fy[l][k];
        double Fxkk = Fx[k][k];
        double Fykk = Fy[k][k];
        Fx[l][k] = (Fxlk * Fxkk + Fylk * Fykk) / rFkk;
        Fy[l][k] = (Fylk * Fxkk - Fxlk * Fykk) / rFkk;
        for (int m = k + 1; m < 5; m++) {
          Fx[l][m] = Fx[l][m] - (Fx[l][k] * Fx[k][m] - Fy[l][k] * Fy[k][m]);
          Fy[l][m] = Fy[l][m] - (Fx[l][k] * Fy[k][m] + Fy[l][k] * Fx[k][m]);
        }
      }
    }

    for (int k = 0; k < 5; k++) {
      for (int l = 0; l < 5 ; l++) {
        if (k == l) {
          Lx[k][k] = 1;
          Ly[k][k] = 0;
          Ux[k][k] = Fx[k][k];
          Uy[k][k] = Fy[k][k];
        }
        if (k > l) {
          Lx[k][l] = Fx[k][l];
          Ly[k][l] = Fy[k][l];
          Ux[k][l] = 0;
          Uy[k][l] = 0;
        }
        if (k < l) {
          Ux[k][l] = Fx[k][l];
          Uy[k][l] = Fy[k][l];
          Lx[k][l] = 0;
          Ly[k][l] = 0;
        }
      }
    }

    // calculate Inverse for L and U
    for (int k = 0; k < 5; k++) {

      LIx[k][k] = 1;
      LIy[k][k] = 0;

      double rUkk = Ux[k][k] * Ux[k][k] + Uy[k][k] * Uy[k][k];
      UIx[k][k] = Ux[k][k] / rUkk;
      UIy[k][k] = -1.0f * Uy[k][k] / rUkk ;

      for (int l = (k + 1); l < 5; l++) {
        LIx[k][l] = 0;
        LIy[k][l] = 0;
        UIx[l][k] = 0;
        UIy[l][k] = 0;
      }

      for (int l = (k - 1); l >= 0; l--) { // U-1
        double sx   = 0;
        double c_sx = 0;
        double sy   = 0;
        double c_sy = 0;
        for (int m = l + 1; m <= k; m++) {
          sx = sx - c_sx;
          double sx_tmp = sx + Ux[l][m] * UIx[m][k] - Uy[l][m] * UIy[m][k];
          c_sx = (sx_tmp - sx) - (Ux[l][m] * UIx[m][k] - Uy[l][m] * UIy[m][k]);
          sx = sx_tmp;

          sy = sy - c_sy;
          double sy_tmp = sy + Ux[l][m] * UIy[m][k] + Uy[l][m] * UIx[m][k];
          c_sy = (sy_tmp - sy) - (Ux[l][m] * UIy[m][k] + Uy[l][m] * UIx[m][k]);
          sy = sy_tmp;
        }
        UIx[l][k] = -1.0f * (UIx[l][l] * sx - UIy[l][l] * sy);
        UIy[l][k] = -1.0f * (UIy[l][l] * sx + UIx[l][l] * sy);
      }

      for (int l = k + 1; l < 5; l++) { // L-1
        double sx = 0;
        double c_sx = 0;
        double sy = 0;
        double c_sy = 0;
        for (int m = k; m < l; m++) {
          sx = sx - c_sx;
          double sx_tmp = sx + Lx[l][m] * LIx[m][k] - Ly[l][m] * LIy[m][k];
          c_sx = (sx_tmp - sx) - (Lx[l][m] * LIx[m][k] - Ly[l][m] * LIy[m][k]);
          sx = sx_tmp;

          sy = sy - c_sy;
          double sy_tmp = sy + Lx[l][m] * LIy[m][k] + Ly[l][m] * LIx[m][k];
          c_sy = (sy_tmp - sy) - (Lx[l][m] * LIy[m][k] + Ly[l][m] * LIx[m][k]);
          sy = sy_tmp;
        }
        LIx[l][k] = -1.0f * sx;
        LIy[l][k] = -1.0f * sy;
      }
    }

    for (int m = 0; m < 5; m++) {
      double resX = 0;
      double c_resX = 0;
      double resY = 0;
      double c_resY = 0;
      for (int k = 0; k < 5; k++) {
        for (int l = 0; l < 5; l++) {
          double Plm = 0;
          if (P[l] == m) Plm = 1;

          resX = resX - c_resX;
          double resX_tmp = resX + (UIx[0][k] * LIx[k][l] - UIy[0][k] * LIy[k][l]) * Plm;
          c_resX = (resX_tmp - resX) - ((UIx[0][k] * LIx[k][l] - UIy[0][k] * LIy[k][l]) * Plm);
          resX = resX_tmp;

          resY = resY - c_resY;
          double resY_tmp = resY + (UIx[0][k] * LIy[k][l] + UIy[0][k] * LIx[k][l]) * Plm;
          c_resY = (resY_tmp - resY) - ((UIx[0][k] * LIy[k][l] + UIy[0][k] * LIx[k][l]) * Plm);
          resY = resY_tmp;
        }
      }
      FINVx[m] = resX;
      FINVy[m] = resY;
    }

    return 1.0f;
  }

  std::complex<double> EvtD0Topippim2pi0::PVTR(int ID, double s)
  {

    double VPix;
    double VPiy;
    double m[5]  = { 0.65100, 1.20360, 1.55817, 1.21000, 1.82206};

    double eps   = 1e-11;

    /*  double down   = (m[ID]*m[ID]-s)*(m[ID]*m[ID]-s)+eps*eps;
        double upreal = (m[ID]*m[ID]-s)/down;
        double upimag = -1.0f * eps/down;  */

    double dm2 = m[ID] * m[ID] - s;

    if (fabs(dm2) < eps && dm2 <= 0) dm2 = -eps;
    if (fabs(dm2) < eps && dm2 > 0)  dm2 = eps;

    VPix = 1.0f / dm2;
    VPiy = 0;

    std::complex<double> VPi(VPix, VPiy);
    return VPi;
  }

  std::complex<double> EvtD0Topippim2pi0::Fvector(double sa, double s0, int l)
  {

    double outputx = 0;
    double outputy = 0;

    double FINVx[5] = {0, 0, 0, 0, 0};
    double FINVy[5] = {0, 0, 0, 0, 0};

    if (l < 5) {
      double g[5][5] = {{ 0.22889, -0.55377, 0.00000, -0.39899, -0.34639},
        { 0.94128, 0.55095, 0.00000, 0.39065, 0.31503},
        { 0.36856, 0.23888, 0.55639, 0.18340, 0.18681},
        { 0.33650, 0.40907, 0.85679, 0.19906, -0.00984},
        { 0.18171, -0.17558, -0.79658, -0.00355, 0.22358}
      };
      double resx = 0;
      double c_resx = 0;
      double resy = 0;
      double c_resy = 0;
      double Plx = PVTR(l, sa).real();
      double Ply = PVTR(l, sa).imag();
      for (int j = 0; j < 5; j++) {
        resx = resx - c_resx;
        double resx_tmp = resx + (FINVx[j] * g[l][j] * Plx - FINVy[j] * g[l][j] * Ply);
        c_resx = (resx_tmp - resx) - (FINVx[j] * g[l][j] * Plx - FINVy[j] * g[l][j] * Ply);
        resx = resx_tmp;

        resy = resy - c_resy;
        double resy_tmp = resy + (FINVx[j] * g[l][j] * Ply + FINVy[j] * g[l][j] * Plx);
        c_resy = (resy_tmp - resy) - (FINVx[j] * g[l][j] * Ply + FINVy[j] * g[l][j] * Plx);
        resy = resy_tmp;
      }
      outputx = resx;
      outputy = resy;
    } else {
      int idx = l - 5;
      double eps = 1e-11;
      double ds = sa - s0;
      if (fabs(ds) < eps && ds <= 0) ds = -eps;
      if (fabs(ds) < eps && ds > 0)  ds = eps;
      double tmp = (1 - s0) / ds;
      outputx = FINVx[idx] * tmp;
      outputy = FINVy[idx] * tmp;
    }

    std::complex<double> output(outputx, outputy);
    return output;
  }

  std::complex<double> EvtD0Topippim2pi0::CalD0Amp()
  {
    return Amp(m_Pip, m_Pim, m_Pi01, m_Pi02);
  }
  std::complex<double> EvtD0Topippim2pi0::CalDbAmp()
  {

    std::vector<double> cpPip;  cpPip.clear();
    std::vector<double> cpPim;  cpPim.clear();
    std::vector<double> cpPi01; cpPi01.clear();
    std::vector<double> cpPi02; cpPi02.clear();

    cpPip.push_back(-m_Pim[0]); cpPim.push_back(-m_Pip[0]); cpPi01.push_back(-m_Pi01[0]); cpPi02.push_back(-m_Pi02[0]);
    cpPip.push_back(-m_Pim[1]); cpPim.push_back(-m_Pip[1]); cpPi01.push_back(-m_Pi01[1]); cpPi02.push_back(-m_Pi02[1]);
    cpPip.push_back(-m_Pim[2]); cpPim.push_back(-m_Pip[2]); cpPi01.push_back(-m_Pi01[2]); cpPi02.push_back(-m_Pi02[2]);
    cpPip.push_back(m_Pim[3]); cpPim.push_back(m_Pip[3]); cpPi01.push_back(m_Pi01[3]); cpPi02.push_back(m_Pi02[3]);

    return Amp(cpPip, cpPim, cpPi01, cpPi02);
  }

  std::complex<double> EvtD0Topippim2pi0::Amp(std::vector<double> Pip, std::vector<double> Pim, std::vector<double> Pi01,
                                              std::vector<double> Pi02)
  {

    std::vector<double> PipPim; PipPim.clear();
    std::vector<double> PipPi01; PipPi01.clear();
    std::vector<double> PipPi02; PipPi02.clear();
    std::vector<double> PimPi01; PimPi01.clear();
    std::vector<double> PimPi02; PimPi02.clear();
    std::vector<double> Pi01Pi02; Pi01Pi02.clear();

    PipPim   = sum_tensor(Pip, Pim);
    PipPi01  = sum_tensor(Pip, Pi01);
    PipPi02  = sum_tensor(Pip, Pi02);
    PimPi01  = sum_tensor(Pim, Pi01);
    PimPi02  = sum_tensor(Pim, Pi02);
    Pi01Pi02 = sum_tensor(Pi01, Pi02);

    std::vector<double> PipPimPi01; PipPimPi01.clear();
    std::vector<double> PipPimPi02; PipPimPi02.clear();
    std::vector<double> PipPi01Pi02; PipPi01Pi02.clear();
    std::vector<double> PimPi01Pi02; PimPi01Pi02.clear();

    PipPimPi01 = sum_tensor(PipPim, Pi01);
    PipPimPi02 = sum_tensor(PipPim, Pi02);
    PipPi01Pi02 = sum_tensor(PipPi01, Pi02);
    PimPi01Pi02 = sum_tensor(PimPi01, Pi02);

    std::vector<double> D0; D0.clear();
    D0 = sum_tensor(PipPimPi01, Pi02);

    double M2_PipPim  = contract_11_0(PipPim, PipPim);
    double M2_PipPi01 = contract_11_0(PipPi01, PipPi01);
    double M2_PipPi02  = contract_11_0(PipPi02, PipPi02);
    double M2_PimPi01  = contract_11_0(PimPi01, PimPi01);
    double M2_PimPi02  = contract_11_0(PimPi02, PimPi02);
    double M2_Pi01Pi02  = contract_11_0(Pi01Pi02, Pi01Pi02);

    double M2_PipPimPi01 = contract_11_0(PipPimPi01, PipPimPi01);
    double M2_PipPimPi02 = contract_11_0(PipPimPi02, PipPimPi02);
    double M2_PipPi01Pi02 = contract_11_0(PipPi01Pi02, PipPi01Pi02);
    double M2_PimPi01Pi02 = contract_11_0(PimPi01Pi02, PimPi01Pi02);

    std::complex<double> GS_rho770_pm = GS(M2_PipPim,  m0_rho7700, w0_rho7700, m2_Pi, m2_Pi, rRes, 1);
    std::complex<double> GS_rho770_p1 = GS(M2_PipPi01, m0_rho770p, w0_rho770p, m2_Pi, m2_Pi0, rRes, 1);
    std::complex<double> GS_rho770_p2 = GS(M2_PipPi02, m0_rho770p, w0_rho770p, m2_Pi, m2_Pi0, rRes, 1);
    std::complex<double> GS_rho770_m1 = GS(M2_PimPi01, m0_rho770p, w0_rho770p, m2_Pi, m2_Pi0, rRes, 1);
    std::complex<double> GS_rho770_m2 = GS(M2_PimPi02, m0_rho770p, w0_rho770p, m2_Pi, m2_Pi0, rRes, 1);
    std::complex<double> GS_rho1450_m1 = GS(M2_PimPi01, m0_rho1450, w0_rho1450, m2_Pi, m2_Pi0, rRes, 1);
    std::complex<double> GS_rho1450_m2 = GS(M2_PimPi02, m0_rho1450, w0_rho1450, m2_Pi, m2_Pi0, rRes, 1);

    std::complex<double> FT_f0980_00 = Flatte(M2_Pi01Pi02, m0_f0980, g1_f0980, g2_f0980, m_Pi, m_Pi, m_Ka, m_Ka);
    std::complex<double> RBW_f21270_pm = RBW(M2_PipPim, m0_f21270, w0_f21270, m2_Pi, m2_Pi, rRes, 2);
    std::complex<double> RBW_f21270_00 = RBW(M2_Pi01Pi02, m0_f21270, w0_f21270, m2_Pi0, m2_Pi0, rRes, 2);

    std::complex<double> PiPiS_pm_0 = Fvector(M2_PipPim, s0_prod, 0);
    std::complex<double> PiPiS_00_0 = Fvector(M2_Pi01Pi02, s0_prod, 0);

    std::complex<double> PiPiS_pm_1 = Fvector(M2_PipPim, s0_prod, 1);
    std::complex<double> PiPiS_00_1 = Fvector(M2_Pi01Pi02, s0_prod, 1);

    std::complex<double> PiPiS_pm_5 = Fvector(M2_PipPim, s0_prod, 5);
    std::complex<double> PiPiS_00_5 = Fvector(M2_Pi01Pi02, s0_prod, 5);

    std::complex<double> PiPiS_pm_6 = Fvector(M2_PipPim, s0_prod, 6);
    std::complex<double> PiPiS_00_6 = Fvector(M2_Pi01Pi02, s0_prod, 6);

    std::complex<double> RBW_a11260_p = RBWa1260(M2_PipPi01Pi02, m0_a11260, g1_a11260, g2_a11260);
    std::complex<double> RBW_a11260_m = RBWa1260(M2_PimPi01Pi02, m0_a11260, g1_a11260, g2_a11260);
    std::complex<double> RBW_a11260_01 = RBWa1260(M2_PipPimPi01, m0_a11260, g1_a11260, g2_a11260);
    std::complex<double> RBW_a11260_02 = RBWa1260(M2_PipPimPi02, m0_a11260, g1_a11260, g2_a11260);

    std::complex<double> RBW_a11420_p = RBW(M2_PipPi01Pi02, m0_a11420, w0_a11420, -1, -1, -1, -1);

    std::complex<double> RBW_a11640_p = RBWa1640(M2_PipPi01Pi02, m0_a11640, w0_a11640);
    std::complex<double> RBW_a11640_m = RBWa1640(M2_PimPi01Pi02, m0_a11640, w0_a11640);

    std::complex<double> RBW_omega_01 = RBW(M2_PipPimPi01, m0_omega, w0_omega, -1, -1, -1, -1);
    std::complex<double> RBW_omega_02 = RBW(M2_PipPimPi02, m0_omega, w0_omega, -1, -1, -1, -1);

    std::complex<double> RBW_phi_01 = RBW(M2_PipPimPi01, m0_phi, w0_phi, -1, -1, -1, -1);
    std::complex<double> RBW_phi_02 = RBW(M2_PipPimPi02, m0_phi, w0_phi, -1, -1, -1, -1);

    std::complex<double> RBW_a21320_p = RBW(M2_PipPi01Pi02, m0_a21320, w0_a21320, -1, -1, -1, -1);
    std::complex<double> RBW_a21320_m = RBW(M2_PimPi01Pi02, m0_a21320, w0_a21320, -1, -1, -1, -1);

    std::complex<double> RBW_pi1300_p = RBWpi1300(M2_PipPi01Pi02, m0_pi1300, w0_pi1300);
    std::complex<double> RBW_pi1300_m = RBWpi1300(M2_PimPi01Pi02, m0_pi1300, w0_pi1300);
    std::complex<double> RBW_pi1300_01 = RBWpi1300(M2_PipPimPi01, m0_pi1300, w0_pi1300);
    std::complex<double> RBW_pi1300_02 = RBWpi1300(M2_PipPimPi02, m0_pi1300, w0_pi1300);

    std::complex<double> RBW_h11170_01 = RBWh11170(M2_PipPimPi01, m0_h11170, w0_h11170);
    std::complex<double> RBW_h11170_02 = RBWh11170(M2_PipPimPi02, m0_h11170, w0_h11170);

    std::complex<double> RBW_pi21670_01 = RBW(M2_PipPimPi01, m0_pi21670, w0_pi21670, -1, -1, -1, -1);
    std::complex<double> RBW_pi21670_02 = RBW(M2_PipPimPi02, m0_pi21670, w0_pi21670, -1, -1, -1, -1);

    // D->XX Projection
    std::vector<double> Proj1_3p; Proj1_3p.clear();
    std::vector<double> Proj1_3m; Proj1_3m.clear();
    std::vector<double> Proj1_3z1; Proj1_3z1.clear();
    std::vector<double> Proj1_3z2; Proj1_3z2.clear();

    Proj1_3p  = ProjectionTensors(PipPi01Pi02, 1);
    Proj1_3m  = ProjectionTensors(PimPi01Pi02, 1);
    Proj1_3z1 = ProjectionTensors(PipPimPi01, 1);
    Proj1_3z2 = ProjectionTensors(PipPimPi02, 1);

    std::vector<double> Proj2_3p; Proj2_3p.clear();
    std::vector<double> Proj2_3m; Proj2_3m.clear();
    std::vector<double> Proj2_3z1; Proj2_3z1.clear();
    std::vector<double> Proj2_3z2; Proj2_3z2.clear();

    Proj2_3p  = ProjectionTensors(PipPi01Pi02, 2);
    Proj2_3m  = ProjectionTensors(PimPi01Pi02, 2);
    Proj2_3z1 = ProjectionTensors(PipPimPi01, 2);
    Proj2_3z2 = ProjectionTensors(PipPimPi02, 2);

    // X->PP Orbital
    std::vector<double> T1_PipPim; T1_PipPim.clear();
    std::vector<double> T1_PipPi01; T1_PipPi01.clear();
    std::vector<double> T1_PipPi02; T1_PipPi02.clear();
    std::vector<double> T1_PimPi01; T1_PimPi01.clear();
    std::vector<double> T1_PimPi02; T1_PimPi02.clear();
    std::vector<double> T1_Pi01Pi02; T1_Pi01Pi02.clear();

    T1_PipPim = OrbitalTensors(PipPim, Pip, Pim, rRes, 1);
    T1_PipPi01 = OrbitalTensors(PipPi01, Pip, Pi01, rRes, 1);
    T1_PipPi02 = OrbitalTensors(PipPi02, Pip, Pi02, rRes, 1);
    T1_PimPi01 = OrbitalTensors(PimPi01, Pim, Pi01, rRes, 1);
    T1_PimPi02 = OrbitalTensors(PimPi02, Pim, Pi02, rRes, 1);
    T1_Pi01Pi02 = OrbitalTensors(Pi01Pi02, Pi01, Pi02, rRes, 1);

    std::vector<double> T2_PipPim; T2_PipPim.clear();
    std::vector<double> T2_Pi01Pi02; T2_Pi01Pi02.clear();

    T2_PipPim = OrbitalTensors(PipPim, Pip, Pim, rRes, 2);
    T2_Pi01Pi02 = OrbitalTensors(Pi01Pi02, Pi01, Pi02, rRes, 2);

    // X->YP Orbital
    std::vector<double> T1_PipPimPi01; T1_PipPimPi01.clear();
    std::vector<double> T1_PipPimPi02; T1_PipPimPi02.clear();
    std::vector<double> T1_PipPi01Pi02; T1_PipPi01Pi02.clear();
    std::vector<double> T1_PipPi02Pi01; T1_PipPi02Pi01.clear();
    std::vector<double> T1_PimPi01Pi02; T1_PimPi01Pi02.clear();
    std::vector<double> T1_PimPi02Pi01; T1_PimPi02Pi01.clear();
    std::vector<double> T1_PipPi01Pim; T1_PipPi01Pim.clear();
    std::vector<double> T1_PipPi02Pim; T1_PipPi02Pim.clear();
    std::vector<double> T1_PimPi01Pip; T1_PimPi01Pip.clear();
    std::vector<double> T1_PimPi02Pip; T1_PimPi02Pip.clear();
    std::vector<double> T1_Pi01Pi02Pip; T1_Pi01Pi02Pip.clear();
    std::vector<double> T1_Pi01Pi02Pim; T1_Pi01Pi02Pim.clear();

    T1_PipPimPi01 = OrbitalTensors(PipPimPi01, PipPim, Pi01, rRes, 1);
    T1_PipPimPi02 = OrbitalTensors(PipPimPi02, PipPim, Pi02, rRes, 1);
    T1_PipPi01Pi02 = OrbitalTensors(PipPi01Pi02, PipPi01, Pi02, rRes, 1);
    T1_PipPi02Pi01 = OrbitalTensors(PipPi01Pi02, PipPi02, Pi01, rRes, 1);
    T1_PimPi01Pi02 = OrbitalTensors(PimPi01Pi02, PimPi01, Pi02, rRes, 1);
    T1_PimPi02Pi01 = OrbitalTensors(PimPi01Pi02, PimPi02, Pi01, rRes, 1);
    T1_PipPi01Pim = OrbitalTensors(PipPimPi01, PipPi01, Pim, rRes, 1);
    T1_PipPi02Pim = OrbitalTensors(PipPimPi02, PipPi02, Pim, rRes, 1);
    T1_PimPi01Pip = OrbitalTensors(PipPimPi01, PimPi01, Pip, rRes, 1);
    T1_PimPi02Pip = OrbitalTensors(PipPimPi02, PimPi02, Pip, rRes, 1);
    T1_Pi01Pi02Pip = OrbitalTensors(PipPi01Pi02, Pi01Pi02, Pip, rRes, 1);
    T1_Pi01Pi02Pim = OrbitalTensors(PimPi01Pi02, Pi01Pi02, Pim, rRes, 1);

    std::vector<double> T2_PipPimPi01; T2_PipPimPi01.clear();
    std::vector<double> T2_PipPimPi02; T2_PipPimPi02.clear();
    std::vector<double> T2_PipPi01Pi02; T2_PipPi01Pi02.clear();
    std::vector<double> T2_PipPi02Pi01; T2_PipPi02Pi01.clear();
    std::vector<double> T2_PimPi01Pi02; T2_PimPi01Pi02.clear();
    std::vector<double> T2_PimPi02Pi01; T2_PimPi02Pi01.clear();
    std::vector<double> T2_PipPi01Pim; T2_PipPi01Pim.clear();
    std::vector<double> T2_PipPi02Pim; T2_PipPi02Pim.clear();
    std::vector<double> T2_PimPi01Pip; T2_PimPi01Pip.clear();
    std::vector<double> T2_PimPi02Pip; T2_PimPi02Pip.clear();
    std::vector<double> T2_Pi01Pi02Pip; T2_Pi01Pi02Pip.clear();
    std::vector<double> T2_Pi01Pi02Pim; T2_Pi01Pi02Pim.clear();

    T2_PipPimPi01 = OrbitalTensors(PipPimPi01, PipPim, Pi01, rRes, 2);
    T2_PipPimPi02 = OrbitalTensors(PipPimPi02, PipPim, Pi02, rRes, 2);
    T2_PipPi01Pi02 = OrbitalTensors(PipPi01Pi02, PipPi01, Pi02, rRes, 2);
    T2_PipPi02Pi01 = OrbitalTensors(PipPi01Pi02, PipPi02, Pi01, rRes, 2);
    T2_PimPi01Pi02 = OrbitalTensors(PimPi01Pi02, PimPi01, Pi02, rRes, 2);
    T2_PimPi02Pi01 = OrbitalTensors(PimPi01Pi02, PimPi02, Pi01, rRes, 2);
    T2_PipPi01Pim = OrbitalTensors(PipPimPi01, PipPi01, Pim, rRes, 2);
    T2_PipPi02Pim = OrbitalTensors(PipPimPi02, PipPi02, Pim, rRes, 2);
    T2_PimPi01Pip = OrbitalTensors(PipPimPi01, PimPi01, Pip, rRes, 2);
    T2_PimPi02Pip = OrbitalTensors(PipPimPi02, PimPi02, Pip, rRes, 2);
    T2_Pi01Pi02Pip = OrbitalTensors(PipPi01Pi02, Pi01Pi02, Pip, rRes, 2);
    T2_Pi01Pi02Pim = OrbitalTensors(PimPi01Pi02, Pi01Pi02, Pim, rRes, 2);

    // D->XX Orbital
    std::vector<double> T1_2pm12; T1_2pm12.clear();
    std::vector<double> T1_2p1m2; T1_2p1m2.clear();
    std::vector<double> T1_2p2m1; T1_2p2m1.clear();

    T1_2pm12 = OrbitalTensors(D0, PipPim, Pi01Pi02, rD, 1);
    T1_2p1m2 = OrbitalTensors(D0, PipPi01, PimPi02, rD, 1);
    T1_2p2m1 = OrbitalTensors(D0, PipPi02, PimPi01, rD, 1);

    std::vector<double> T2_2pm12; T2_2pm12.clear();
    std::vector<double> T2_2p1m2; T2_2p1m2.clear();
    std::vector<double> T2_2p2m1; T2_2p2m1.clear();

    T2_2pm12 = OrbitalTensors(D0, PipPim, Pi01Pi02, rD, 2);
    T2_2p1m2 = OrbitalTensors(D0, PipPi01, PimPi02, rD, 2);
    T2_2p2m1 = OrbitalTensors(D0, PipPi02, PimPi01, rD, 2);

    // D->XP Orbital
    std::vector<double> T1_3pm; T1_3pm.clear();
    std::vector<double> T1_3mp; T1_3mp.clear();
    std::vector<double> T1_3z12; T1_3z12.clear();
    std::vector<double> T1_3z21; T1_3z21.clear();

    T1_3pm  = OrbitalTensors(D0, PipPi01Pi02, Pim, rD, 1);
    T1_3mp  = OrbitalTensors(D0, PimPi01Pi02, Pip, rD, 1);
    T1_3z12 = OrbitalTensors(D0, PipPimPi01, Pi02, rD, 1);
    T1_3z21 = OrbitalTensors(D0, PipPimPi02, Pi01, rD, 1);

    std::vector<double> T2_3pm;  T2_3pm.clear();
    std::vector<double> T2_3mp;  T2_3mp.clear();
    std::vector<double> T2_3z12; T2_3z12.clear();
    std::vector<double> T2_3z21; T2_3z21.clear();

    T2_3pm  = OrbitalTensors(D0, PipPi01Pi02, Pim, rD, 2);
    T2_3mp  = OrbitalTensors(D0, PimPi01Pi02, Pip, rD, 2);
    T2_3z12 = OrbitalTensors(D0, PipPimPi01, Pi02, rD, 2);
    T2_3z21 = OrbitalTensors(D0, PipPimPi02, Pi01, rD, 2);

    std::complex<double> amplitude(0, 0);

    // D0 -> a1(1260)+ {rho(770)+ pi0[S]} pi-
    double SF_Ap_S_Vp1P = contract_11_0(contract_21_1(Proj1_3p, T1_PipPi01), T1_3pm);
    double SF_Ap_S_Vp2P = contract_11_0(contract_21_1(Proj1_3p, T1_PipPi02), T1_3pm);

    amplitude += fitpara[0] * (SF_Ap_S_Vp1P * RBW_a11260_p * GS_rho770_p1 + SF_Ap_S_Vp2P * RBW_a11260_p * GS_rho770_p2);

    // D0 -> a1(1260)+ {rho(770)+ pi0[D]} pi-
    double SF_Ap_D_Vp1P = contract_11_0(contract_21_1(T2_PipPi01Pi02, T1_PipPi01), T1_3pm);
    double SF_Ap_D_Vp2P = contract_11_0(contract_21_1(T2_PipPi02Pi01, T1_PipPi02), T1_3pm);

//  std::cout<<SF_Ap_D_VP_1<<","<<SF_Ap_D_VP_2<<","<<SF_Ap_D_VP_3<<","<<SF_Ap_D_VP_4<<","<<std::endl;
//  std::cout<<"-------"<<std::endl;
    amplitude += fitpara[1] * (SF_Ap_D_Vp1P * RBW_a11260_p * GS_rho770_p1 + SF_Ap_D_Vp2P * RBW_a11260_p * GS_rho770_p2);

    // D0 -> a1(1260)+ {f2(1270) pi+ [P]} pi-
    double SF_Ap_P_TP = contract_11_0(contract_21_1(contract_42_2(Proj2_3p, T2_Pi01Pi02), T1_Pi01Pi02Pip), T1_3pm);

    amplitude += fitpara[2] * (SF_Ap_P_TP * RBW_a11260_p * RBW_f21270_00);

    // D0 -> a1(1260)+ {f0 pi+ [P]} pi-
    double SF_Ap_P_SP = contract_11_0(T1_3pm, T1_Pi01Pi02Pip);

    amplitude += fitpara[3] * (SF_Ap_P_SP * RBW_a11260_p * PiPiS_00_0);
    amplitude += fitpara[4] * (SF_Ap_P_SP * RBW_a11260_p * PiPiS_00_5);
    amplitude += fitpara[5] * (SF_Ap_P_SP * RBW_a11260_p * PiPiS_00_6);

    // D0 -> a1(1260)- { rho(770)- pi0 [S]} pi+
    double SF_Am_S_Vm1P = contract_11_0(contract_21_1(Proj1_3m, T1_PimPi01), T1_3mp);
    double SF_Am_S_Vm2P = contract_11_0(contract_21_1(Proj1_3m, T1_PimPi02), T1_3mp);

    amplitude += fitpara[6] * fitpara[0] * (SF_Am_S_Vm1P * RBW_a11260_m * GS_rho770_m1 + SF_Am_S_Vm2P * RBW_a11260_m * GS_rho770_m2);

    // D0 -> a1(1260)- {rho(770)- pi0[D]} pi+
    double SF_Am_D_Vm1P = contract_11_0(contract_21_1(T2_PimPi01Pi02, T1_PimPi01), T1_3mp);
    double SF_Am_D_Vm2P = contract_11_0(contract_21_1(T2_PimPi02Pi01, T1_PimPi02), T1_3mp);

    amplitude += fitpara[6] * fitpara[1] * (SF_Am_D_Vm1P * RBW_a11260_m * GS_rho770_m1 + SF_Am_D_Vm2P * RBW_a11260_m * GS_rho770_m2);

    // D0 -> a1(1260)- {f2(1270) pi- [P]} pi+
    double SF_Am_P_TP = contract_11_0(contract_21_1(contract_42_2(Proj2_3m, T2_Pi01Pi02), T1_Pi01Pi02Pim), T1_3mp);

    amplitude += fitpara[6] * fitpara[2] * (SF_Am_P_TP * RBW_a11260_m * RBW_f21270_00);

    // D0 -> a1(1260)- {f0 pi- [P]} pi+
    double SF_Am_P_SP = contract_11_0(T1_3mp, T1_Pi01Pi02Pim);

    amplitude += fitpara[6] * fitpara[3] * (SF_Am_P_SP * RBW_a11260_m * PiPiS_00_0);
    amplitude += fitpara[6] * fitpara[4] * (SF_Am_P_SP * RBW_a11260_m * PiPiS_00_5);
    amplitude += fitpara[6] * fitpara[5] * (SF_Am_P_SP * RBW_a11260_m * PiPiS_00_6);

    // D -> a1(1260)0 pi0
    double SF_A01_S_Vp1P = contract_11_0(contract_21_1(Proj1_3z1, T1_PipPi01), T1_3z12);
    double SF_A02_S_Vp2P = contract_11_0(contract_21_1(Proj1_3z2, T1_PipPi02), T1_3z21);
    double SF_A01_S_Vm1P = contract_11_0(contract_21_1(Proj1_3z1, T1_PimPi01), T1_3z12);
    double SF_A02_S_Vm2P = contract_11_0(contract_21_1(Proj1_3z2, T1_PimPi02), T1_3z21);
    double SF_A01_S_VzP  = contract_11_0(contract_21_1(Proj1_3z1, T1_PipPim), T1_3z12);
    double SF_A02_S_VzP  = contract_11_0(contract_21_1(Proj1_3z2, T1_PipPim), T1_3z21);

    amplitude += fitpara[7] * fitpara[0] * (SF_A01_S_Vp1P * RBW_a11260_01 * GS_rho770_p1 + SF_A02_S_Vp2P * RBW_a11260_02 * GS_rho770_p2
                                            + SF_A01_S_Vm1P * RBW_a11260_01 * GS_rho770_m1 + SF_A02_S_Vm2P * RBW_a11260_02 * GS_rho770_m2);

    double SF_A01_D_Vp1P = contract_11_0(contract_21_1(T2_PipPi01Pim, T1_PipPi01), T1_3z12);
    double SF_A02_D_Vp2P = contract_11_0(contract_21_1(T2_PipPi02Pim, T1_PipPi02), T1_3z21);
    double SF_A01_D_Vm1P = contract_11_0(contract_21_1(T2_PimPi01Pip, T1_PimPi01), T1_3z12);
    double SF_A02_D_Vm2P = contract_11_0(contract_21_1(T2_PimPi02Pip, T1_PimPi02), T1_3z21);

    amplitude += fitpara[7] * fitpara[1] * (SF_A01_D_Vp1P * RBW_a11260_01 * GS_rho770_p1 + SF_A02_D_Vp2P * RBW_a11260_02 * GS_rho770_p2
                                            + SF_A01_D_Vm1P * RBW_a11260_01 * GS_rho770_m1 + SF_A02_D_Vm2P * RBW_a11260_02 * GS_rho770_m2);

    double SF_A01_P_TP = contract_11_0(contract_21_1(contract_42_2(Proj2_3z1, T2_PipPim), T1_PipPimPi01), T1_3z12);
    double SF_A02_P_TP = contract_11_0(contract_21_1(contract_42_2(Proj2_3z2, T2_PipPim), T1_PipPimPi02), T1_3z21);

    amplitude += fitpara[7] * fitpara[2] * (-1.0) * (SF_A01_P_TP * RBW_a11260_01 * RBW_f21270_pm + SF_A02_P_TP * RBW_a11260_02 *
                                                     RBW_f21270_pm);

    double SF_A01_P_SP = contract_11_0(T1_3z12, T1_PipPimPi01);
    double SF_A02_P_SP = contract_11_0(T1_3z21, T1_PipPimPi02);

    amplitude += fitpara[7] * fitpara[3] * (-1.0) * (SF_A01_P_SP * RBW_a11260_01 * PiPiS_pm_0 + SF_A02_P_SP * RBW_a11260_02 *
                                                     PiPiS_pm_0);
    amplitude += fitpara[7] * fitpara[4] * (-1.0) * (SF_A01_P_SP * RBW_a11260_01 * PiPiS_pm_5 + SF_A02_P_SP * RBW_a11260_02 *
                                                     PiPiS_pm_5);
    amplitude += fitpara[7] * fitpara[5] * (-1.0) * (SF_A01_P_SP * RBW_a11260_01 * PiPiS_pm_6 + SF_A02_P_SP * RBW_a11260_02 *
                                                     PiPiS_pm_6);

    // D0 -> a1(1420)+ {f0 pi0+[S]} pi-
    amplitude += fitpara[8] * (SF_Ap_P_SP * RBW_a11420_p * FT_f0980_00);

    // D0 -> a1(1640)+ {rho pi[S]} pi-
    amplitude += fitpara[9] * (SF_Ap_S_Vp1P * RBW_a11640_p * GS_rho770_p1 + SF_Ap_S_Vp2P * RBW_a11640_p * GS_rho770_p2);

    // D0 -> a1(1640)- {rho pi[S]} pi+
    amplitude += fitpara[10] * (SF_Am_S_Vm1P * RBW_a11640_m * GS_rho770_m1 + SF_Am_S_Vm2P * RBW_a11640_m * GS_rho770_m2);

    // D0 -> a2(1320)+ {rho+ pi0} pi-
    double SF_Tp_D_Vp1P = contract_22_0(contract_22_2(contract_31_2(contract_41_3(epsilon_uvmn, contract_21_1(Proj1_3p, T1_PipPi01)),
                                                      PipPi01Pi02), contract_42_2(Proj2_3p, T2_3pm)), T2_PipPi01Pi02);
    double SF_Tp_D_Vp2P = contract_22_0(contract_22_2(contract_31_2(contract_41_3(epsilon_uvmn, contract_21_1(Proj1_3p, T1_PipPi02)),
                                                      PipPi01Pi02), contract_42_2(Proj2_3p, T2_3pm)), T2_PipPi02Pi01);

    amplitude += fitpara[11] * (SF_Tp_D_Vp1P * GS_rho770_p1 * RBW_a21320_p + SF_Tp_D_Vp2P * GS_rho770_p2 * RBW_a21320_p);

    // D0 -> a2(1320)- {rho- pi0} pi+
    double SF_Tm_D_Vm1P = contract_22_0(contract_22_2(contract_31_2(contract_41_3(epsilon_uvmn, contract_21_1(Proj1_3m, T1_PimPi01)),
                                                      PimPi01Pi02), contract_42_2(Proj2_3m, T2_3mp)), T2_PimPi01Pi02);
    double SF_Tm_D_Vm2P = contract_22_0(contract_22_2(contract_31_2(contract_41_3(epsilon_uvmn, contract_21_1(Proj1_3m, T1_PimPi02)),
                                                      PimPi01Pi02), contract_42_2(Proj2_3m, T2_3mp)), T2_PimPi02Pi01);
    amplitude += fitpara[12] * (SF_Tm_D_Vm1P * GS_rho770_m1 * RBW_a21320_m + SF_Tm_D_Vm2P * GS_rho770_m2 * RBW_a21320_m);

    // D0 -> h1(1170)0 {rho  pi0} pi0
    amplitude += fitpara[13] * (SF_A01_S_Vp1P * RBW_h11170_01 * GS_rho770_p1 + SF_A02_S_Vp2P * RBW_h11170_02 * GS_rho770_p2 -
                                SF_A01_S_Vm1P * RBW_h11170_01 * GS_rho770_m1 - SF_A02_S_Vm2P * RBW_h11170_02 * GS_rho770_m2 - SF_A01_S_VzP * RBW_h11170_01 *
                                GS_rho770_pm - SF_A02_S_VzP * RBW_h11170_02 * GS_rho770_pm);

    // D0 -> pi(1300)- {rho(770)- pi0} pi+
    double SF_Pm_P_Vm1P = contract_11_0(T1_PimPi01, T1_PimPi01Pi02);
    double SF_Pm_P_Vm2P = contract_11_0(T1_PimPi02, T1_PimPi02Pi01);

    amplitude += fitpara[14] * (SF_Pm_P_Vm1P * GS_rho770_m1 * RBW_pi1300_m + SF_Pm_P_Vm2P * GS_rho770_m2 * RBW_pi1300_m);

    // D0 -> pi(1300)- {f2 pi-} pi+
//  double SF_Pm_D_TP = contract_22_0(T2_Pi01Pi02,T2_Pi01Pi02Pim);
//  amplitude += fitpara[14]*fitpara[13]*(SF_Pm_D_TP*RBW_f21270_00*RBW_pi1300_m);

    // D0 -> pi(1300)- {f0 pi-} pi+
    amplitude += fitpara[15] * fitpara[14] * (RBW_pi1300_m * PiPiS_00_0);
//  amplitude += fitpara[15]*fitpara[13]*(RBW_pi1300_m*PiPiS_00_5);
    amplitude += fitpara[16] * fitpara[14] * (RBW_pi1300_m * PiPiS_00_6);

    // D0 -> pi(1300)+ {rho(770)+ pi0} pi-
    double SF_Pp_P_Vp1P = contract_11_0(T1_PipPi01, T1_PipPi01Pi02);
    double SF_Pp_P_Vp2P = contract_11_0(T1_PipPi02, T1_PipPi02Pi01);

    amplitude += fitpara[17] * (SF_Pp_P_Vp1P * GS_rho770_p1 * RBW_pi1300_p + SF_Pp_P_Vp2P * GS_rho770_p2 * RBW_pi1300_p);

    // D0 -> pi(1300)+ {f2 pi-} pi+
//  double SF_Pp_D_TP = contract_22_0(T2_Pi01Pi02,T2_Pi01Pi02Pip);
//  amplitude += fitpara[14]*fitpara[17]*(SF_Pp_D_TP*RBW_f21270_00*RBW_pi1300_p);

    // D0 -> pi(1300)+ {f0 pi+} pi-
    amplitude += fitpara[15] * fitpara[17] * (RBW_pi1300_p * PiPiS_00_0);
//  amplitude += fitpara[15]*fitpara[17]*(RBW_pi1300_p*PiPiS_00_5);
    amplitude += fitpara[16] * fitpara[17] * (RBW_pi1300_p * PiPiS_00_6);

    // D0 -> pi(1300)0 {rho pi} pi0
    double SF_P01_P_Vp1P = contract_11_0(T1_PipPi01, T1_PipPi01Pim);
    double SF_P02_P_Vp2P = contract_11_0(T1_PipPi02, T1_PipPi02Pim);
    double SF_P01_P_Vm1P = contract_11_0(T1_PimPi01, T1_PimPi01Pip);
    double SF_P02_P_Vm2P = contract_11_0(T1_PimPi02, T1_PimPi02Pip);

    amplitude += fitpara[18] * (SF_P01_P_Vp1P * RBW_pi1300_01 * GS_rho770_p1 + SF_P02_P_Vp2P * RBW_pi1300_02 * GS_rho770_p2 +
                                SF_P01_P_Vm1P * RBW_pi1300_01 * GS_rho770_m1 + SF_P02_P_Vm2P * RBW_pi1300_02 * GS_rho770_m2);


//  double SF_P01_D_TP = contract_22_0(T2_PipPim,T2_PipPimPi01);
//  double SF_P02_D_TP = contract_22_0(T2_PipPim,T2_PipPimPi02);
//  amplitude += fitpara[14]*fitpara[18]*(-1.0)*(SF_P01_D_TP*RBW_f21270_pm*RBW_pi1300_01 + SF_P02_D_TP*RBW_f21270_pm*RBW_pi1300_02);

    amplitude += fitpara[15] * fitpara[18] * (-1.0) * (RBW_pi1300_01 * PiPiS_pm_0 + RBW_pi1300_02 * PiPiS_pm_0);
//  amplitude += fitpara[15]*fitpara[18]*(-1.0)*(RBW_pi1300_01*PiPiS_pm_5 + RBW_pi1300_02*PiPiS_pm_5);
    amplitude += fitpara[16] * fitpara[18] * (-1.0) * (RBW_pi1300_01 * PiPiS_pm_6 + RBW_pi1300_02 * PiPiS_pm_6);

    // D0 -> pi2(1670)0[f2(1270) pi0] pi0
    double SF_PT01_S_TP = contract_22_0(contract_42_2(Proj2_3z1, T2_PipPim), T2_3z12);
    double SF_PT02_S_TP = contract_22_0(contract_42_2(Proj2_3z2, T2_PipPim), T2_3z21);

    amplitude += fitpara[19] * (-1.0) * (SF_PT01_S_TP * RBW_f21270_pm * RBW_pi21670_01 + SF_PT02_S_TP * RBW_f21270_pm * RBW_pi21670_02);

    // D0 -> rho+ rho- [S]
    double SF_Vp1Vm2_S = contract_11_0(T1_PipPi01, T1_PimPi02);
    double SF_Vp2Vm1_S = contract_11_0(T1_PipPi02, T1_PimPi01);

    amplitude += fitpara[20] * (SF_Vp1Vm2_S * GS_rho770_p1 * GS_rho770_m2 + SF_Vp2Vm1_S * GS_rho770_p2 * GS_rho770_m1);

    // D0 -> rho+ rho- [P]
    double SF_Vp1Vm2_P = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, T1_PipPi01), T1_PimPi02), T1_2p1m2), D0);
    double SF_Vp2Vm1_P = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, T1_PipPi02), T1_PimPi01), T1_2p2m1), D0);

    amplitude += fitpara[21] * (SF_Vp1Vm2_P * GS_rho770_p1 * GS_rho770_m2 + SF_Vp2Vm1_P * GS_rho770_p2 * GS_rho770_m1);

    // D0 -> rho+ rho- [D]
    double SF_Vp1Vm2_D = contract_11_0(contract_21_1(T2_2p1m2, T1_PipPi01), T1_PimPi02);
    double SF_Vp2Vm1_D = contract_11_0(contract_21_1(T2_2p2m1, T1_PipPi02), T1_PimPi01);
    amplitude += fitpara[22] * (SF_Vp1Vm2_D * GS_rho770_p1 * GS_rho770_m2 + SF_Vp2Vm1_D * GS_rho770_p2 * GS_rho770_m1);

    amplitude += fitpara[23] * (SF_Vp1Vm2_D * GS_rho770_p1 * GS_rho1450_m2 + SF_Vp2Vm1_D * GS_rho770_p2 * GS_rho1450_m1);

    // D0 -> rho0 (Pi0 Pi0)S
    double SF_VpmS12_P = contract_11_0(T1_PipPim, T1_2pm12);

    amplitude += fitpara[24] * (SF_VpmS12_P * GS_rho770_pm * PiPiS_00_0);
    amplitude += fitpara[25] * (SF_VpmS12_P * GS_rho770_pm * PiPiS_00_5);
    amplitude += fitpara[26] * (SF_VpmS12_P * GS_rho770_pm * PiPiS_00_6);

    // D0 -> f0f0
    //00
    amplitude += fitpara[27] * (PiPiS_pm_0 * PiPiS_00_0 + PiPiS_00_0 * PiPiS_pm_0);
    amplitude += fitpara[28] * (PiPiS_pm_0 * PiPiS_00_1 + PiPiS_00_0 * PiPiS_pm_1);
    amplitude += fitpara[29] * (PiPiS_pm_1 * PiPiS_00_5 + PiPiS_00_1 * PiPiS_pm_5);
    amplitude += fitpara[30] * (PiPiS_pm_5 * PiPiS_00_5 + PiPiS_00_5 * PiPiS_pm_5);
    amplitude += fitpara[31] * (PiPiS_pm_5 * PiPiS_00_6 + PiPiS_00_5 * PiPiS_pm_6);

    // D0 -> f2(1270) f0
    double SF_TpmS00_D = contract_22_0(T2_PipPim, T2_2pm12);
    double SF_T00Spm_D = contract_22_0(T2_Pi01Pi02, T2_2pm12);

    amplitude += fitpara[32] * (SF_TpmS00_D * RBW_f21270_pm * PiPiS_00_5 + SF_T00Spm_D * RBW_f21270_00 * PiPiS_pm_5);
    amplitude += fitpara[33] * (SF_TpmS00_D * RBW_f21270_pm * PiPiS_00_6 + SF_T00Spm_D * RBW_f21270_00 * PiPiS_pm_6);


    // D -> 1--(rho pi) pi0
    double SF_V1_Vz  = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, PipPimPi01), T1_PipPimPi01), T1_PipPim),
                                     contract_21_1(Proj1_3z1, T1_3z12));
    double SF_V1_Vp1 = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, PipPimPi01), T1_PipPi01Pim), T1_PipPi01),
                                     contract_21_1(Proj1_3z1, T1_3z12));
    double SF_V1_Vm1 = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, PipPimPi01), T1_PimPi01Pip), T1_PimPi01),
                                     contract_21_1(Proj1_3z1, T1_3z12));

    double SF_V2_Vz  = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, PipPimPi02), T1_PipPimPi02), T1_PipPim),
                                     contract_21_1(Proj1_3z2, T1_3z21));
    double SF_V2_Vp2 = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, PipPimPi02), T1_PipPi02Pim), T1_PipPi02),
                                     contract_21_1(Proj1_3z2, T1_3z21));
    double SF_V1_Vm2 = contract_11_0(contract_21_1(contract_31_2(contract_41_3(epsilon_uvmn, PipPimPi02), T1_PimPi02Pip), T1_PimPi02),
                                     contract_21_1(Proj1_3z2, T1_3z21));


    // D0 -> omega(rho pi) pi0
    amplitude += (-1.0) * fitpara[34] * (SF_V1_Vp1 * RBW_omega_01 * GS_rho770_p1 - SF_V1_Vz * RBW_omega_01 * GS_rho770_pm - SF_V1_Vm1 *
                                         RBW_omega_01 * GS_rho770_m1 + SF_V2_Vp2 * RBW_omega_02 * GS_rho770_p2 - SF_V2_Vz * RBW_omega_02 * GS_rho770_pm - SF_V1_Vm2 *
                                         RBW_omega_02 * GS_rho770_m2);


    // D0 -> phi(rho pi) pi0
    amplitude += (-1.0) * fitpara[35] * (SF_V1_Vp1 * RBW_phi_01 * GS_rho770_p1 - SF_V1_Vz * RBW_phi_01 * GS_rho770_pm - SF_V1_Vm1 *
                                         RBW_phi_01 * GS_rho770_m1 + SF_V2_Vp2 * RBW_phi_02 * GS_rho770_p2 - SF_V2_Vz * RBW_phi_02 * GS_rho770_pm - SF_V1_Vm2 * RBW_phi_02 *
                                         GS_rho770_m2);

    return amplitude;

  }

  int EvtD0Topippim2pi0::CalAmp()
  {

    m_AmpD0 = CalD0Amp();
    m_AmpDb = CalDbAmp();

    return 0;
  }

  double EvtD0Topippim2pi0::mag2(std::complex<double> x)
  {
    double temp = x.real() * x.real() + x.imag() * x.imag();
    return temp;
  }

  double EvtD0Topippim2pi0::arg(std::complex<double> x)
  {
    double temp = atan(x.imag() / x.real());
    if (x.real() < 0) temp = temp + TMath::Pi();
    return temp;
  }
  double EvtD0Topippim2pi0::Get_strongPhase()
  {
    double temp = arg(m_AmpD0) - arg(m_AmpDb);
    while (temp < -TMath::Pi()) {
      temp += 2.0 * TMath::Pi();
    }
    while (temp > TMath::Pi()) {
      temp -= 2.0 * TMath::Pi();
    }
    return temp;
  }

  double EvtD0Topippim2pi0::AmplitudeSquare(int Charm, int Tagmode)
  {

    EvtVector4R dp1 = GetDaugMomLab(0), dp2 = GetDaugMomLab(1), dp3 = GetDaugMomLab(2), dp4 = GetDaugMomLab(3); // pi+ pi- pi0 pi0
    EvtVector4R mp = dp1 + dp2 + dp3 + dp4;

    double emp = mp.get(0);
    EvtVector3R boostp3(-mp.get(1) / emp, -mp.get(2) / emp, -mp.get(3) / emp);

    EvtVector4R dp1bst = boostTo(dp1, boostp3);
    EvtVector4R dp2bst = boostTo(dp2, boostp3);
    EvtVector4R dp3bst = boostTo(dp3, boostp3);
    EvtVector4R dp4bst = boostTo(dp4, boostp3);

    double p4pip[4], p4pim[4], p4pi01[4], p4pi02[4];
    for (int i = 0; i < 3; i++) {
      p4pip[i] = dp1bst.get(i + 1);
      p4pim[i] = dp2bst.get(i + 1);
      p4pi01[i] = dp3bst.get(i + 1);
      p4pi02[i] = dp4bst.get(i + 1);
    }
    p4pip[3] = dp1bst.get(0);
    p4pim[3] = dp2bst.get(0);
    p4pi01[3] = dp3bst.get(0);
    p4pi02[3] = dp4bst.get(0);

    setInput(p4pip, p4pim, p4pi01, p4pi02);
    CalAmp();
    std::complex<double> ampD0, ampDb;
    if (Charm > 0) {
      ampD0 = Get_AmpD0();
      ampDb = Get_AmpDb();
    } else {
      ampD0 = Get_AmpDb();
      ampDb = Get_AmpD0();
    }

    double ampsq = 1e-20;
    double r_tag = 0, R_tag = 0, delta_tag = 0;

    if (Tagmode == 1 || Tagmode == 2 || Tagmode == 3) {
      if (Tagmode == 1) { // Kpi
        r_tag = 0.0586;
        R_tag = 1;
        delta_tag = 192.1 / 180.0 * 3.1415926;
      } else if (Tagmode == 2) { // Kpipi0
        r_tag = 0.0441;
        R_tag = 0.79;
        delta_tag = 196.0 / 180.0 * 3.1415926;
      } else if (Tagmode == 3) { // K3pi
        r_tag = 0.0550;
        R_tag = 0.44;
        delta_tag = 161.0 / 180.0 * 3.1415926;
      }
      std::complex<double> qcf(r_tag * R_tag * cos(-delta_tag), r_tag * R_tag * sin(-delta_tag));
      std::complex<double> ampD0_part1 = ampD0 - qcf * ampDb;
      ampsq = mag2(ampD0_part1) + r_tag * r_tag * (1 - R_tag * R_tag) * (mag2(ampDb));
    } else {
      ampsq = mag2(ampD0);
    }
    return (ampsq <= 0) ? 1e-20 : ampsq;
  }

}
