#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Fork.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

void ProcessListener::run()
{
  Fork& fork(m_con->getFork());
  int pid = fork.get_id();
  std::string process_name = m_con->getName();
  if (fork.wait() < 0) {
    LogFile::fatal("Failed to wait forked process %s", process_name.c_str());
    return;
  }
  m_con->lock();
  RCCallback& callback(*(m_con->getCallback()));
  NSMNode& node(callback.getNode());
  if (m_con->getInfo().isAvailable()) {
    unsigned int state = m_con->getInfo().getState();
    switch (state) {
      case RunInfoBuffer::RUNNING:
        if (node.getState() == RCState::RUNNING_S ||
            node.getState() == RCState::LOADING_TS ||
            node.getState() == RCState::STARTING_TS) {
          std::string emsg = StringUtil::form("Foked process %s (pid = %d) was crashed", process_name.c_str(), pid);
          LogFile::error(emsg);
          callback.reply(NSMMessage(NSMCommand::ERROR, emsg));
          m_con->getInfo().reportError(RunInfoBuffer::PROCESS_DOWN);
        }
        break;
      case RunInfoBuffer::READY: {
        std::string emsg = StringUtil::form("Foked process %s (pid = %d) was not started", process_name.c_str(), pid);
        LogFile::error(emsg);
        callback.reply(NSMMessage(NSMCommand::ERROR, emsg));
        m_con->getInfo().reportError(RunInfoBuffer::PROCESS_DOWN);
      } break;
      case RunInfoBuffer::NOTREADY:
      default: {
        if (node.getState() == RCState::LOADING_TS) {
          std::string emsg = StringUtil::form("Foked process %s (pid = %d) was not booted", process_name.c_str(), pid);
          LogFile::error(emsg);
          callback.reply(NSMMessage(NSMCommand::ERROR, emsg));
          m_con->getInfo().reportError(RunInfoBuffer::PROCESS_DOWN);
        } else {
          std::string emsg = StringUtil::form("Forked process %s (pid = %d) was finished", process_name.c_str(), pid);
          LogFile::debug(emsg);
          DAQLogMessage lmsg(node.getName(), LogFile::INFO, emsg);
          callback.reply(NSMMessage(lmsg));
        }
      } break;
    }
  }
  fork.set_id(-1);
  m_con->unlock();
}
