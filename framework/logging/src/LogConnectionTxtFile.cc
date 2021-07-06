/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/LogConnectionTxtFile.h>
#include <framework/logging/LogMessage.h>

using namespace Belle2;
using namespace std;

LogConnectionTxtFile::LogConnectionTxtFile(const string& filename, bool append): m_fileStream{filename.c_str(), append ? ios::app : ios::out}
{
  if (!m_fileStream.is_open()) throw std::runtime_error("Cannot open output file '" + filename + "': " + strerror(errno));
}

LogConnectionTxtFile::~LogConnectionTxtFile() = default;

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
