/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KORALW_H
#define KORALW_H

#include <mdst/dataobjects/MCParticleGraph.h>
#include <TLorentzRotation.h>

#include <string>

#define NUM_XPAR 10000

namespace Belle2 {

  /**
   * C++ interface for the FORTRAN 4-fermion final state generator KoralW.
   */
  class KoralW {

  public:

    /** Constructor. */
    KoralW() : m_applyBoost(0.0), m_crossSection(0.0), m_crossSectionError(0.0)
    {for (int i = 0; i < 10000; i++) m_xpar[i] = 0.0;}


    //KoralW() : m_applyBoost(0.0), m_crossSection(0.0), m_crossSectionError(0.0) {};

    /** Destructor. */
    ~KoralW() {};

    /** Sets the Lorentz boost vector which should be applied to the generated particles.
     * @param boostVector The Lorentz boost vector which is applied to the generated particles.
     */
    void setBoost(TLorentzRotation boostVector) { m_boostVector = boostVector; }

    /** Enables the boost of the generated particles.
     * @param applyBoost Set to true to enabled the boost. Also make sure you have set the boost vector using setBoost().
     */
    void enableBoost(bool applyBoost = true) { m_applyBoost = applyBoost; }

    /** Initializes the generator.
     * @param dataPath The path to the default input data file for KoralW.
     * @param userDataFile The path and filename of the user input data file, which defines the user settings for the generator.
     * @param randomSeed The random seed for the generator.
     */
    void init(const std::string& dataPath, const std::string& userDataFile, int randomSeed);

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     */
    void generateEvent(MCParticleGraph& mcGraph);

    /**
     * Terminates the generator.
     * Closes the internal Fortran generator and retrieves the total cross section.
     */
    void term();

    /** Returns the total cross section of the generated process.
     * @return The total cross section.
     */
    double getCrossSection() { return m_crossSection; }

    /** Returns the error on the total cross section of the generated process.
     * @return The error on the total cross section.
     */
    double getCrossSectionError() { return m_crossSectionError; }


  protected:

    bool m_applyBoost;              /**< Apply a boost to the MCParticles. */
    TLorentzRotation m_boostVector; /**< The Lorentz boost vector for the transformation CMS to LAB frame. */

    double m_crossSection;      /**< The cross section of the generated KoralW events. */
    double m_crossSectionError; /**< The error on the cross section of the generated KoralW events. */

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param vtx The vertex of the particle in [mm].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     *
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */

    void storeParticle(MCParticleGraph& mcGraph, const float* mom, const float* vtx, int pdg, bool isVirtual = false, bool isInitial = false);

  private:

    double m_xpar[NUM_XPAR];  /**< Double parameters for KoralW. */

  };
}



#endif /* KORALW_H */
