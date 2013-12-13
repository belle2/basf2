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
#include <string>
#include <vector>
#include <unordered_set>

//Hack: allow defining std::hash for dynamic_bitset
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>

namespace std {
  /** Define hash for dynamic_bitset. */
  template<> struct hash<boost::dynamic_bitset<> > {
    std::size_t operator()(const boost::dynamic_bitset<>& bs) const {
      return boost::hash_value(bs.m_bits);
    }
  };
}

namespace Belle2 {


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

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    /**
     * Main routine for making combinations
     * @param plist reference to output particle list StoreObjPtr
     * @param plists refernce to vector of input particle lists StoreObjPtr's
     * @param chargeState charge state (0=unflavored or 1=flavored)
     */
    void combination(StoreObjPtr<ParticleList>& plist,
                     std::vector<StoreObjPtr<ParticleList> >& plists,
                     unsigned chargeState);

    /**
     * Check that all FS particles of a combination differ
     * @param stack stack for pointers to Particle
     * @param Particles reference to StoreArray holding Particles
     * @return true if all FS particles of a combination differ
     */
    bool differentSources(std::vector<Particle*> stack,
                          StoreArray<Particle>& Particles);

    /**
     * Check that the combination is unique
     * @param indexStack reference to stack of combination indices
     * @param indices combination indices to check
     * @return true if indices not found in the stack; if true indices pushed to stack
     */
    bool uniqueCombination(std::unordered_set<boost::dynamic_bitset<> >& indexStack,
                           const std::vector<int>& indices);

    /**
     */

    int m_pdg;                /**< PDG code of combined particles */
    std::string m_listName;   /**< output particle list name */
    std::vector<std::string> m_inputListNames; /**< input particle list names */
    double m_massCutLow;      /**< lower mass cut */
    double m_massCutHigh;     /**< upper mass cut */
    bool m_persistent;  /**< toggle output particle list btw. transient/persistent */

  };

} // Belle2 namespace

#endif
