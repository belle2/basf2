#ifndef _Belle2_nsm_read_argv_h
#define _Belle2_nsm_read_argv_h

#include <daq/slc/base/ConfigFile.h>

namespace Belle2 {

  int nsm_read_argv(int argc, const char** argv,
                    int (*help)(const char** argv), char** argv_in,
                    ConfigFile& config, std::string& name,
                    std::string& username, int nargv);

}

#endif
