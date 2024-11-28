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

#include "generators/evtgen/models/EvtBtoKK0K0.h"

using std::endl;
using namespace std::complex_literals;

namespace Belle2 {

  /** register the model in EvtGen */
  B2_EVTGEN_REGISTER_MODEL(EvtBtoKK0K0);

  EvtBtoKK0K0::~EvtBtoKK0K0() {}

  std::string EvtBtoKK0K0::getName()
  {
    return "BTOKK0K0";
  }

  EvtDecayBase* EvtBtoKK0K0::clone()
  {
    return new EvtBtoKK0K0;
  }

  void EvtBtoKK0K0::decay(EvtParticle* p)
  {
    // follow PhysRevD.85.112010

    int Ntry = 0;
    const int max_Ntry = 10000;

    while (true) {
      p->initializePhaseSpace(getNDaug(), getDaugs());

      EvtParticle* ChargedKaon = p->getDaug(0);
      EvtParticle* NeutralKaon_1 = p->getDaug(1);
      EvtParticle* NeutralKaon_2 = p->getDaug(2);

      EvtVector4R mom_ChargedKaon = ChargedKaon->getP4();
      EvtVector4R mom_NeutralKaon_1 = NeutralKaon_1->getP4();
      EvtVector4R mom_NeutralKaon_2 = NeutralKaon_2->getP4();

      EvtVector4R mom_sum_1 = mom_ChargedKaon + mom_NeutralKaon_1;
      EvtVector4R mom_sum_2 = mom_ChargedKaon + mom_NeutralKaon_2;

      double s_1 = mom_sum_1.mass2();
      double s_2 = mom_sum_2.mass2();

      // follow the index of PhysRevD.85.112010
      double s13 = -1;
      double s23 = -1;
      if (s_1 > s_2) {
        s23 = s_1;
        s13 = s_2;
      } else {
        s23 = s_2;
        s13 = s_1;
      }

      // the maximum value of probability is obtained by brute fource method. scan all region and find.
      const double Probability_max = 871.390583;
      double Probability_value = Probability(s13, s23);
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


  void EvtBtoKK0K0::initProbMax()
  {
    noProbMax();
  }


  void EvtBtoKK0K0::init()
  {

    // check that there are no arguments
    checkNArg(0);

    // there should be three daughters: K+ KL0 KL0
    checkNDaug(3);

    // Check that the daughters are correct
    EvtId KaonPlusType = getDaug(0);
    EvtId KaonZeroType_1 = getDaug(1);
    EvtId KaonZeroType_2 = getDaug(2);

    int KaonPlustyp = 0;
    int KaonZerotyp = 0;

    if ((KaonPlusType == EvtPDL::getId("K+")) || (KaonPlusType == EvtPDL::getId("K-"))) KaonPlustyp++;
    if ((KaonZeroType_1 == EvtPDL::getId("K0")) ||
        (KaonZeroType_1 == EvtPDL::getId("anti-K0")) ||
        (KaonZeroType_1 == EvtPDL::getId("K_S0")) ||
        (KaonZeroType_1 == EvtPDL::getId("K_L0"))) KaonZerotyp++;
    if ((KaonZeroType_2 == EvtPDL::getId("K0")) ||
        (KaonZeroType_2 == EvtPDL::getId("anti-K0")) ||
        (KaonZeroType_2 == EvtPDL::getId("K_S0")) ||
        (KaonZeroType_2 == EvtPDL::getId("K_L0"))) KaonZerotyp++;

    if ((KaonPlustyp != 1) || (KaonZerotyp != 2)) {

      std::cout << "The first dauther should be charged Kaon. The second and third daughters should be K0, anti-K0, K_L0, or K_S0.\n";
      ::abort();
    }

    // initialize q0 and pstar0
    GetMasses();
    GetZeros();

  }

  double EvtBtoKK0K0::Probability(double s13, double s23)
  {
    std::complex<double> total_amplitude = Amplitude(s13, s23, "f980", false) + Amplitude(s13, s23, "f1500", false) + Amplitude(s13,
                                           s23, "f1525", false) + Amplitude(s13, s23, "f1710", false) + Amplitude(s13, s23, "chic0", false) + Amplitude(s13, s23, "NR", false);
    std::complex<double> total_amplitude_isobar = Amplitude(s13, s23, "f980", true) + Amplitude(s13, s23, "f1500",
                                                  true) + Amplitude(s13, s23, "f1525", true) + Amplitude(s13, s23, "f1710", true) + Amplitude(s13, s23, "chic0",
                                                      true) + Amplitude(s13, s23, "NR", true);
    return std::abs(total_amplitude) * std::abs(total_amplitude) + std::abs(total_amplitude_isobar) * std::abs(total_amplitude_isobar);
  }


  std::complex<double> EvtBtoKK0K0::Amplitude(double s13, double s23, const char* resonance, bool isobar = false)
  {

    if (strcmp(resonance, "f980") == 0) {
      std::complex<double> a;
      if (isobar == false) a = c_f980 * std::exp(1i * DegreeToRadian(phi_f980));
      else a = c_f980 * std::exp(1i * DegreeToRadian(phi_f980));
      return a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s23, s13, resonance));
    } else if (strcmp(resonance, "f1500") == 0) {
      std::complex<double> a;
      if (isobar == false) a = c_f1500 * std::exp(1i * DegreeToRadian(phi_f1500));
      else a = c_f1500 * std::exp(1i * DegreeToRadian(phi_f1500));
      return a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s23, s13, resonance));
    } else if (strcmp(resonance, "f1525") == 0) {
      std::complex<double> a;
      if (isobar == false) a = c_f1525 * std::exp(1i * DegreeToRadian(phi_f1525));
      else a = c_f1525 * std::exp(1i * DegreeToRadian(phi_f1525));
      return a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s23, s13, resonance));
    } else if (strcmp(resonance, "f1710") == 0) {
      std::complex<double> a;
      if (isobar == false) a = c_f1710 * std::exp(1i * DegreeToRadian(phi_f1710));
      else a = c_f1710 * std::exp(1i * DegreeToRadian(phi_f1710));
      return a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s23, s13, resonance));
    } else if (strcmp(resonance, "chic0") == 0) {
      std::complex<double> a;
      if (isobar == false) a = c_chic0 * std::exp(1i * (DegreeToRadian(phi_chic0) + DegreeToRadian(delta_chic0)));
      else a = c_chic0 * std::exp(1i * (DegreeToRadian(phi_chic0) - DegreeToRadian(delta_chic0)));
      return a * (DynamicalAmplitude(s13, s23, resonance) + DynamicalAmplitude(s23, s13, resonance));
    } else if (strcmp(resonance, "NR") == 0) {

      double Omega = 0.5 * (mB + (1.0 / 3.0) * (mKp + mKL0 + mKL0));
      double m12 = std::sqrt(mB * mB + mKp * mKp + mKL0 * mKL0 + mKL0 * mKL0 - s13 - s23); // sqrt(s12)
      double x = m12 - Omega;

      std::complex<double> aS0;
      std::complex<double> aS1;
      std::complex<double> aS2;

      if (isobar == false) {
        aS0 = aS0_c_NR * (1 + b_NR) * std::exp(1i * DegreeToRadian(aS0_phi_NR));
        aS1 = aS1_c_NR * (1 + b_NR) * std::exp(1i * DegreeToRadian(aS1_phi_NR));
        aS2 = aS2_c_NR * (1 + b_NR) * std::exp(1i * DegreeToRadian(aS2_phi_NR));
      } else {
        aS0 = aS0_c_NR * (1 - b_NR) * std::exp(1i * DegreeToRadian(aS0_phi_NR));
        aS1 = aS1_c_NR * (1 - b_NR) * std::exp(1i * DegreeToRadian(aS1_phi_NR));
        aS2 = aS2_c_NR * (1 - b_NR) * std::exp(1i * DegreeToRadian(aS2_phi_NR));
      }

      return 2.0 * (aS0 + aS1 * x + aS2 * x * x);
    } else {
      printf("[Amplitude] unsupported resonance\n");
      ::abort();
    }

  }

  std::complex<double> EvtBtoKK0K0::DynamicalAmplitude(double s13, double s23, const char* resonance)
  {

    if (strcmp(resonance, "f980") == 0) {
      return Flatte(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else if (strcmp(resonance, "f1500") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else if (strcmp(resonance, "f1525") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else if (strcmp(resonance, "f1710") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else if (strcmp(resonance, "chic0") == 0) {
      return RBW(s13, s23, resonance) * BlattWeisskopf_pstarrprime(s13, s23, resonance) * BlattWeisskopf_qr(s13, s23,
             resonance) * Zemach(s13, s23, resonance);
    } else {
      printf("[Amplitude] unsupported resonance\n");
      ::abort();
    }

  }

  std::complex<double> EvtBtoKK0K0::Flatte(double s13, double s23, const char* resonance)
  {
    double m0;
    double gpi;
    double gK;

    if (strcmp(resonance, "f980") == 0) {
      m0 = m0_f980;
      gpi = gpi_f980;
      gK = gKgpi_f980 * gpi_f980;
    } else {
      printf("[Flatte] unsupported resonance\n");
      ::abort();
    }

    double m = Calculate_m(s13, s23);

    double rho_pipi = std::sqrt(1 - 4 * mpic * mpic / (m * m));
    double rho_KK = std::sqrt(1 - 4 * mK * mK / (m * m));

    return 1.0 / ((m0 * m0 - m * m) - 1i * (gpi * rho_pipi + gK * rho_KK));
  }

  std::complex<double> EvtBtoKK0K0::RBW(double s13, double s23, const char* resonance)
  {
    double m0;

    double m = Calculate_m(s13, s23);

    if (strcmp(resonance, "f1500") == 0) {
      m0 = m0_f1500;
    } else if (strcmp(resonance, "f1525") == 0) {
      m0 = m0_f1525;
    } else if (strcmp(resonance, "f1710") == 0) {
      m0 = m0_f1710;
    } else if (strcmp(resonance, "chic0") == 0) {
      m0 = m0_chic0;
    } else {
      printf("[RBW] unsupported resonance\n");
      ::abort();
    }

    return 1.0 / (m0 * m0 - m * m - 1i * m0 * MassDepWidth(s13, s23, resonance));
  }

  double EvtBtoKK0K0::MassDepWidth(double s13, double s23, const char* resonance)
  {
    // Gamma(m) when s12 and s23
    int spin = -1;
    double m0;
    double q0;
    double Gamma0;

    double q_mag = Calculate_q_mag(s13, s23);
    double m = Calculate_m(s13, s23);

    if (strcmp(resonance, "f1500") == 0) {
      spin = spin_f1500;
      m0 = m0_f1500;
      q0 = q0_f1500;
      Gamma0 = Gamma0_f1500;
    } else if (strcmp(resonance, "f1525") == 0) {
      spin = spin_f1525;
      m0 = m0_f1525;
      q0 = q0_f1525;
      Gamma0 = Gamma0_f1525;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
      m0 = m0_f1710;
      q0 = q0_f1710;
      Gamma0 = Gamma0_f1710;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
      m0 = m0_chic0;
      q0 = q0_chic0;
      Gamma0 = Gamma0_chic0;
    } else {
      printf("[MassDepWidth] unsupported resonance\n");
      ::abort();
    }

    return Gamma0 * std::pow(q_mag / q0, 2 * spin + 1) * (m0 / m) * std::pow(BlattWeisskopf_qr(s13, s23, resonance), 2);
  }

  double EvtBtoKK0K0::BlattWeisskopf_pstarrprime(double s13, double s23, const char* resonance)
  {
    // X_L(|p*|r') when s12 and s23
    int spin = -1;
    double z0 = -1;
    double z = Calculate_pstar_mag(s13, s23) * rprime;

    if (strcmp(resonance, "f980") == 0) {
      spin = spin_f980;
      z0 = -1;
    } else if (strcmp(resonance, "f1500") == 0) {
      spin = spin_f1500;
      z0 = pstar0_f1500 * rprime;
    } else if (strcmp(resonance, "f1525") == 0) {
      spin = spin_f1525;
      z0 = pstar0_f1525 * rprime;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
      z0 = pstar0_f1710 * rprime;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
      z0 = pstar0_chic0 * rprime;
    } else {
      printf("[BlattWeisskopf_pstarrprime] unsupported resonance\n");
      ::abort();
    }

    if (spin == 0) return 1;
    else if (spin == 1) return std::sqrt((1 + z0 * z0) / (1 + z * z));
    else if (spin == 2) return std::sqrt((9 + 3 * z0 * z0 + z0 * z0 * z0 * z0) / (9 + 3 * z * z + z * z * z * z));
    else {
      printf("[BlattWeisskopf_pstarrprime] unsupported spin\n");
      ::abort();
    }

  }

  double EvtBtoKK0K0::BlattWeisskopf_qr(double s13, double s23, const char* resonance)
  {
    // X_L(|q|r) when s12 and s23
    int spin = -1;
    double z0 = -1;
    double z = Calculate_q_mag(s13, s23) * r;

    if (strcmp(resonance, "f980") == 0) {
      spin = spin_f980;
      z0 = -1;
    } else if (strcmp(resonance, "f1500") == 0) {
      spin = spin_f1500;
      z0 = q0_f1500 * r;
    } else if (strcmp(resonance, "f1525") == 0) {
      spin = spin_f1525;
      z0 = q0_f1525 * r;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
      z0 = q0_f1710 * r;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
      z0 = q0_chic0 * r;
    } else {
      printf("[BlattWeisskopf_qr] unsupported resonance\n");
      ::abort();
    }

    if (spin == 0) return 1;
    else if (spin == 1) return std::sqrt((1 + z0 * z0) / (1 + z * z));
    else if (spin == 2) return std::sqrt((9 + 3 * z0 * z0 + z0 * z0 * z0 * z0) / (9 + 3 * z * z + z * z * z * z));
    else {
      printf("[BlattWeisskopf_qr] unsupported spin\n");
      ::abort();
    }

  }

  double EvtBtoKK0K0::Zemach(double s13, double s23, const char* resonance)
  {

    int spin;
    double p_dot_q = Calculate_q_dot_p_mag(s13, s23);
    double p_mag = Calculate_p_mag(s13, s23);
    double q_mag = Calculate_q_mag(s13, s23);

    if (strcmp(resonance, "f980") == 0) {
      spin = spin_f980;
    } else if (strcmp(resonance, "f1500") == 0) {
      spin = spin_f1500;
    } else if (strcmp(resonance, "f1525") == 0) {
      spin = spin_f1525;
    } else if (strcmp(resonance, "f1710") == 0) {
      spin = spin_f1710;
    } else if (strcmp(resonance, "chic0") == 0) {
      spin = spin_chic0;
    } else {
      printf("[Zemach] unsupported resonance\n");
      ::abort();
    }

    if (spin == 0) return 1.0;
    else if (spin == 1) return 4 * p_dot_q;
    else if (spin == 2) return (16.0 / 3.0) * (3 * p_dot_q * p_dot_q - p_mag * p_mag * q_mag * q_mag);
    else {
      printf("[Zemach] unsupported spin\n");
      ::abort();
    }

  }

  double EvtBtoKK0K0::Calculate_m(double s13, double s23)
  {
    return std::sqrt(mB * mB + mKp * mKp + mKL0 * mKL0 + mKL0 * mKL0 - s13 - s23);
  }

  double EvtBtoKK0K0::Calculate_q_mag(double s13, double s23)
  {
    double m = Calculate_m(s13, s23);
    return std::sqrt(m * m / 4.0 - mKL0 * mKL0);
  }

  double EvtBtoKK0K0::Calculate_pstar_mag(double s13, double s23)
  {
    double m = Calculate_m(s13, s23);
    return std::sqrt(std::pow(mB * mB - m * m - mKp * mKp, 2) / 4.0 - m * m * mKp * mKp) / mB;
  }

  double EvtBtoKK0K0::Calculate_p_mag(double s13, double s23)
  {
    double m = Calculate_m(s13, s23);
    double s12 = m * m;
    return std::sqrt(std::pow(mB * mB - s12 - mKp * mKp, 2) / (4 * s12) - mKp * mKp);
  }

  double EvtBtoKK0K0::Calculate_q_dot_p_mag(double s13, double s23)
  {
    return std::abs((s13 - s23) / 4.0);
  }

  double EvtBtoKK0K0::DegreeToRadian(double degree)
  {
    return (3.141592 * degree) / 180.0;
  }

  void EvtBtoKK0K0::GetZeros()
  {

    // get q0 and pstar0 for f1500
    q0_f1500 = std::sqrt((m0_f1500 * m0_f1500) / 4.0 - mKL0 * mKL0);
    pstar0_f1500 = std::sqrt(std::pow(mB * mB - m0_f1500 * m0_f1500 - mKp * mKp, 2) / 4.0 - m0_f1500 * m0_f1500 * mKp * mKp) / mB;

    // get q0 and pstar0 for f1525
    q0_f1525 = std::sqrt((m0_f1525 * m0_f1525) / 4.0 - mKL0 * mKL0);
    pstar0_f1525 = std::sqrt(std::pow(mB * mB - m0_f1525 * m0_f1525 - mKp * mKp, 2) / 4.0 - m0_f1525 * m0_f1525 * mKp * mKp) / mB;

    // get q0 and pstar0 for f1710
    q0_f1710 = std::sqrt((m0_f1710 * m0_f1710) / 4.0 - mKL0 * mKL0);
    pstar0_f1710 = std::sqrt(std::pow(mB * mB - m0_f1710 * m0_f1710 - mKp * mKp, 2) / 4.0 - m0_f1710 * m0_f1710 * mKp * mKp) / mB;

    // get q0 and pstar0 for chic0
    q0_chic0 = std::sqrt((m0_chic0 * m0_chic0) / 4.0 - mKL0 * mKL0);
    pstar0_chic0 = std::sqrt(std::pow(mB * mB - m0_chic0 * m0_chic0 - mKp * mKp, 2) / 4.0 - m0_chic0 * m0_chic0 * mKp * mKp) / mB;

  }

  void EvtBtoKK0K0::GetMasses()
  {
    mB = EvtPDL::getMass(EvtPDL::getId("B+"));
    mKp = EvtPDL::getMass(EvtPDL::getId("K+"));
    mKL0 = EvtPDL::getMass(EvtPDL::getId("K_L0"));
    mK = (EvtPDL::getMass(EvtPDL::getId("K+")) + EvtPDL::getMass(EvtPDL::getId("K0"))) / 2.0;
    mpic = EvtPDL::getMass(EvtPDL::getId("pi+"));
  }

}

