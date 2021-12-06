/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/core/InputController.h>

#include <TChain.h>
#include <TFile.h>

using namespace Belle2;

bool InputController::s_canControlInput = false;
long InputController::s_nextEntry = -1;
long InputController::s_nextExperiment = -1;
long InputController::s_nextRun = -1;
long InputController::s_nextEvent = -1;
long InputController::s_currentEntry = 0;
std::pair<long, long> InputController::s_eventNumbers = { -1, -1};
bool InputController::s_doEventMixing = false;
std::pair<long, long> InputController::s_nextEntries = {0, 0};
bool InputController::s_processedBothPaths = false;
const TChain* InputController::s_chain = nullptr;

void InputController::setChain(const TChain* chain)
{
  s_chain = chain;
  if (s_eventNumbers.first == -1) {
    s_eventNumbers.first = (long)s_chain->GetEntries();
  } else {
    s_eventNumbers.second = (long)s_chain->GetEntries();
  }
}

long InputController::getNextEntry()
{
  if (s_doEventMixing) {
    // Process events in order (0,0), (0,1), ..., (0,n), (1,0), (1,1), ...
    long nextEvent = -1;
    if (s_processedBothPaths) {
      nextEvent = s_nextEntries.first;
      if (s_nextEntries.first == s_eventNumbers.first) {
        assert(!"Reached end of both files. This should not happen.");
      }
      s_processedBothPaths = false;
    } else {
      nextEvent = s_nextEntries.second;
      ++s_nextEntries.second;
      if (s_nextEntries.second == s_eventNumbers.second) {
        s_nextEntries.second = 0;
        ++s_nextEntries.first;
      }
      s_processedBothPaths = true;
    }
    return nextEvent;
  }
  return s_nextEntry;
}

void InputController::resetForChildProcess()
{
  s_canControlInput = false;
  s_nextEntry = -1;
  s_nextExperiment = -1;
  s_nextRun = -1;
  s_nextEvent = -1;
  s_currentEntry = 0;
  //s_chain is not touched, so numEntries() still works
}

std::string InputController::getCurrentFileName()
{
  if (!s_chain)
    return "";

  const TFile* f = s_chain->GetFile();
  if (!f)
    return "";

  return f->GetName();
}

long InputController::numEntries()
{
  if (s_doEventMixing) {
    return s_eventNumbers.first * s_eventNumbers.second;
  }
  if (s_chain)
    return s_chain->GetEntries();

  return 0;
}

std::pair<long, long> InputController::numEntriesMergePaths()
{
  return s_eventNumbers;
}
