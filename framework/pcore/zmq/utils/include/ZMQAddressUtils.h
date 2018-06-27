/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <string>

namespace Belle2 {
  /// The type of a ZMQ socket address (which socket to use)
  enum ZMQAddressType {
    c_input, /// Input socket
    c_output, /// Output socket
    c_pub, /// Multicast publish socket
    c_sub, /// Multicast subscribe socket
    c_control /// Multicast control socket
  };

  class ZMQAddressUtils {
  public:
    /// Generate a random socket name in the form tcp://hostname:port
    static std::string randomSocketName(const std::string& hostname);
    /// Generate a random socket name in the form ipc:///socketname
    static std::string randomSocketName();
    /// Create a full socket address for the given type from a random socket address, ba adding a suffix.
    static std::string getSocketAddress(const std::string& socketAddress, ZMQAddressType socketPart);
  };
}
