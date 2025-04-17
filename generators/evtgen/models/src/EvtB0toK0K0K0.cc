/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/evtgen/EvtGenModelRegister.h>

#include <stdlib.h>
#include <cmath>
#include <cstring>
#include "EvtGenBase/EvtRandom.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtReport.hh"
#include "EvtGenBase/EvtConst.hh"
#include "EvtGenBase/EvtId.hh"

#include "generators/evtgen/models/EvtB0toK0K0K0.h"

using std::endl;
using namespace std::complex_literals;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtB0toK0K0K0);

  EvtB0toK0K0K0::~EvtB0toK0K0K0() {}

  std::string EvtB0toK0K0K0::getName()
  {
    return "B0TOK0K0K0";
  }

  EvtDecayBase* EvtB0toK0K0K0::clone()
  {
    return new EvtB0toK0K0K0;
  }

  void EvtB0toK0K0K0::decay(EvtParticle* p)
  {
    // follow PhysRevD.85.054023

    int Ntry = 0;
    const int max_Ntry = 10000;

    while (true) {
      p->initializePhaseSpace(getNDaug(), getDaugs());

      EvtParticle* NeutralKaon_1 = p->getDaug(0);
      EvtParticle* NeutralKaon_2 = p->getDaug(1);
      EvtParticle* NeutralKaon_3 = p->getDaug(2);

      EvtVector4R mom_NeutralKaon_1 = NeutralKaon_1->getP4();
      EvtVector4R mom_NeutralKaon_2 = NeutralKaon_2->getP4();
      EvtVector4R mom_NeutralKaon_3 = NeutralKaon_3->getP4();

      EvtVector4R mom_sum_12 = mom_NeutralKaon_1 + mom_NeutralKaon_2;
      EvtVector4R mom_sum_13 = mom_NeutralKaon_1 + mom_NeutralKaon_3;
      EvtVector4R mom_sum_23 = mom_NeutralKaon_2 + mom_NeutralKaon_3;

      double s12 = mom_sum_12.mass2();
      double s13 = mom_sum_13.mass2();
      double s23 = mom_sum_23.mass2();

      // follow the index of PhysRevD.85.054023
      double smax = std::max(std::max(s12, s13), s23);
      double smin = std::min(std::min(s12, s13), s23);

      // the maximum value of probability is obtained by brute fource method. scan all region and find.
      const double Probability_max = 0.409212;
      double Probability_value = Probability(smax, smin);
      double Probability_ratio = Probability_value / Probability_max;

      double xbox = EvtRandom::Flat(0.0, 1.0);

      if (xbox < Probability_ratio) break;
      Ntry++;
      if (Ntry > max_Ntry) {
        std::cout << "try to set the kinematics more than 10000 times. abort.\n";
        ::abort();
      }
    }


    return;

  }


  void EvtB0toK0K0K0::initProbMax()
  {
    noProbMax();
  }


  void EvtB0toK0K0K0::init()
  {

    // check that there are no arguments
    checkNArg(0);

    // there should be three daughters: K+ KL0 KL0
    checkNDaug(3);

    // Check that the daughters are correct
    EvtId KaonZeroType_1 = getDaug(0);
    EvtId KaonZeroType_2 = getDaug(1);
    EvtId KaonZeroType_3 = getDaug(2);

    int KaonZerotyp = 0;

    if ((KaonZeroType_1 == EvtPDL::getId("K0")) ||
        (KaonZeroType_1 == EvtPDL::getId("anti-K0")) ||
        (KaonZeroType_1 == EvtPDL::getId("K_S0")) ||
        (KaonZeroType_1 == EvtPDL::getId("K_L0"))) KaonZerotyp++;
    if ((KaonZeroType_2 == EvtPDL::getId("K0")) ||
        (KaonZeroType_2 == EvtPDL::getId("anti-K0")) ||
        (KaonZeroType_2 == EvtPDL::getId("K_S0")) ||
        (KaonZeroType_2 == EvtPDL::getId("K_L0"))) KaonZerotyp++;
    if ((KaonZeroType_3 == EvtPDL::getId("K0")) ||
        (KaonZeroType_3 == EvtPDL::getId("anti-K0")) ||
        (KaonZeroType_3 == EvtPDL::getId("K_S0")) ||
        (KaonZeroType_3 == EvtPDL::getId("K_L0"))) KaonZerotyp++;

    if (KaonZerotyp != 3) {

      std::cout << "All daughters should be K0, anti-K0, K_L0, or K_S0.\n";
      ::abort();
    }

    // initialize q0 and pstar0
    GetMasses();
    GetZeros();

  }

  double EvtB0toK0K0K0::Probability(double s13, double s23)
  {
    std::complex<double> total_amplitude = Amplitude(s13, s23, "f980") + Amplitude(s13, s23, "f1710") + Amplitude(s13, s23,
                                           "f2010") + Amplitude(s13, s23, "chic0") + Amplitude(s13, s23, "NR");
    return std::abs(total_amplitude) * std::abs(total_amplitude);
  }


  std::complex<double> EvtB0toK0K0K0::Amplitude(double s13, double s23, const char* resonance)
  {

    // this factors scale each resonance. Target FFs are written in the paper
    double MagicNumber_f980 = std::sqrt(0.44 / 30.222805945);
    double MagicNumber_f1710 = std::sqrt(0.07 / 136.555962030);
    double MagicNumber_f2010 = std::sqrt(0.09 / 310762.546712675);
    double MagicNumber_chic0 = std::sqrt(0.07 / 677.282967269);
    double MagicNumber_NR = std::sqrt(2.16 / 51.262701105);

    double s12 = mB0 * mB0 + mKS0 * mKS0 + mKL0 * mKL0 + mKL0 * mKL0 - s13 - s23;

    if (strcmp(resonance, "f980") == 0) {
      std::complex<double> a;
      a = c_f980 * std::exp(1i * phi_f980);
      return MagicNumber_f980 * a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s12, s23,
                                     resonance) + DynamicalAmplitude(s12, s13, resonance));
    } else if (strcmp(resonance, "f1710") == 0) {
      std::complex<double> a;
      a = c_f1710 * std::exp(1i * phi_f1710);
      return MagicNumber_f1710 * a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s12, s23,
                                      resonance) + DynamicalAmplitude(s12, s13, resonance));
    } else if (strcmp(resonance, "f2010") == 0) {
      std::complex<double> a;
      a = c_f2010 * std::exp(1i * phi_f2010);
      return MagicNumber_f2010 * a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s12, s23,
                                      resonance) + DynamicalAmplitude(s12, s13, resonance));
    } else if (strcmp(resonance, "chic0") == 0) {
      std::complex<double> a;
      a = c_chic0 * std::exp(1i * phi_chic0);
      return MagicNumber_chic0 * a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s12, s23,
                                      resonance) + DynamicalAmplitude(s12, s13, resonance));
    } else if (strcmp(resonance, "NR") == 0) {

      std::complex<double> a;
      a = c_NR * std::exp(1i * phi_NR);

      return MagicNumber_NR * a * (std::exp(alpha_NR * s13) + std::exp(alpha_NR * s12) + std::exp(alpha_NR * s23));
    } else {
      printf("[Amplitude] unsupported resonance\n");
      exit(1);
    }

  }

  std::complex<double> EvtB0toK0K0K0::DynamicalAmplitude(double s13, double s23, const char* resonance)   // resonance: 1+2
  {

    if (strcmp(resonance, "f980") == 0) {
      return Flatte(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    }
    if (strcmp(resonance, "f1710") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else if (strcmp(resonance, "f2010") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else if (strcmp(resonance, "chic0") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else {
      printf("[Amplitude] unsupported resonance\n");
      exit(1);
    }

  }

  std::complex<double> EvtB0toK0K0K0::Flatte(double s13, double s23, const char* resonance)
  {
    double m0;
    double gpi;
    double gK;

    if (strcmp(resonance, "f980") == 0) {
      m0 = m0_f980;
      gpi = gpi_f980;
      gK = gK_f980;
    } else {
      printf("[Flatte] unsupported resonance\n");
      exit(1);
    }

    double m = Calculate_m(s13, s23);

    double Gammapipi = gpi * ((1.0 / 3.0) * std::sqrt(1 - 4.0 * mpi0 * mpi0 / (m * m)) + (2.0 / 3.0) * std::sqrt(
                                1 - 4.0 * mpic * mpic / (m * m)));
    double GammaKK = gK * (0.5 * std::sqrt(1 - 4.0 * mKp * mKp / (m * m)) + 0.5 * std::sqrt(1 - 4.0 * mK0 * mK0 / (m * m)));

    return 1.0 / ((m0 * m0 - m * m) - 1i * (Gammapipi + GammaKK));
  }

  std::complex<double> EvtB0toK0K0K0::RBW(double s13, double s23, const char* resonance)
  {
    double m0;

    double m = Calculate_m(s13, s23);

    if (strcmp(resonance, "f1710") == 0) {
      m0 = m0_f1710;
    } else if (strcmp(resonance, "f2010") == 0) {
      m0 = m0_f2010;
    } else if (strcmp(resonance, "chic0") == 0) {
      m0 = m0_chic0;
    } else {
      printf("[RBW] unsupported resonance\n");
      exit(1);
    }

    return 1.0 / (m0 * m0 - m * m - 1i * m0 * MassDepWidth(s13, s23, resonance));
  }

  double EvtB0toK0K0K0::MassDepWidth(double s13, double s23, const char* resonance)
  {
    // Gamma(m) when s12 and s23
    int spin = -1;
    double m0;
    double q0;
    double Gamma0;

    double q_mag = Calculate_q_mag(s13, s23);
    double m = Calculate_m(s13, s23);

    if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
      m0 = m0_f1710;
      q0 = q0_f1710;
      Gamma0 = Gamma0_f1710;
    } else if (strcmp(resonance, "f2010") == 0) {
      spin = spin_f2010;
      m0 = m0_f2010;
      q0 = q0_f2010;
      Gamma0 = Gamma0_f2010;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
      m0 = m0_chic0;
      q0 = q0_chic0;
      Gamma0 = Gamma0_chic0;
    } else {
      printf("[MassDepWidth] unsupported resonance\n");
      exit(1);
    }

    return Gamma0 * std::pow(q_mag / q0, 2 * spin + 1) * (m0 / m) * std::pow(BlattWeisskopf_qr(s13, s23, resonance), 2);
  }

  double EvtB0toK0K0K0::BlattWeisskopf_pstarrprime(double s13, double s23, const char* resonance)
  {
    // X_L(|p*|r') when s12 and s23
    int spin = -1;
    double z0 = -1;
    double z = Calculate_pstar_mag(s13, s23) * rprime;

    if (strcmp(resonance, "f980") == 0) {
      spin = spin_f980;
      z0 = -1;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
      z0 = pstar0_f1710 * rprime;
    } else if (strcmp(resonance, "f2010") == 0) {
      spin = spin_f2010;
      z0 = pstar0_f2010 * rprime;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
      z0 = pstar0_chic0 * rprime;
    } else {
      printf("[BlattWeisskopf_pstarrprime] unsupported resonance\n");
      exit(1);
    }

    if (spin == 0) return 1;
    else if (spin == 1) return std::sqrt((1 + z0 * z0) / (1 + z * z));
    else if (spin == 2) return std::sqrt((9 + 3 * z0 * z0 + z0 * z0 * z0 * z0) / (9 + 3 * z * z + z * z * z * z));
    else {
      printf("[BlattWeisskopf_pstarrprime] unsupported spin\n");
      exit(1);
    }

  }

  double EvtB0toK0K0K0::BlattWeisskopf_qr(double s13, double s23, const char* resonance)
  {
    // X_L(|q|r) when s12 and s23
    int spin = -1;
    double z0 = -1;
    double z = Calculate_q_mag(s13, s23) * r;

    if (strcmp(resonance, "f980") == 0) {
      spin = spin_f980;
      z0 = -1;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
      z0 = q0_f1710 * r;
    } else if (strcmp(resonance, "f2010") == 0) {
      spin = spin_f2010;
      z0 = q0_f2010 * r;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
      z0 = q0_chic0 * r;
    } else {
      printf("[BlattWeisskopf_qr] unsupported resonance\n");
      exit(1);
    }

    if (spin == 0) return 1;
    else if (spin == 1) return std::sqrt((1 + z0 * z0) / (1 + z * z));
    else if (spin == 2) return std::sqrt((9 + 3 * z0 * z0 + z0 * z0 * z0 * z0) / (9 + 3 * z * z + z * z * z * z));
    else {
      printf("[BlattWeisskopf_qr] unsupported spin\n");
      exit(1);
    }

  }

  double EvtB0toK0K0K0::Zemach(double s13, double s23, const char* resonance)   // resonance: 1+2
  {

    int spin;
    double p_dot_q = Calculate_q_dot_p_mag(s13, s23);
    double p_mag = Calculate_p_mag(s13, s23);
    double q_mag = Calculate_q_mag(s13, s23);

    if (strcmp(resonance, "f980") == 0) {
      spin = spin_f980;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
    } else if (strcmp(resonance, "f2010") == 0) {
      spin = spin_f2010;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
    } else {
      printf("[Zemach] unsupported resonance\n");
      exit(1);
    }

    if (spin == 0) return 1.0;
    else if (spin == 2) return (8.0 / 3.0) * (3 * p_dot_q * p_dot_q - p_mag * p_mag * q_mag * q_mag);
    else {
      printf("[Zemach] unsupported spin\n");
      exit(1);
    }

  }

  double EvtB0toK0K0K0::Calculate_m(double s13, double s23)   // resonance: 1+2
  {
    return std::sqrt(mB0 * mB0 + mKS0 * mKS0 + mKL0 * mKL0 + mKL0 * mKL0 - s13 - s23);
  }

  double EvtB0toK0K0K0::Calculate_q_mag(double s13, double s23)   // resonance: 1+2
  {
    double m = Calculate_m(s13, s23);
    return std::sqrt(m * m / 4.0 - mKL0 * mKL0);
  }

  double EvtB0toK0K0K0::Calculate_pstar_mag(double s13, double s23)   // resonance: 1+2
  {
    double m = Calculate_m(s13, s23);
    return std::sqrt(std::pow(mB0 * mB0 - m * m - mKS0 * mKS0, 2) / 4.0 - m * m * mKS0 * mKS0) / mB0;
  }

  double EvtB0toK0K0K0::Calculate_p_mag(double s13, double s23)   // resonance: 1+2
  {
    double m = Calculate_m(s13, s23);
    double s12 = m * m;
    return std::sqrt(std::pow(mB0 * mB0 - s12 - mKS0 * mKS0, 2) / (4 * s12) - mKS0 * mKS0);
  }

  double EvtB0toK0K0K0::Calculate_q_dot_p_mag(double s13, double s23)   // resonance: 1+2
  {
    return std::abs((s13 - s23) / 4.0);
  }

  void EvtB0toK0K0K0::GetZeros()
  {

    // get q0 and pstar0 for f1710
    q0_f1710 = std::sqrt((m0_f1710 * m0_f1710) / 4.0 - mKL0 * mKL0);
    pstar0_f1710 = std::sqrt(std::pow(mB0 * mB0 - m0_f1710 * m0_f1710 - mKS0 * mKS0,
                                      2) / 4.0 - m0_f1710 * m0_f1710 * mKS0 * mKS0) / mB0;

    // get q0 and pstar0 for f2010
    q0_f2010 = std::sqrt((m0_f2010 * m0_f2010) / 4.0 - mKL0 * mKL0);
    pstar0_f2010 = std::sqrt(std::pow(mB0 * mB0 - m0_f2010 * m0_f2010 - mKS0 * mKS0,
                                      2) / 4.0 - m0_f2010 * m0_f2010 * mKS0 * mKS0) / mB0;

    // get q0 and pstar0 for chic0
    q0_chic0 = std::sqrt((m0_chic0 * m0_chic0) / 4.0 - mKL0 * mKL0);
    pstar0_chic0 = std::sqrt(std::pow(mB0 * mB0 - m0_chic0 * m0_chic0 - mKS0 * mKS0,
                                      2) / 4.0 - m0_chic0 * m0_chic0 * mKS0 * mKS0) / mB0;

  }

  void EvtB0toK0K0K0::GetMasses()
  {
    mB0 = EvtPDL::getMass(EvtPDL::getId("B0"));
    mKp = EvtPDL::getMass(EvtPDL::getId("K+"));
    mKL0 = EvtPDL::getMass(EvtPDL::getId("K_L0"));
    mKS0 = EvtPDL::getMass(EvtPDL::getId("K_S0"));
    mK0 = EvtPDL::getMass(EvtPDL::getId("K0"));
    mpic = EvtPDL::getMass(EvtPDL::getId("pi+"));
    mpi0 = EvtPDL::getMass(EvtPDL::getId("pi0"));
  }

}

