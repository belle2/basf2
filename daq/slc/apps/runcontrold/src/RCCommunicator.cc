#include "daq/slc/apps/runcontrold/RCCommunicator.h"

using namespace Belle2;

RCCommunicator::RCCommunicator() throw()
{

}

RCCommunicator::~RCCommunicator() throw()
{

}

bool RCCommunicator::isReady() throw()
{
  _mutex.lock();
  bool is_ready = _is_ready;
  _mutex.unlock();
  return is_ready;
}

void RCCommunicator::setReady(bool ready) throw()
{
  _mutex.lock();
  _is_ready = ready;
  _mutex.unlock();
}
