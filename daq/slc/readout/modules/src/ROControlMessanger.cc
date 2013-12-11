#include "daq/slc/readout/modules/ROControlMessanger.h"

using namespace Belle2;

ROControlMessanger::~ROControlMessanger()
{

}

ROControlMessanger::ROControlMessanger(const std::string& nodename,
                                       int nodeid)
{
  setNode(nodename, nodeid);
}

void ROControlMessanger::setNode(const std::string& nodename,
                                 int nodeid)
{
  _nodeid = nodeid;
  _nodename = nodename;
  _buf_path = "/run_info_buf_" + _nodename;
  _fifo_path = "/tmp/run_log_fifo_" + _nodename;
}

bool ROControlMessanger::create()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  _buf.open(_buf_path);
  _msg.create(_fifo_path, "r");
  return true;
}

bool ROControlMessanger::open()
{

  _buf.open(_buf_path);
  _msg.open(_fifo_path, "w");
  return true;
}

bool ROControlMessanger::close()
{
  _buf.close();
  _msg.close();
  return true;
}

bool ROControlMessanger::unlink()
{
  SharedMemory::unlink(_buf_path);
  _msg.unlink(_fifo_path);
  return true;
}

bool ROControlMessanger::start(int exp_no, int cold_no, int hot_no)
{
  _buf.lock();
  _buf.setExpNumber(exp_no);
  _buf.setColdNumber(cold_no);
  _buf.setHotNumber(hot_no);
  _buf.setNodeId(_nodeid);
  _buf.setState(1);
  _buf.notify();
  _buf.unlock();
  return true;
}

bool ROControlMessanger::reportStarted()
{
  return _msg.send(RunLogMessanger::NOTICE, "START");
}

bool ROControlMessanger::reportStopped()
{
  return _msg.send(RunLogMessanger::NOTICE, "STOP");
}

bool ROControlMessanger::stop()
{
  _buf.clear();
  return true;
}

int ROControlMessanger::waitStarted(int timeout)
{
  int priority = 0;
  std::string message = _msg.recieve(priority, timeout);
  if (priority < 0) {
    return -1;
  } else if (priority == RunLogMessanger::NOTICE) {
    if (message == "START") return 1;
    else return 0;
  } else {
    return -priority;
  }
}

int ROControlMessanger::waitStopped(int timeout)
{
  int priority = 0;
  std::string message = _msg.recieve(priority, timeout);
  if (priority < 0) {
    return -1;
  } else if (priority == RunLogMessanger::NOTICE) {
    if (message == "STOP") return 1;
    else return 0;
  } else {
    return -priority;
  }
}
