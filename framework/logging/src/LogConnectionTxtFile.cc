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
#include <fstream>

using namespace Belle2;
using namespace std;

LogConnectionTxtFile::LogConnectionTxtFile(const string& filename, bool append)
{
  m_fileStream = std::make_unique<ofstream>(filename.c_str(), append ? ios::app : ios::out);
}

LogConnectionTxtFile::~LogConnectionTxtFile()
{
}

bool LogConnectionTxtFile::isConnected()
{
  return (bool)m_fileStream;
}

bool LogConnectionTxtFile::sendMessage(const LogMessage& message)
{
  if (isConnected()) {
    (*m_fileStream) << message;
    return true;
  } else return false;
}

void LogConnectionTxtFile::finalizeOnAbort()
{
  m_fileStream.reset(nullptr);
}
