#ifndef EUDAQ_INCLUDED_Exception
#define EUDAQ_INCLUDED_Exception

#include <eutel/eudaq/Utils.h>
#include <exception>
#include <string>

#ifndef EUDAQ_FUNC
# define EUDAQ_FUNC ""
#endif

#define EUDAQ_THROWX(exc, msg) throw ::eudaq::InitException(exc(msg), __FILE__, __LINE__, EUDAQ_FUNC)
#define EUDAQ_THROW(msg) EUDAQ_THROWX(::eudaq::LoggedException, (msg))

#define EUDAQ_EXCEPTIONX(name, base) \
  class name : public base {         \
  public:                            \
    name(const std::string & msg)      \
      : base(msg) {}                   \
  }

#define EUDAQ_EXCEPTION(name) EUDAQ_EXCEPTIONX(name, ::eudaq::Exception)

namespace eudaq {

  class Exception : public std::exception {
  public:
    Exception(const std::string& msg);
    const char* what() const throw()
    {
      if (m_text.length() == 0) make_text();
      return m_text.c_str();
    }
    // This shouldn't really be const, but it must be callable on temporary objects...
    const Exception& SetLocation(const std::string& file = "",
                                 unsigned line = 0,
                                 const std::string& func = "") const;
    virtual ~Exception() throw()
    {
    }
  protected:
    std::string m_msg;
  private:
    void make_text() const;
    mutable std::string m_text;
    mutable std::string m_file, m_func;
    mutable unsigned m_line;
  };

  class LoggedException : public Exception {
  public:
    LoggedException(const std::string& msg);
    void Log() const;
    virtual ~LoggedException() throw();
  private:
    mutable bool m_logged;
  };

  namespace {
    void do_log(const Exception&)
    {
    }
    void do_log(const LoggedException& e)
    {
      e.Log();
    }
  }

  template <typename T>
  const T& InitException(const T& e, const std::string& file, int line = 0, const std::string func = "")
  {
    e.SetLocation(file, line, func);
    do_log(e); // If it is a LoggedException, send it to be logged already
    return e;
  }

  // Some useful predefined exceptions
  EUDAQ_EXCEPTION(FileNotFoundException);
  EUDAQ_EXCEPTION(FileExistsException);
  EUDAQ_EXCEPTION(FileNotWritableException);
  EUDAQ_EXCEPTION(FileReadException);
  EUDAQ_EXCEPTION(FileWriteException);
  EUDAQ_EXCEPTION(FileFormatException);
  EUDAQ_EXCEPTION(CommunicationException);
  EUDAQ_EXCEPTIONX(BusError, CommunicationException);

}

#endif // EUDAQ_INCLUDED_Exception
