#include "daq/slc/readout/ProcessStatusBuffer.h"

#include "daq/slc/base/Debugger.h"

using namespace Belle2;

ProcessStatusBuffer::~ProcessStatusBuffer()
{

}

ProcessStatusBuffer::ProcessStatusBuffer(const std::string& nodename,
                                         int nodeid)
{
  setNode(nodename, nodeid);
}

void ProcessStatusBuffer::setNode(const std::string& nodename,
                                  int nodeid)
{
  _nodeid = nodeid;
  _nodename = nodename;
  _buf_path = "/run_info_buf_" + _nodename;
  _fifo_path = "/run_log_fifo_" + _nodename;
}

bool ProcessStatusBuffer::create()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  _buf.open(_buf_path);
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

bool ProcessStatusBuffer::reportReady()
{
  return _msg.send(ProcessLogBuffer::NOTICE, "READY");
}

bool ProcessStatusBuffer::reportRunning()
{
  return _msg.send(ProcessLogBuffer::NOTICE, "RUNNING");
}

bool ProcessStatusBuffer::reportError(const std::string message)
{
  return _msg.send(ProcessLogBuffer::ERROR, message);
}

bool ProcessStatusBuffer::reportFatal(const std::string message)
{
  return _msg.send(ProcessLogBuffer::FATAL, message);
}
