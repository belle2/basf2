/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <TrackTimeEstimator.hModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;

REG_MODULE(TrackTimeEstimator.h);

TrackTimeEstimator.hModule::TrackTimeEstimator.hModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(Computes the track time, defined as the difference between the average of SVD clusters time and the SVDEvent T0)DOC");

  // Parameter definitions

}

void TrackTimeEstimator.hModule::initialize()
{
}

void TrackTimeEstimator.hModule::event()
{
}


