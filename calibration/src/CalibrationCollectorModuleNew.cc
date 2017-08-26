#include <calibration/CalibrationCollectorModuleNew.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModuleNew::CalibrationCollectorModuleNew() : HistoModule(), m_dir(nullptr)
{
}

void CalibrationCollectorModuleNew::initialize()
{
  REG_HISTOGRAM
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

void CalibrationCollectorModuleNew::defineHisto()
{
  m_dir = gDirectory;
  B2INFO("Collector Module ''" << getName() << "'' saving output to TDirectory " << m_dir->GetPath());
  inDefineHisto();
}
