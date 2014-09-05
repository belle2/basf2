#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Fork.h>

#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

void ProcessListener::run()
{
  Fork& fork(m_con->getFork());
  std::string process_name = m_con->getName();
  if (fork.wait() < 0) {
    LogFile::fatal("Failed to wait forked process %s", process_name.c_str());
    return;
  }
  m_con->lock();
  NSMCommunicator* comm = m_con->getCallback()->getCommunicator();
  NSMNode& node(m_con->getCallback()->getNode());
  if (m_con->getInfo().isAvailable()) {
    unsigned int state = m_con->getInfo().getState();
    if (node.getState() == RCState::RUNNING_S ||
        node.getState() == RCState::STARTING_TS) {
      switch (state) {
        case RunInfoBuffer::RUNNING:
          LogFile::error("Forked process %s was crashed", process_name.c_str());
          node.setState(RCState::RECOVERING_RS);
          comm->sendError(StringUtil::form("Foked process %s was crashed", process_name.c_str()));
          m_con->getInfo().reportError(RunInfoBuffer::PROCESS_DOWN);
          break;
        case RunInfoBuffer::READY:
          LogFile::warning("Forked process %s was not started", process_name.c_str());
          node.setState(RCState::RECOVERING_RS);
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
    }
  }
  fork.set_id(-1);
  m_con->unlock();
}
