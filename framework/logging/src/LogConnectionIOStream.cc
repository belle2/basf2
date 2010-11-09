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


bool LogConnectionIOStream::sendMessage(LogMessage message)
{
  if (isConnected()) {
    (*m_stream) << message;
    return true;
  } else return false;
}
