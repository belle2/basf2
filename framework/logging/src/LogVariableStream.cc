/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/LogVariableStream.h>
#include <framework/core/Environment.h>

LogVariableStream& LogVariableStream::operator<<(LogModRealm const& var)
{
  if (var.getRealm() == Belle2::Environment::Instance().getRealm()) {
    m_logLevelOverride = var.getLogLevel();
  }
  return *this;
}
