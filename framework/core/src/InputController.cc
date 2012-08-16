#include <framework/core/InputController.h>

using namespace Belle2;

bool InputController::s_canControlInput = false;
long InputController::s_nextEntry = -1;
long InputController::s_nextExperiment = -1;
long InputController::s_nextRun = -1;
long InputController::s_nextEvent = -1;
long InputController::s_numEntries = 0;
long InputController::s_currentEntry = 0;
