/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo, Anze Zupanc                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef PARTICLESTATSMODULE_H
#define PARTICLESTATSMODULE_H
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TMatrix.h>

namespace Belle2 {

  /** This module summarises the ParticleLists in the event.
  It determines event pass correlations between Lists. */

  class ParticleStatsModule : public Module {
  private:
    /** Number of events with Particle candidates**/
    int m_nPass;
    /** Count the total number of particles */
    int m_nParticles;
    /** Name of the TTree. */
    std::vector<std::string> m_strParticleLists;
    /** Pass matrix for the particle lists. */
    TMatrix*  m_PassMatrix;
    /** Particle multiplicity matrix for the particle lists. */
    TMatrix*  m_MultiplicityMatrix;
    /** Decay descriptor of the particle being selected */
    DecayDescriptor m_decaydescriptor;

  public:
    /** Constructor. */
    ParticleStatsModule();

    /** Initialises the module.
     */
    virtual void initialize() override;
    /** Method called for each event. */

    virtual void event() override;

    /** Write TTree to file, and close file if necessary. */
    virtual void terminate() override;
  };
} // end namespace Belle2

#endif // PARTICLESTATSMODULE_H
