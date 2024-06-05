/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/core/InputController.h>
#include <framework/logging/Logger.h>

#include <TChain.h>
#include <TFile.h>

using namespace Belle2;

bool InputController::s_canControlInput = false;
std::pair<long, long> InputController::s_nextEntry = { -1, -1};
long InputController::s_nextExperiment = -1;
long InputController::s_nextRun = -1;
long InputController::s_nextEvent = -1;
std::pair<long, long> InputController::s_currentEntry = { 0, 0};
std::pair<long, long> InputController::s_skippedEntries = { 0, 0};
std::pair<const TChain*, const TChain*> InputController::s_chain = { nullptr, nullptr};
bool InputController::s_doEventMerging = false;
Module* InputController::s_steerRootInputModule = nullptr;

void InputController::enableEventMerging(Module* steerRootInputModule)
{
  if (s_doEventMerging) {
    B2FATAL("Event merging is already enabled, can not merge twice in the same job, aborting");
  };
  s_doEventMerging = true;
  s_steerRootInputModule = steerRootInputModule;
}


void InputController::setChain(const TChain* chain, bool independentPath)
{
  if (!independentPath) {
    s_chain.first = chain;
  } else {
    s_chain.second = chain;
  }
}

void InputController::eventLoaded(long entry, bool independentPath)
{
  if (!independentPath) s_nextEntry.first = -1;
  else s_nextEntry.second = -1;
  s_nextExperiment = -1;
  s_nextRun = -1;
  s_nextEvent = -1;
  if (!independentPath) s_currentEntry.first = entry;
  else s_currentEntry.second = entry;
}

void InputController::resetForChildProcess()
{
  s_canControlInput = false;
  s_nextEntry = { -1, -1};
  s_nextExperiment = -1;
  s_nextRun = -1;
  s_nextEvent = -1;
  s_currentEntry = { 0, 0};
  //s_chain is not touched, so numEntries() still works
}

std::string InputController::getCurrentFileName(bool independentPath)
{
  const TChain* chain = !independentPath ? s_chain.first : s_chain.second;
  if (!chain)
    return "";

  const TFile* f = chain->GetFile();
  if (!f)
    return "";

  return f->GetName();
}

long InputController::numEntries(bool independentPath)
{
  if (!independentPath) {
    if (s_chain.first)
      return s_chain.first->GetEntries();
  } else {
    if (s_chain.second)
      return s_chain.second->GetEntries();
  }

  return 0;
}

long InputController::getNumEntriesToProcess()
{
  if (!s_doEventMerging) {
    return numEntries();
  } else {
    return numEntries(true) * numEntries(false);
  }
}
