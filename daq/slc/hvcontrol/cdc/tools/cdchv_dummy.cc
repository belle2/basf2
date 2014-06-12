#include "daq/slc/hvcontrol/cdc/CDCHVMessage.h"

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <ctype.h>

using namespace Belle2;

const int MAX_UNITS = 9;
const int MAX_STORES = 7;

std::vector<CDCHVMessage*> hvmsg_v;
Mutex mutex[MAX_UNITS];

class HVEmulator {

private:
  int m_unit;

public:
  HVEmulator(int unit) : m_unit(unit) {}
  ~HVEmulator() throw() {}

public:
  void run() {
    while (true) {
      mutex[m_unit].lock();
      CDCHVMessage& msg(hvmsg_v[m_unit][0]);
      if (msg.isSwitchOn()) {
        double vd = msg.getVoltageDemand();
        double vm = msg.getVoltageMon();
        double diff = (vm - vd) / vd;
        double v;
        if (diff < 0.05 && diff > -0.05) {
          if (vm < vd) v = vm + rand() % 10;
          else v = vm - rand() % 10;
          if (v < 0) v = 0;
          msg.setVoltageMon(v);
        } else {
          if (vm < vd) v = vm + rand() % 100;
          else v = vm - rand() % 100;
          if (v < 0) v = 0;
          msg.setVoltageMon(v);
        }
        msg.setCurrentMon(v * 0.2);
      } else {
        msg.setVoltageMon(0);
        msg.setCurrentMon(0);
      }
      mutex[m_unit].unlock();
      sleep(2);
    }
  }
};

std::string toReplyText(CDCHVMessage& msg)
{
  std::stringstream ss;
  ss << "#" << msg.getUnit() << msg.getChannel();
  switch (msg.getCommand()) {
    case CDCHVMessage::ALL: {
      ss << StringUtil::form("GET=%04X,%04X,%d,%04X,%04X",
                             msg.getVoltageMon(), msg.getCurrentMon(),
                             msg.isSwitchOn(), msg.getRampUpSpeed(),
                             msg.getRampDownSpeed());
    } break;
    case CDCHVMessage::SWITCH:  ss << StringUtil::form("SW=%d", msg.isSwitchOn()); break;
    case CDCHVMessage::RAMPUP_SPEED: ss << StringUtil::form("RVU=%04X", msg.getRampUpSpeed()); break;
    case CDCHVMessage::RAMPDOWN_SPEED: ss << StringUtil::form("RVD=%04X", msg.getRampDownSpeed()); break;
    case CDCHVMessage::VOLTAGE_DEMAND: ss << StringUtil::form("CH5=%04X", msg.getVoltageDemand()); break;
    case CDCHVMessage::VOLTAGE_LIMIT: ss << StringUtil::form("CH2=%04X", msg.getVoltageLimit()); break;
    case CDCHVMessage::CURRENT_LIMIT: ss << StringUtil::form("CH7=%04X", msg.getCurrentLimit()); break;
    case CDCHVMessage::VOLTAGE_MON: ss << StringUtil::form("MNH1=%04X", msg.getVoltageMon()); break;
    case CDCHVMessage::CURRENT_MON: ss << StringUtil::form("NMH2=%04X", msg.getCurrentMon()); break;
    default: break;
  }
  return ss.str();
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    LogFile::debug("Usage : ./hvmaster <crateid> <ip> <port>");
    return 1;
  }
  for (int unit = 0; unit < MAX_UNITS; unit++) {
    CDCHVMessage* hvmsg = new CDCHVMessage[MAX_UNITS];
    for (int i = 0; i < MAX_STORES; i++) {
      hvmsg[i].setUnit(unit + 1);
      hvmsg[i].setChannel(1);
    }
    hvmsg_v.push_back(hvmsg);
    PThread(new HVEmulator(unit));
  }
  TCPServerSocket server_socket(argv[2], atoi(argv[3]));
  server_socket.open();
  while (true) {
    TCPSocket socket;
    try {
      socket = server_socket.accept();
    } catch (const IOException& e) {
      LogFile::debug("Server socket Error:%s", e.what());
      return 1;
    }
    TCPSocketWriter writer(socket);
    TCPSocketReader reader(socket);
    while (true) {
      CDCHVMessage msg;
      try {
        std::stringstream ss;
        while (true) {
          char c = reader.readChar();
          if (c == '\r') break;
          //std::cout << c;
          ss << c;
        }
        //std::cout << std::endl;
        msg.read(ss.str());
      } catch (const IOException& e) {
        LogFile::debug("Socket reading error : %s", e.what());
        break;
      }
      try {
        std::stringstream ss;
        CDCHVMessage msg_org = msg;
        CDCHVMessage::Command command = msg.getCommand();
        for (int iunit = 0; iunit < MAX_UNITS; iunit++) {
          CDCHVMessage& hvmsg(hvmsg_v[iunit][0]);
          msg = msg_org;
          if ((msg.getUnit() == 0 && msg.getChannel() == 0) ||
              (msg.getUnit() == hvmsg.getUnit() && msg.getChannel() == hvmsg.getChannel())) {
            if (msg.getMessageType() == CDCHVMessage::GET) {
              mutex[iunit].lock();
              msg = hvmsg;
              mutex[iunit].unlock();
              msg.setMessageType(CDCHVMessage::GET);
              msg.setCommand(command);
              if (ss.str().size() > 0) ss << ",";
              ss << toReplyText(msg);
            } else if (msg.getMessageType() == CDCHVMessage::SET) {
              switch (msg.getCommand()) {
                case CDCHVMessage::ALL: {
                  hvmsg.setVoltageDemand(msg.getVoltageDemand());
                  hvmsg.setSwitchOn(msg.isSwitchOn());
                  hvmsg.setVoltageLimit(msg.getVoltageLimit());
                  hvmsg.setCurrentLimit(msg.getCurrentLimit());
                  hvmsg.setRampUpSpeed(msg.getRampUpSpeed());
                  hvmsg.setRampDownSpeed(msg.getRampDownSpeed());
                } break;
                case CDCHVMessage::SWITCH: hvmsg.setSwitchOn(msg.isSwitchOn()); break;
                case CDCHVMessage::RAMPUP_SPEED: hvmsg.setRampUpSpeed(msg.getRampUpSpeed()); break;
                case CDCHVMessage::RAMPDOWN_SPEED: hvmsg.setRampUpSpeed(msg.getRampDownSpeed()); break;
                case CDCHVMessage::VOLTAGE_DEMAND: hvmsg.setVoltageDemand(msg.getVoltageDemand()); break;
                case CDCHVMessage::VOLTAGE_LIMIT: hvmsg.setVoltageLimit(msg.getVoltageLimit()); break;
                case CDCHVMessage::CURRENT_LIMIT: hvmsg.setCurrentLimit(msg.getCurrentLimit()); break;
                case CDCHVMessage::STORE: {
                  mutex[iunit].lock();
                  hvmsg_v[iunit][msg.getStoreId()] = hvmsg;
                  mutex[iunit].unlock();
                }; break;
                case CDCHVMessage::RECALL: {
                  mutex[iunit].lock();
                  hvmsg_v[iunit][msg.getStoreId()].setSwitchOn(hvmsg.isSwitchOn());
                  hvmsg_v[iunit][msg.getStoreId()].setVoltageMon(hvmsg.getVoltageMon());
                  hvmsg_v[iunit][msg.getStoreId()].setCurrentMon(hvmsg.getCurrentMon());
                  hvmsg = hvmsg_v[iunit][msg.getStoreId()];
                  mutex[iunit].unlock();
                }; break;
                default: break;
              }
            }
          }
        }
        if (msg.getMessageType() == CDCHVMessage::GET) {
          //std::cout << ss.str() << std::endl;
          ss << "\r";
          std::string s = ss.str();
          writer.write(s.c_str(), s.size());
        }
      } catch (const IOException& e) {
        LogFile::debug("Socket writing error : %s", e.what());
        break;
      }
    }
  }
  return 0;
}

