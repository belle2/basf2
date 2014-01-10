#include "daq/slc/system/LogFile.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Date.h>
#include <daq/slc/base/StringUtil.h>

#include <sstream>
#include <cstdio>
#include <cstdarg>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Belle2;

std::string LogFile::__filepath;
std::fstream LogFile::__stream;
unsigned int LogFile::__filesize;
Mutex LogFile::__mutex;
SystemLog::Priority LogFile::__threshold;

void LogFile::open(const std::string& filename, SystemLog::Priority threshold)
{
  ConfigFile config("slowcontrol");
  __filepath = config.get("LOGFILE_DIR") + "/" + filename + ".log";
  __threshold = threshold;
  open();
}

void LogFile::open()
{
  struct stat st;
  if (stat(__filepath.c_str(), &st)) {
    __filesize = st.st_blksize;
    __stream.open(__filepath.c_str(), std::ios::out | std::ios::app);
  } else {
    __filesize = 0;
    __stream.open(__filepath.c_str(), std::ios::out);
  }
  debug("log file opened");
}

void LogFile::debug(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put(msg, SystemLog::DEBUG, ap);
  va_end(ap);
}

void LogFile::info(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put(msg, SystemLog::INFO, ap);
  va_end(ap);
}

void LogFile::notice(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put(msg, SystemLog::NOTICE, ap);
  va_end(ap);
}

void LogFile::warning(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put(msg, SystemLog::WARNING, ap);
  va_end(ap);
}

void LogFile::error(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put(msg, SystemLog::ERROR, ap);
  va_end(ap);
}

void LogFile::fatal(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put(msg, SystemLog::FATAL, ap);
  va_end(ap);
}


int LogFile::put(const std::string& msg, SystemLog::Priority priority, va_list ap)
{
  __mutex.lock();
  if (__threshold > priority) {
    __mutex.unlock();
    return 0;
  }
  Date date;
  if (__filesize >= 1024 * 1024 * 2) {
    system(Belle2::form("mv %s %s.%s", __filepath.c_str(),
                        __filepath.c_str(), date.toString("%Y.%m.%d.%H.%M")).c_str());
    open();
  }
  std::stringstream ss;
  ss << "[" << date.toString();
  switch (priority) {
    case SystemLog::DEBUG:
    case SystemLog::INFO:    ss << "] [INFO] "; break;
    case SystemLog::NOTICE:  ss << "] [NOTICE] "; break;
    case SystemLog::WARNING: ss << "] [WARNING] "; break;
    case SystemLog::ERROR:   ss << "] [ERROR] "; break;
    case SystemLog::FATAL:   ss << "][FATAL] "; break;
    default:      ss << "][UNKNOWN] "; break;
  }
  char s[1024];
  vsprintf(s, msg.c_str(), ap);
  ss << s << std::endl;
  std::string str = ss.str();
  __stream << str;
  __filesize += str.size();
  __mutex.unlock();
  return (int) str.size();
}
