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
#include <analysis/dataobjects/ContinuumSuppression.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/Thrust.h>
#include <analysis/utility/KsfwMoments.h>

#include <string>
#include <vector>

namespace Belle2 {
  /**
   * \addtogroup modules
   * @{ ContinuumSuppressionBuilderModule @}
   */

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
    virtual void initialize();

    /** n */
    virtual void event();


  private:

    std::string m_particleList;  /**< Name of the ParticleList */

    // TODO: add a parameter to select use_finalstate_for_sig==0 or ==1.
    // std::vector<std::string> m_finalStateForSignalSelection;

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
