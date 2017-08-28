#include <calibration/CalibrationCollectorModuleNew.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModuleNew::CalibrationCollectorModuleNew() : HistoModule(), m_dir(nullptr), m_object(nullptr)
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
  /** It seems that the beginRun() function is called in each basf2 subprocess when the run changes in each process.
    * This is nice because it allows us to write the new (exp,run) object creation in the beginRun function as though
    * the other processes don't exist.
    */

  // Run the user's startRun() implementation if there is one
  startRun();
}

void CalibrationCollectorModuleNew::defineHisto()
{
  m_dir = gDirectory->mkdir(getName().c_str());
  B2INFO("Collector Module ''" << getName() << "'' saving output to TDirectory " << m_dir->GetPath());
  inDefineHisto();
}
