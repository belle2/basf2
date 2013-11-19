#include "ArichHVCommunicator.h"

#include <nsm/nsm2.h>
#include "ArichHVStatus.h"

#include "base/Debugger.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace Belle2;

std::string ArichHVCommunicator::sendRequest(ArichHVMessage& msg)
throw(IOException)
{
  _mutex.lock();
  if (_available) {
    try {
      std::string str = msg.toString() + "\r";
      _writer.write(str.c_str(), str.size());
      if (msg.getCommand() == ArichHVMessage::GET) {
        std::stringstream ss;
        char c;
        while (true) {
          c = _reader.readChar();
          if (c == '\r') break;
          ss << c;
        }
        return ss.str();
      }
    } catch (const IOException& e) {
      _mutex.unlock();
      throw (e);
    }
  }
  _mutex.unlock();
  return "";
}

void ArichHVCommunicator::run()
{
  while (true) {
    _mutex.lock();
    _available = false;
    try {
      _socket.connect(_host, _port);
      _writer = TCPSocketWriter(_socket);
      _reader = TCPSocketReader(_socket);
    } catch (const IOException& e) {
      _socket.close();
      _mutex.unlock();
      Belle2::debug("Socket error: %s", e.what());
      sleep(5);
      continue;
    }
    _available = true;
    _mutex.unlock();

    ArichHVStatus* status = NULL;
    while (true) {
      try {
        status = (ArichHVStatus*)_data->get();
        break;
      } catch (const IOException& e) {
        Belle2::debug("NSM error: %s", e.what());
        sleep(5);
      }
    }
    HVChannelInfo* info = new HVChannelInfo();
    while (true) {
      try {
        for (size_t ns = 0; ns < _crate->getNSlot(); ns++) {
          for (size_t nc = 0; nc < _crate->getNChannel(); nc++) {
            //HVChannelInfo* info = _crate->getChannel(ns, nc);
            ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::ALL);
            info->setSlot(ns + 1);
            info->setChannel(nc + 1);
            msg.setChannelInfo(info);
            msg.read(sendRequest(msg));
            msg.setParamType(ArichHVMessage::CURRENT_LIMIT);
            msg.read(sendRequest(msg));
            msg.setParamType(ArichHVMessage::VOLTAGE_LIMIT);
            msg.read(sendRequest(msg));
            msg.setParamType(ArichHVMessage::VOLTAGE_DEMAND);
            msg.read(sendRequest(msg));
            int index = ns * _crate->getNChannel() + nc;
            status->status[index] = info->getStatus();
            status->voltage_mon[index] = info->getVoltageMonitored();
            status->current_mon[index] = info->getCurrentMonitored();
            status->voltage_demand[index] = info->getVoltageDemand();
            status->rampup_speed[index] = info->getRampUpSpeed();
            status->rampdown_speed[index] = info->getRampDownSpeed();
            status->voltage_limit[index] = info->getVoltageLimit();
            status->current_limit[index] = info->getVoltageLimit();
          }
        }
      } catch (const IOException& e) {
        _mutex.lock();
        _socket.close();
        _available = false;
        _mutex.unlock();
        Belle2::debug("Socket error: %s", e.what());
        sleep(5);
        continue;
      }
      sleep(2);
    }
  }
}
