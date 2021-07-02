/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/LogConnectionJSON.h>
#include <framework/logging/LogMessage.h>
#include <boost/property_tree/json_parser.hpp>

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
