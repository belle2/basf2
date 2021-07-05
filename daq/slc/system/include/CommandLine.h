/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <string>
#include <stdio.h>
#include <string.h>

namespace Belle2 {
  class CommandLine {
  public:
    CommandLine() {}; // Constructor
    ~CommandLine() {}; // Destructor

    static int run_shell(const std::string& cmdstr, std::string& respstr);

  };

}

#endif
