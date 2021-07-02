/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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

  /// Summary of some address helpers
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
