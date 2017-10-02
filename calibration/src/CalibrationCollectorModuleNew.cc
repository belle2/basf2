#include <calibration/CalibrationCollectorModuleNew.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModuleNew::CalibrationCollectorModuleNew() :
  HistoModule(),
  m_dir(nullptr),
  m_manager()
{
}

void CalibrationCollectorModuleNew::initialize()
{
  StoreObjPtr<EventMetaData>::required();
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
  m_runRange->add(m_emd->getExperiment(), m_emd->getRun());

  // Run the user's startRun() implementation if there is one
  startRun();
}

void CalibrationCollectorModuleNew::defineHisto()
{
  m_dir = gDirectory->mkdir(getName().c_str());
  m_manager.setDirectory(m_dir);
  B2INFO("Saving output to TDirectory " << m_dir->GetPath());
  B2DEBUG(100, "Creating directories for individual collector objects.");
  m_manager.createDirectories();
  m_runRange = new RunRangeNew();
  m_runRange->SetName("RunRange");
  m_dir->Add(m_runRange);
  inDefineHisto();
}

void CalibrationCollectorModuleNew::endRun()
{
  closeRun();
  m_manager.writeCurrentObjects(*m_emd);
  m_manager.clearCurrentObjects(*m_emd);
}

void CalibrationCollectorModuleNew::terminate()
{
  m_manager.replaceObjects();
}
