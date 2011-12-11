/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RANDOMNUMBERS_H
#define RANDOMNUMBERS_H

#include <TRandom3.h>


namespace Belle2 {

  /**
   * The class for handling the random number generation.
   */
  class RandomNumbers {

  public:

    /**
     * Initialize the random number generator.
     *
     * @param seed The random number generator seed. A value of 0 means a unique random seed.
    */
    static void initialize(unsigned int seed = 0);

    /**
     * Initialize the random number generator.
     *
     * @param seed The random number generator seed.
     * @param random The random number generator.
    */
    static void initialize(unsigned int seed, const TRandom3& random);

    /**
     * Sets the seed for the random number generator.
     *
     * @param seed The random number generator seed.
    */
    static void setSeed(unsigned int seed);

    /**
     * Obtain a seed for the initialization of an external random number generator.
     *
     * @return The random number generator seed.
    */
    static unsigned int getSeed();

    /**
     * Get the initial random number generator seed.
     *
     * @return The initial random number generator seed.
    */
    static unsigned int getInitialSeed() {return s_initialSeed;};

    /**
     * Get the initial random number generator object.
     *
     * @return The initial random number generator object.
    */
    static const TRandom3 getInitialRandom() {return s_initialRandom;};


    //--------------------------------------------------
    //                   Python API
    //--------------------------------------------------

    /**
     * Exposes methods of the RandomNumbers class to Python.
     */
    static void exposePythonAPI();


  private:

    static unsigned int s_initialSeed;  /**< The random number generator seed set by the user. */
    static TRandom3 s_initialRandom;    /**< The initial random number generator. */

  };

} //end of namespace Belle2

#endif /*RANDOMNUMBERS_H */
