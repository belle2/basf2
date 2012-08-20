/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>

#include <framework/core/RandomNumbers.h>
#include <framework/logging/Logger.h>


using namespace Belle2;


unsigned int RandomNumbers::s_initialSeed = 0;
TRandom3 RandomNumbers::s_initialRandom;


void RandomNumbers::initialize(unsigned int seed)
{
  s_initialSeed = seed;
  gRandom->SetSeed(seed);
  s_initialRandom = *static_cast<TRandom3*>(gRandom);
}


void RandomNumbers::initialize(unsigned int seed, const TRandom3& random)
{
  s_initialSeed = seed;
  s_initialRandom = random;
  *static_cast<TRandom3*>(gRandom) = random;
}


void RandomNumbers::setSeed(unsigned int seed)
{
  initialize(seed);
  B2INFO("The random number seed is set to " << seed);
}


unsigned int RandomNumbers::getSeed()
{
  if (s_initialSeed > 0) {
    return s_initialSeed;
  } else {
    return gRandom->Integer(static_cast<unsigned int>(-1)) + 1;
  }
}


//=====================================================================
//                          Python API
//=====================================================================

void RandomNumbers::exposePythonAPI()
{
  using namespace boost::python;

  //Expose RandomNumbers class
  def("set_random_seed", &RandomNumbers::setSeed);
}
