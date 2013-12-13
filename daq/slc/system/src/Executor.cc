#include "daq/slc/system/Executor.h"

#include "daq/slc/base/Debugger.h"

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

void Executor::setExecutable(const char* format, ...)
{
  va_list ap;
  static char ss[1024];
  va_start(ap, format);
  vsprintf(ss, format, ap);
  va_end(ap);
  _path = ss;
}

void Executor::setExecutable(const std::string& path)
{
  _path = path;
}

void Executor::addArg(const char* format, ...)
{
  va_list ap;
  static char ss[1024];
  va_start(ap, format);
  vsprintf(ss, format, ap);
  va_end(ap);
  _arg_v.push_back(ss);
}

void Executor::addArg(const std::string& arg)
{
  _arg_v.push_back(arg);
}

bool Executor::execute()
{
  char* argv[30];
  argv[0] = (char*)_path.c_str();
  std::cout << "" << argv[0] << " ";
  for (size_t i = 0; i < _arg_v.size(); i++) {
    argv[i + 1] = (char*)_arg_v[i].c_str();
    std::cout << "" << argv[i + 1] << " ";
  }
  std::cout << std::endl;
  argv[_arg_v.size() + 1] = NULL;
  if (execvp(_path.c_str(), argv) == -1) {
    Belle2::debug("Faield to start receiver basf2 script");
    return false;
  }

  return true;
}
