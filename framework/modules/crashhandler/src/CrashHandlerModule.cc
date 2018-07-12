/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/crashhandler/CrashHandlerModule.h>

#include <framework/core/ModuleManager.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>
#include <framework/datastore/StoreObjPtr.h>

#include <sys/wait.h>
#include <unistd.h>

#include <map>
#include <cstring>
#include <cstdlib>

using namespace Belle2;

REG_MODULE(CrashHandler)

namespace {
  std::map<std::pair<pid_t, CrashHandlerModule*>, RingBuffer*> s_pidToRingbuffer;
  void cleanupIPC()
  {
    const pid_t pid = getpid();
    for (auto& r : s_pidToRingbuffer) {
      if (r.first.first == pid) {
        delete r.second;
        r.second = nullptr;
      }
    }
  }

  //TODO not sure if this is necessary in this case.
  //TODO should probably be used in SubEventModule, though.
  /** Helper class to not screw up statistics for this module in the current block. */
  class FixStatisticsContext {
  public:
    FixStatisticsContext():
      m_processStatistics("", DataStore::c_Persistent)
    {
      m_processStatistics->suspendGlobal();
    }
    ~FixStatisticsContext()
    {
      m_processStatistics->startModule();
      m_processStatistics->resumeGlobal();
    }
  private:
    StoreObjPtr<ProcessStatistics> m_processStatistics;
  };
}

CrashHandlerModule::CrashHandlerModule():
  Module(),
  EventProcessor(),
  m_procHandler(1, true)
{
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);
  setDescription(R"DOCSTRING(
Internal module for graceful crash handling. If any of the wrapped modules
abort, crash, etc., execution will continue after this module. The return value
of this module is set to 0 (False) in case of aborts (the DataStore will not
contain any output from the wrapped modules); and to 1(true) if all modules
executed normally.

.. warning:: This is a stopgap measure to avoid stopping the HLT because of
    software errors and has severe performance penalties.  It does not absolve
    you of the responsibility to fix your software.)DOCSTRING");

  addParam("path", m_path,
           "Path with crash-prone modules. All modules need the ability to run "
           "in parallel (`PARALLELPROCESSINGCERTIFIED <ModulePropFlags.PARALLELPROCESSINGCERTIFIED>` flag "
           "set using `Module.set_property_flags`).");
}

CrashHandlerModule::~CrashHandlerModule()
{
}

void CrashHandlerModule::initialize()
{
  m_nEvents = 0;
  m_nCrashedEvents = 0;
  m_moduleList = m_path->buildModulePathList();

  B2WARNING("Please note: The CrashHandler module is a stopgap measure to avoid stopping the HLT because of software errors. It does not absolve you of the responsibility to fix your software.");
  if (!m_path) {
    B2FATAL("CrashHandlerModule: path not set?");
  }
  auto flag = Module::c_ParallelProcessingCertified;
  if (!ModuleManager::allModulesHaveFlag(m_moduleList, flag))
    B2ERROR("CrashHandlerModule: All provided modules must have the c_ParallelProcessingCertified flag!");

  FixStatisticsContext context;

  processInitialize(m_moduleList, false);
}

void CrashHandlerModule::beginRun()
{
  m_processID = ProcHandler::EvtProcID();

  FixStatisticsContext context;
  processBeginRun();
}
void CrashHandlerModule::endRun()
{
  FixStatisticsContext context;
  processEndRun();
}

void CrashHandlerModule::event()
{
  if (!m_rx) {
    atexit(cleanupIPC);

    //late initialisation since this needs to happen in each process
    const int bufferSizeInts = 8000000; //~32M, within Ubuntu's shmmax limit
    RingBuffer* rb = new RingBuffer(bufferSizeInts);
    s_pidToRingbuffer[ {getpid(), this}] = rb;
    m_rx = new RxModule(rb);
    m_tx = new TxModule(rb);
    m_tx->disableSendRandomState();
    m_tx->initialize();
    //rx gets partial initialisation
    m_rx->initStreamer();
  }

  FixStatisticsContext context;

  auto loglevel = getLogConfig().getLogLevel();
  getLogConfig().setLogLevel(LogConfig::c_Warning); //silence 'worker process forked' messages
  m_procHandler.startWorkerProcesses();
  getLogConfig().setLogLevel(loglevel);
  if (m_procHandler.processList().empty()) { //child process
    //reset stats to handle merges correctly
    StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
    processStatistics->clear();

    //don't call processBeginRun/EndRun() again (we do that in our implementations)
    m_previousEventMetaData = *(StoreObjPtr<EventMetaData>());

    //stuff usually done in processCore()
    PathIterator moduleIter(m_path);
    B2DEBUG(100, "In child process, calling processEvent()");
    processEvent(moduleIter, false);

    m_tx->event();
    _exit(0);
  } else { //monitoring process
    B2DEBUG(100, "In monitoring process");
    bool ok = m_procHandler.waitForAllProcesses();
    setReturnValue(ok);
    m_nEvents++;
    if (ok) {
      //ok, receive event
      m_rx->readEvent();
    } else {
      m_nCrashedEvents++;
      B2WARNING("Detected crash! Please check previous messages for a stack trace. So far, crashes were encountered in " <<
                (m_nCrashedEvents * 100.0 / m_nEvents) << " % of events.");
    }
  }
}

void CrashHandlerModule::terminate()
{
  FixStatisticsContext context;

  if (m_rx)
    m_rx->terminate();
  if (m_tx)
    m_tx->terminate();
  if (!ProcHandler::parallelProcessingUsed() or m_processID == ProcHandler::EvtProcID()) {
    processTerminate(m_moduleList);
  } else {
    //we're in another process than we actually belong to, only call terminate where approriate
    ModulePtrList tmpModuleList;
    for (const ModulePtr& m : m_moduleList) {
      if (m->hasProperties(c_TerminateInAllProcesses))
        tmpModuleList.push_back(m);
    }
    processTerminate(tmpModuleList);
  }

  delete m_rx;
  delete m_tx;

  auto it = s_pidToRingbuffer.find({getpid(), this});
  if (it != s_pidToRingbuffer.end()) {
    delete it->second;
    it->second = nullptr;
  }
}

