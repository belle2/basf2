/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef BABAYAGANLO_H
#define BABAYAGANLO_H

#include <mdst/dataobjects/MCParticleGraph.h>
#include <TLorentzRotation.h>
#include <utility>

namespace Belle2 {

  /**
   * C++ Interface for the Fortran Bhabha and exclusive two photon generator BABAYAGA.NLO.
   *
   */

  class BabayagaNLO {
  public:

    /** Constructor.
     * Sets the default settings.
     */
    BabayagaNLO();

    /** Destrucotr. */
    ~BabayagaNLO();

    /** Sets the default settings for the BhWide Fortran generator. */
    void setDefaultSettings();

    /** Sets the theta scattering angle range for the scattered particles.
      * @param angleRange A pair of values, representing the min and max theta angle of the scattered particles in [deg].
      */
    void setScatAngle(std::pair<double, double> angleRange) { m_ScatteringAngleRange = angleRange; }

    /** Sets the number of events used to search maximum
     * @param nSearchMax Number of events for maximum search
     */
    void setNSearchMax(int nSearchMax) { m_nSearchMax = nSearchMax; }

    /** Maximum differential cross section
     * @param fMax Maximum differential cross section
     */
    void setFMax(double fMax) { m_fMax = fMax; }

    /** Sets maximum acollinearity angle between finale state leptons/photons in degrees.
     * @param maxAcollinearity Maximum acollinearity angle between finale state leptons/photons in degrees
     */
    void setMaxAcollinearity(double maxAcollinearity) { m_maxAcollinearity = maxAcollinearity; }

    /** Sets nominal ECM
    * @param cmsEnergyNominal nominal ecm in GeV
    */
    void setCmsEnergyNominal(double cmsEnergyNominal) { m_cmsEnergyNominal = cmsEnergyNominal; }

    /** Sets minimum energy for leptons/photons in the final state, in GeV.
     * @param minEnergy Minimum energy for leptons/photons in the final state, in GeV
     */
    void setMinEnergy(double minEnergy) { m_eMin = minEnergy; }

    /** Sets soft/hard photon energy separator.
    * @param epsilon soft/hard photon energy separator, in units of ECMS/2
    */
    void setEpsilon(double epsilon) { m_epsilon = epsilon; }

    /** Sets vacuum polarization
     * @param vacPol Vacuum polarization option
     */
    void setVacPol(const std::string& vacPol) { m_vacPol = vacPol; }

    /** Sets model: matched or ps
     * @param model matched or ps
     */
    void setModel(const std::string& model) { m_model = model; }

    /** Sets mode: weighted or unweighted
     * @param mode weighted or unweighted
     */
    void setMode(const std::string& mode) { m_mode = mode; }

    /** Sets Order: born, alpha or exp
     * @param order born, alpha or exp
     */
    void setOrder(const std::string& order) { m_order = order; }

    /** Sets final state minimum energy.
     * @param finalState Minimum energy for leptons/photons in the final state, in GeV
     */
    void setFinalState(const std::string& finalState) { m_finalState = finalState; }

    /** TEMPORARY SOLUTION! Approximate energy spread per beam (CMS)
     * @param spread energy spread per beam (CMS)
     */
    void setEnergySpread(double spread) { m_EnergySpread = spread; }

    /** Calculate VP uncertainty by internal reweighting (on/off)
     * @param vpuncertainty boolean on/off
     */
    void setVPUncertainty(bool vpuncertainty) { m_VPUncertainty = vpuncertainty; }

    /** Sets the fixed number of nphot (hard) photons
     * @param nPhot number of nphot (hard) photons
     */
    void setNPhotons(int nPhot) { m_nPhot = nPhot; }

    /** Sets User mode similar to TEEGG: ETRON, EGAMMA, GAMMA or PRESCALE or NONE (default)
     * @param usermode ETRON, EGAMMA, GAMMA or NONE (default)
     */
    void setUserMode(const std::string& usermode) { m_userMode = usermode; }

    /** Sets NSK VP data file
     * @param NSKDataFile (default)
     */
    void setNSKDataFile(const std::string& NSKDataFile) { m_NSKDataFile = NSKDataFile; }

    /** Sets the minimum CMS energy of the tagged e-/e+
     * @param eemin minimum CMS energy of the tagged e-/e+
     */
    void setEEMIN(double eemin) { m_eemin = eemin; }

    /** Sets the minimum CMS angle between the tagged e-/e+ and -z axis
     * @param temin minimum CMS energy of the tagged e-/e+
     */
    void setTEMIN(double temin) { m_temin = temin; }

    /** Sets the minimum CMS energy of the gamma
     * @param egmin minimum CMS energy of the gamma
     */
    void setEGMIN(double egmin) { m_egmin = egmin; }

    /** Sets the minimum CMS angle between the gamma and -z axis
     * @param tgmin minimum CMS angle between the gamma and -z axis
     */
    void setTGMIN(double tgmin) { m_tgmin = tgmin; }

    /** Sets the minimum CMS energy to veto e-/e+
     * @param eeveto minimum CMS energy to veto e-/e+
     */
    void setEEVETO(double eeveto) { m_eeveto = eeveto; }

    /** Sets the maximum CMS theta of e-/e+ in final state
     * @param teveto maximum CMS theta of e-/e+ in final state
     */
    void setTEVETO(double teveto) { m_teveto = teveto; }

    /** Sets the minimum CMS energy to veto gamma
     * @param egveto minimum CMS energy to veto gamma
     */
    void setEGVETO(double egveto) { m_egveto = egveto; }

    /** Sets the maximum CMS angle between the gamma and -z axis
     * @param tgveto maximum CMS angle between the gamma and -z axis
     */
    void setTGVETO(double tgveto) { m_tgveto = tgveto; }

    /** Sets the maximum prescale value
     * @param maxprescale maximum prescale value (at maxmin theta)
     */
    void setMaxPrescale(double maxprescale) { m_maxprescale = maxprescale; }

    /**
     * Initializes the generator.
     */
    void init();

    /**
     * Initializes the extra info.
     */
    void initExtraInfo();

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     */
    void generateEvent(MCParticleGraph& mcGraph, double ecm, TVector3 vertex, TLorentzRotation boost);

    /** Returns weight
     * @param SDif Returns weight
     */
    double getSDif() {return m_sDif;};

    /**
     * Terminates the generator.
     * Closes the internal Fortran generator.
     */
    void term();

  protected:
    bool m_applyBoost;              /**< Apply a boost to the MCParticles. */
    double m_pi;                    /**< pi=3.1415.... */
    double m_conversionFactor;      /**< Conversion factor for hbarc to nb. */
    double m_alphaQED0;             /**< QED coupling constant at Q=0. */
    double m_massElectron;          /**< muon mass. */
    double m_massMuon;              /**< electron mass. */
    double m_massW;                 /**< W mass  [GeV] for on shell sin2theta and GF. */
    double m_massZ;                 /**< Z mass  [GeV]. */
    double m_widthZ;                /**< Z width [GeV] (may be recalculated by EW library). */

    std::string m_vacPol;           /**< vacuum polarization: off, hadr5 (Jegerlehner) or hmnt (Teubner). */
    std::string m_finalState;       /**< final state: ee, gg or mm. */
    std::string m_model;            /**< model: matched or ps. */
    std::string m_mode;             /**< mode: weighted or unweighted. */
    std::string m_order;            /**< order: born, alpha or exp. */
    double m_EnergySpread;          /**< TEMPORARY SOLUTION! Approximate energy spread per beam (CMS). */
    double m_cmsEnergyNominal;      /**< Nominal CMS Energy = 2*Ebeam [GeV]. */
    std::pair<double, double> m_ScatteringAngleRange; /**< Min and Max val. for the scattering angle [deg] of the electron/positron. */
    std::pair<double, double> m_ScatteringAngleRangePhoton; /**< Min and Max val. for the scattering angle [deg] of the g in gg mode. */
    double m_epsilon; /**< Soft/hard photon separator in units of CMS/2. */
    double m_eMin; /**<  [GeV]. */
    double m_maxAcollinearity; /**< Maximum acollinearity of the electron-positron pair. */
    int m_nSearchMax;  /**< Events used to search maximum of differential cross section. */
    int m_nPhot; /**< fixed number of nphot (hard) photons are generated. A negative value means all possible photons. */
    bool m_VPUncertainty;   /**< vary all VP related parameters and extracted total uncertainty. */
    std::string m_NSKDataFile;   /**< data file for NSK VP. */

    std::string m_userMode; /**< User mode similar to TEEGG: ETRON, EGAMMA, GAMMA or PRESCALE */
    double m_eemin; /**< Minimum CMS energy of the tagged e-/e+ (GeV). */
    double m_temin; /**< Minimum CMS angle between the tagged e-/e+ and -z axis (deg). */
    double m_egmin; /**< Minimum CMS energy of the gamma (GeV). */
    double m_tgmin; /**< Minimum CMS angle between the gamma and -z axis (deg). */
    double m_eeveto; /**< Minimum CMS energy to veto e-/e+ (GeV). */
    double m_teveto; /**< Maximum CMS theta of e-/e+ in final state (deg). */
    double m_egveto; /**< Minimum CMS energy to veto gamma (GeV). */
    double m_tgveto; /**< Maximum CMS angle between the gamma and -z axis (deg). */
    double m_maxprescale; /**< Maximum prescale value. */

    double m_fMax;  /**< Maximum of differential cross section. */
    double m_sDif;  /**< Differential xsec/weight used for event. */

    TLorentzRotation m_boostVector; /**< The Lorentz boost vector for the transformation CMS to LAB frame. */

    /** Apply the settings to the internal Fortran generator. */
    void applySettings();

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                       bool isVirtual = false, bool isInitial = false, bool isISRFSR = false);

  private:

    int    m_npar[100];  /**< Integer parameters for BabayagaNLO. */
    double m_xpar[100];  /**< Double parameters for BabayagaNLO. */
  };
}


#endif /* BABAYAGANLO_H */
