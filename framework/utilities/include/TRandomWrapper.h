/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TRandom.h>

namespace Belle2 {

  /** Wrap TRandom to be useable as a uniform random number generator with STL algorithms like std::shuffle. */
  struct TRandomWrapper {

    /** Define the result type to be a normal unsigned int. */
    typedef unsigned int result_type;

    /** Minimum value returned by the random number generator. */
    static constexpr result_type min() { return 0; }

    /** Maximum value returned by the random number generator. */
    static constexpr result_type max() { return std::numeric_limits<result_type>::max(); }

    /** Return a random value in the range [min(), max()]. */
    result_type operator()() { return gRandom->Integer(max()); }
  };

}
