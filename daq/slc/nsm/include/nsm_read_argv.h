/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
