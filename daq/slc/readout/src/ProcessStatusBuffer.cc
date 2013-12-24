#include "daq/slc/readout/ProcessStatusBuffer.h"

#include "daq/slc/base/Debugger.h"

#include <stdio.h>

using namespace Belle2;

ProcessStatusBuffer::ProcessStatusBuffer()
{
}

ProcessStatusBuffer::ProcessStatusBuffer(const std::string& nodename,
                                         int nodeid)
{
  setNode(nodename, nodeid);
}

ProcessStatusBuffer::~ProcessStatusBuffer()
{

}

void ProcessStatusBuffer::setNode(const std::string& nodename,
                                  int nodeid)
{
  _nodeid = nodeid;
  _nodename = nodename;
  std::string username = getenv("USER");
  _buf_path = "/run_info_buf_" + username + "_" + _nodename;
  _fifo_path = "/run_log_fifo_" + username + "_" + _nodename;
}

bool ProcessStatusBuffer::create()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  if (!_buf.open(_buf_path)) {
    perror("shm_open");
    Belle2::debug("Failed to open %s", _buf_path.c_str());
  }
  _buf.init();
  _msg.create(_fifo_path);
  return true;
}

bool ProcessStatusBuffer::open()
{
  _buf.open(_buf_path);
  _msg.open(_fifo_path);
  return true;
}

bool ProcessStatusBuffer::close()
{
  _buf.close();
  _msg.close();
  return true;
}

bool ProcessStatusBuffer::unlink()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  return true;
}

bool ProcessStatusBuffer::waitStarted()
{
  _buf.lock();
  if (_buf.getState() == 0) {
    _buf.wait();
  }
  _buf.unlock();
  return true;
}

bool ProcessStatusBuffer::isStopped()
{
  _buf.lock();
  if (_buf.getState() == 0) {
    _buf.unlock();
    return true;
  }
  _buf.unlock();
  return false;
}

bool ProcessStatusBuffer::reportReady()
{
  return _msg.send(SystemLog::NOTICE, "READY");
}

bool ProcessStatusBuffer::reportRunning()
{
  return _msg.send(SystemLog::NOTICE, "RUNNING");
}

bool ProcessStatusBuffer::reportDebug(const std::string message)
{
  return _msg.send(SystemLog::DEBUG, message);
}

bool ProcessStatusBuffer::reportInfo(const std::string message)
{
  return _msg.send(SystemLog::INFO, message);
}

bool ProcessStatusBuffer::reportNotice(const std::string message)
{
  return _msg.send(SystemLog::NOTICE, message);
}

bool ProcessStatusBuffer::reportWarning(const std::string message)
{
  return _msg.send(SystemLog::WARNING, message);
}

bool ProcessStatusBuffer::reportError(const std::string message)
{
  return _msg.send(SystemLog::ERROR, message);
}

bool ProcessStatusBuffer::reportFatal(const std::string message)
{
  return _msg.send(SystemLog::FATAL, message);
}
