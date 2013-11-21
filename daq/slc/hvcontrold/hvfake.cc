#include "ArichHVMessage.h"
#include "HVCrateInfo.h"
#include "HVCommand.h"

#include <system/TCPServerSocket.h>
#include <system/TCPSocketWriter.h>
#include <system/TCPSocketReader.h>
#include <system/Mutex.h>
#include <system/PThread.h>

#include <base/Debugger.h>
#include <base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <ctype.h>

using namespace Belle2;

Mutex mutex;
HVCrateInfo* crate = new HVCrateInfo(0, 1, 4);

class HVGenerator {
public:
  void run() {
    while (true) {
      mutex.lock();
      for (size_t ns = 0; ns < crate->getNSlot(); ns++) {
        for (size_t nc = 0; nc < crate->getNChannel(); nc++) {
          HVChannelInfo* info = crate->getChannel(ns, nc);
          if (info->isSwitchOn()) {
            info->setVoltageMonitored(info->getVoltageDemand() + rand() % 100);
            info->setCurrentMonitored(info->getVoltageDemand() + rand() % 100);
          } else {
            info->setVoltageMonitored(0);
            info->setCurrentMonitored(0);
          }
        }
      }
      mutex.unlock();
      sleep(2);
    }
  }
};

int main(int argc, char** argv)
{
  if (argc < 3) {
    Belle2::debug("Usage : ./hvmaster <ip> <port>");
    return 1;
  }
  TCPServerSocket server_socket(argv[1], atoi(argv[2]));
  server_socket.open();
  PThread(new HVGenerator());
  HVChannelInfo* ch_info  = new HVChannelInfo();
  while (true) {
    TCPSocket socket;
    try {
      socket = server_socket.accept();
    } catch (const IOException& e) {
      Belle2::debug("Server socket Error:%s", e.what());
      return 1;
    }
    TCPSocketWriter writer(socket);
    TCPSocketReader reader(socket);
    while (true) {
      std::stringstream ss;
      try {
        while (true) {
          char c = reader.readChar();
          if (c == '\r') break;
          ss << c;
        }
      } catch (const IOException& e) {
        Belle2::debug("Socket reading error : %s", e.what());
        break;
      }
      std::string s = ss.str();
      int slot = 0;
      int ch = 0;
      if (s.at(0) == '#') {
        if (isdigit(s.at(1))) slot = s.at(1) - '0';
        if (isdigit(s.at(2))) ch = s.at(2) - '0';
        s.erase(0, 3);
        if (s.at(0) == ' ') s.erase(0, 1);
      }
      HVChannelInfo* info = NULL;
      mutex.lock();
      if (slot == 0 || ch == 0) {
        info = ch_info;
      } else {
        info = crate->getChannel(slot - 1, ch - 1);
      }
      ArichHVMessage msg;
      msg.setChannelInfo(info);
      s = Belle2::form("#%d%d ", slot, ch) + s;
      msg.read(s);
      if (msg.getCommand() == ArichHVMessage::GET) {
        ss.str("");
        switch (msg.getParamType()) {
          case ArichHVMessage::ALL: {
            ss << "#" << slot << ch << "GET=" << info->getVoltageMonitored() << ","
               << info->getCurrentMonitored() << ","
               << info->isSwitchOn() << ","
               << info->getRampUpSpeed() << ","
               << info->getRampDownSpeed();
          }; break;
          case ArichHVMessage::SWITCH:
            ss << "#" << slot << ch << "SW=" << info->isSwitchOn(); break;
          case ArichHVMessage::RAMPUP_SPEED:
            ss << "#" << slot << ch << "RVU=" << info->getRampUpSpeed(); break;
          case ArichHVMessage::RAMPDOWN_SPEED:
            ss << "#" << slot << ch << "RVD=" << info->getRampDownSpeed(); break;
          case ArichHVMessage::VOLTAGE_DEMAND:
            ss << "#" << slot << ch << "CH5=" << info->getVoltageDemand(); break;
          case ArichHVMessage::VOLTAGE_LIMIT:
            ss << "#" << slot << ch << "CH2=" << info->getVoltageLimit(); break;
          case ArichHVMessage::CURRENT_LIMIT:
            ss << "#" << slot << ch << "CH7=" << info->getCurrentLimit(); break;
          case ArichHVMessage::VOLTAGE_MON:
            ss << "#" << slot << ch << "MNH1=" << info->getVoltageMonitored(); break;
          case ArichHVMessage::CURRENT_MON:
            ss << "#" << slot << ch << "NMH2=" << info->getCurrentMonitored(); break;
          case ArichHVMessage::DATE: break;
        }
        ss << "\r";
        s = ss.str();
        try {
          writer.write(s.c_str(), s.size());
        } catch (const IOException& e) {
          Belle2::debug("Socket writing error : %s", e.what());
          break;
        }
      }
      mutex.unlock();
    }
  }
  return 0;
}

