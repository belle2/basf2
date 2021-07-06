/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Daemon_hh
#define _Daemon_hh

namespace Belle2 {

  class Daemon {

  public:
    static bool start(const char* title,
                      int argc, char** argv,
                      int nargc, const char* msg);

    static bool restart();

  private:
    static int g_argc;
    static char** g_argv;

  };

};

#endif
