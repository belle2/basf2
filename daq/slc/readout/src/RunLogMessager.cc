#include "daq/slc/readout/RunLogMessanger.h"

using namespace Belle2;

bool RunLogMessanger::open(const std::string& path, const std::string& mode)
{
  try {
    _path = path;
    _fifo.open(path, mode);
  } catch (const IOException& e) {
    _fifo.close();
    return false;
  }
  return true;
}

bool RunLogMessanger::create(const std::string& path, const std::string& mode)
{
  try {
    _path = path;
    _fifo = Fifo::mkfifo(path);
  } catch (const IOException& e) {
    return false;
  }
  return open(path, mode);
}

void RunLogMessanger::close()
{
  _fifo.close();
}

void RunLogMessanger::unlink(const std::string& path)
{
  if (path.size() > 0) _path = path;
  _fifo.unlink(_path);
}
