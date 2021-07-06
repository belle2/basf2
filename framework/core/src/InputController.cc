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
const TChain* InputController::s_chain = nullptr;

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
  if (s_chain)
    return s_chain->GetEntries();

  return 0;
}
