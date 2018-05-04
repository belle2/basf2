/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr, Martin Ritter                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <boost/python.hpp>
#include <random>
#include <sstream>

#include <framework/core/RandomNumbers.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/ProcHelper.h>
#include <framework/logging/Logger.h>


using namespace Belle2;


/** The random number generator seed set by the user. initialized to a
 * non-deterministic 64byte hex string if not set by user  */
std::string RandomNumbers::s_initialSeed;
/** event dependent random generator to be used for event processing */
RandomGenerator* RandomNumbers::s_evtRng{nullptr};
/** event independent random generator to be used for begin/end run processing */
RandomGenerator* RandomNumbers::s_runRng{nullptr};
/** barrier index offset to be used in begin/endRun. Obtained from event dependent generator */
int RandomNumbers::s_barrierOffset;

void RandomNumbers::initialize()
{
  //use std::random_device to get non deterministic initial seed to be used if
  //user doesn't set one
  std::random_device random_device;
  std::stringstream seed;
  for (int i = 0; i < 8; ++i) {
    seed << std::hex << random_device();
  }
  initialize(seed.str());
}

void RandomNumbers::initialize(const std::string& seed)
{
  s_initialSeed = seed;
  if (!s_evtRng) {
    s_evtRng = new RandomGenerator();
  }
  if (!s_runRng) {
    s_runRng = new RandomGenerator();
  }
  RandomGenerator* gen = dynamic_cast<RandomGenerator*>(gRandom);
  if (!gen) {
    delete gRandom;
    B2DEBUG(100, "Replacing gRandom from " << gRandom << " to " << gen);
  }
  gRandom = s_evtRng;
  s_evtRng->setMode(RandomGenerator::c_independent);
  s_evtRng->setSeed((const unsigned char*)seed.c_str(), seed.size());
  s_runRng->setSeed((const unsigned char*)seed.c_str(), seed.size());
}

void RandomNumbers::barrier()
{
  RandomGenerator* gen = dynamic_cast<RandomGenerator*>(gRandom);
  if (!gen) {
    B2ERROR("Random Generator gRandom is not Belle2::RandomGenerator, cannot increase barrier");
  } else {
    gen->barrier();
  }
}

void RandomNumbers::setSeed(const std::string& seed)
{
  initialize(seed);
  B2INFO("The random number seed is set to \"" << seed << '"');
}

void RandomNumbers::initializeBeginRun()
{
  gRandom = s_runRng;
  s_runRng->setMode(RandomGenerator::c_runDependent);
  //This might be called in in main or output process. In that case we don't
  //know how many random barriers came before but we can look at the s_evtRng
  //as this was passed from the previous process and contains the current
  //barrier number in the event flow. We save it in intializeEvent so let's use
  //it now.
  s_runRng->setBarrier(s_barrierOffset);
};

void RandomNumbers::initializeEndRun()
{
  gRandom = s_runRng;
  s_runRng->setMode(RandomGenerator::c_runDependent);
  //We set the barrier index to it's minimum possible value: usually barrier
  //index starts at 0 but for endRun we set it to a negative number large
  //enough that there is no realistic chance that beginRun had the same barrier
  //index.
  s_runRng->setBarrier(INT_MIN + s_barrierOffset);
}

void RandomNumbers::initializeEvent()
{
  useEventDependent();
  //we pass the random generator to other processes in multiprocessing so we only
  //want to initialize it in the input process (or if there is no multi processing)
  if (!ProcHandler::parallelProcessingUsed() or ProcHandler::isProcess(ProcType::c_Input)) {
    // when in event loop we want an error if there is no EventMetaData
    s_evtRng->setMode(RandomGenerator::c_eventDependent);
    s_evtRng->initialize();
  }
  //Ok, now we either have reset the barrier to 0 if we are an input process or
  //have decided to keep the state from the previous process. In any case, this
  //is the start barrier we want to use for begin/end of run processing so we
  //keep it.
  s_barrierOffset = s_evtRng->getBarrier();
}

void RandomNumbers::useEventDependent()
{
  gRandom = s_evtRng;
}

//=====================================================================
//                          Python API
//=====================================================================

namespace {
  /** small helper function to convert any python object to a string representation */
  std::string pythonObjectToString(const boost::python::object& obj)
  {
    return boost::python::extract<std::string>(obj.attr("__str__")());
  }
  /** set the seed from an abitary python object */
  void setPythonSeed(const boost::python::object& obj)
  {
    RandomNumbers::setSeed(pythonObjectToString(obj));
  }
}

void RandomNumbers::exposePythonAPI()
{
  using namespace boost::python;

  //don't show c++ signature in python doc to keep it simple
  docstring_options options(true, true, false);
  //Expose RandomNumbers class
  def("set_random_seed", &setPythonSeed, args("seed"), R"DOCSTRING(
Set the random seed. The argument can be any object and will be converted to a
string using the builtin str() function and will be used to initialize the
random generator.)DOCSTRING");
  def("get_random_seed", &RandomNumbers::getSeed, "Return the current random seed");
}
