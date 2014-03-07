#include "daq/slc/apps/hvcontrold/arich/ArichHVMonitor.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include <daq/slc/system/LogFile.h>

#include <sstream>
#include <unistd.h>

using namespace Belle2;

void ArichHVMonitor::run()
{
  while (true) {
    if (!m_comm->connect()) {
      LogFile::debug("Socket connection error to HV crate (id=%d)", m_comm->getId());
      sleep(5);
      continue;
    }
    HVNodeInfo& info(m_comm->getNodeInfo());
    while (true) {
      try {
        for (size_t i = 0; i < info.getChannelList().size(); i++) {
          HVChannelInfo& ch_info(info.getChannel(i));
          HVChannelConfig& config(ch_info.getConfig());
          HVChannelStatus& status(ch_info.getStatus());
          if (config.getCrate() != m_comm->getId()) continue;
          ArichHVMessage msg =
            m_comm->readParams(config.getSlot(), config.getChannel());
          status.setVoltageMon(msg.getVoltageMon());
          status.setCurrentMon(msg.getCurrentMon());
          switch (msg.getStatus()) {
            case ArichHVMessage::OFF: {
              status.setState(HVState::OFF_S.getId());
            } break;
            case ArichHVMessage::ON: {
              double vd = msg.getVoltageDemand();
              double vm = msg.getVoltageMon();
              if (vd > 0) {
                double diff = (vm - vd) / vd;
                if (diff < 0.05 && diff > -0.05) {
                  status.setState(HVState::STABLE_S);
                } else if (vm - vd < 0) {
                  status.setState(HVState::RAMPINGUP_TS);
                } else if (vm - vd > 0) {
                  status.setState(HVState::RAMPINGDOWN_TS);
                }
              }
            } break;
            case ArichHVMessage::OCP: {
              status.setState(HVState::OVER_CURRENT_ES);
            } break;
            case ArichHVMessage::OVP: {
              status.setState(HVState::OVER_VOLTAGE_ES);
            } break;
          }
        }
      } catch (const IOException& e) {
        for (size_t i = 0; i < info.getChannelList().size(); i++) {
          HVChannelInfo& ch_info(info.getChannel(i));
          HVChannelConfig& config(ch_info.getConfig());
          if (config.getCrate() != m_comm->getId()) continue;
          ch_info.getStatus().setState(State::UNKNOWN);
        }
        LogFile::error("Socket error: %s", e.what());
        sleep(5);
        break;
      }
      sleep(2);
    }
  }
}
