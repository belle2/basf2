#include "CDC_TESTHistoManager.hh"

extern "C" void* createCDC_TESTHistoManager()
{
  return new B2DQM::CDC_TESTHistoManager();
}

