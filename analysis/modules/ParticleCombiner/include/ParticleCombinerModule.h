/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLECOMBINERMODULE_H
#define PARTICLECOMBINERMODULE_H

#include <framework/core/Module.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/ParticleCombiner/ParticleCombiner.h>

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
     * Checks if the particle itself is self conjugated
     * @param outputList reference to output particle list StoreObjPtr
     */
    bool isParticleSelfConjugated(StoreObjPtr<ParticleList>& outputList);

    /**
     * Check if the particleStack pass all cut requirements
     * @param particleStack Reference to stack of pointers to Particle
     * @return True if all requirements are passed, false otherwise
     */
    bool checkCuts(const Particle* particle);

    int m_pdg;                /**< PDG code of combined particles */
    std::string m_listName;   /**< output particle list name */
    std::vector<std::string> m_inputListNames; /**< input particle list names */
    std::map<std::string, std::tuple<double, double> > m_cuts; /**< variables -> low/high cut */
    bool m_persistent;  /**< toggle output particle list btw. transient/persistent */

  };

} // Belle2 namespace

#endif
