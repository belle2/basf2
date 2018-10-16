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
   * Creates for each Particle in given ParticleList an RestOfEvent (ROE) dataobject and makes BASF2 relation between them.
   * Takes as frst input parameter the name of target particle list, around which the RestOfEvent will be constructed.
   * The ROE particles are taken from dafault particle lists 'gamma:myroe', 'pi+:myroe' and 'K_L0:myroe', which defines
   * the default particle type hypotheses: gammas, pions and K-longs.
   * Second input parameter is an array of final-state particle list names, which are preselected by PID, for example:
   * if the module is launched with particleListsInput=['K+:good','e+:good'], it will create ROE object filled with these
   * particle track hypotheses first and all other particles with default particle hypotheses.
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
