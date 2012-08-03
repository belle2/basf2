/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BBBREM_H
#define BBBREM_H

#include <framework/gearbox/Unit.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include <TLorentzRotation.h>

#include <string>

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
    BBBrem() : m_applyBoost(false), m_unweighted(true), m_cmsEnergy(10.58), m_photonEFrac(0.000001) {};

    /** Destructor. */
    ~BBBrem() {};

    /** Sets the Lorentz boost vector which should be applied to the generated particles.
     * @param boostVector The Lorentz boost vector which is applied to the generated particles.
     */
    void setBoost(TLorentzRotation boostVector) { m_boostVector = boostVector; }

    /** Enables the boost of the generated particles.
     * @param applyBoost Set to true to enabled the boost. Also make sure you have set the boost vector using setBoost().
     */
    void enableBoost(bool applyBoost = true) { m_applyBoost = applyBoost; }

    /** Initializes the generator.
     * @cmsEnergy The center of mass energy in [GeV].
     * @minPhotonEFrac The minimum photon energy fraction. The min. photon energy is this fraction * beam energy.
     * @unweighted Set to true to generate unweighted events (e.g. for detector simulation), set to false for weighted events.
     * @maxWeight The maximum weight. Only required in the case of unweighted events.
     */
    void init(double cmsEnergy, double minPhotonEFrac, bool unweighted = true, double maxWeight = 2000.0);

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     * @return Returns the weight of the event.
     */
    double generateEvent(MCParticleGraph& mcGraph);

    /** Returns the total cross section of the generated process in millibarn.
     * @return The total cross section.
     */
    double getCrossSection() { return m_crossSection; }

    /** Returns the error on the total cross section of the generated process  in millibarn.
     * @return The error on the total cross section.
     */
    double getCrossSectionError() { return m_crossSectionError; }

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
    bool m_applyBoost;              /**< Apply a boost to the MCParticles. */
    bool m_unweighted;              /**< True if BBBrem should produce unweighted events. */
    double m_maxWeight;             /**< The maximum weight. Used for the event rejection procedure to produce unweighted events. */
    double m_maxWeightDelivered;    /**< The maximum weight given by the BBBrem generation. */
    double m_sumWeightDelivered;    /**< The sum of all weights returned by the BBBrem generation.*/
    double m_sumWeightDeliveredSqr; /**< The square of the sum of all weights returned by the BBBrem generation.*/
    double m_cmsEnergy;             /**< Center of mass energy (sqrt(s)). */
    double m_photonEFrac;           /**< Minimum photon energy fraction. */

    double m_crossSection;          /**< The cross-section in millibarns. */
    double m_crossSectionError;     /**< The error on the cross-section in millibarns. */

    TLorentzRotation m_boostVector; /**< The Lorentz boost vector for the transformation CMS to LAB frame. */

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
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, bool isVirtual = false);


  private:

    static const double tomb = 3.8937966e5 / 1e6;   /**< Conversion factor (hc)^2 */
    static const double twopi = 2.0 * M_PI;         /**< 2*pi. To keep things short. */

    //Variable names directly taken from the FORTRAN code. Sorry.
    double alpha;
    double rme;    /**< in MeV */
    double s;
    double rme2;
    double rme2s;
    double rls;
    double z0;
    double a1, a2, ac;
    double sigapp;
    double eb, pb, rin2pb;
    double p1[4];
    double p2[4];
    double q1[4];
    double q2[4];
    double qk[4];

    double weight;
  };

}


#endif /* BBBREM_H */
