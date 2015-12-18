#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Process.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

void ProcessListener::run()
{
  Process& process(m_con->getProcess());
  std::string process_name = m_con->getParName();
  if (process.wait() < 0) {
    LogFile::fatal("Failed to wait processed process %s",
                   process_name.c_str());
    return;
  }
  RCCallback& callback(*(m_con->getCallback()));
  LogFile::debug(process_name + " : termineted");
  process.set_id(-1);
  callback.set(m_con->getParName() + ".pid", -1);
  try {
    callback.monitor();
  } catch (const RCHandlerFatalException& e) {
    LogFile::fatal(e.what());
    callback.reply(NSMMessage(NSMCommand::FATAL, e.what()));
  } catch (const RCHandlerException& e) {
    LogFile::error(e.what());
    callback.reply(NSMMessage(NSMCommand::ERROR, e.what()));
  } catch (const std::exception& e) {
    LogFile::fatal("Unknown exception: %s. terminating process", e.what());
  }
}
