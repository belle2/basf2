#include "daq/slc/system/LogFile.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Date.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstdarg>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Belle2;

bool LogFile::__opened = false;
std::string LogFile::__filepath;
std::ofstream LogFile::__stream;
unsigned int LogFile::__filesize = 0;
Mutex LogFile::__mutex;
SystemLog::Priority LogFile::__threshold;

void LogFile::open(const std::string& filename, SystemLog::Priority threshold)
{
  if (!__opened) {
    ConfigFile config("slowcontrol");
    __filepath = config.get("logfile.dir") + "/" + filename + "." +  Date().toString("%Y.%m.%d") + ".log";
    __threshold = threshold;
    __opened = true;
    open();
  }
}

void LogFile::open()
{
  if (!__opened) return;
  struct stat st;
  if (stat(__filepath.c_str(), &st) == 0) {
    __filesize = st.st_blksize;
    __stream.open(__filepath.c_str(), std::ios::out | std::ios::app);
  } else {
    __filesize = 0;
    __stream.open(__filepath.c_str(), std::ios::out);
  }
  debug("/* ---------- log file opened ---------- */");
  debug("log file : %s", __filepath.c_str());
}

void LogFile::debug(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, SystemLog::DEBUG, ap);
  va_end(ap);
}

void LogFile::info(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, SystemLog::INFO, ap);
  va_end(ap);
}

void LogFile::notice(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, SystemLog::NOTICE, ap);
  va_end(ap);
}

void LogFile::warning(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, SystemLog::WARNING, ap);
  va_end(ap);
}

void LogFile::error(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, SystemLog::ERROR, ap);
  va_end(ap);
}

void LogFile::fatal(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, SystemLog::FATAL, ap);
  va_end(ap);
}


void LogFile::put(SystemLog::Priority priority, const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, priority, ap);
  va_end(ap);
}

int LogFile::put_impl(const std::string& msg, SystemLog::Priority priority, va_list ap)
{
  __mutex.lock();
  if (__threshold > priority) {
    __mutex.unlock();
    return 0;
  }
  Date date;
  if (__filesize >= 1024 * 1024 * 2) {
    /*
    __stream.close();
    rename(__filepath.c_str(),
           (__filepath + "." + date.toString("%Y.%m.%d.%H.%M")).c_str());
    open();
    */
  }
  std::stringstream ss;
  ss << "[" << date.toString();
  std::string color = "\x1b[49m\x1b[39m";
  switch (priority) {
    case SystemLog::DEBUG:   color = "\x1b[49m\x1b[39m"; ss << "] [DEBUG] "; break;
    case SystemLog::INFO:    color = "\x1b[49m\x1b[32m"; ss << "] [INFO] "; break;
    case SystemLog::NOTICE:  color = "\x1b[49m\x1b[34m"; ss << "] [NOTICE] "; break;
    case SystemLog::WARNING: color = "\x1b[49m\x1b[35m"; ss << "] [WARNING] "; break;
    case SystemLog::ERROR:   color = "\x1b[49m\x1b[31m"; ss << "] [ERROR] "; break;
    case SystemLog::FATAL:   color = "\x1b[41m\x1b[37m"; ss << "] [FATAL] "; break;
    default:                 ss << "] [UNKNOWN] "; break;
  }
  static char s[1024 * 100];
  vsprintf(s, msg.c_str(), ap);
  ss << s << std::endl;
  std::string str = ss.str();
  std::cerr << color << str << "\x1b[49m\x1b[39m";
  if (__opened) {
    __stream << str;
    __stream.flush();
    __filesize += str.size();
  }
  __mutex.unlock();
  return (int) str.size();
}
