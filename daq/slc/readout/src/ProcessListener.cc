#include "daq/slc/readout/ProcessListener.h"

#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void ProcessListener::run()
{
  Fork forkid = m_con->getFork();
  std::string process_name = m_con->getName();
  if (forkid.wait() < 0) {
    LogFile::fatal("Failed to wait forked process %s", process_name.c_str());
    return;
  }
  m_con->lock();
  NSMCommunicator* comm = m_con->getCallback()->getCommunicator();
  NSMNode& node(m_con->getCallback()->getNode());
  unsigned int state = m_con->getInfo().getState();
  switch (state) {
      //case RunInfoBuffer::ERROR:
    case RunInfoBuffer::RUNNING:
      LogFile::error("Forked process %s was crashed", process_name.c_str());
      comm->sendError(StringUtil::form("Foked process %s was crashed", process_name.c_str()));
      //m_con->setState(RCState::ERROR_ES);
      break;
    case RunInfoBuffer::READY:
      LogFile::warning("Forked process %s was not started", process_name.c_str());
      comm->sendError(StringUtil::form("Foked process %s was no started", process_name.c_str()));
      m_con->getInfo().reportError(RunInfoBuffer::PROCESS_DOWN);
      break;
    case RunInfoBuffer::NOTREADY:
    default:
      LogFile::debug("Forked process %s was finished", process_name.c_str());
      comm->sendLog(DAQLogMessage(node.getName(), LogFile::INFO,
                                  StringUtil::form("Foked process %s was finished",
                                                   process_name.c_str())));
      break;
  }
  m_con->unlock();
}
