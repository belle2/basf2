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
  enum ZMQAddressType {
    c_input,
    c_output,
    c_pub,
    c_sub,
    c_control
  };

  class ZMQAddressUtils {
  public:
    static std::string random_name(size_t length, bool only_digits = false);
    static std::string random_socket_name(bool port, const std::string& hostname = "127.0.0.1");
    static std::string getSocketAddress(const std::string& socketAddress, const ZMQAddressType socketPart);
  };
}
