/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/LogConnectionJSON.h>
#include <framework/logging/LogMessage.h>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <iostream>
#include <cstdio>

namespace Belle2 {

  LogConnectionJSON::LogConnectionJSON(bool complete): m_console{STDOUT_FILENO, false}, m_complete{complete}
  {
  }

  bool LogConnectionJSON::sendMessage(const LogMessage& message)
  {
    if (!isConnected()) return false;
    m_console.write(message.toJSON(m_complete));
    return true;
  }

}
