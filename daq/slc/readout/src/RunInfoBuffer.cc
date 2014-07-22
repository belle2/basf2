#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/system/LogFile.h>

#include <cstring>

using namespace Belle2;

size_t RunInfoBuffer::size() throw()
{
  return m_mutex.size() + m_cond.size() +
         sizeof(ronode_info);
}

bool RunInfoBuffer::open(const std::string& nodename,
                         int nodeid, bool recreate)
{
  m_nodename = nodename;
  std::string username = getenv("USER");
  m_path = "/run_info_" + username + "_" + nodename;
  //if (recreate) SharedMemory::unlink(_path);
  if (!m_memory.open(m_path, size())) {
    perror("shm_open");
    LogFile::fatal("Failed to open %s", m_path.c_str());
    return false;
  }
  char* buf = (char*)m_memory.map(0, size());
  if (buf == NULL) {
    return false;
  }
  m_mutex = MMutex(buf);
  buf += m_mutex.size();
  m_cond = MCond(buf);
  buf += m_cond.size();
  m_info = (ronode_info*)buf;
  if (recreate) init();
  if (nodeid > 0) setNodeId(nodeid);
  return true;
}

bool RunInfoBuffer::init()
{
  if (m_info == NULL) return false;
  m_mutex.init();
  m_cond.init();
  memset(m_info, 0, sizeof(ronode_info));
  return true;
}

void RunInfoBuffer::clear()
{
  if (m_info == NULL) return;
  m_mutex.lock();
  memset(m_info, 0, sizeof(ronode_info));
  m_mutex.unlock();
}

bool RunInfoBuffer::close()
{
  m_memory.close();
  return true;
}

bool RunInfoBuffer::unlink()
{
  m_memory.unlink();
  m_memory.close();
  return true;
}

bool RunInfoBuffer::lock() throw()
{
  if (m_info == NULL) return false;
  return m_mutex.lock();
}

bool RunInfoBuffer::unlock() throw()
{
  if (m_info == NULL) return false;
  return m_mutex.unlock();
}

bool RunInfoBuffer::wait() throw()
{
  if (m_info == NULL) return false;
  return m_cond.wait(m_mutex);
}

bool RunInfoBuffer::wait(int time) throw()
{
  if (m_info == NULL) return false;
  return m_cond.wait(m_mutex, time, 0);
}

bool RunInfoBuffer::notify() throw()
{
  if (m_info == NULL) return false;
  return m_cond.broadcast();
}

bool RunInfoBuffer::waitRunning(int timeout)
{
  if (m_info == NULL) return false;
  lock();
  if (getState() != RunInfoBuffer::RUNNING) {
    if (!wait(timeout)) {
      unlock();
      return false;
    }
  }
  unlock();
  return true;
}

bool RunInfoBuffer::reportRunning()
{
  if (m_info == NULL) return false;
  lock();
  setState(RunInfoBuffer::RUNNING);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportError(EFlag eflag)
{
  if (m_info == NULL) return false;
  lock();
  setErrorFlag(eflag);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportReady()
{
  if (m_info == NULL) return false;
  lock();
  setState(RunInfoBuffer::READY);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportNotReady()
{
  if (m_info == NULL) return false;
  lock();
  setState(RunInfoBuffer::NOTREADY);
  notify();
  unlock();
  return true;
}

