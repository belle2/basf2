/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/roisend/RoiSenderCallback.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/psql/PostgreSQLInterface.h>
#include <daq/slc/runcontrol/RCNodeDaemon.h>
#include <daq/slc/system/Daemon.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RCNodeDaemon(config, new RoiSenderCallback(), NULL,
                 new PostgreSQLInterface(config.get("database.host"),
                                         config.get("database.dbname"),
                                         config.get("database.user"),
                                         config.get("database.password"),
                                         config.getInt("database.port"))).run();
  }
  return 0;
}
