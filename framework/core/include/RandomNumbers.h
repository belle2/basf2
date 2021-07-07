/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <string>
#include <framework/core/RandomGenerator.h>

namespace Belle2 {

  /**
   * The class for handling the random number generation.
   */
  class RandomNumbers {

  public:
    /**
     * Initialize the random number generator with a unique random seed;
     */
    static void initialize();

    /**
     * Initialize the random number generator.
     *
     * @param seed The random number generator seed.
     */
    static void initialize(const std::string& seed);

    /**
     * Truth that the random number generator has been initialized.
     */
    static bool isInitialized();

    /**
     * Initialize run independent random generator for begin run. Called by
     * EventProcessor, should not be called by other users
     */
    static void initializeBeginRun();

    /**
     * Initialize run independent random generator for end run. Called by
     * EventProcessor, should not be called by other users
     */
    static void initializeEndRun();

    /**
     * Initialize event information. Called by EventProcessor, should not be
     * called by other users
     *
     * @param force if true force initialization even in parallel processing mode
     *              for all processes and ignore the generator state that might
     *              have been passed in the event stream.
     *              This is only used on HLT event processor
     */
    static void initializeEvent(bool force = false);

    /**
     * Set Event dependent Random Generator as current one. Called by EventProcessor, should not be
     * called by other users
     */
    static void useEventDependent();

    /** return reference to the event dependent random generator */
    static RandomGenerator& getEventRandomGenerator() { return *s_evtRng; }

    /** Increase random barrier.
     * current random generator will be reseeded with a different "barrier
     * index" which makes it's state independent from previous calls */
    static void barrier();

    /**
     * Sets the seed for the random number generator.
     *
     * @param seed The random number generator seed.
     */
    static void setSeed(const std::string& seed);

    /**
     * Get the random number generator seed.
     *
     * @return The initial random number generator seed.
     */
    static std::string getSeed() {return s_initialSeed;};

    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /**
     * Exposes methods of the RandomNumbers class to Python.
     */
    static void exposePythonAPI();


  private:
    /** event dependent random generator to be used for event processing */
    static RandomGenerator* s_evtRng;
    /** event independent random generator to be used for begin/end run processing */
    static RandomGenerator* s_runRng;
    /** The random number generator seed set by the user. initialized to a
     * non-deterministic 64byte hex string if not set by user  */
    static std::string s_initialSeed;
    /** barrier index offset to be used in begin/endRun. Obtained from event dependent generator */
    static int s_barrierOffset;
  };

} //end of namespace Belle2
