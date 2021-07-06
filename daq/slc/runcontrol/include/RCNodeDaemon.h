/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_RCNodeDaemon_h
#define _Belle2_RCNodeDaemon_h

#include "daq/slc/database/DBInterface.h"

#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  class RCNodeDaemon {

  public:
    RCNodeDaemon(ConfigFile& config,
                 RCCallback* callback,
                 RCCallback* callback2 = NULL,
                 DBInterface* db = NULL);
    ~RCNodeDaemon() {}

  public:
    void run();

  private:
    std::string m_title;
    NSMNodeDaemon m_daemon;

  };

}

#endif
