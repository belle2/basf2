/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/core/SteerRootInputModule.h>

#include <framework/core/Environment.h>
#include <framework/core/InputController.h>


using namespace Belle2;

//REG_MODLUE needed for --execute-path functionality
//Note: should not appear in module list since we're not in the right directory
REG_MODULE(SteerRootInput)

SteerRootInputModule::SteerRootInputModule() : Module()
{
  setDescription("Internal module used by Path.add_independent_merge_path(). This shouldn't appear in 'basf2 -m' output. If it does, check REG_MODULE() handling.");

  // ATTENTION! This module is NOT SAFE for parallel processing (do not set flag c_ParallelProcessingCertified)

  addParam("eventMixing", m_eventMixing, "merge each event of main path with each event of independent path", false);
}

SteerRootInputModule::~SteerRootInputModule() = default;

void SteerRootInputModule::init(bool eventMixing)
{
  m_eventMixing = eventMixing;
}

void SteerRootInputModule::initialize()
{
  if (m_eventMixing && InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0
      && InputController::getNextEvent() >= 0) {
    B2ERROR("Event mixing not possible if you want to skip to a certain exp/run/evt with the RootInputModule."
            " Please skip to a certain entry in the File instead (use parameter 'skipNEvents').");
  }

  // Tell the InputController/Environment how many events we want to process
  std::pair<long, long> numEntries = { InputController::numEntries(false), InputController::numEntries(true)};
  std::pair<long, long> skippedEntries = { InputController::getSkippedEntries(false), InputController::getSkippedEntries(true)};
  long evtsToProcess = -1;
  if (m_eventMixing) {
    InputController::enableEventMixing();
    // we want to be able to process each event of first path along with each event of second path
    evtsToProcess = (numEntries.first - skippedEntries.first) * (numEntries.second - skippedEntries.second);
  } else {
    // we want to end processing if one of the paths runs out of events
    evtsToProcess = std::min((numEntries.first - skippedEntries.first), (numEntries.second - skippedEntries.second));
  }
  if (Environment::Instance().getNumberEventsOverride() == 0 || evtsToProcess < Environment::Instance().getNumberEventsOverride()) {
    Environment::Instance().setNumberEventsOverride(evtsToProcess);
  }
}

void SteerRootInputModule::event()
{
  if (!m_eventMixing) {
    return;
  }

  // We expect that both RootInputModules called InputController::eventLoaded(...) so
  // both indices should be -1
  if (InputController::getNextEntry(false) != -1 && InputController::getNextEntry(true) != -1) {
    B2FATAL("This should not happen. Expected that both paths were processed at this point.");
  }

  std::pair<long, long> numEntries = { InputController::numEntries(false), InputController::numEntries(true)};
  std::pair <long, long> currEntries = { InputController::getCurrentEntry(false), InputController::getCurrentEntry(true)};

  if (currEntries.first >= numEntries.first) {
    B2FATAL("Reached end of file (main path). This should not happen.");
  }
  if (currEntries.second >= numEntries.second) {
    B2FATAL("Reached end of file (independent path). This should not happen.");
  }

  // Process events in order (0,0), (0,1), ..., (0,n), (1,0), (1,1), ...
  // This module is executed AFTER the RootInputModules
  // So set indices for next call of event()
  // First pair of events (0,0) already processed when this method is called for the first time
  std::pair <long, long> nextEntries = {currEntries.first, currEntries.second + 1};
  if (nextEntries.second == numEntries.second) {
    nextEntries.second = InputController::getSkippedEntries(true);
    ++nextEntries.first;
  }

  // next event for main path
  InputController::setNextEntry(nextEntries.first, false);
  // and next event for independent path
  InputController::setNextEntry(nextEntries.second, true);
}
