#include "daq/slc/apps/hvcontrold/arich/ArichHVMessage.h"
#include "daq/slc/apps/hvcontrold/HVNodeInfo.h"
#include "daq/slc/apps/hvcontrold/HVCommand.h"

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/Debugger.h>
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

std::vector<ArichHVMessage*> hvmsg_v;
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
      ArichHVMessage& msg(hvmsg_v[m_unit][0]);
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

std::string toReplyText(ArichHVMessage& msg)
{
  std::stringstream ss;
  ss << "#" << msg.getUnit() << msg.getChannel();
  switch (msg.getCommand()) {
    case ArichHVMessage::ALL: {
      ss << Belle2::form("GET=%04X,%04X,%d,%04X,%04X",
                         msg.getVoltageMon(), msg.getCurrentMon(),
                         msg.isSwitchOn(), msg.getRampUpSpeed(),
                         msg.getRampDownSpeed());
    } break;
    case ArichHVMessage::SWITCH:  ss << Belle2::form("SW=%d", msg.isSwitchOn()); break;
    case ArichHVMessage::RAMPUP_SPEED: ss << Belle2::form("RVU=%04X", msg.getRampUpSpeed()); break;
    case ArichHVMessage::RAMPDOWN_SPEED: ss << Belle2::form("RVD=%04X", msg.getRampDownSpeed()); break;
    case ArichHVMessage::VOLTAGE_DEMAND: ss << Belle2::form("CH5=%04X", msg.getVoltageDemand()); break;
    case ArichHVMessage::VOLTAGE_LIMIT: ss << Belle2::form("CH2=%04X", msg.getVoltageLimit()); break;
    case ArichHVMessage::CURRENT_LIMIT: ss << Belle2::form("CH7=%04X", msg.getCurrentLimit()); break;
    case ArichHVMessage::VOLTAGE_MON: ss << Belle2::form("MNH1=%04X", msg.getVoltageMon()); break;
    case ArichHVMessage::CURRENT_MON: ss << Belle2::form("NMH2=%04X", msg.getCurrentMon()); break;
    default: break;
  }
  return ss.str();
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    Belle2::debug("Usage : ./hvmaster <crateid> <ip> <port>");
    return 1;
  }
  for (int unit = 0; unit < MAX_UNITS; unit++) {
    ArichHVMessage* hvmsg = new ArichHVMessage[MAX_UNITS];
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
      Belle2::debug("Server socket Error:%s", e.what());
      return 1;
    }
    TCPSocketWriter writer(socket);
    TCPSocketReader reader(socket);
    while (true) {
      ArichHVMessage msg;
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
        Belle2::debug("Socket reading error : %s", e.what());
        break;
      }
      try {
        std::stringstream ss;
        ArichHVMessage msg_org = msg;
        ArichHVMessage::Command command = msg.getCommand();
        for (int iunit = 0; iunit < MAX_UNITS; iunit++) {
          ArichHVMessage& hvmsg(hvmsg_v[iunit][0]);
          msg = msg_org;
          if ((msg.getUnit() == 0 && msg.getChannel() == 0) ||
              (msg.getUnit() == hvmsg.getUnit() && msg.getChannel() == hvmsg.getChannel())) {
            if (msg.getMessageType() == ArichHVMessage::GET) {
              mutex[iunit].lock();
              msg = hvmsg;
              mutex[iunit].unlock();
              msg.setMessageType(ArichHVMessage::GET);
              msg.setCommand(command);
              if (ss.str().size() > 0) ss << ",";
              ss << toReplyText(msg);
            } else if (msg.getMessageType() == ArichHVMessage::SET) {
              switch (msg.getCommand()) {
                case ArichHVMessage::ALL: {
                  hvmsg.setVoltageDemand(msg.getVoltageDemand());
                  hvmsg.setSwitchOn(msg.isSwitchOn());
                  hvmsg.setVoltageLimit(msg.getVoltageLimit());
                  hvmsg.setCurrentLimit(msg.getCurrentLimit());
                  hvmsg.setRampUpSpeed(msg.getRampUpSpeed());
                  hvmsg.setRampDownSpeed(msg.getRampDownSpeed());
                } break;
                case ArichHVMessage::SWITCH: hvmsg.setSwitchOn(msg.isSwitchOn()); break;
                case ArichHVMessage::RAMPUP_SPEED: hvmsg.setRampUpSpeed(msg.getRampUpSpeed()); break;
                case ArichHVMessage::RAMPDOWN_SPEED: hvmsg.setRampUpSpeed(msg.getRampDownSpeed()); break;
                case ArichHVMessage::VOLTAGE_DEMAND: hvmsg.setVoltageDemand(msg.getVoltageDemand()); break;
                case ArichHVMessage::VOLTAGE_LIMIT: hvmsg.setVoltageLimit(msg.getVoltageLimit()); break;
                case ArichHVMessage::CURRENT_LIMIT: hvmsg.setCurrentLimit(msg.getCurrentLimit()); break;
                case ArichHVMessage::STORE: {
                  mutex[iunit].lock();
                  hvmsg_v[iunit][msg.getStoreId()] = hvmsg;
                  mutex[iunit].unlock();
                }; break;
                case ArichHVMessage::RECALL: {
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
        if (msg.getMessageType() == ArichHVMessage::GET) {
          //std::cout << ss.str() << std::endl;
          ss << "\r";
          std::string s = ss.str();
          writer.write(s.c_str(), s.size());
        }
      } catch (const IOException& e) {
        Belle2::debug("Socket writing error : %s", e.what());
        break;
      }
    }
  }
  return 0;
}

