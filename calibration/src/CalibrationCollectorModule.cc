#include <calibration/CalibrationCollectorModule.h>
#include <calibration/CalibrationAlgorithm.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModule::CalibrationCollectorModule()
{
  addParam("granularity", m_granularity,
           "Granularity of data collection. Data is separated by runs (=run) or not separated at all (=all)", std::string("run"));
}



void CalibrationCollectorModule::initialize()
{
  if (m_granularity != "run" && m_granularity != "all")
    B2ERROR("Invalid granularity option provided: '" << m_granularity << "' Allowed options are: 'run' or 'all'");

  registerObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME, new RunRange());

  prepare();
}


void CalibrationCollectorModule::event()
{
  StoreObjPtr<EventMetaData> emd;
  pair<int, int> exprun = {emd->getExperiment(), emd->getRun()};

  // Granularity=all removes data spliting by runs by setting
  // always the same exp, run for calibration data objects
  if (m_granularity == "all")
    exprun = { -1, -1};

  // For getObject<> to work
  m_currentExpRun = exprun;

  // Even for granularity=all, we want to remember all runs...
  getObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME).add(emd->getExperiment(), emd->getRun());

  collect();
}
