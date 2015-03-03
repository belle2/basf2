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
#include <vector>
#include <list>
#include <iostream>
#include <sstream>

int g_crateid = 0;
Belle2::DBInterface* g_db = NULL;

void getValueAll(int max_unit, Belle2::ArichHVCommunicator& comm)
{
  comm.requestSwitch(0, 0);
  comm.requestRampUpSpeed(0, 0);
  comm.requestRampDownSpeed(0, 0);
  comm.requestVoltageDemand(0, 0);
  comm.requestVoltageLimit(0, 0);
  comm.requestCurrentLimit(0, 0);
}

namespace Belle2 {

  class Listener {
  private:
    const int m_max_unit;
    const int m_interval;
    ArichHVCommunicator& m_comm;

  public:
    Listener(int max_unit, int interval, ArichHVCommunicator& comm)
      : m_max_unit(max_unit), m_interval(interval), m_comm(comm) {}
    ~Listener() throw() {}
    void run() {
      std::list<ArichHVMessage> msg_l;
      while (true) {
        if (msg_l.empty()) msg_l = m_comm.read();
        ArichHVMessage msg = msg_l.front();
        msg_l.pop_front();
        if (msg.getMessageType() == ArichHVMessage::GET) continue;
        int unit = msg.getUnit();
        int channel = msg.getChannel();
        ArichHVUnitListIter it(m_comm.find(unit, channel));
        if (it == m_comm.getUnits().end()) continue;
        ArichHVMessage::Command command = msg.getCommand();
        ArichHVCalib& calib(it->getCalib());
        HVValue& value(it->getValue());
        HVStatus& status(it->getStatus());
        switch (command) {
          case ArichHVMessage::ALL: {
            value.setTurnOn(msg.getStatus() > 0);
            value.setVoltageDemand(calib.decodeVoltage(msg.getVoltageDemand()));
            value.setVoltageLimit(calib.decodeVoltage(msg.getVoltageLimit()));
            value.setCurrentLimit(calib.decodeCurrent(msg.getCurrentLimit()));
            value.setRampUpSpeed(calib.decodeVoltage(msg.getRampUpSpeed()));
            value.setRampDownSpeed(calib.decodeVoltage(msg.getRampDownSpeed()));
          } break;
          case ArichHVMessage::SWITCH:
            value.setTurnOn(msg.getStatus() > 0);
            break;
          case ArichHVMessage::RAMPUP_SPEED:
            value.setRampUpSpeed(calib.decodeVoltage(msg.getRampUpSpeed()));
            break;
          case ArichHVMessage::RAMPDOWN_SPEED:
            value.setRampDownSpeed(calib.decodeVoltage(msg.getRampDownSpeed()));
            break;
          case ArichHVMessage::VOLTAGE_DEMAND:
            value.setVoltageDemand(calib.decodeVoltage(msg.getVoltageDemand()));
            break;
          case ArichHVMessage::VOLTAGE_LIMIT:
            value.setVoltageLimit(calib.decodeVoltage(msg.getVoltageLimit()));
            break;
          case ArichHVMessage::CURRENT_LIMIT:
            value.setCurrentLimit(calib.decodeCurrent(msg.getCurrentLimit()));
            break;
          case ArichHVMessage::MON:
            status.setState((HVStatus::State)msg.getStatus());
            status.setVoltageMon(calib.decodeVoltage(msg.getVoltageMon()));
            status.setCurrentMon(calib.decodeCurrent(msg.getCurrentMon()));
            break;
          case ArichHVMessage::STATE:
            status.setState((HVStatus::State)msg.getStatus());
            break;
          case ArichHVMessage::VOLTAGE_MON:
            status.setVoltageMon(calib.decodeVoltage(msg.getVoltageMon()));
            break;
          case ArichHVMessage::CURRENT_MON:
            status.setCurrentMon(calib.decodeCurrent(msg.getCurrentMon()));
            break;
          default: break;
        }
      }
    }
  };

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
        m_comm.requestSwitch(0, 0);
        m_comm.requestVoltageMonitor(0, 0);
        m_comm.requestCurrentMonitor(0, 0);
        sleep(m_interval);
        if (g_db) g_db->connect();
        for (int i = 0; i < m_max_unit; i++) {
          ArichHVUnit& unit(m_comm.getUnit(i));
          HVValue& param(unit.getValue());
          HVChannel& ch(unit.getChannel());
          if (g_db) {
            g_db->execute("insert into hvinfo (record_time, crate, "
                          "unit, ch, switchon, rampup, rampdown, vset, "
                          "vlimit, climit, state, vmon, cmon) "
                          "values (current_timestamp, %d, %d, %d, %d, %.0f, %.0f, %.0f, "
                          "%.0f, %.0f, %d, %.1f, %.1f);",
                          g_crateid, ch.getSlot(),
                          ch.getChannel(), param.isTurnOn(),
                          param.getRampUpSpeed(),
                          param.getRampDownSpeed(),
                          param.getVoltageDemand(),
                          param.getVoltageLimit(),
                          param.getCurrentLimit(),
                          unit.getStatus().getState(),
                          unit.getStatus().getVoltageMon(),
                          unit.getStatus().getCurrentMon());
          }
        }
        if (g_db) g_db->close();
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
          msg.read(m_comm.useChannel(), reader);
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
                    msg_out.setVoltageDemand(calib.encodeVoltage(param.getVoltageDemand()));
                    msg_out.setSwitchOn(param.isTurnOn());
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
                  case ArichHVMessage::MON:
                    msg_out.setStatus((ArichHVMessage::StatusFlag)unit.getStatus().getState());
                    msg_out.setVoltageMon(calib.encodeVoltage(unit.getStatus().getVoltageMon()));
                    msg_out.setCurrentMon(calib.encodeCurrent(unit.getStatus().getCurrentMon()));
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
                    getValueAll(m_max_unit, m_comm);
                  }; break;
                  case ArichHVMessage::RECALL: {
                    m_comm.recall(msg.getStoreId());
                    getValueAll(m_max_unit, m_comm);
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
  if (argc < 3) {
    std::cout << "Usage : " << argv[0] << " <config> <calib>" << std::endl;
    return 1;
  }
  ConfigFile cfile("slowcontrol",
                   StringUtil::form("hvcontrol/%s", argv[1]),
                   StringUtil::form("hvcontrol/%s", argv[2]));
  g_crateid = cfile.getInt("hv.crateid");
  const std::string host = cfile.get("hv.hostname");
  const int port = cfile.getInt("hv.port");
  const int max_unit  = cfile.getInt("hv.nchannels");
  const int mode = cfile.getInt("hv.digitmode");
  const int interval = cfile.getInt("hv.interval");
  const int usech = cfile.getInt("hv.usechannel");
  if (cfile.getInt("hv.usedb")) {
    g_db = new PostgreSQLInterface(cfile.get("database.host"),
                                   cfile.get("database.dbname"),
                                   cfile.get("database.user"),
                                   cfile.get("database.password"),
                                   cfile.getInt("database.port"));
  } else {
    g_db = NULL;
  }
  ArichHVCommunicator comm(g_crateid, host, port, usech);
  for (int i = 0; i < max_unit; i++) {
    float voffset = cfile.getFloat(StringUtil::form("hv.channel[%d].voffset", i + 1));
    float vslope = cfile.getFloat(StringUtil::form("hv.channel[%d].vslope", i + 1));
    float coffset = cfile.getFloat(StringUtil::form("hv.channel[%d].coffset", i + 1));
    float cslope = cfile.getFloat(StringUtil::form("hv.channel[%d].cslope", i + 1));
    if (vslope <= 0) vslope = 1;
    if (cslope <= 0) cslope = 1;
    if (mode != 0) {
      voffset = coffset = 0;
      vslope = cslope = 1;
    }
    int slot = cfile.getInt(StringUtil::form("hv.channel[%d].slot", i + 1));
    int channel = cfile.getInt(StringUtil::form("hv.channel[%d].channel", i + 1));
    comm.addUnit(ArichHVUnit(ArichHVCalib(voffset, vslope, coffset, cslope),
                             HVChannel(g_crateid, slot, channel)));
  }
  comm.connect();
  getValueAll(max_unit, comm);
  if (interval > 0) {
    PThread(new Listener(max_unit, interval, comm));
    PThread(new Monitor(max_unit, interval, comm));
  }

  TCPServerSocket server_socket("0.0.0.0", 44001);
  server_socket.open();
  while (true) {
    TCPSocket socket;
    try {
      socket = server_socket.accept();
      PThread(new Brigde(max_unit, socket, comm));
    } catch (const IOException& e) {
      LogFile::debug("Server socket Error:%s", e.what());
      return 1;
    }
  }
  return (0);
}

