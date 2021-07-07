/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/gearbox/StringHandler.h>
#include <framework/gearbox/Gearbox.h>

using namespace std;

namespace Belle2::gearbox {
  B2_GEARBOX_REGISTER_INPUTHANDLER(StringHandler, "string");

  InputContext* StringHandler::open(const std::string&)
  {
    return new StringContext(m_uri);
  }
} //Belle2 namespace
