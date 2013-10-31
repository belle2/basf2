#include "Log.h"

#include "Date.h"

#include <cstdarg>
#include <cstdio>
#include <sstream>

using namespace Belle2;

Log::Log() throw() : _level(Log::DEBUG), _message()
{

}

Log::Log(LogLevel level, const char* message, ...) throw()
  : _level(level), _refno(0), _time()
{
  va_list ap;
  char ss[1024];
  va_start(ap, message);
  vsprintf(ss, message, ap);
  va_end(ap);
  _message = ss;
}

Log::Log(LogLevel level, const Time& time, const char* message, ...) throw()
  : _level(level), _refno(0), _time(time)
{
  va_list ap;
  char ss[1024];
  va_start(ap, message);
  vsprintf(ss, message, ap);
  va_end(ap);
  _message = ss;
}

Log::Log(LogLevel level, LogRefNo refno, const char* message, ...) throw()
  : _level(level), _refno(refno), _time()
{
  va_list ap;
  char ss[1024];
  va_start(ap, message);
  vsprintf(ss, message, ap);
  va_end(ap);
  _message = ss;
}

Log::Log(LogLevel level, const Time& time, LogRefNo refno, const char* message, ...) throw()
  : _level(level), _refno(refno), _time(time)
{
  va_list ap;
  char ss[1024];
  va_start(ap, message);
  vsprintf(ss, message, ap);
  va_end(ap);
  _message = ss;
}

void Log::setMessage(const char* message, ...) throw()
{
  va_list ap;
  char ss[1024];
  va_start(ap, message);
  vsprintf(ss, message, ap);
  va_end(ap);
  _message = ss;
}

const std::string Log::toString(const char* fmt) const throw()
{
  std::stringstream ss;
  std::string label;
  switch (_level) {
    default:
    case Log::DEBUG: label = "DEBUG"; break;
    case Log::INFO: label = "INFO"; break;
    case Log::NOTICE: label = "NOTICE"; break;
    case Log::WARNING: label = "WARNING"; break;
    case Log::ERROR: label = "ERROR"; break;
    case Log::FATAL: label = "FATAL"; break;
  }

  ss << "[" << label << "]["
     << Date(_time.getSecond()).toString(fmt) << "] ";
  if (_refno > 0) ss << "ref# =" << _refno << " ";
  ss << _message << std::endl;
  return ss.str();
}

void Log::writeObject(Writer& writer) const throw(IOException)
{
  writer.writeInt(_level);
  writer.writeObject(_time);
  writer.writeInt(_refno);
  writer.writeString(_message);
}

void Log::readObject(Reader& reader) throw(IOException)
{
  _level = reader.readInt();
  reader.readObject(_time);
  _refno = reader.readInt();
  _message = reader.readString();
}
