#ifndef _Belle2_Executor_h
#define _Belle2_Executor_h

#include <string>
#include <vector>

namespace Belle2 {

  class Executor {

  public:
    void setExecutable(const char* format, ...);
    void setExecutable(const std::string& path);
    void addArg(const char* format, ...);
    void addArg(const std::string& arg);
    bool execute();

  private:
    std::string m_path;
    std::vector<std::string> m_arg_v;

  };

}

#endif

