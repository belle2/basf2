/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2017 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionFileDescriptor.h>
#include <framework/logging/LogMessage.h>
#include <sstream>
#include <cstring> // strerror()
#include <unistd.h> // isatty(), dup()
#include <cstdlib> // getenv()

using namespace Belle2;
using namespace std;

LogConnectionFileDescriptor::LogConnectionFileDescriptor(int outputFD, bool color):
  m_fd(dup(outputFD)), m_color(color)
{
  // check fd
  if (m_fd < 0) throw std::runtime_error(std::string("Error duplicating file descriptor: ") + std::strerror(errno));
}

LogConnectionFileDescriptor::~LogConnectionFileDescriptor()
{
  if (isConnected()) close(m_fd);
}

bool LogConnectionFileDescriptor::isConnected()
{
  return m_fd >= 0;
}

bool LogConnectionFileDescriptor::terminalSupportsColors(int fileDescriptor)
{
  //enable color for TTYs with color support (list taken from gtest)
  const bool isTTY = isatty(fileDescriptor);
  const string termName = getenv("TERM") ? getenv("TERM") : "";
  const bool useColor = isTTY and
                        (termName == "xterm" or termName == "xterm-color" or termName == "xterm-256color" or
                         termName == "sceen" or termName == "screen-256color" or termName == "tmux" or
                         termName == "tmux-256color" or termName == "rxvt-unicode" or
                         termName == "rxvt-unicode-256color" or termName == "linux" or termName == "cygwin");
  return useColor;
}

bool LogConnectionFileDescriptor::sendMessage(const LogMessage& message)
{
  if (!isConnected()) return false;
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
    write(m_fd, c.data(), c.size());
  }
  // format message
  const std::string out; // todo = message.str();
  // and write to given file descriptor
  write(m_fd, out.data(), out.size());
  if (m_color) {
    const std::string c = "\x1b[m";
    write(m_fd, c.data(), c.size());
  }
  return true;
}
