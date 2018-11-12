/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2018 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Ritter                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionTxtFile.h>
#include <framework/logging/LogMessage.h>

using namespace Belle2;
using namespace std;

LogConnectionTxtFile::LogConnectionTxtFile(const string& filename, bool append): m_fileStream{filename.c_str(), append ? ios::app : ios::out}
{
  if (!m_fileStream.is_open()) throw std::runtime_error("Cannot open output file '" + filename + "': " + strerror(errno));
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
    m_fileStream << message;
    return true;
  } else return false;
}

void LogConnectionTxtFile::finalizeOnAbort()
{
  m_fileStream.close();
}
