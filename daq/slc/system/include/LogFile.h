#ifndef _Belle2_LogFile_h
#define  _Belle2_LogFile_h

#include <daq/slc/base/SystemLog.h>
#include <daq/slc/system/Mutex.h>

#include <string>
#include <fstream>
#include <stdarg.h>

namespace Belle2 {

  struct LogFile {

  private:
    LogFile() {}
    ~LogFile() {}

  private:
    static bool g_opened;
    static std::string g_filepath;
    static std::ofstream g_stream;
    static unsigned int g_filesize;
    static Mutex g_mutex;
    static SystemLog::Priority g_threshold;

    // member functions
  public:
    static void open(const std::string& filename, SystemLog::Priority priority = SystemLog::UNKNOWN);
    static void open();
    static void debug(const std::string& msg, ...);
    static void info(const std::string& msg, ...);
    static void notice(const std::string& msg, ...);
    static void warning(const std::string& msg, ...);
    static void error(const std::string& msg, ...);
    static void fatal(const std::string& msg, ...);
    static void put(SystemLog::Priority priority, const std::string& msg, ...);

  private:
    static int put_impl(const std::string& msg, SystemLog::Priority priority, va_list ap);

  };

};

#endif
