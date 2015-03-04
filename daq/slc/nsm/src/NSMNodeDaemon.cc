#include "daq/slc/nsm/NSMNodeDaemon.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

void NSMNodeDaemon::add(NSMCallback* callback,
                        const std::string& host, int port)
{
  if (callback != NULL && host.size() > 0 && port > 0) {
    NSMCommunicator* com = new NSMCommunicator();
    NSMNode& node(callback->getNode());
    com->init(node, host, port);
    com->setCallback(callback);
    callback->init(*com);
    callback->alloc_open(*com);
    if (m_timeout == 0) m_timeout = callback->getTimeout();
    m_callback.push_back(callback);
  }
}

void NSMNodeDaemon::run() throw()
{
  NSMCommunicatorList& com_v(NSMCommunicator::get());
  try {
    double t0 = Time().get();
    while (true) {
      try {
        NSMCommunicator& com(NSMCommunicator::select(m_timeout));
        com.getCallback().perform(com);
      } catch (const TimeoutException& e) {}
      double t = Time().get();
      if (t - t0 >= m_timeout) {
        for (size_t i = 0; i < com_v.size(); i++) {
          NSMCommunicator& com(*com_v[i]);
          com.getCallback().timeout(com);
          com.getCallback().alloc_open(com);
        }
        t0 = t;
      }
    }
  } catch (const std::exception& e) {
    LogFile::fatal("NSM node brdge : Caught exception %s\n"
                   "Terminate process...", e.what());
  }
  for (size_t i = 0; i < com_v.size(); i++) {
    NSMCommunicator& com(*com_v[i]);
    com.getCallback().term();
  }
}

