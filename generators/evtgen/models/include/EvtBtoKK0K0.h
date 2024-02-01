/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include "EvtGenBase/EvtDecayIncoherent.hh"
#include "EvtGenBase/EvtParticle.hh"
#include <string>
#include <complex>

namespace Belle2 {

  /** The evtgen model to produce B+ -> K+ K0 K0 decay sample.
    *
    * In this model, Dalitz plot is calculated, based on [PhysRevD.85.112010]
    *
    * You can use the model as follows:
    *
    * Decay MyB+
    * 1.0 K+ K_L0 K_L0           BTOKK0K0;
    * Enddecay
    */
  class EvtBtoKK0K0 : public  EvtDecayIncoherent {


  public:

    /**
     * Constructor.
     */
    EvtBtoKK0K0() {}

    /**
     * Destructor.
     */
    virtual ~EvtBtoKK0K0();

    /**
     * The function which returns the name of the model.
     */
    std::string getName();

    /**
     * The function which makes a copy of the model.
     */
    EvtDecayBase* clone();

    /**
     * The function to determine kinematics of daughter particles based on PDF.
     */
    void decay(EvtParticle* p);

    /**
     * The function to sets a maximum probability.
     * In this model, noProbMax() is called because maximum probability is hardcoded
     */
    void initProbMax();

    /**
     * The function for an initialization.
     */
    void init();

    /**
     * The function to get probability of decay
     */
    double Probability(double s13, double s23);

    /**
     * The function to get amplitude of decay
     */
    std::complex<double> Amplitude(double s13, double s23, const char* resonance, bool isobar);

    /**
     * The function to get dynamic amplitude of decay
     */
    std::complex<double> DynamicalAmplitude(double s13, double s23, const char* resonance);

    /**
     * Flatte form line shape
     */
    std::complex<double> Flatte(double s13, double s23, const char* resonance);

    /**
     * RBW line shape
     */
    std::complex<double> RBW(double s13, double s23, const char* resonance);

    /**
     * The function to get the mass dependent width
     */
    double MassDepWidth(double s13, double s23, const char* resonance);

    /**
     * Blatt-Weisskopf angular momentum barrier factor with pstar and r'
     */
    double BlattWeisskopf_pstarrprime(double s13, double s23, const char* resonance);

    /**
     * Blatt-Weisskopf angular momentum barrier factor with q and r
     */
    double BlattWeisskopf_qr(double s13, double s23, const char* resonance);

    /**
     * Zemach tensor
     */
    double Zemach(double s13, double s23, const char* resonance);

    /**
     * The function to calculate m from s13 and s23
     */
    double Calculate_m(double s13, double s23);

    /**
     * The function to calculate magnitude of q from s13 and s23
     */
    double Calculate_q_mag(double s13, double s23);

    /**
     * The function to calculate magnitude of pstar from s13 and s23
     */
    double Calculate_pstar_mag(double s13, double s23);

    /**
     * The function to calculate magnitude of p from s13 and s23
     */
    double Calculate_p_mag(double s13, double s23);

    /**
     * The function to calculate p dot q from s13 and s23
     */
    double Calculate_q_dot_p_mag(double s13, double s23);

    /**
     * The function to convert degree to radian
     */
    double DegreeToRadian(double degree);

    /**
     * The function to get q0 and pstar0
     */
    void GetZeros();

    /**
     * The function to get mass for particles from evt.pdl
     */
    void GetMasses();

  private:
    /** spin of f980   */
    int spin_f980{ 0 };

    /** nominal mass of f980   */
    double m0_f980{ 0.965 };

    /** coupling constant g_pi of f980   */
    double gpi_f980{ 0.165 };

    /** ratio of coupling constant g_k/g_pi of f980   */
    double gKgpi_f980{ 4.21 };

    /** isobar parameter c of f980   */
    double c_f980{ 3.35 };

    /** isobar parameter phi of f980   */
    double phi_f980{ 31.0 };

    /** spin of f1500   */
    int spin_f1500{ 0 };

    /** nominal mass of f1500   */
    double m0_f1500{ 1.505 };

    /** nominal width of f1500   */
    double Gamma0_f1500{ 0.109 };

    /** q value when m = m0 for f1500. q is the momenta of one resonance daughter. It is initialized by `GetZeros()` function   */
    double q0_f1500{ 0 };

    /** pstar value when m = m0 for f1500. pstar is momentum of the ‘‘bachelor’’ particle evaluated in the rest frame of B. It is initialized by `GetZeros()` function   */
    double pstar0_f1500{ 0 };

    /** isobar parameter c of f1500   */
    double c_f1500{ 0.2 };

    /** isobar parameter phi of f1500   */
    double phi_f1500{ -83.0 };

    /** spin of f1525   */
    int spin_f1525{ 2 };

    /** nominal mass of f1525   */
    double m0_f1525{ 1.525 };

    /** nominal width of f1525   */
    double Gamma0_f1525{ 0.073 };

    /** q value when m = m0 for f1525. q is the momenta of one resonance daughter. It is initialized by `GetZeros()` function   */
    double q0_f1525{ 0 };

    /** pstar value when m = m0 for f1525. pstar is momentum of the ‘‘bachelor’’ particle evaluated in the rest frame of B. It is initialized by `GetZeros()` function   */
    double pstar0_f1525{ 0 };

    /** isobar parameter c of f1525   */
    double c_f1525{ 0.00179 };

    /** isobar parameter phi of f1525   */
    double phi_f1525{-58.0};

    /** spin of f1710   */
    int spin_f1710{ 0 };

    /** nominal mass of f1710   */
    double m0_f1710{ 1.720 };

    /** nominal width of f1710   */
    double Gamma0_f1710{ 0.135 };

    /** q value when m = m0 for f1710. q is the momenta of one resonance daughter. It is initialized by `GetZeros()` function   */
    double q0_f1710{ 0 };

    /** pstar value when m = m0 for f1710. pstar is momentum of the ‘‘bachelor’’ particle evaluated in the rest frame of B. It is initialized by `GetZeros()` function   */
    double pstar0_f1710{ 0 };

    /** isobar parameter c of f1710   */
    double c_f1710{ 0.24 };

    /** isobar parameter phi of f1710   */
    double phi_f1710{ -22.0 };

    /** spin of chic0   */
    int spin_chic0{ 0 };

    /** nominal mass of chic0   */
    double m0_chic0{ 3.41475 };

    /** nominal width of chic0   */
    double Gamma0_chic0{ 0.0103 };

    /** q value when m = m0 for chic0. q is the momenta of one resonance daughter. It is initialized by `GetZeros()` function   */
    double q0_chic0{ 0 };

    /** pstar value when m = m0 for chic0. pstar is momentum of the ‘‘bachelor’’ particle evaluated in the rest frame of B. It is initialized by `GetZeros()` function   */
    double pstar0_chic0{ 0 };

    /** isobar parameter c of chic0   */
    double c_chic0{ 0.113 };

    /** isobar parameter phi of chic0   */
    double phi_chic0{ 45.0 };

    /** isobar parameter delta of chic0   */
    double delta_chic0{ -12.0 };

    /** NR coefficient b   */
    double b_NR{ -0.018 };

    /** c of NR coefficient a_S0   */
    double aS0_c_NR{ 1.0 };

    /** phi of NR coefficient a_S0   */
    double aS0_phi_NR{ 0.0 };

    /** c of NR coefficient a_S1   */
    double aS1_c_NR{ 1.0 };

    /** phi of NR coefficient a_S1   */
    double aS1_phi_NR{ 129.0 };

    /** c of NR coefficient a_S2   */
    double aS2_c_NR{ 0.51 };

    /** phi of NR coefficient a_S2   */
    double aS2_phi_NR{ -85.0 };

    /** effective meson radii, r   */
    double r{ 4.0 };

    /** effective meson radii, r'   */
    double rprime{ 0.0 };

    /** mass of B meson. Initialized by evt.pdl   */
    double mB{ 0.0 };

    /** mass of charged Kaon. Initialized by evt.pdl   */
    double mKp{ 0.0 };

    /** mass of K_L0. Initialized by evt.pdl   */
    double mKL0{ 0.0 };

    /** average of the mass between charged Kaon and neutral Kaon. Initialized by evt.pdl   */
    double mK{ 0.0 };

    /** mass of charged pion. Initialized by evt.pdl   */
    double mpic{ 0.0 };
  };

} // Belle 2 Namespace
