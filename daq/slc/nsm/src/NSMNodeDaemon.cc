/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/nsm/NSMNodeDaemon.h"

#include "daq/slc/nsm/NSMCommunicator.h"
#include "daq/slc/base/TimeoutException.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>

using namespace Belle2;

void NSMNodeDaemon::add(NSMCallback* callback,
                        const std::string& host, int port)
{
  try {
    if (callback != NULL && host.size() > 0 && port > 0) {
      NSMCommunicator* com = new NSMCommunicator();
      NSMNode& node(callback->getNode());
      com->init(node, host, port);
      com->setCallback(callback);
      callback->init(*com);
      callback->alloc_open(*com);
      m_timeout = callback->getTimeout();
      if (callback->getLogNode().getName().size() == 0) {
        ConfigFile file("slowcontrol");
        callback->setLogNode(NSMNode(file.get("log.collector")));
      }
      m_callback.push_back(callback);
    }
  } catch (const std::exception& e) {
    LogFile::fatal(e.what());
    exit(1);
  }
}

void NSMNodeDaemon::run()
{
  NSMCommunicatorList& com_v(NSMCommunicator::get());
  try {
    {
      std::string nodename = com_v[0]->getCallback().getNode().getName();
      std::string filename = ("/tmp/nsmvget." + StringUtil::tolower(nodename));
      std::ifstream fin(filename.c_str());
      std::vector<std::string> nodes, vnames;
      std::string node, vname;
      while (fin >> node >> vname) {
        nodes.push_back(node);
        vnames.push_back(vname);
      }
      ::remove(filename.c_str());
      for (size_t i = 0; i < com_v.size(); i++) {
        NSMCommunicator& com(*com_v[i]);
        for (size_t j = 0; j < nodes.size(); j++) {
          com.getCallback().vget(nodes[j], vnames[j]);
        }
      }
    }
    double t0 = Time().get();
    while (true) {
      try {
        NSMCommunicator& com(NSMCommunicator::select(m_timeout));
        com.getCallback().perform(com);
      } catch (const TimeoutException& e) {}
      for (size_t i = 0; i < com_v.size(); i++) {
        NSMCommunicator& com(*com_v[i]);
        while (com.hasQueue()) {
          com.setMessage(com.popQueue());
          com.getCallback().perform(com);
        }
      }
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

