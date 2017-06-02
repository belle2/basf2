#include <calibration/CalibrationCollectorModule.h>
#include <calibration/CalibrationAlgorithm.h>

using namespace std;
using namespace Belle2;

CalibrationCollectorModule::CalibrationCollectorModule() : Module(), m_expRunEvents()
{
  addParam("granularity", m_granularity,
           "Granularity of data collection. Data is separated by runs (=run) or not separated at all (=all)", std::string("run"));

  addParam("maxEventsPerRun", m_maxEventsPerRun,
           "Maximum number of events that will be collected per run. Effectively the code in the collect() function is only "
           "run for this number of events on each run. Then the collect() function is switched off until a new "
           "run that hasn't collected the maximum yet begins. -1 is the default and means that the collector runs over all events."
           "\n\nNote that this is useful for debugging and hard limiting the number of events passed to the collected. However "
           "you should be limiting the collected data yourself! Check if your collected data object has enough entries for an algorithm "
           "to complete and then stop filling. Controlling this limit via a module param is encouraged.", int(-1));

  addParam("preScale", m_preScale,
           "This controls the rate at which events are actually passed to the collect() function. An event passing through this module "
           "will only have the collect() function run on it it passes a random selection scaled by this parameter i.e. For preScale=1.0 "
           "all events are collected, but for preScale=0.5 only 50\% will be. Since this is based on a random choice, you should set the "
           "random seed to a fixed value if you want repeatable results.\n\n"
           "Should be a float in range [0.0,1.0], default=1.0", float(1.0));
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
  // Should we collect data this event based on the number collected in the run?
  if (m_runCollectOnRun) {
    // If yes, does our preScale return true?
    if (getPreScaleChoice()) {
      collect();
      // Since we collected, do we care about incrementing the number of events collected?
      if (m_maxEventsPerRun > -1) {
        (*m_eventsCollectedInRun) += 1;
        // Now that we incremented, have we exceeded our maximum collected events in this run?
        if ((*m_eventsCollectedInRun) >= m_maxEventsPerRun) {
          // If we have, we should skip collection until further notice
          B2INFO("Reached maximum number of events processed by collector for this run ("
                 << (*m_eventsCollectedInRun)
                 << " >= "
                 << m_maxEventsPerRun
                 << "). Turning off collection.");
          m_runCollectOnRun = false;
        }
      }
    }
  }
}

void CalibrationCollectorModule::beginRun()
{
  // Which ExpRun are we in?
  StoreObjPtr<EventMetaData> emd;
  pair<int, int> exprun = {emd->getExperiment(), emd->getRun()};

  // Do we care about the number of events collected in each (input data) ExpRun?
  // If so, we want to create values for the events collected map
  if (m_maxEventsPerRun > -1) {
    // Do we have a count for this ExpRun yet? If not create one
    auto i_eventsInExpRun = m_expRunEvents.find(exprun);
    if (i_eventsInExpRun == m_expRunEvents.end())
      m_expRunEvents[exprun] = 0;

    // Set our pointer to the correct location for this ExpRun
    m_eventsCollectedInRun = &m_expRunEvents[exprun];
    // Want to reset our flag to start collection if necessary
    if ((*m_eventsCollectedInRun) < m_maxEventsPerRun) {
      B2INFO("New run has had less events than the maximum collected so far ("
             << (*m_eventsCollectedInRun)
             << " < "
             << m_maxEventsPerRun
             << "). Turning on collection.");
      m_runCollectOnRun = true;
    } else {
      B2INFO("New run has had more events than the maximum collected so far ("
             << (*m_eventsCollectedInRun)
             << " >= "
             << m_maxEventsPerRun
             << "). Turning off collection.");
      m_runCollectOnRun = false;
    }
  }

  // Granularity=all removes data spliting by runs by setting
  // always the same exp, run for calibration data objects
  if (m_granularity == "all")
    exprun = { -1, -1};

  // For getObject<> to work
  m_currentExpRun = exprun;

  // Even for granularity=all, we want to remember all runs...
  getObject<RunRange>(CalibrationAlgorithm::RUN_RANGE_OBJ_NAME).add(emd->getExperiment(), emd->getRun());

  // Run the user's startRun() implementation if there is one
  startRun();
}
