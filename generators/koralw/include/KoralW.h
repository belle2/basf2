/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* Belle 2 headers. */
#include <mdst/dataobjects/MCParticleGraph.h>

/* ROOT headers. */
#include <Math/LorentzRotation.h>

/* C++ headers. */
#include <string>

namespace Belle2 {

  /**
   * C++ interface for the FORTRAN 4-fermion final state generator KoralW.
   */
  class KoralW {

  public:

    /** Constructor. */
    KoralW() :
      m_crossSection(0.0),
      m_crossSectionError(0.0),
      m_cmsEnergy(0.0),
      m_seed1(900000000),
      m_seed2(10000),
      m_seed3(1000)
    {for (int i = 0; i < m_numXPar; i++) m_XPar[i] = 0.0;}


    /** Destructor. */
    ~KoralW() {};

    /** Sets the CMS energy.
     * @param cmsEnergy The CMS energy in [GeV].
     */
    void setCMSEnergy(double cmsEnergy) { m_cmsEnergy = cmsEnergy; }

    /** Initializes the generator.
     * @param dataPath The path to the default input data file for KoralW.
     * @param userDataFile The path and filename of the user input data file, which defines the user settings for the generator.
     * @param randomSeed The random seed for the generator.
     */
    void init(const std::string& dataPath, const std::string& userDataFile);

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     * @param vertex generated vertex.
     * @param boost generated boost.
     */
    void generateEvent(MCParticleGraph& mcGraph, TVector3 vertex, ROOT::Math::LorentzRotation boost);

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

    double m_crossSection;      /**< The cross section of the generated KoralW events. */

    double m_crossSectionError; /**< The error on the cross section of the generated KoralW events. */

    double m_cmsEnergy; /**< CMS Energy = 2*Ebeam [GeV]. */

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param vtx The vertex of the particle in [mm].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     *
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const float* mom, const float* vtx, int pdg, TVector3 vertex,
                       ROOT::Math::LorentzRotation boost,
                       bool isVirtual = false, bool isInitial = false);

  private:

    static constexpr int m_numXPar = 10000; /**< Number of parameters for KoralW. */

    double m_XPar[m_numXPar];  /**< Values of parameters for KoralW. */

    unsigned int m_seed1; /**< First seed for the random number generator. */

    unsigned int m_seed2; /**< Second seed for the random number generator. */

    unsigned int m_seed3; /**< Third seed for the random number generator. */

  };
}
