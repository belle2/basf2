/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCMATCHERPARTICLESMODULE_H
#define MCMATCHERPARTICLESMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <analysis/dataobjects/ParticleList.h>
#include <string>


namespace Belle2 {

  /**
   * MC matching module: module performs MC matching (sets the relation Particle -> MCParticle) for all particles
   * and its (grand)^N-daughter particles in the user-specified ParticleList. The MC matching algorithm is implemented
   * in the MCMatching namespace.
   */
  class MCMatcherParticlesModule : public Module {

  public:

    /**
     * Constructor
     */
    MCMatcherParticlesModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;

  private:

    std::string m_listName;  /**< steering variable: name of the input ParticleList */
    StoreObjPtr<ParticleList> m_plist; /**< the input ParticleList. */
  };
}
#endif
