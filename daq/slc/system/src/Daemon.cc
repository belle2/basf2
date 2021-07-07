/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/Daemon.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <cstring>

using namespace Belle2;

bool Daemon::start(const char* title,
                   int argc, char** argv,
                   int nargc, const char* msg)
{
  g_argc = argc;
  g_argv = argv;
  bool isdaemon = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      isdaemon = true;
      nargc++;
    } else if (strcmp(argv[i], "-h") == 0) {
      LogFile::debug("Usage : %s %s [-d]", argv[0], msg);
      return false;
    }
  }
  if (argc < nargc + 1) {
    LogFile::debug("Usage : %s %s [-d]", argv[0], msg);
    return false;
  }
  LogFile::open(StringUtil::form("%s/%s", argv[0], title));
  if (isdaemon) {
    daemon(0, 0);
  }
  return true;
}


bool Daemon::restart()
{
  return execv(g_argv[0], g_argv) != -1;
}

int Daemon::g_argc = 0;
char** Daemon::g_argv = NULL;
