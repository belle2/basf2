#ifndef _Belle2_Executor_h
#define _Belle2_Executor_h

#include <string>
#include <vector>

namespace Belle2 {

  class Executor {

  public:
    Executor() {}
    Executor(const std::string& path) : m_path(path) {}

  public:
    void setExecutable(const char* format, ...);
    void setExecutable(const std::string& path);
    void addArg(const char* format, ...);
    void addArg(const std::string& arg);
    void addArg(int arg) { addArg("%d", arg); }
    void addArg(short arg) { addArg("%d", arg); }
    void addArg(float arg) { addArg("%f", arg); }
    void addArg(double arg) { addArg("%f", arg); }
    bool execute(bool isdaemon = false);

  private:
    std::string m_path;
    std::vector<std::string> m_arg_v;

  };

}

#endif

