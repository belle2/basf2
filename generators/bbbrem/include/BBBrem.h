/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticleGraph.h>

#include <TLorentzRotation.h>

namespace Belle2 {

  /**
   * Generator for low scattering angle radiative Bhabha events (Beam-Beam Bremsstrahlung).
   * Based on the FORTRAN source code from R. Kleiss. In order to be consistent with the
   * original source code, the variable names are kept and might therefore violate the
   * coding conventions.
   *
   * Original authors: r. Kleiss and h. Burkhardt
   * Reference: [arXiv:hep-ph/9401333v1]
   */
  class BBBrem {

  public:

    /** Constructor. */

    BBBrem() :
      m_eventCount(0),
      m_unweighted(true),
      m_weightCount(0),
      m_weightCountOver(0),
      m_maxWeight(0.0),
      m_maxWeightDelivered(0.0),
      m_sumWeightDelivered(0.0),
      m_sumWeightDeliveredSqr(0.0),
      m_sumWeightDeliveredOver(0.0),
      m_sumWeightDeliveredSqrOver(0.0),
      m_cmsEnergy(10.58),
      m_photonEFrac(0.000001),
      m_crossSection(0.0),
      m_crossSectionError(0.0),
      m_crossSectionOver(0.0),
      m_crossSectionErrorOver(0.0),
      m_densityCorrectionMode(0),
      m_densityCorrectionParameter(0.0),
      alpha(0.0),
      rme(0.0),
      s(0.0),
      rme2(0.0),
      rme2s(0.0),
      rls(0.0),
      z0(0.0),
      a1(0.0),
      a2(0.0),
      ac(0.0),
      sigapp(0.0),
      eb(0.0),
      pb(0.0),
      rin2pb(0.0),
      weight(0.0)
    {for (int i = 0; i < 4; i++) p1[i] = p2[i] = q1[i] = q2[i] = qk[i] = 0.0;}


    //BBBrem() : m_applyBoost(false), m_unweighted(true), m_cmsEnergy(10.58), m_photonEFrac(0.000001) {};

    /** Destructor. */
    ~BBBrem() {};

    /** Initializes the generator.
     * @cmsEnergy The center of mass energy in [GeV].
     * @minPhotonEFrac The minimum photon energy fraction. The min. photon energy is this fraction * beam energy.
     * @unweighted Set to true to generate unweighted events (e.g. for detector simulation), set to false for weighted events.
     * @maxWeight The maximum weight. Only required in the case of unweighted events.
     */
    void init(double cmsEnergy, double minPhotonEFrac, bool unweighted = true, double maxWeight = 2000.0, int densitymode = 1,
              double densityparameter = 1.68e-17);

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     * @return Returns the weight of the event.
     */
    double generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, TLorentzRotation boost);

    /** Returns the total cross section of the generated process in millibarn.
     * @return The total cross section.
     */
    double getCrossSection() { return m_crossSection; }

    /** Returns the error on the total cross section of the generated process  in millibarn.
     * @return The error on the total cross section.
     */
    double getCrossSectionError() { return m_crossSectionError; }

    /** Returns the total overweight bias cross section of the generated process in millibarn.
     * @return The total overweight bias cross section.
     */
    double getCrossSectionOver() { return m_crossSectionOver; }

    /** Returns the error on the total overweight bias cross section of the generated process  in millibarn.
     * @return The error on the total overweight bias cross section.
     */
    double getCrossSectionErrorOver() { return m_crossSectionErrorOver; }

    /** Returns the maximum weight given by the BBBrem generation.
     * @return The maximum weight given by the BBBrem generation.
     */
    double getMaxWeightDelivered() { return m_maxWeightDelivered; }

    /** Returns the sum of all weights returned by the BBBrem generation.
     * @return The sum of all weights returned by the BBBrem generation.
     */
    double getSumWeightDelivered() { return m_sumWeightDelivered; }

    /**
     * Terminates the generator.
     */
    void term();


  protected:

    int m_eventCount;               /**< Internal event counter. Used to calculate the cross-section. */
    bool m_unweighted;              /**< True if BBBrem should produce unweighted events. */
    long m_weightCount;             /**< Internal weighted event counter. Used to calculate the cross-section. */
    long m_weightCountOver;         /**< Internal overweighted event counter. Used to calculate the cross-section. */
    double m_maxWeight;             /**< The maximum weight. Used for the event rejection procedure to produce unweighted events. */
    double m_maxWeightDelivered;    /**< The maximum weight given by the BBBrem generation. */
    double m_sumWeightDelivered;    /**< The sum of all weights returned by the BBBrem generation.*/
    double m_sumWeightDeliveredSqr; /**< The square of the sum of all weights returned by the BBBrem generation.*/
    double m_sumWeightDeliveredOver;    /**< The sum of all overweights.*/
    double m_sumWeightDeliveredSqrOver; /**< The square of the sum of all overweights.*/
    double m_cmsEnergy;             /**< Center of mass energy (sqrt(s)). */
    double m_photonEFrac;           /**< Minimum photon energy fraction. */

    double m_crossSection;          /**< The cross-section in millibarns. */
    double m_crossSectionError;     /**< The error on the cross-section in millibarns. */
    double m_crossSectionOver;      /**< The overweight bias cross-section in millibarns. */
    double m_crossSectionErrorOver; /**< The overweight bias error on the cross-section in millibarns. */

    int m_densityCorrectionMode;   /**< Mode for bunch density correction. */
    double m_densityCorrectionParameter;   /**< Density correction parameter tc. */

    /** Calculate the outgoing leptons and the event weight for one single radiative Bhabha scattering.
     * The main method. A direct translation from the BBBrem Fortran source code.
     */
    void calcOutgoingLeptonsAndWeight();

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param vtx The vertex of the particle in [mm].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     *
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, TVector3 vertex, TLorentzRotation boost,
                       bool isVirtual = false, bool isInitial = false);

  private:

    static constexpr double tomb = 3.8937966e5 / 1e6;   /**< Conversion factor (hc)^2 */
    static constexpr double twopi = 2.0 * M_PI;         /**< 2*pi. To keep things short. */

    //Variable names directly taken from the FORTRAN code. Sorry.
    double alpha;  /**< variable   */
    double rme;    /**< in MeV */
    double s;      /**< variable   */
    double rme2;   /**< variable   */
    double rme2s;  /**< variable   */
    double rls;    /**< variable   */
    double z0;     /**< variable   */
    double a1;  /**< variable   */
    double a2;  /**< variable   */
    double ac;  /**< variable   */
    double sigapp;      /**< variable   */
    double eb;     /**< variable   */
    double pb;     /**< variable   */
    double rin2pb; /**< variable   */
    double p1[4];  /**< variable   */
    double p2[4];  /**< variable   */
    double q1[4];  /**< variable   */
    double q2[4];  /**< variable   */
    double qk[4];  /**< variable   */
    double weight; /**< variable   */
  };

}


