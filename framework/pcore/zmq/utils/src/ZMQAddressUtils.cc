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

std::string ZMQAddressUtils::randomName(size_t length, bool only_digits)
{
  auto randchar = [&only_digits]() -> char {
    if (only_digits)
    {
      const char charset[] = "0123456789";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[gRandom->Integer(max_index)];
    } else {
      const char charset[] =
      "0123456789"
      "abcdefghijklmnopqrstuvwxyz";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[gRandom->Integer(max_index)];
    }
  };

  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);

  return str;
}

std::string ZMQAddressUtils::randomSocketName(const std::string& hostname)
{
  std::string socket_name = "tcp://" + hostname + ":" + randomName(5, true);
  return socket_name;
}

std::string ZMQAddressUtils::randomSocketName()
{
  std::string socket_name = "ipc://" + randomName(10, false) + ".socket";
  while (std::ifstream(socket_name)) {
    socket_name = "ipc://" + randomName(10, false) + ".socket";
  }

  return socket_name;
}

std::string ZMQAddressUtils::getSocketAddress(const std::string& socketAddress, ZMQAddressType socketPart)
{
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
  return socketAddress;
}