#include "daq/slc/hvcontrol/arich/ArichHVCommunicator.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Date.h>

#include <daq/slc/system/TCPServerSocket.h>
#include <daq/slc/system/TCPSocketWriter.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <unistd.h>
#include <ncurses.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>

int g_crateid = 0;
Belle2::DBInterface* g_db = NULL;

namespace Belle2 {

  class Monitor {
  private:
    const int m_max_unit;
    const int m_interval;
    ArichHVCommunicator& m_comm;
  public:
    Monitor(int max_unit, int interval, ArichHVCommunicator& comm)
      : m_max_unit(max_unit), m_interval(interval), m_comm(comm) {}
    ~Monitor() throw() {}
    void run() {
      while (true) {
        for (int i = 0; i < m_comm.getNUnits(); i++) {
          ArichHVUnit& unit(m_comm.getUnit(i));
          HVValue& value(unit.getValue());
          HVStatus& status(unit.getStatus());
          if (value.isTurnOn()) {
            double vd = value.getVoltageDemand();
            double vm = status.getVoltageMon();
            double diff = (vm - vd) / vd;
            double v;
            if (diff < 0.01 && diff > -0.01) {
              if (vm < vd) v = vm + rand() % 5;
              else v = vm - rand() % 5;
              if (v < 0) v = 0;
              status.setVoltageMon(v);
            } else if (diff < 0.05 && diff > -0.05) {
              if (vm < vd) v = vm + rand() % 50;
              else v = vm - rand() % 50;
              if (v < 0) v = 0;
              status.setVoltageMon(v);
            } else {
              if (vm < vd) v = vm + rand() % 1000;
              else v = vm - rand() % 1000;
              if (v < 0) v = 0;
              status.setVoltageMon(v);
            }
            status.setCurrentMon(v * 0.02);
          } else {
            status.setVoltageMon(0);
            status.setCurrentMon(0);
          }
        }
        sleep(m_interval);
      }
    }
  };

  class Brigde {
  private:
    const int m_max_unit;
    TCPSocket m_socket;
    ArichHVCommunicator& m_comm;

  public:
    Brigde(int max_unit, TCPSocket& socket, ArichHVCommunicator& comm)
      : m_max_unit(max_unit), m_socket(socket), m_comm(comm) {}
    ~Brigde() throw() {}
    void run() {
      TCPSocketWriter writer(m_socket);
      TCPSocketReader reader(m_socket);
      while (true) {
        ArichHVMessage msg;
        try {
          std::stringstream ss;
          while (true) {
            char c = reader.readChar();
            if (c == '\r') break;
            ss << c;
          }
          std::cerr << ss.str() << std::endl;
          msg.read(m_comm.useChannel(), ss.str());
        } catch (const IOException& e) {
          LogFile::debug("Socket reading error : %s", e.what());
          break;
        }
        try {
          std::stringstream ss;
          ArichHVMessage msg_out = msg;
          ArichHVMessage::Command command = msg.getCommand();
          for (int i = 0; i < m_max_unit; i++) {
            ArichHVUnit& unit(m_comm.getUnit(i));
            ArichHVCalib& calib(unit.getCalib());
            HVValue& param(unit.getValue());
            int slot = unit.getChannel().getSlot();
            int ch = unit.getChannel().getChannel();
            if ((msg.getUnit() == 0 || msg.getUnit() == slot) &&
                (msg.getChannel() == 0 || msg.getChannel() == ch)) {
              if (msg.getMessageType() == ArichHVMessage::GET) {
                msg_out.setUnit(slot);
                msg_out.setChannel(ch);
                msg_out.setMessageType(ArichHVMessage::GET);
                msg_out.setCommand(command);
                switch (command) {
                  case ArichHVMessage::ALL: {
                    msg_out.setSwitchOn(param.isTurnOn());
                    msg_out.setVoltageDemand(calib.encodeVoltage(param.getVoltageDemand()));
                    msg_out.setVoltageLimit(calib.encodeVoltage(param.getVoltageLimit()));
                    msg_out.setCurrentLimit(calib.encodeCurrent(param.getCurrentLimit()));
                    msg_out.setRampUpSpeed(calib.encodeVoltage(param.getRampUpSpeed()));
                    msg_out.setRampDownSpeed(calib.encodeVoltage(param.getRampDownSpeed()));
                  } break;
                  case ArichHVMessage::SWITCH:
                    msg_out.setSwitchOn(param.isTurnOn());
                    break;
                  case ArichHVMessage::RAMPUP_SPEED:
                    msg_out.setRampUpSpeed(calib.encodeVoltage(param.getRampUpSpeed()));
                    break;
                  case ArichHVMessage::RAMPDOWN_SPEED:
                    msg_out.setRampDownSpeed(calib.encodeVoltage(param.getRampDownSpeed()));
                    break;
                  case ArichHVMessage::VOLTAGE_DEMAND:
                    msg_out.setVoltageDemand(calib.encodeVoltage(param.getVoltageDemand()));
                    break;
                  case ArichHVMessage::VOLTAGE_LIMIT:
                    msg_out.setVoltageLimit(calib.encodeVoltage(param.getVoltageLimit()));
                    break;
                  case ArichHVMessage::CURRENT_LIMIT:
                    msg_out.setCurrentLimit(calib.encodeCurrent(param.getCurrentLimit()));
                    break;
                  case ArichHVMessage::STATE:
                    msg_out.setStatus((ArichHVMessage::StatusFlag)unit.getStatus().getState());
                    break;
                  case ArichHVMessage::VOLTAGE_MON:
                    msg_out.setVoltageMon(calib.encodeVoltage(unit.getStatus().getVoltageMon()));
                    break;
                  case ArichHVMessage::CURRENT_MON:
                    msg_out.setCurrentMon(calib.encodeCurrent(unit.getStatus().getCurrentMon()));
                    break;
                  default: break;
                }
                if (ss.str().size() > 0) ss << ",";
                ss << msg_out.toReplyText(m_comm.useChannel());
              } else if (msg.getMessageType() == ArichHVMessage::SET) {
                switch (command) {
                  case ArichHVMessage::ALL: {
                    m_comm.setVoltageDemand(slot, ch, calib.decodeVoltage(msg.getVoltageDemand()));
                    if (msg.getSwitchOn()) m_comm.switchOn(slot, ch);
                    else m_comm.switchOff(slot, ch);
                    m_comm.setVoltageLimit(slot, ch, calib.decodeVoltage(msg.getVoltageLimit()));
                    m_comm.setCurrentLimit(slot, ch, calib.decodeCurrent(msg.getCurrentLimit()));
                    m_comm.setRampUpSpeed(slot, ch, calib.decodeVoltage(msg.getRampUpSpeed()));
                    m_comm.setRampDownSpeed(slot, ch, calib.decodeVoltage(msg.getRampDownSpeed()));
                  } break;
                  case ArichHVMessage::SWITCH:
                    if (msg.getSwitchOn()) m_comm.switchOn(slot, ch);
                    else m_comm.switchOff(slot, ch);
                    break;
                  case ArichHVMessage::RAMPUP_SPEED:
                    m_comm.setRampUpSpeed(slot, ch, calib.decodeVoltage(msg.getRampUpSpeed()));
                    break;
                  case ArichHVMessage::RAMPDOWN_SPEED:
                    m_comm.setRampDownSpeed(slot, ch, calib.decodeVoltage(msg.getRampDownSpeed()));
                    break;
                  case ArichHVMessage::VOLTAGE_DEMAND:
                    m_comm.setVoltageDemand(slot, ch, calib.decodeVoltage(msg.getVoltageDemand()));
                    break;
                  case ArichHVMessage::VOLTAGE_LIMIT:
                    m_comm.setVoltageLimit(slot, ch, calib.decodeVoltage(msg.getVoltageLimit()));
                    break;
                  case ArichHVMessage::CURRENT_LIMIT:
                    m_comm.setCurrentLimit(slot, ch, calib.decodeCurrent(msg.getCurrentLimit()));
                    break;
                  case ArichHVMessage::STORE: {
                    m_comm.store(msg.getStoreId());
                  }; break;
                  case ArichHVMessage::RECALL: {
                    m_comm.recall(msg.getStoreId());
                  }; break;
                  default: break;
                }
              }
            }
          }
          if (msg_out.getMessageType() == ArichHVMessage::GET) {
            std::cerr << ss.str() << std::endl;
            ss << "\r";
            std::string s = ss.str();
            writer.write(s.c_str(), s.size());
          }
        } catch (const IOException& e) {
          return ;
        }
      }
    }

  };

}

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::cout << "Usage : " << argv[0] << " <config>" << std::endl;
    return 1;
  }
  ConfigFile cfile("slowcontrol",
                   StringUtil::form("hvcontrol/%s", argv[1]));
  g_crateid = cfile.getInt("hv.crateid");
  const std::string host = cfile.get("hv.hostname");
  const int port = cfile.getInt("hv.port");
  const int max_unit  = cfile.getInt("hv.nchannels");
  const int interval = cfile.getInt("hv.interval");
  const int usech = cfile.getInt("hv.usechannel");
  const int debug = cfile.getInt("hv.usechannel");
  ArichHVCommunicator comm(g_crateid, host, 0, usech, debug);
  for (int i = 0; i < max_unit; i++) {
    float voffset = 0;
    float vslope = 1;
    float coffset = 0;
    float cslope = 1;
    int slot = cfile.getInt(StringUtil::form("hv.channel[%d].slot", i + 1));
    int channel = cfile.getInt(StringUtil::form("hv.channel[%d].channel", i + 1));
    comm.addUnit(ArichHVUnit(ArichHVCalib(voffset, vslope, coffset, cslope),
                             HVChannel(g_crateid, slot, channel)));
  }
  for (int i = 0; i < max_unit; i++) {
    comm.getUnit(i).getValue().setRampUpSpeed(150);
    comm.getUnit(i).getValue().setRampDownSpeed(200);
    comm.getUnit(i).getValue().setVoltageLimit(9000);
    comm.getUnit(i).getValue().setCurrentLimit(200);
  }
  if (interval > 0) {
    PThread(new Monitor(max_unit, interval, comm));
  }

  TCPServerSocket server_socket("0.0.0.0", port);
  server_socket.open();
  while (true) {
    TCPSocket socket;
    try {
      socket = server_socket.accept();
      Brigde(max_unit, socket, comm).run();
    } catch (const IOException& e) {
      LogFile::debug("Server socket Error:%s", e.what());
      return 1;
    }
  }
  return (0);
}

