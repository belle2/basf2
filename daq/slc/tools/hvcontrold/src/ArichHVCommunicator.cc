#include "ArichHVCommunicator.h"
#include "HVState.h"

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
      std::cout << str << std::endl;
      _writer.write(str.c_str(), str.size());
      if (msg.getCommand() == ArichHVMessage::GET) {
        std::stringstream ss;
        char c;
        while (true) {
          _socket.select(10);
          c = _reader.readChar();
          if (c == '\r') break;
          ss << c;
        }
        std::cout << ss.str() << std::endl;
        _mutex.unlock();
        return ss.str();
      }
    } catch (const IOException& e) {
      Belle2::debug("Socket error on HV crate: %s", e.what());
      _socket.close();
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
      Belle2::debug("Socket connection error to HV crate: %s", e.what());
      sleep(5);
      continue;
    }
    _available = true;
    _mutex.unlock();

    ArichHVStatus* status = NULL;
    while (status == NULL) {
      if (_callback != NULL && _callback->isReady()) {
        try {
          status = (ArichHVStatus*)_data->get();
          break;
        } catch (const IOException& e) {
          Belle2::debug("NSM error: %s", e.what());
        }
      }
      sleep(2);
    }
    HVChannelInfo* ch_info_tmp = new HVChannelInfo();
    HVChannelStatus* ch_status_tmp = new HVChannelStatus();
    while (true) {
      try {
        for (size_t ns = 0; ns < _crate->getNSlot(); ns++) {
          for (size_t nc = 0; nc < _crate->getNChannel(); nc++) {
            ArichHVMessage msg(ArichHVMessage::GET, ArichHVMessage::ALL);
            ch_info_tmp->setSlot(ns + 1);
            ch_info_tmp->setChannel(nc + 1);
            msg.setChannelInfo(ch_info_tmp);
            msg.setChannelStatus(ch_status_tmp);
            msg.read(sendRequest(msg));
            msg.setParamType(ArichHVMessage::CURRENT_LIMIT);
            msg.read(sendRequest(msg));
            msg.setParamType(ArichHVMessage::VOLTAGE_LIMIT);
            msg.read(sendRequest(msg));
            msg.setParamType(ArichHVMessage::VOLTAGE_DEMAND);
            msg.read(sendRequest(msg));
            int index = ns * _crate->getNChannel() + nc;
            HVChannelStatus* ch_status = _crate->getChannelStatus(ns, nc);
            switch (ch_status_tmp->getStatus()) {
              case 0: {
                ch_status->setStatus(HVState::OFF_STABLE_S.getId());
              } break;
              case 1: {
                if (ch_info_tmp->getVoltageDemand() > 0) {
                  double diff = (ch_status_tmp->getVoltageMonitored() - ch_info_tmp->getVoltageDemand())
                                / ((double)ch_info_tmp->getVoltageDemand());
                  if (diff < 0.05 && diff > -0.05) {
                    ch_status->setStatus(HVState::ON_STABLE_S.getId());
                  } else if (ch_status_tmp->getVoltageMonitored() - ch_status->getVoltageMonitored() > 0) {
                    ch_status->setStatus(HVState::ON_RAMPINGUP_TS.getId());
                  } else if (ch_status_tmp->getVoltageMonitored() - ch_status->getVoltageMonitored() < 0) {
                    ch_status->setStatus(HVState::ON_RAMPINGDOWN_TS.getId());
                  }
                }
              } break;
              case 2: {
                ch_status->setStatus(HVState::ON_OCP_ES.getId());
              } break;
              case 3: {
                ch_status->setStatus(HVState::ON_OVP_ES.getId());
              } break;
            }
            ch_status->setVoltageMonitored(ch_status_tmp->getVoltageMonitored());
            ch_status->setCurrentMonitored(ch_status_tmp->getCurrentMonitored());
            status->status[index] = ch_status->getStatus();
            status->voltage_mon[index] = ch_status->getVoltageMonitored();
            status->current_mon[index] = ch_status->getCurrentMonitored();
            status->voltage_demand[index] = ch_info_tmp->getVoltageDemand();
            status->rampup_speed[index] = ch_info_tmp->getRampUpSpeed();
            status->rampdown_speed[index] = ch_info_tmp->getRampDownSpeed();
            status->voltage_limit[index] = ch_info_tmp->getVoltageLimit();
            status->current_limit[index] = ch_info_tmp->getCurrentLimit();
          }
        }
        _callback->sendStatus();
      } catch (const IOException& e) {
        _mutex.lock();
        _socket.close();
        _available = false;
        for (size_t ns = 0; ns < _crate->getNSlot(); ns++) {
          for (size_t nc = 0; nc < _crate->getNChannel(); nc++) {
            HVChannelStatus* ch_status = _crate->getChannelStatus(ns, nc);
            ch_status->setStatus(HVState::OFF_ERROR_ES.getId());
          }
        }
        _mutex.unlock();
        Belle2::debug("Socket error: %s", e.what());
        sleep(5);
        break;
      }
      sleep(2);
    }
  }
}
