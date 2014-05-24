/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionTxtFile.h>
#include <framework/logging/LogMessage.h>

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
  delete m_fileStream;
}


bool LogConnectionTxtFile::isConnected()
{
  return (m_fileStream != NULL);
}


bool LogConnectionTxtFile::sendMessage(const LogMessage& message)
{
  if (isConnected()) {
    (*m_fileStream) << message;
    return true;
  } else return false;
}
