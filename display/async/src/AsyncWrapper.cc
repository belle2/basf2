/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <display/async/AsyncWrapper.h>
#include <framework/core/Path.h>

#include <framework/core/EventProcessor.h>
#include <framework/core/PathManager.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>
#include <framework/logging/LogSystem.h>

#include <signal.h>
#include <unistd.h>

#include <cstdlib>

using namespace Belle2;

bool AsyncWrapper::s_isAsync = false;
RingBuffer* AsyncWrapper::s_currentRingBuffer = NULL;

bool AsyncWrapper::newEventAvailable()
{
  if (!s_isAsync) {
    B2ERROR("AsyncWrapper::newEventAvailable() used in synchronous thread??");
    return true;
  }

  char* evtbuf = new char[EvtMessage::c_MaxEventSize];
  int size = s_currentRingBuffer->spyq((int*)evtbuf);
  delete[] evtbuf;

  return (size != 0);

}

void AsyncWrapper::stopMainProcess()
{
  if (s_isAsync)
    kill(getppid(), SIGINT);
}


AsyncWrapper::AsyncWrapper(Module* wrapMe): Module(), m_wrappedModule(wrapMe), m_procHandler(0), m_ringBuffer(0), m_rx(0), m_tx(0)
{
  setParamList(wrapMe->getParamList()); //inherit parameters from wrapped module

  addParam("discardOldEvents", m_discardOldEvents, "Discard old events when buffer is full. If false, the main process will wait until there is enough space in the buffer. (i.e. synchronous operation)", true);
}

AsyncWrapper::~AsyncWrapper()
{
  delete m_wrappedModule;
}

void AsyncWrapper::initialize()
{
  B2INFO("Initializing AsyncWrapper...");

  m_procHandler = new ProcHandler();
  const int bufferSizeInts = 8000000; //~32M, within Ubuntu's shmmax limit
  m_ringBuffer = new RingBuffer(bufferSizeInts);
  m_rx = new RxModule(m_ringBuffer);
  m_tx = new TxModule(m_ringBuffer);
  m_tx->setBlockingInsert(!m_discardOldEvents); //actually decouple this process

  //fork out one extra process
  m_procHandler->init_EvtProc(1);
  if (m_procHandler->isEvtProc()) {
    //forked thread:
    //allow access to async parts
    s_isAsync = true;
    s_currentRingBuffer = m_ringBuffer;

    PathManager pathMgr;
    PathPtr path = pathMgr.createPath();
    path->addModule(ModulePtr(m_rx));
    path->addModule(ModulePtr(m_wrappedModule));

    //LogSystem::Instance().resetMessageCounter(); //for testing parallel processing

    EventProcessor eventProc(pathMgr);
    eventProc.process(path);
    B2INFO("Asynchronous process done!");
    exit(0);
  } else {
    //main thread: chain tx and return
    m_tx->initialize();
  }
}

void AsyncWrapper::event()
{
  if (!m_procHandler->isEvtProc()) {
    m_tx->event();
  }
}

void AsyncWrapper::terminate()
{
  if (!m_procHandler->isEvtProc()) {
    m_tx->terminate();

    B2INFO("Waiting for asynchronous process...");
    EvtMessage term(NULL, 0, MSG_TERMINATE);
    while (m_ringBuffer->insq((int*)term.buffer(), (term.size() - 1) / sizeof(int) + 1) < 0) {
      usleep(200);
    }
    m_procHandler->wait_event_processes();
    B2INFO("Done, cleaning up...");
    delete m_tx;
    delete m_rx;
    delete m_ringBuffer;
    delete m_procHandler;
  }
}
