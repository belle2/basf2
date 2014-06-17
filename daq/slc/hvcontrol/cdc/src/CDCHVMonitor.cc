#include "daq/slc/hvcontrol/cdc/CDCHVMonitor.h"

#include <daq/slc/hvcontrol/HVState.h>

#include <daq/slc/system/LogFile.h>

#include <sstream>
#include <unistd.h>

using namespace Belle2;

void CDCHVMonitor::run()
{
  while (true) {
    if (!m_comm->connect()) {
      LogFile::debug("Socket connection error to HV crate (id=%d)",
                     m_comm->getId());
      sleep(5);
      continue;
    }
    while (true) {
      HVControlCallback* callback = m_comm->getCallback();
      const HVConfig& config(callback->getConfig());
      try {
        for (size_t i = 0; i < config.getNChannels(); i++) {
          const HVChannel& channel(config.getChannel(i));
          if (channel.getCrate() != m_comm->getId()) continue;
          HVChannelStatus& status(callback->getChannelStatus(i));
          CDCHVMessage msg = m_comm->readParams(/*channel.getSlot(),*/
                               channel.getChannel());
          status.setVoltageMon(msg.getVoltageMon());
          status.setCurrentMon(msg.getCurrentMon());
          switch (msg.getStatus()) {
            case CDCHVMessage::OFF: {
              status.setState(HVState::OFF_S.getId());
            } break;
            case CDCHVMessage::ON: {
              double vd = msg.getVoltageDemand();
              double vm = msg.getVoltageMon();
              if (vd > 0) {
                double diff = (vm - vd) / vd;
                if (diff < 0.05 && diff > -0.05) {
                  status.setState(callback->getStateDemand());
                } else if (vm - vd < 0) {
                  status.setState(HVState::RAMPINGUP_TS);
                } else if (vm - vd > 0) {
                  status.setState(HVState::RAMPINGDOWN_TS);
                }
              }
            } break;
            case CDCHVMessage::OCP: {
              status.setState(HVState::OVER_CURRENT_ES);
            } break;
            case CDCHVMessage::OVP: {
              status.setState(HVState::OVER_VOLTAGE_ES);
            } break;
          }
        }
      } catch (const IOException& e) {
        for (size_t i = 0; i < config.getNChannels(); i++) {
          if (config.getChannel(i).getCrate() != m_comm->getId()) continue;
          HVChannelStatus& status(callback->getChannelStatus(i));
          status.setState(Enum::UNKNOWN);
        }
        LogFile::error("Socket error: %s", e.what());
        sleep(5);
        break;
      }
      sleep(2);
    }
  }
}
