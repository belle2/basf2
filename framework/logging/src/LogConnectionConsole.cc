/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2017 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionConsole.h>
#include <framework/logging/LogMessage.h>
#include <boost/python.hpp>
#include <sstream>
#include <cstring> // strerror()
#include <unistd.h> // isatty(), dup()
#include <cstdlib> // getenv()

using namespace Belle2;

bool LogConnectionConsole::s_pythonLoggingEnabled{false};

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
  if (m_color) {
    stream << "\x1b[m";
  }
  write(stream.str());
  return true;
}
