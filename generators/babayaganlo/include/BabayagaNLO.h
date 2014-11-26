/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

    /** Sets the Lorentz boost vector which should be applied to the generated particles.
     * @param boostVector The Lorentz boost vector which is applied to the generated particles.
     */
    void setBoost(TLorentzRotation boostVector) { m_boostVector = boostVector; }

    /** Enables the boost of the generated particles.
     * @param applyBoost Set to true to enabled the boost. Also make sure you have set the boost vector using setBoost().
     */
    void enableBoost(bool applyBoost = true) { m_applyBoost = applyBoost; }

    /** Sets the theta scattering angle range for the scattered particles.
      * @param angleRange A pair of values, representing the min and max theta angle of the scattered particles in [deg].
      */
    void setScatAngle(std::pair<double, double> angleRange) { m_ScatteringAngleRange = angleRange; }

    /** Sets the CMS energy.
     * @param cmsEnergy The CMS energy in [GeV].
     */
    void setCMSEnergy(double cmsEnergy) { m_cmsEnergy = cmsEnergy; }

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

    /** Sets minimum energy for leptons/photons in the final state, in GeV.
     * @param minEnergy Minimum energy for leptons/photons in the final state, in GeV
     */
    void setMinEnergy(double minEnergy) { m_eMin = minEnergy; }

    /** Sets soft/hard photon energy separator.
    * @param epsilon soft/hard photon energy separator, in units of ECMS/2
    */
    void setEpsilon(double epsilon) { m_epsilon = epsilon; }

    /** Sets final state,.
     * @param finalState Minimum energy for leptons/photons in the final state, in GeV
     */
    void setFinalState(std::string finalState) { m_finalState = finalState; }

    void init();

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     */
    void generateEvent(MCParticleGraph& mcGraph);

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
    std::string m_finalState;       /**< final state: ee, gg or mm. */
    std::string m_model;            /**< model: matched or ps only. */
    std::string m_order;            /**< Fix number of final state photons. */
    double m_cmsEnergy;             /**< CMS Energy = 2*Ebeam [GeV]. */

    std::pair<double, double> m_ScatteringAngleRange; /**< Min and Max value for the scattering angle [deg] of the electron/positron. */
    std::pair<double, double> m_ScatteringAngleRangePhoton; /**< Min and Max value for the scattering angle [deg] of the photons in gg mode. */
    double m_epsilon; /**< Soft/hard photon separator in units of CMS/2. */
    double m_eMin; /**<  [GeV]. */
    double m_maxAcollinearity; /**< Maximum acollinearity of the electron-positron pair. */
    int m_nPhotMode; /**< Fix number of final state photons (EXPERTS ONLY!). */
    int m_nSearchMax;  /**< Events used to search maximum of differential cross section. */
    double m_fMax;  /**< Maximum of differential cross section. */
    double m_sDif;  /**< Differential xsec/weight used for event. */

//     bool m_applyBoost;              /**< Apply a boost to the MCParticles. */
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
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, bool isVirtual = false, bool isInitial = false);

  private:

    int    m_npar[100];  /**< Integer parameters for BabayagaNLO. */
    double m_xpar[100];  /**< Double parameters for BabayagaNLO. */
  };
}


#endif /* BABAYAGANLO_H */
