#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/system/LogFile.h>

#include <cstring>

using namespace Belle2;

size_t RunInfoBuffer::size() throw()
{
  return m_mutex.size() + m_cond.size() +
         sizeof(unsigned int) * 5 + m_bufsize;
}

bool RunInfoBuffer::open(const std::string& nodename,
                         int bufsize, bool recreate)
{
  m_nodename = nodename;
  m_bufsize = bufsize;
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
  m_buf = (unsigned int*)buf;
  if (recreate) init();
  return true;
}

bool RunInfoBuffer::init()
{
  if (m_buf == NULL) return false;
  m_mutex.init();
  m_cond.init();
  memset(m_buf, 0, sizeof(unsigned int) * 5 + m_bufsize);
  return true;
}

void RunInfoBuffer::clear()
{
  if (m_buf == NULL) return;
  m_mutex.lock();
  memset(m_buf, 0, sizeof(unsigned int) * 5 + m_bufsize);
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
  if (m_buf == NULL) return false;
  return m_mutex.lock();
}

bool RunInfoBuffer::unlock() throw()
{
  if (m_buf == NULL) return false;
  return m_mutex.unlock();
}

bool RunInfoBuffer::wait() throw()
{
  if (m_buf == NULL) return false;
  return m_cond.wait(m_mutex);
}

bool RunInfoBuffer::wait(int time) throw()
{
  if (m_buf == NULL) return false;
  return m_cond.wait(m_mutex, time, 0);
}

bool RunInfoBuffer::notify() throw()
{
  if (m_buf == NULL) return false;
  return m_cond.broadcast();
}

bool RunInfoBuffer::waitRunning(int timeout)
{
  if (m_buf == NULL) return false;
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
  if (m_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::RUNNING);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportError()
{
  if (m_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::ERROR);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportReady()
{
  if (m_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::READY);
  notify();
  unlock();
  return true;
}

bool RunInfoBuffer::reportNotReady()
{
  if (m_buf == NULL) return false;
  lock();
  setState(RunInfoBuffer::NOTREADY);
  notify();
  unlock();
  return true;
}

