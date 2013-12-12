#include "daq/slc/readout/modules/ROMessanger.h"

using namespace Belle2;

ROMessanger::~ROMessanger()
{

}

ROMessanger::ROMessanger(const std::string& nodename,
                         int nodeid)
{
  setNode(nodename, nodeid);
}

void ROMessanger::setNode(const std::string& nodename,
                          int nodeid)
{
  _nodeid = nodeid;
  _nodename = nodename;
  _buf_path = "/run_info_buf_" + _nodename;
  _fifo_path = "/tmp/run_log_fifo_" + _nodename;
}

bool ROMessanger::create()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  _buf.open(_buf_path);
  _msg.create(_fifo_path, "r");
  return true;
}

bool ROMessanger::open()
{

  _buf.open(_buf_path);
  _msg.open(_fifo_path, "w");
  return true;
}

bool ROMessanger::close()
{
  _buf.close();
  _msg.close();
  return true;
}

bool ROMessanger::unlink()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  return true;
}

bool ROMessanger::reportReady()
{
  return _msg.send(RunLogMessanger::NOTICE, "READY");
}

bool ROMessanger::reportRunning()
{
  return _msg.send(RunLogMessanger::NOTICE, "RUNNING");
}

