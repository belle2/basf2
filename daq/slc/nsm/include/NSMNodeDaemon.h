/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_NSMNodeDaemon_h
#define _Belle2_NSMNodeDaemon_h

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class NSMNodeDaemon {

  public:
    NSMNodeDaemon() : m_timeout(0) {}
    NSMNodeDaemon(NSMCallback* callback1,
                  const std::string host1, int port1,
                  NSMCallback* callback2 = NULL,
                  const std::string host2 = "", int port2 = 0)
      : m_timeout(0)
    {
      add(callback1, host1, port1);
      add(callback2, host2, port2);
    }
    virtual ~NSMNodeDaemon() {}

  public:
    void add(NSMCallback* callback, const std::string& host, int port);

  public:
    void run();

  private:
    int m_timeout;
    std::vector<NSMCallback*> m_callback;

  };

}

#endif
