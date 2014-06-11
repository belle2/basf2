/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLECOMBINERMODULE_H
#define PARTICLECOMBINERMODULE_H

#include <framework/core/Module.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <tuple>


namespace Belle2 {
  class Particle;

  /**
   * particle combiner module
   */
  class ParticleCombinerModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleCombinerModule();

    /**
     * Destructor
     */
    virtual ~ParticleCombinerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Check if the particleStack pass all cut requirements
     * @param particleStack Reference to stack of pointers to Particle
     * @return True if all requirements are passed, false otherwise
     */
    bool checkCuts(const Particle* particle);

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    std::string m_strDecay;   /**< Input DecayString specifying the decay being reconstructed */
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the decay being reconstructed */
    ParticleGenerator* m_generator; /**< Generates the combinations */

    std::string m_listName;   /**< output particle list name */
    std::string m_antiListName;   /**< output anti-particle list name */

    bool m_isSelfConjugatedParticle; /**< flag that indicates whether an anti-particle mother does not exist and should not be reconstructed as well*/

    std::vector<std::string> m_inputListNames; /**< input particle list names */
    std::map<std::string, std::tuple<double, double> > m_cuts; /**< variables -> low/high cut */
    bool m_persistent;  /**< toggle output particle list btw. transient/persistent */

  };

} // Belle2 namespace

#endif
