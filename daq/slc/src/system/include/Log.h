#ifndef _Belle2_Log_hh
#define _Belle2_Log_hh

#include "Time.h"

#include "base/Serializable.h"
#include "base/Writer.h"
#include "base/Reader.h"

#include <string>

namespace Belle2 {

  typedef unsigned int LogLevel;
  typedef unsigned int LogRefNo;

  class Log : public Serializable {

  public:
    static const LogLevel DEBUG = 0;
    static const LogLevel INFO = 1;
    static const LogLevel NOTICE = 2;
    static const LogLevel WARNING = 3;
    static const LogLevel ERROR = 4;
    static const LogLevel FATAL = 5;

  public:
    Log() throw();
    Log(LogLevel level, const char* message, ...) throw();
    Log(LogLevel level, const Time& time, const char* message, ...) throw();
    Log(LogLevel level, LogRefNo refno, const char* message, ...) throw();
    Log(LogLevel level, const Time& time, LogRefNo refno, const char* message, ...) throw();
    virtual ~Log() throw() {}

  public:
    LogLevel getLevel() const throw() { return _level; }
    LogRefNo getRefNo() const throw() { return _refno; }
    const Time& getTime() const throw() { return _time; }
    Time& getTime() throw() { return _time; }
    const std::string& getMessage() const throw() { return _message; }
    void setLevel(LogLevel level) throw() { _level = level; }
    void setRefNo(LogRefNo refno) throw() { _refno = refno; }
    void setTime(const Time& time) throw() { _time = time; }
    void setMessage(const char* message, ...) throw();
    const std::string toString(const char* fmt = "%H:%M:%S") const throw();

  public:
    virtual void writeObject(Writer& writer) const throw(IOException);
    virtual void readObject(Reader& reader) throw(IOException);

  protected:
    LogLevel _level;
    LogLevel _refno;
    Time _time;
    std::string _message;

  };

}

#endif

