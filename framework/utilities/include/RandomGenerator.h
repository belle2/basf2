/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

// #include <framework/utilities/SingletonDestroyerT.h>

#include <TRandom3.h>

namespace BASF2 {

  /** Central Random Number Generator based on TRandom3 of the ROOT package.
   *
   *  So far, this is more or less a trial balloon. It is not clear, if this is
   *  A) necessary,
   *  B) a reasonable approach within multiprocessing. However, resetting the seed for every partial process might be enough.
   *
   *  If the raw TRandom3 class is sufficient, a change from a class to a namespace might be sufficient.
   *  Otherwise we might increase this to a full Singleton.
   */
  class RandomGenerator {
  public:
    /** Actual random generator.
     *
     *  For usage see <a href="http://root.cern.ch/root/html/TRandom3.html">ROOT documentation</a>.
     */
    static TRandom3 randomGen;

    /* * Accessor for an instance of RandomGenerator.
     *
     *  @return A reference to an instance of this class.
     */
//    static RandomGenerator& Instance();



//  private:
//  RandomGenerator ();
//  virtual ~RandomGenerator ();
  };

}

#endif /* RANDOMGENERATOR_H */
