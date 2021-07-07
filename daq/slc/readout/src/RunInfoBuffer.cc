/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/readout/RunInfoBuffer.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/LockGuard.h>

#include <cstring>
#include <cstdlib>

using namespace Belle2;

size_t RunInfoBuffer::size()
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
  LogFile::debug("Initialized %s", m_path.c_str());
  return true;
}

void RunInfoBuffer::clear()
{
  if (m_info == NULL) return;
  MLockGuard lockGuard(m_mutex);
  memset(m_info, 0, sizeof(ronode_info));
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

bool RunInfoBuffer::lock()
{
  if (m_info == NULL) return false;
  return m_mutex.lock();
}

bool RunInfoBuffer::unlock()
{
  if (m_info == NULL) return false;
  return m_mutex.unlock();
}

bool RunInfoBuffer::wait()
{
  if (m_info == NULL) return false;
  return m_cond.wait(m_mutex);
}

bool RunInfoBuffer::wait(int time)
{
  if (m_info == NULL) return false;
  return m_cond.wait(m_mutex, time, 0);
}

bool RunInfoBuffer::notify()
{
  if (m_info == NULL) return false;
  return m_cond.broadcast();
}

bool RunInfoBuffer::waitRunning(int timeout)
{
  if (m_info == NULL) return false;
  MLockGuard lockGuard(m_mutex);
  if (getState() != RunInfoBuffer::RUNNING) {
    if (!wait(timeout)) {
      return false;
    }
  }
  return true;
}

bool RunInfoBuffer::waitReady(int timeout)
{
  if (m_info == NULL) return false;
  MLockGuard lockGuard(m_mutex);
  if (getState() != RunInfoBuffer::READY &&
      getState() != RunInfoBuffer::RUNNING) {
    if (!wait(timeout)) {
      if (getState() != RunInfoBuffer::READY &&
          getState() != RunInfoBuffer::RUNNING) {
        return false;
      }
    }
  }
  return true;
}

bool RunInfoBuffer::reportRunning()
{
  if (m_info == NULL) return false;
  MLockGuard lockGuard(m_mutex);
  setState(RunInfoBuffer::RUNNING);
  notify();
  return true;
}

bool RunInfoBuffer::reportError(EFlag eflag)
{
  if (m_info == NULL) return false;
  MLockGuard lockGuard(m_mutex);
  setErrorFlag(eflag);
  notify();
  return true;
}

bool RunInfoBuffer::reportReady()
{
  if (m_info == NULL) return false;
  MLockGuard lockGuard(m_mutex);
  setState(RunInfoBuffer::READY);
  notify();
  return true;
}

bool RunInfoBuffer::reportNotReady()
{
  if (m_info == NULL) return false;
  MLockGuard lockGuard(m_mutex);
  setState(RunInfoBuffer::NOTREADY);
  notify();
  return true;
}

void RunInfoBuffer::copyEventHeader(int* buf)
{
  if (m_info == NULL) return;
  memcpy(&(m_info->header), buf, sizeof(event_header));
}
