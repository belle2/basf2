/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#ifndef _Belle2_LogFile_h
#define  _Belle2_LogFile_h

#include <daq/slc/system/Mutex.h>

#include <daq/slc/base/Date.h>

#include <string>
#include <fstream>
#include <stdarg.h>

namespace Belle2 {

  struct LogFile {

  public:
    enum Priority {
      UNKNOWN = 0, DEBUG, INFO, NOTICE, WARNING, ERROR, FATAL
    };

  public:
    static Priority getPriority(const std::string& str);

  private:
    LogFile() {}
    ~LogFile() {}

  private:
    static bool g_stderr;
    static bool g_opened;
    static std::string g_filepath;
    static std::string g_linkpath;
    static std::ofstream g_stream;
    static unsigned int g_filesize;
    static Mutex g_mutex;
    static Priority g_threshold;
    static std::string g_filename;
    static Date g_date;

    // member functions
  public:
    static void open(const std::string& filename,
                     Priority priority = UNKNOWN);
    static void open();
    static void close();
    static void debug(const std::string& msg, ...);
    static void info(const std::string& msg, ...);
    static void notice(const std::string& msg, ...);
    static void warning(const std::string& msg, ...);
    static void error(const std::string& msg, ...);
    static void fatal(const std::string& msg, ...);
    static void put(Priority priority, const std::string& msg, ...);
    static void setStdErr(bool stderr)
    {
      g_stderr = stderr;
    }

  private:
    static int put_impl(const std::string& msg, Priority priority, va_list ap);

  };

};

#endif
