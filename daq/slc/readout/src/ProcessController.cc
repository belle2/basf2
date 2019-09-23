#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/LogListener.h"
#include "daq/slc/runcontrol/RCHandlerException.h"

#include "daq/slc/system/Executor.h"
#include "daq/slc/system/PThread.h"
#include "daq/slc/system/LogFile.h"

#include "daq/slc/base/StringUtil.h"
#include <daq/slc/system/LockGuard.h>

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
  m_callback->add(new NSMVHandlerInt(m_parname + ".pid", true, false, 0), false, false);
  return true;
}

void ProcessController::clear()
{
  m_info.clear();
}

bool ProcessController::waitReady(int timeout)
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
  m_process.kill(SIGABRT);
  if (pipe(m_iopipe) < 0) {
    perror("pipe");
    return false;
  }
  m_process = Process(new ProcessSubmitter(this, m_iopipe));
  m_th_log = PThread(new LogListener(this, m_iopipe));
  m_th_process = PThread(new ProcessListener(this));
  //close(iopipe[1]);
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
  GenericLockGuard<RunInfoBuffer> lockGuard(m_info);
  m_info.setExpNumber(expno);
  m_info.setRunNumber(runno);
  m_info.setSubNumber(0);
  /*
  if (m_info.getState() != RunInfoBuffer::RUNNING) {
    throw (RCHandlerException(m_parname + " is not running"));
  }
  */
  return true;
}

bool ProcessController::stop()
{
  GenericLockGuard<RunInfoBuffer> lockGuard(m_info);
  m_info.setExpNumber(0);
  m_info.setRunNumber(0);
  m_info.setSubNumber(0);
  m_info.setInputCount(0);
  m_info.setInputNBytes(0);
  m_info.setOutputCount(0);
  m_info.setOutputNBytes(0);
  return true;
}

bool ProcessController::pause()
{
  GenericLockGuard<RunInfoBuffer> lockGuard(m_info);
  if (m_info.isRunning()) {
    m_info.setState(RunInfoBuffer::PAUSED);
  } else {
    LogFile::warning("Process is not running. Pause request was ignored.");
  }
  return true;
}

bool ProcessController::resume()
{
  GenericLockGuard<RunInfoBuffer> lockGuard(m_info);
  if (m_info.isPaused()) {
    m_info.setState(RunInfoBuffer::RUNNING);
  } else {
    LogFile::warning("Process is not paused. Resume request was ignored.");
  }
  return true;
}

bool ProcessController::abort(unsigned int timeout)
{
  m_info.clear();
  m_process.kill(SIGINT);

  bool sigintWasSuccessful = false;
  for (unsigned int timer = 0; timer < timeout; ++timer) {
    if (not m_process.isAlive()) {
      sigintWasSuccessful = true;
      break;
    } else {
      usleep(1000000);
    }
  }

  if (sigintWasSuccessful) {
    LogFile::debug("Process successfully killed with SIGINT.");
  } else {
    LogFile::warning("Process could not be killed with SIGINT, sending SIGABRT.");
    m_process.kill(SIGABRT);
  }

  if (m_callback != NULL)
    m_callback->set(m_parname + ".pid", -1);
  m_process.wait();
  m_th_log.cancel();
  m_th_process.cancel();
  close(m_iopipe[1]);
  close(m_iopipe[0]);
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
