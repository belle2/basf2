/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/AsyncWrapper.h>
#include <framework/core/Path.h>

#include <framework/core/EventProcessor.h>
#include <framework/core/ModuleManager.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>
#include <framework/logging/LogSystem.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>

using namespace Belle2;

bool AsyncWrapper::s_isAsync = false;
RingBuffer* AsyncWrapper::s_currentRingBuffer = NULL;
namespace {
  static std::vector<RingBuffer*> rbList;
  void cleanupIPC()
  {
    if (!AsyncWrapper::isAsync()) {
      for (RingBuffer* rb : rbList)
        delete rb;
      rbList.clear();
    }
  }
}

int AsyncWrapper::numAvailableEvents()
{
  if (!s_isAsync) {
    B2ERROR("AsyncWrapper::numAvailableEvents() used in synchronous thread??");
    return true;
  }

  return s_currentRingBuffer->numq();
}

AsyncWrapper::AsyncWrapper(const std::string& moduleType): Module(), m_ringBuffer(0), m_rx(0), m_tx(0)
{
  m_wrappedModule = ModuleManager::Instance().registerModule(moduleType);
  setParamList(m_wrappedModule->getParamList()); //inherit parameters from wrapped module

  addParam("discardOldEvents", m_discardOldEvents,
           "Discard old events when buffer is full. If false, the main process will wait until there is enough space in the buffer. (i.e. synchronous operation)",
           true);
}

AsyncWrapper::~AsyncWrapper()
{
}

void AsyncWrapper::initialize()
{
  B2INFO("Initializing AsyncWrapper...");

  ProcHandler::initialize(1);
  const int bufferSizeInts = 8000000; //~32M, within Ubuntu's shmmax limit
  m_ringBuffer = new RingBuffer(bufferSizeInts);
  rbList.push_back(m_ringBuffer);
  m_rx = new RxModule(m_ringBuffer);
  m_rx->disableMergeableHandling();
  m_tx = new TxModule(m_ringBuffer);
  m_tx->disableMergeableHandling();
  m_tx->setBlockingInsert(!m_discardOldEvents); //actually decouple this process

  //fork out one extra process
  if (ProcHandler::startWorkerProcesses(1)) {
    //forked thread:
    //allow access to async parts
    s_isAsync = true;
    s_currentRingBuffer = m_ringBuffer;

    PathPtr path(new Path);
    path->addModule(ModulePtr(m_rx));
    path->addModule(m_wrappedModule);

    //LogSystem::Instance().resetMessageCounter(); //for testing parallel processing

    EventProcessor eventProc;
    eventProc.process(path);
    B2INFO("Asynchronous process done!");
    exit(0);
  } else {
    atexit(cleanupIPC);

    //main thread: chain tx and return
    m_tx->initialize();
  }
}

void AsyncWrapper::event()
{
  if (!ProcHandler::isWorkerProcess()) {
    if (waitpid(-1, NULL, WNOHANG) != 0) {
      StoreObjPtr<EventMetaData> eventMetaData;
      eventMetaData->setEndOfData();
    }

    m_tx->event();
  }
}

void AsyncWrapper::terminate()
{
  if (!ProcHandler::isWorkerProcess()) {
    m_tx->terminate();

    m_ringBuffer->kill();
    B2INFO("Waiting for asynchronous process...");
    // TODO:
    ProcHandler::waitForAllProcesses();
    B2INFO("Done, cleaning up...");
    delete m_tx;
    delete m_rx;
    delete m_ringBuffer;
    for (RingBuffer*& rb : rbList)
      if (rb == m_ringBuffer)
        rb = nullptr;
  }
}
