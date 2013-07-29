/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef PARTICLESTATSMODULE_H
#define PARTICLESTATSMODULE_H
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TMatrix.h>

using namespace Belle2;


namespace Belle2 {

  /** This module summarises the ParticleLists in the event.
  It determines event pass correlations between Lists. */

  class ParticleStatsModule : public Module {
  private:
    /** Event counter **/
    int m_nEvents;
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

  public:
    /** Constructor. */
    ParticleStatsModule();
    /** Destructor. */
    ~ParticleStatsModule() {}
    /** Initialises the module. At the moment this contains the
    construction of a DecayDescriptor object. This will by replaced
    by a simple call to the parser. */
    void initialize();
    /** Method called for each event. */
    void event();
    /** Write TTree to file, and close file if necessary. */
    void terminate();
  };
} // end namespace Belle2

#endif // PARTICLESTATSMODULE_H
