/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Roca                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FLAVORTAGINFOBUILDERMODULE_H
#define FLAVORTAGINFOBUILDERMODULE_H

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

#include <string>
#include <vector>

namespace Belle2 {

  /**
   * Creates for each Particle in given ParticleList an RestOfEvent dataobject and makes BASF2 relation between them.
   *
   *    *
   */
  class FlavorTagInfoBuilderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    FlavorTagInfoBuilderModule();

    /**  */
    virtual void initialize();

    /** n */
    virtual void event();


  private:

    std::string m_particleList;  /**< Name of the ParticleList */
  };
}

#endif /* FLAVORTAGINFOBUILDERMODULE_H */
