/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/pcore/zmq/utils/ZMQAddressUtils.h>
#include <TRandom.h>
#include <algorithm>
#include <fstream>

using namespace Belle2;

namespace {
  std::string randomName()
  {
    auto randchar = []() -> char {
      const char charset[] = "abcdefghijklmnopqrstuvwxyz";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[gRandom->Integer(max_index)];
    };

    const unsigned int length = 4;
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
  }

  std::string randomPort()
  {
    // Port should be in the range of 50000 to 60000. As the last digit will be added by the getSocketAddress, we will not do it here.
    const unsigned int port = 5000 + gRandom->Integer(1000);
    return std::to_string(port);
  }
}

std::string ZMQAddressUtils::randomSocketName(const std::string& hostname)
{
  std::string socket_name = "tcp://" + hostname + ":" + randomPort();
  return socket_name;
}

std::string ZMQAddressUtils::randomSocketName()
{
  std::string socket_name = "ipc://" + randomName() + ".socket";
  while (std::ifstream(socket_name)) {
    socket_name = "ipc://" + randomName() + ".socket";
  }

  return socket_name;
}

std::string ZMQAddressUtils::getSocketAddress(const std::string& socketAddress, ZMQAddressType socketPart)
{
  const std::string& prefix = "tcp://";
  if (socketAddress.compare(0, prefix.size(), prefix) == 0) {
    if (socketPart == ZMQAddressType::c_input) {
      return socketAddress + "0";
    } else if (socketPart == ZMQAddressType::c_output) {
      return socketAddress + "1";
    } else if (socketPart == ZMQAddressType::c_pub) {
      return socketAddress + "2";
    } else if (socketPart == ZMQAddressType::c_sub) {
      return socketAddress + "3";
    } else if (socketPart == ZMQAddressType::c_control) {
      return socketAddress + "4";
    }
  } else {
    if (socketPart == ZMQAddressType::c_input) {
      return socketAddress + "_input";
    } else if (socketPart == ZMQAddressType::c_output) {
      return socketAddress + "_output";
    } else if (socketPart == ZMQAddressType::c_pub) {
      return socketAddress + "_pub";
    } else if (socketPart == ZMQAddressType::c_sub) {
      return socketAddress + "_sub";
    } else if (socketPart == ZMQAddressType::c_control) {
      return socketAddress + "_control";
    }
  }
  return socketAddress;
}