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
  RCCallback& callback(*(m_con->getCallback()));
  if (process.wait() < 0) {
    callback.log(LogFile::FATAL, "Failed to wait processed process %s",
                 process_name.c_str());
    return;
  }
  LogFile::debug(process_name + " : termineted");
  process.set_id(-1);
  callback.set(m_con->getParName() + ".pid", -1);
  try {
    callback.monitor();
  } catch (const RCHandlerFatalException& e) {
    callback.log(LogFile::FATAL, e.what());
  } catch (const RCHandlerException& e) {
    callback.log(LogFile::ERROR, e.what());
  } catch (const std::exception& e) {
    callback.log(LogFile::FATAL, "Unknown exception: %s. terminating process", e.what());
  }
}
