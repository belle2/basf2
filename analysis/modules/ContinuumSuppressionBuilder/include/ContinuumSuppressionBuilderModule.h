/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONTINUUMSUPPRESSIONBUILDERMODULE_H
#define CONTINUUMSUPPRESSIONBUILDERMODULE_H

#include <framework/core/Module.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <string>
#include <vector>

namespace Belle2 {
  /**
   * Creates for each Particle in given ParticleList an ContinuumSuppression dataobject and makes BASF2 relation between them.
   *
   *    *
   */
  class ContinuumSuppressionBuilderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    ContinuumSuppressionBuilderModule();

    /**  */
    virtual void initialize() override;

    /** n */
    virtual void event() override;


  private:

    std::string m_particleList;  /**< Name of the ParticleList */
    std::string m_ROEMask;  /**< ROE mask */

    /**
     * for debugging purposes
     */
    void printEvent();

    /**
     * for debugging purposes
     */
    void printParticle(const Particle* particle);
  };
}

#endif /* CONTINUUMSUPPRESSIONBUILDERMODULE_H */
