#include <eutel/eudaq/Exception.h>

#include <eutel/eudaq/Logger.h>

namespace eudaq {

  Exception::Exception(const std::string& msg)
    : m_msg(msg), m_line(0)
  {}

  const Exception& Exception::SetLocation(const std::string& file,
                                          unsigned line,
                                          const std::string& func) const
  {
    m_file = file;
    m_line = line;
    m_func = func;
    return *this;
  }

  void Exception::make_text() const
  {
    m_text = m_msg;
    if (m_file.length() > 0) {
      m_text += "\n  From " + m_file;
      if (m_line > 0) {
        m_text += ":" + to_string(m_line);
      }
    }
    if (m_func.length() > 0) m_text += "\n  In " + m_func;
  }

  LoggedException::LoggedException(const std::string& msg)
    : Exception(msg), m_logged(false)
  {
  }

  void LoggedException::Log() const
  {
    if (m_logged) return;
    // Only log the message once
    eudaq::GetLogger().SendLogMessage(eudaq::LogMessage(m_msg, eudaq::LogMessage::LVL_THROW));
    m_logged = true;
  }

  LoggedException::~LoggedException() noexcept
  {
    // Make sure the message has been logged before we die
    Log();
  }
}
