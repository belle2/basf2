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

using namespace Belle2;
using namespace std;

LogConnectionIOStream::LogConnectionIOStream(ostream& outputStream, bool color) :
  m_color(color)
{
  m_stream = new ostream(outputStream.rdbuf());
}


LogConnectionIOStream::~LogConnectionIOStream()
{
  if (m_stream != NULL) delete m_stream;
}


bool LogConnectionIOStream::isConnected()
{
  return (m_stream != NULL);
}


bool LogConnectionIOStream::sendMessage(LogMessage message)
{
  static const char* color_str[] = {
    "\x1b[32m",        // Debug  : green
    "",                // Info   : terminal default
    "\x1b[33m",        // Warning: yellow
    "\x1b[31m",        // Error  : red
    "\x1b[07m\x1b[31m" // Fatal  : red reversed
  };
  if (isConnected()) {
    if (m_color) {
      (*m_stream) << color_str[message.getLogLevel()];
    }
    (*m_stream) << message;
    if (m_color) {
      (*m_stream) << "\x1b[m" << flush;
    }
    return true;
  } else return false;
}
