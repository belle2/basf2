/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Process.h>

using namespace Belle2;

void ProcessListener::run()
{
  Process& process(m_con->getProcess());
  std::string process_name = m_con->getParName();
  RCCallback& callback(*(m_con->getCallback()));
  if (not process.wait()) {
    callback.log(LogFile::FATAL, "Failed to wait processed process %s",
                 process_name.c_str());
    return;
  }
  callback.log(LogFile::DEBUG, process_name + " : termineted");
  callback.log(LogFile::DEBUG, "%s : waitpid_returned %d status = %d", process_name.c_str(), process.waitpid_result(),
               process.waitpid_status());
  process.set_id(-1);
  callback.set(m_con->getParName() + ".pid", -1);
  /*
  try {
    callback.monitor();
  } catch (const RCHandlerFatalException& e) {
    callback.log(LogFile::FATAL, e.what());
  } catch (const RCHandlerException& e) {
    callback.log(LogFile::ERROR, e.what());
  } catch (const std::exception& e) {
    callback.log(LogFile::FATAL, "Unknown exception: %s. terminating process", e.what());
  }
  */
}
