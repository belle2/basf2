/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionIOStream.h>
#include <framework/logging/LogMessage.h>

#include <iostream>

#include <unistd.h> // isatty()
#include <cstdlib>


using namespace Belle2;
using namespace std;

LogConnectionIOStream::LogConnectionIOStream(ostream& outputStream, bool color) :
  m_stream(outputStream),
  m_color(color)
{
}


LogConnectionIOStream::~LogConnectionIOStream()
{
}


bool LogConnectionIOStream::isConnected()
{
  return m_stream.rdbuf() != nullptr;
}

bool LogConnectionIOStream::terminalSupportsColors()
{
  //enable color for TTYs with color support (list taken from gtest)
  const bool isTTY = isatty(fileno(stdout));
  const string termName = getenv("TERM") ? getenv("TERM") : "";
  const bool useColor = isTTY and (
                          termName == "xterm" or
                          termName == "xterm-color" or
                          termName == "xterm-256color" or
                          termName == "linux" or
                          termName == "cygwin");
  return useColor;
}


bool LogConnectionIOStream::sendMessage(const LogMessage& message)
{
  if (isConnected()) {
    if (m_color) {
      static const char* color_str[] = {
        "\x1b[32m",        // Debug  : green
        "",                // Info   : terminal default
        "\x1b[34m",        // Result : blue
        "\x1b[33m",        // Warning: yellow
        "\x1b[31m",        // Error  : red
        "\x1b[07m\x1b[31m" // Fatal  : red reversed
      };
      m_stream << color_str[message.getLogLevel()];
    }
    m_stream << message;
    if (m_color) {
      m_stream << "\x1b[m" << flush;
    }
    return true;
  } else {
    return false;
  }
}
