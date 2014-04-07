/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo                                          *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef SKIMFILTERMODULE_H
#define SKIMFILTERMODULE_H
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <string>
#include <TTree.h>
#include <TFile.h>
#include <TMatrix.h>

namespace Belle2 {

  /** This module filters events based on presence of candidates in a list of ParticleLists.
  For detailed Particle List statistics use the ParticleStats module. */

  class SkimFilterModule : public Module {
  private:
    /** Event counter **/
    int m_nEvents;
    /** Number of events with Particle candidates**/
    int m_nPass;
    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

  public:
    /** Constructor. */
    SkimFilterModule();
    /** Destructor. */
    ~SkimFilterModule() {}
    /** Initialises the module.
     */
    void initialize();
    /** Method called for each event. */
    void event();
    /** Write TTree to file, and close file if necessary. */
    void terminate();
  };
} // end namespace Belle2

#endif // SKIMFILTERMODULE_H

