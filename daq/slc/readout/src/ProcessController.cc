#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/LogListener.h"

#include "daq/slc/system/Executor.h"
#include "daq/slc/system/PThread.h"
#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/StringUtil.h"

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

bool ProcessController::init(const std::string& parname, int nodeid)
{
  m_name = StringUtil::tolower(m_callback->getNode().getName());
  m_parname = parname;
  LogFile::open(m_name + "_" + m_parname);
  if (!m_info.open(m_name + "_" + m_parname, nodeid, true)) {
    return false;
  }
  m_callback->add(new NSMVHandlerInt(m_parname + ".pid", true, false, 0));
  return true;
}

void ProcessController::clear()
{
  m_info.clear();
}

bool ProcessController::waitReady(int timeout) throw()
{
  if (!m_info.waitReady(timeout)) {
    return false;
  }
  return true;
}

bool ProcessController::load(int timeout)
{
  m_info.clear();
  m_process.cancel();
  m_process.kill(SIGQUIT);
  int iopipe[2];
  if (pipe(iopipe) < 0) {
    perror("pipe");
    return false;
  }
  m_process = Process(new ProcessSubmitter(this, iopipe));
  PThread(new LogListener(this, iopipe));
  PThread(new ProcessListener(this));
  close(iopipe[1]);
  if (timeout > 0) {
    if (!m_info.waitReady(timeout)) {
      throw (RCHandlerException("Failed to boot " + m_parname));
      return false;
    }
  }
  m_callback->set(m_parname + ".pid", m_process.get_id());
  return true;
}

bool ProcessController::start(int expno, int runno)
{
  m_info.lock();
  m_info.setExpNumber(expno);
  m_info.setRunNumber(runno);
  m_info.setSubNumber(0);
  /*
  if (m_info.getState() != RunInfoBuffer::RUNNING) {
    m_info.unlock();
    throw (RCHandlerException(m_parname + " is not running"));
  }
  */
  m_info.unlock();
  return true;
}

bool ProcessController::stop()
{
  m_info.lock();
  m_info.setExpNumber(0);
  m_info.setRunNumber(0);
  m_info.setSubNumber(0);
  m_info.setInputCount(0);
  m_info.setInputNBytes(0);
  m_info.setOutputCount(0);
  m_info.setOutputNBytes(0);
  m_info.unlock();
  return true;
}

bool ProcessController::pause()
{
  m_info.lock();
  if (m_info.isRunning()) {
    m_info.setState(RunInfoBuffer::PAUSING);
  } else {
    LogFile::warning("Process is not running. Pause request was ignored.");
  }
  m_info.unlock();
  return true;
}

bool ProcessController::resume()
{
  m_info.lock();
  if (m_info.isPaused()) {
    m_info.setState(RunInfoBuffer::RESUMING);
  } else {
    LogFile::warning("Process is not paused. Resume request was ignored.");
  }
  m_info.unlock();
  return true;
}

bool ProcessController::abort()
{
  m_info.clear();
  m_process.kill(SIGINT);
  if (getExecutable() == "basf2") {
    usleep(10000);
    m_process.kill(SIGQUIT);
    m_process.kill(SIGKILL);
  }
  if (m_callback != NULL)
    m_callback->set(m_parname + ".pid", -1);
  return true;
}

void ProcessSubmitter::run()
{
  //close(1);
  dup2(m_iopipe[1], 1);
  //close(2);
  dup2(m_iopipe[1], 2);
  close(m_iopipe[0]);
  Executor executor;
  if (m_con->getExecutable().size() == 0) {
    m_con->setExecutable("basf2");
  }
  executor.setExecutable(m_con->getExecutable());
  for (size_t i = 0; i < m_con->m_arg_v.size(); i++) {
    executor.addArg(m_con->m_arg_v[i]);
  }
  executor.execute();
}

void ProcessController::addArgument(const char* format, ...)
{
  va_list ap;
  static char ss[1024];
  va_start(ap, format);
  vsprintf(ss, format, ap);
  va_end(ap);
  m_arg_v.push_back(ss);
}
