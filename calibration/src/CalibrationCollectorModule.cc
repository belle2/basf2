#include <calibration/CalibrationCollectorModule.h>
#include <calibration/CalibrationAlgorithm.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModule::CalibrationCollectorModule()
{
  addParam("granularity", m_granularity,
           "Granularity of data collection. Data is separated by runs (=run) or not separated at all (=all)", std::string("run"));

  addParam("maxEventsPerRun", m_maxEventsPerRun,
           "Maximum number of events that will be collected per run. Effectively the code in the collect() function is only "
           "run for this number of events at the start of each run. Then the collect() function is switched off until a new "
           "run begins. -1 is the default and means that the collector runs over all events.", int(-1));
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
  // Should we be collecting?
  if (m_runCollect) {
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

    // Do we care about the number of events collected?
    if (m_maxEventsPerRun > -1) {
      m_eventNumInRun += 1;
      // If so, have we exceeded our maximum collected events in this run?
      if (m_eventNumInRun >= m_maxEventsPerRun) {
        // If we have, we should skip collection until further notice
        m_runCollect = false;
      }
    }
  }
}

void CalibrationCollectorModule::beginRun()
{
  // Want to reset our counters/flags to start collection on a fresh run
  m_eventNumInRun = 0;
  m_runCollect = true;
  prepareRun();
}
