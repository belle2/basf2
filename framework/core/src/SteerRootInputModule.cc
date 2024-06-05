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
REG_MODULE(SteerRootInput);

SteerRootInputModule::SteerRootInputModule() : Module()
{
  setDescription("Internal module used by Path.add_independent_merge_path(). This shouldn't appear in 'basf2 -m' output. If it does, check REG_MODULE() handling.");

  // ATTENTION! This module is NOT SAFE for parallel processing (do not set flag c_ParallelProcessingCertified)

  addParam("eventMixing", m_eventMixing, "merge each event of main path with each event of independent path", false);
  // I have to set this parameter explicitly (cannot use InputController::getCurrentFileName() as we're dealing with TChains and might be at different files)
  addParam("mergeSameFile", m_mergeSameFile, "if you want to merge a file with itself, mixing evts (1,3) is the same as (3,1)",
           false);
}

SteerRootInputModule::~SteerRootInputModule() = default;

void SteerRootInputModule::init(bool eventMixing, bool mergeSameFile)
{
  m_eventMixing = eventMixing;
  m_mergeSameFile = mergeSameFile;
}

void SteerRootInputModule::initialize()
{
  m_eventMetaData_main.isRequired();
  m_eventMetaData_indep.isRequired();

  if (!m_mergedEventConsistency.isOptional()) {
    B2ERROR("Object MergedEventConsistency not found. This is not expected.");
  }

  std::pair<long, long> numEntries = { InputController::numEntries(false), InputController::numEntries(true)};
  std::pair<long, long> skippedEntries = { InputController::getSkippedEntries(false), InputController::getSkippedEntries(true)};

  if (m_mergeSameFile) {
    if (!m_eventMixing) {
      B2ERROR("It does not make sense to merge a file with itself if you're not doing event mixing.");
    } else {
      // quick check if you're really trying to mix a file with itself
      if (numEntries.first != numEntries.second) {
        B2ERROR("Are you sure you are mixing a file with itself? Inconsistent number of events found.");
      }
      // if merging a file with itself, it is only possible if you're skipping the same number of events in both of them
      if (skippedEntries.first != skippedEntries.second) {
        B2ERROR("Cannot mix file with itself if you're skipping a different number of events in both RootInputModules");
      }
    }
  }

  if (m_eventMixing && InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0
      && InputController::getNextEvent() >= 0) {
    B2ERROR("Event mixing not possible if you want to skip to a certain exp/run/evt with the RootInputModule."
            " Please skip to a certain entry in the File instead (use parameter 'skipNEvents').");
  }

  // Tell the InputController that we are controlling it
  InputController::enableEventMerging(this);
  // Tell the InputController/Environment how many events we want to process
  // These are UPPER thresholds. We use EventMetaData::setEndOfData() to end processing early if we're done
  // We don't know how many events will have to be skipped (inconsistent charge) so there is not much more we can do here
  long evtsToProcess = -1;
  if (m_eventMixing) {
    // we want to be able to process each event of first path along with each event of second path
    evtsToProcess = (numEntries.first - skippedEntries.first) * (numEntries.second - skippedEntries.second);
  } else {
    // we want to end processing if both paths are done (may skip events in one path because of unphysical combinations)
    evtsToProcess = std::max((numEntries.first - skippedEntries.first), (numEntries.second - skippedEntries.second));
  }
  if (Environment::Instance().getNumberEventsOverride() == 0 || evtsToProcess < Environment::Instance().getNumberEventsOverride()) {
    Environment::Instance().setNumberEventsOverride(evtsToProcess);
  }
}

void SteerRootInputModule::event()
{
  // We expect that both RootInputModules called InputController::eventLoaded(...) so
  // both indices should be -1 (otherwise something bad happened)
  if (InputController::getNextEntry(false) != -1 && InputController::getNextEntry(true) != -1) {
    B2FATAL("This should not happen. Expected that both paths were processed at this point.");
  }

  // Were the previous events the last one to be processed?
  // (see below for explanation why we have to do that)
  if (m_processedLastEvent) {
    m_eventMetaData_main->setEndOfData();
    setReturnValue(false);
    return;
  }

  // We want to do a few checks if the combination of events make sense
  // But the module has to be run in any case to set events, which should be processed next
  bool goodMerge = true;
  // And this will be set to the events to be processed next
  std::pair <long, long> nextEntries;

  std::pair<long, long> numEntries = { InputController::numEntries(false), InputController::numEntries(true)};
  std::pair <long, long> currEntries = { InputController::getCurrentEntry(false), InputController::getCurrentEntry(true)};

  // Make sure charge is consistent (if this option is enabled)
  if (m_mergedEventConsistency.isValid()) {
    goodMerge = m_mergedEventConsistency->getConsistent();
  }

  // Make sure an event is not merged with itself (not physical)
  if (goodMerge) {
    goodMerge = (m_eventMetaData_main->getEvent() != m_eventMetaData_indep->getEvent() ||
                 m_eventMetaData_main->getRun() != m_eventMetaData_indep->getRun() ||
                 m_eventMetaData_main->getExperiment() != m_eventMetaData_indep->getExperiment());
  }

  if (m_eventMixing) {
    // Process events in order (0,0), (0,1), ..., (0,n), (1,0), (1,1), ...
    // If merging file with itself (1,0) is the same as (0,1)
    // This module is executed AFTER the RootInputModules
    // So set indices for next call of event()
    // First pair of events (0,0) already processed when this method is called for the first time
    nextEntries = std::make_pair(currEntries.first, currEntries.second + 1);
    // if end of second file reached, load next event in first files
    if (nextEntries.second == numEntries.second) {
      ++nextEntries.first;
      if (!m_mergeSameFile) {
        nextEntries.second = InputController::getSkippedEntries(true);
      } else {
        // we made sure that the same number of events is skipped in both RootInputModules
        nextEntries.second = nextEntries.first + 1;
      }
    }
  } else {
    // only advance to next event in first file if we don't have to skip this combination of events
    if (goodMerge) {
      nextEntries = std::make_pair(currEntries.first + 1, currEntries.second + 1);
    } else {
      nextEntries = std::make_pair(currEntries.first, currEntries.second + 1);
    }
  }

  // Finally, check if we're actually done with everything
  // Well, using EventMetaData::setEndOfData() is not perfect since it will also skip the current events..
  // So we have to process one more event and set flag m_processedLastEvent (but path will be ended)
  if (nextEntries.first >= numEntries.first || nextEntries.second >= numEntries.second) {
    // Let's make this a warning since the EventProcessor also drops a warning that processing is stopped
    // but of course this behavior is expected in our case :)
    B2INFO("Reached end of file(s), so we're done. End processing.");
    m_processedLastEvent = true;
    // we need to set them to a valid number (doesn't matter what exactly it is)
    nextEntries = currEntries;
  }

  // set the events to be processed next
  InputController::setNextEntry(nextEntries.first, false);
  InputController::setNextEntry(nextEntries.second, true);
  // and set the return value (should this combination of events be further processed)
  setReturnValue(goodMerge);
}
