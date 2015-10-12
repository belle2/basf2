#include <calibration/CalibrationCollectorModule.h>
#include <calibration/CalibrationAlgorithm.h>

using namespace std;
using namespace Belle2;
using namespace calibration;

CalibrationCollectorModule::CalibrationCollectorModule()
{
  addParam("prefix", m_prefix,
           "Prefix for calibration dataobjets in datastore for this module (used also by algorithms to find data). If empty (default), will be set to module default name",
           std::string(""));
  addParam("granularity", m_granularity,
           "Granularity of data collection. Data is separated by runs (=run) or not separated at all (=all)", std::string("run"));
}



void CalibrationCollectorModule::initialize()
{
  if (m_prefix == "")
    m_prefix = getType();

  if (m_granularity != "run" && m_granularity != "all")
    B2ERROR("Invalid granularity option provided: '" << m_granularity << "' Allowed options are: 'run' or 'all'");

  registerObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME, new RunRange());

  prepare();
}


void CalibrationCollectorModule::event()
{
  StoreObjPtr<EventMetaData> emd;
  std::pair<int, int> exprun = {emd->getExperiment(), emd->getRun()};

  // Granularity=all removes data spliting by runs by setting
  // always the same exp, run for calibration data objects
  if (m_granularity == "all")
    exprun = std::make_pair(-1, -1);

  if (exprun != m_currentExpRun) {
    m_currentExpRun = exprun;
    // Even for granularity=all, we want to remember all runs...
    getObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME).add(emd->getExperiment(), emd->getRun());
  } else {
    // This ensures the object is created for each collect() call
    // Just to be sure...
    getObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME);
  }

  collect();
}
