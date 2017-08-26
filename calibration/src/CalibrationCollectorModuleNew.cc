#include <calibration/CalibrationCollectorModuleNew.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModuleNew::CalibrationCollectorModuleNew() : HistoModule()
{
}

void CalibrationCollectorModuleNew::initialize()
{
  prepare();
}


void CalibrationCollectorModuleNew::event()
{
  collect();
}

void CalibrationCollectorModuleNew::beginRun()
{
  // Run the user's startRun() implementation if there is one
  startRun();
}

void CalibrationCollectorModuleNew::defineHistos()
{
  ;
}
