/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/TrackIsoCalculator/TrackIsoScoreCalculatorModule.h>

/* --------------- WARNING ---------------------------------------------- *
If you have more complex parameter types in your class then simple int,
double or std::vector of those you might need to uncomment the following
include directive to avoid an undefined reference on compilation.
* ---------------------------------------------------------------------- */
// #include <framework/core/ModuleParam.templateDetails.h>



using namespace Belle2;

REG_MODULE(TrackIsoScoreCalculator);

TrackIsoScoreCalculatorModule::TrackIsoScoreCalculatorModule() : Module()
{
  // Set module properties
  setDescription(
    R"DOC(Define a semi-continuous variable to quantify the isolation level of each standard charged particle. The definition is based on the counting of layers where a nearby track helix is found, as well as on the weight that each sub-detector has on the PID for the given particle hypothesis.  )DOC");

  // Parameter definitions
  addParam("detectors", m_detectors, "List of detectors to consider.");

}

TrackIsoScoreCalculatorModule::~TrackIsoScoreCalculatorModule()
{
}

void TrackIsoScoreCalculatorModule::initialize()
{
}

void TrackIsoScoreCalculatorModule::beginRun()
{
}

void TrackIsoScoreCalculatorModule::event()
{
}

void TrackIsoScoreCalculatorModule::endRun()
{
}

void TrackIsoScoreCalculatorModule::terminate()
{
}
