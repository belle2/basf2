/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Sviatoslav Bilokin                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RESTOFEVENTBUILDERMODULE_H
#define RESTOFEVENTBUILDERMODULE_H

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>

namespace Belle2 {

  /**
   * Creates for each Particle in given ParticleList an RestOfEvent dataobject and makes BASF2 relation between them.
   *
   *    *
   */
  class RestOfEventBuilderModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    RestOfEventBuilderModule();

    /** Overriden initialize method */
    virtual void initialize() override;

    /** Overriden event method */
    virtual void event() override;


  private:

    std::string m_particleList;  /**< Name of the ParticleList */
    std::vector<std::string> m_particleListsInput;  /**< Name of the input particle lists of pi+ gamma and Klongs*/

    /**
     * Compares the stored particle with a daughter to check that the mdst source is not the same
     * @param The ROE particle
     * @param The FSP daughter
     * @returns true if a copy, false if not
     */
    bool compareParticles(const Particle* storedParticle, const Particle* fspDaughter);
    /**
    * Adds all particles from input particle lists that are not used in reconstruction of given particle.
    *
    * @param reconstructed particle for which RestOfEvent is determined
    * @param pointer to the RestOfEvent to be filled with remaining tracks
    */
    void addRemainingParticles(const Particle* particle, RestOfEvent* roe);
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

#endif /* RESTOFEVENTBUILDERMODULE_H */
