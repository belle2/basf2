/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <logging/LogConnectionIOStream.h>

using namespace Belle2;
using namespace std;

LogConnectionIOStream::LogConnectionIOStream(ostream& outputStream)
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


bool LogConnectionIOStream::sendMessage(LogCommon::ELogLevel logLevel, const std::string& message, const std::string& package,
                                        const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo)
{
  if (isConnected()) {
    (*m_stream) << "[" << LogCommon::logLevelToString(logLevel) << "] ";

    if (sendLocationInfo) {
      (*m_stream) << file << ":" << package << ", " << function << "():" << line << ": ";
    }

    (*m_stream) << message << endl;
    return true;
  } else return false;
}
