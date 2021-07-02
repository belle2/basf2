/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/LogConnectionConsole.h>
#include <framework/logging/LogMessage.h>
#include <boost/python.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <cstring> // strerror()
#include <unistd.h> // isatty(), dup()
#include <cstdlib> // getenv()

using namespace Belle2;

bool LogConnectionConsole::s_pythonLoggingEnabled{false};
bool LogConnectionConsole::s_escapeNewlinesEnabled{false};

LogConnectionConsole::LogConnectionConsole(int outputFD, bool color):
  m_fd(dup(outputFD)), m_color(color)
{
  // check fd
  if (m_fd < 0) throw std::runtime_error(std::string("Error duplicating file descriptor: ") + std::strerror(errno));
}

LogConnectionConsole::~LogConnectionConsole()
{
  if (m_fd > 0) close(m_fd);
}

bool LogConnectionConsole::isConnected()
{
  return s_pythonLoggingEnabled || m_fd >= 0;
}

bool LogConnectionConsole::terminalSupportsColors(int fileDescriptor)
{
  //enable color for TTYs with color support (list taken from gtest)
  const bool isTTY = isatty(fileDescriptor);
  const std::string termName = getenv("TERM") ? getenv("TERM") : "";
  const bool useColor = isTTY and
                        (termName == "xterm" or termName == "xterm-color" or termName == "xterm-256color" or
                         termName == "sceen" or termName == "screen-256color" or termName == "tmux" or
                         termName == "tmux-256color" or termName == "rxvt-unicode" or
                         termName == "rxvt-unicode-256color" or termName == "linux" or termName == "cygwin");
  return useColor;
}

void LogConnectionConsole::write(const std::string& message)
{
  if (s_pythonLoggingEnabled) {
    auto pymessage = boost::python::import("sys").attr("stdout");
    pymessage.attr("write")(message);
    pymessage.attr("flush")();
  } else {
    ::write(m_fd, message.data(), message.size());
  }
}

bool LogConnectionConsole::sendMessage(const LogMessage& message)
{
  if (!isConnected()) return false;
  // format message
  std::stringstream stream;
  if (m_color) {
    const std::string color_str[] = {
      "\x1b[32m",        // Debug  : green
      "",                // Info   : terminal default
      "\x1b[34m",        // Result : blue
      "\x1b[33m",        // Warning: yellow
      "\x1b[31m",        // Error  : red
      "\x1b[07m\x1b[31m" // Fatal  : red reversed
    };
    const std::string& c{color_str[message.getLogLevel()]};
    stream << c;
  }
  stream << message;
  std::string messagestr = stream.str();
  if (s_escapeNewlinesEnabled) {
    // remove trailing whitespace
    boost::trim_right_if(messagestr, boost::is_any_of(" \t\n\r"));
    // escape all remaining newlines
    boost::replace_all(messagestr, "\n", "\\n");
    // and make sure we end in an actual newline
    messagestr += "\n";
  }
  if (m_color) {
    messagestr += "\x1b[m";
  }
  write(messagestr);
  return true;
}

void LogConnectionConsole::finalizeOnAbort()
{
  // If python logging is enabled we need to give jupyter some time to flush
  // the output as this happens only in the output thread. Seems flushing again is fine :D
  if (LogConnectionConsole::getPythonLoggingEnabled()) {
    boost::python::import("sys").attr("stdout").attr("flush")();
  }
}
