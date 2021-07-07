/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/slc/nsm/nsm_read_argv.h>

#include <cstring>
#include <cstdlib>

namespace Belle2 {

  int nsm_read_argv(int argc, const char** argv,
                    int (*help)(const char** argv), char** argv_in,
                    ConfigFile& config, std::string& name,
                    std::string& username, int nargv)
  {
    name = "nsm";
    int argc_in = 0;
    username = getenv("USER");
    for (int i = 0; i < argc; i++) {
      if (strcmp(argv[i], "-g") == 0) {
        name = "nsm.global";
      } else if (strcmp(argv[i], "-h") == 0) {
        return help(argv);
      } else if (strcmp(argv[i], "-c") == 0) {
        if (i + 1 < argc && argv[i + 1][0] != '-') {
          i++;
          config.read(argv[i]);
        }
      } else if (strcmp(argv[i], "-n") == 0) {
        if (i + 1 < argc && argv[i + 1][0] != '-') {
          i++;
          username = argv[i];
        }
      } else {
        argv_in[argc_in] = new char[100];
        strcpy(argv_in[argc_in], argv[i]);
        argc_in++;
      }
    }
    if (argc_in < nargv) {
      help(argv);
      exit(1);
    }
    return argc_in;
  }

}
