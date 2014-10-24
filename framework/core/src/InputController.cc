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
const TChain* InputController::s_chain = NULL;

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
