/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <logging/LogConnectionTxtFile.h>

using namespace Belle2;
using namespace std;

LogConnectionTxtFile::LogConnectionTxtFile(const string& filename, bool append)
{
  if (append) {
    m_fileStream = new ofstream(filename.c_str(), ios::app);
  } else m_fileStream = new ofstream(filename.c_str(), ios::out);
}


LogConnectionTxtFile::~LogConnectionTxtFile()
{
  if (m_fileStream != NULL) delete m_fileStream;
}


bool LogConnectionTxtFile::isConnected()
{
  return (m_fileStream != NULL);
}


bool LogConnectionTxtFile::sendMessage(LogCommon::ELogLevel logLevel, const std::string& message, const std::string& package,
                                       const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo)
{
  if (isConnected()) {
    (*m_fileStream) << "[" << LogCommon::logLevelToString(logLevel) << "] ";

    if (sendLocationInfo) {
      (*m_fileStream) << file << ":" << package << ", " << function << "():" << line << ": ";
    }

    (*m_fileStream) << message << endl;
    return true;
  } else return false;
}
