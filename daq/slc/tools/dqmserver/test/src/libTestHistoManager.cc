#include "TestHistoManager.hh"

extern "C" void* createTestHistoManager()
{
  return new B2DQM::TestHistoManager();
}

