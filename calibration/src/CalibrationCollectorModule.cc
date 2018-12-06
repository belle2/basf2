#include <calibration/CalibrationCollectorModule.h>
#include <framework/pcore/ProcHandler.h>

using namespace std;
using namespace Belle2;
using namespace Calibration;

CalibrationCollectorModule::CalibrationCollectorModule() :
  HistoModule(),
  m_dir(nullptr),
  m_manager(),
  m_expRunEvents()
{
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
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
           "all events are collected, but for preScale=0.5 only 50 percent will be. Since this is based on a random choice, you should set the "
           "random seed to a fixed value if you want repeatable results.\n\n"
           "Should be a float in range [0.0,1.0], default=1.0", float(1.0));

}

void CalibrationCollectorModule::initialize()
{
  m_evtMetaData.isRequired();
  REG_HISTOGRAM
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
  /** It seems that the beginRun() function is called in each basf2 subprocess when the run changes in each process.
    * This is nice because it allows us to write the new (exp,run) object creation in the beginRun function as though
    * the other processes don't exist.
    */
  // Current (Exp,Run)
  ExpRun expRun = make_pair(m_emd->getExperiment(), m_emd->getRun());
  m_runRange->add(expRun.first, expRun.second);

  // Do we care about the number of events collected in each (input data) ExpRun?
  // If so, we want to create values for the events collected map
  if (m_maxEventsPerRun > -1) {
    // Do we have a count for this ExpRun yet? If not create one
    auto i_eventsInExpRun = m_expRunEvents.find(expRun);
    if (i_eventsInExpRun == m_expRunEvents.end()) {
      m_expRunEvents[expRun] = 0;
    }

    // Set our pointer to the correct location for this ExpRun
    m_eventsCollectedInRun = &m_expRunEvents[expRun];
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
  if (m_granularity == "all") {
    m_expRun = { -1, -1};
  } else {
    m_expRun = expRun;
  }
  m_manager.createExpRunDirectories(m_expRun);
  // Run the user's startRun() implementation if there is one
  startRun();
}

void CalibrationCollectorModule::defineHisto()
{
  if (!ProcHandler::parallelProcessingUsed() || ProcHandler::isWorkerProcess()) {
    m_dir = gDirectory->mkdir(getName().c_str());
    m_manager.setDirectory(m_dir);
    B2INFO("Saving output to TDirectory " << m_dir->GetPath());
    B2DEBUG(100, "Creating directories for individual collector objects.");
    m_manager.createDirectories();
    m_runRange = new RunRange();
    m_runRange->setGranularity(m_granularity);
    m_runRange->SetName(Calibration::RUN_RANGE_OBJ_NAME.c_str());
    m_dir->Add(m_runRange);
  }
  inDefineHisto();
}

void CalibrationCollectorModule::endRun()
{
  closeRun();
  // Moving between runs possibly creates new objects if getObjectPtr is called and granularity is run
  // So we should write and clear the current memory objects.
  if (m_granularity == "run") {
    ExpRun expRun = make_pair(m_emd->getExperiment(), m_emd->getRun());
    m_manager.writeCurrentObjects(expRun);
    m_manager.clearCurrentObjects(expRun);
  }
}

void CalibrationCollectorModule::terminate()
{
  finish();
  // actually this should be done by the write() called by HistoManager....

  // Haven't written objects yet if collecting with granularity == all
  // Write them now that everything is done.
//  if (m_granularity == "all") {
//    m_manager.writeCurrentObjects(m_expRun);
//    m_manager.clearCurrentObjects(m_expRun);
//  }
  m_manager.deleteHeldObjects();
}
