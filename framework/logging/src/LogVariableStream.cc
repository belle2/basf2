/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 Belle II Collaboration                               *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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
