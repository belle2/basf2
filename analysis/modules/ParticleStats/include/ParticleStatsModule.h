/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013-2020 - Belle II Collaboration                        *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo, Anze Zupanc, Sam Cunliffe               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <string>
#include <TMatrix.h>

namespace Belle2 {

  /** This module summarises the ParticleLists in the event.
  It determines event pass correlations between Lists. */

  class ParticleStatsModule : public Module {
  private:
    /** Shall we also calculate and print out the pass matrix? */
    bool m_printPassMatrix = true;
    /** Number of events with Particle candidates**/
    int m_nPass = 0;
    /** The number of particle lists */
    unsigned m_nLists = 0;
    /** Count the total number of particles */
    int m_nParticles = 0;
    /** The particle lists to produce statistics about */
    std::vector<std::string> m_strParticleLists;
    /** Pass matrix for the particle lists */
    TMatrix*  m_PassMatrix = nullptr;
    /** Particle multiplicities for the particle lists */
    TMatrix*  m_MultiplicityMatrix = nullptr;
    /** Decay descriptor of the particle being selected */
    DecayDescriptor m_decaydescriptor;
    /** StoreArray of Particles */
    StoreArray<Particle> m_particles;
    /** Name of output file */
    std::string m_outputFile;

  public:
    /** Constructor */
    ParticleStatsModule();
    /** Initialises the module */
    virtual void initialize() override;
    /** Fill the particle stats matrices for each event */
    virtual void event() override;
    /** Print out the arrays as a table in B2INFO */
    virtual void terminate() override;
  };
} // end namespace Belle2

