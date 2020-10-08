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
