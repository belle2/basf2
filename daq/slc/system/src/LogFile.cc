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

bool LogFile::g_opened = false;
std::string LogFile::g_filepath;
std::ofstream LogFile::g_stream;
unsigned int LogFile::g_filesize = 0;
Mutex LogFile::g_mutex;
LogFile::Priority LogFile::g_threshold;
std::string LogFile::g_filename;

void LogFile::open(const std::string& filename, Priority threshold)
{
  if (!g_opened) {
    ConfigFile config("slowcontrol");
    g_filename = filename;
    g_filepath = config.get("logfile.dir") + "/" + filename
                 + Date().toString(".%Y.%m.%d.log");
    g_threshold = threshold;
    g_opened = true;
    open();
  }
}

void LogFile::open()
{
  if (!g_opened) return;
  struct stat st;
  if (stat(g_filepath.c_str(), &st) == 0) {
    g_filesize = st.st_blksize;
    g_stream.open(g_filepath.c_str(), std::ios::out | std::ios::app);
  } else {
    g_filesize = 0;
    g_stream.open(g_filepath.c_str(), std::ios::out);
  }
  debug("/* ---------- log file opened ---------- */");
  debug("log file : %s", g_filepath.c_str());
}

void LogFile::close()
{
  if (!g_opened) return;
  g_stream.close();
  g_opened = false;
}

void LogFile::debug(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, DEBUG, ap);
  va_end(ap);
}

void LogFile::info(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, INFO, ap);
  va_end(ap);
}

void LogFile::notice(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, NOTICE, ap);
  va_end(ap);
}

void LogFile::warning(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, WARNING, ap);
  va_end(ap);
}

void LogFile::error(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, ERROR, ap);
  va_end(ap);
}

void LogFile::fatal(const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, FATAL, ap);
  va_end(ap);
}


void LogFile::put(Priority priority, const std::string& msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  put_impl(msg, priority, ap);
  va_end(ap);
}

int LogFile::put_impl(const std::string& msg, Priority priority, va_list ap)
{
  g_mutex.lock();
  if (g_threshold > priority) {
    g_mutex.unlock();
    return 0;
  }
  Date date;
  if (g_filesize >= 1024 * 1024 * 2) {
    g_stream.close();
    ConfigFile config("slowcontrol");
    rename(g_filepath.c_str(),
           (config.get("logfile.dir") + "/" + g_filename +
            date.toString(".%Y.%m.%d.%H.%M.log")).c_str());
    g_mutex.unlock();
    open();
    g_mutex.lock();
  }
  std::stringstream ss;
  ss << "[" << date.toString();
  std::string color = "\x1b[49m\x1b[39m";
  switch (priority) {
    case DEBUG:   color = "\x1b[49m\x1b[39m"; ss << "] [DEBUG] "; break;
    case INFO:    color = "\x1b[49m\x1b[32m"; ss << "] [INFO] "; break;
    case NOTICE:  color = "\x1b[49m\x1b[34m"; ss << "] [NOTICE] "; break;
    case WARNING: color = "\x1b[49m\x1b[35m"; ss << "] [WARNING] "; break;
    case ERROR:   color = "\x1b[49m\x1b[31m"; ss << "] [ERROR] "; break;
    case FATAL:   color = "\x1b[41m\x1b[37m"; ss << "] [FATAL] "; break;
    default:                 ss << "] [UNKNOWN] "; break;
  }
  static char s[1024 * 100];
  vsprintf(s, msg.c_str(), ap);
  ss << s << std::endl;
  std::string str = ss.str();
  std::cerr << color << str << "\x1b[49m\x1b[39m";
  if (g_opened) {
    g_stream << str;
    g_stream.flush();
    g_filesize += str.size();
  }
  g_mutex.unlock();
  return (int) str.size();
}
