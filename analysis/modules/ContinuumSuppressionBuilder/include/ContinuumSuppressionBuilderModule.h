/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/ContinuumSuppression.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>

namespace Belle2 {
  /**
   * Creates for each Particle in given ParticleList an ContinuumSuppression
   * dataobject and makes BASF2 relation between them.
   */
  class ContinuumSuppressionBuilderModule : public Module {
  public:

    /** constructor */
    ContinuumSuppressionBuilderModule();
    /** initialize the module (setup the data store) */
    virtual void initialize() override;
    /** process event */
    virtual void event() override;

  private:

    StoreArray<ContinuumSuppression> m_csarray; /**< StoreArray of ContinuumSuppression */
    StoreObjPtr<ParticleList> m_plist; /**< input particle list */

    std::string m_particleListName;  /**< Name of the ParticleList */
    std::string m_ROEMask;  /**< ROE mask */

    /** print an event for debugging purposes */
    void printEvent();
    /** print a particle for debugging purposes */
    void printParticle(const Particle* particle);
  };
}

