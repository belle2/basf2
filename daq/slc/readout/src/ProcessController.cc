#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/readout/ProcessListener.h"
#include "daq/slc/readout/LogListener.h"

#include "daq/slc/system/Executor.h"
#include "daq/slc/system/LogFile.h"

#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace Belle2;

bool ProcessController::init(const std::string& name_in,
                             int bufsize)
{
  m_name = (name_in.size() > 0) ? name_in : m_callback->getNode().getName();
  LogFile::open(m_name);
  if (!m_info.open(m_name, bufsize, true)) {
    return false;
  }
  return true;
}

void ProcessController::clear()
{
  m_info.clear();
}

bool ProcessController::load(int timeout)
{
  m_info.clear();
  m_fork.cancel();
  int iopipe[2];
  if (pipe(iopipe) < 0) {
    perror("pipe");
  }
  m_fork = Fork(new ProcessSubmitter(this, iopipe));
  PThread(new LogListener(this, iopipe));
  PThread(new ProcessListener(this));
  close(iopipe[1]);
  if (timeout > 0) {
    if (!m_info.waitRunning(timeout)) {
      m_callback->setReply("Failed to boot " + m_name);
      return false;
    }
  }
  return true;
}

bool ProcessController::start()
{
  m_info.lock();
  NSMMessage& msg(m_callback->getMessage());
  if (msg.getNParams() > 2) {
    m_info.setExpNumber(msg.getParam(0));
    m_info.setRunNumber(msg.getParam(1));
    m_info.setSubNumber(msg.getParam(2));
  }
  //_info.setNodeId(_callback->getNode()->getData()->getId());
  if (m_info.getState() != RunInfoBuffer::RUNNING) {
    m_callback->setReply(m_name + " is not running");
    m_info.unlock();
    return false;
  }
  m_info.unlock();
  return true;
}

bool ProcessController::stop()
{
  return true;
}

bool ProcessController::abort()
{
  m_info.clear();
  m_fork.cancel();
  return true;
}

void ProcessSubmitter::run()
{
  close(1);
  dup2(m_iopipe[1], 1);
  close(2);
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
  if (m_con->getExecutable() == "basf2") {
    executor.addArg("--no-stats");
  }
  executor.execute();
}

