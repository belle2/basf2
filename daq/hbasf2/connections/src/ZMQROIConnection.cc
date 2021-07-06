/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/connections/ZMQROIConnection.h>
#include <arpa/inet.h>

using namespace Belle2;

ZMQDataAndROIOutput::ZMQDataAndROIOutput(const std::string& dataAddress, const std::string& roiAddress,
                                         bool addEventSize, const std::shared_ptr<ZMQParent>& parent) :
  m_dataOutput(dataAddress, addEventSize, parent), m_roiOutput(roiAddress, false, parent) {}

void ZMQDataAndROIOutput::handleEvent(std::unique_ptr<ZMQNoIdMessage> message)
{
  if (message->isMessage(EMessageTypes::c_rawDataMessage) or message->isMessage(EMessageTypes::c_eventMessage)) {
    B2ASSERT("The data message needs to be present!", message->getDataMessage().size() > 0);
    B2ASSERT("The roi message needs to be present!", message->getAdditionalDataMessage().size() > 0);

    m_dataOutput.handleEvent(std::move(message->getDataMessage()));
    m_roiOutput.handleEvent(std::move(message->getAdditionalDataMessage()));
  }
}

void ZMQDataAndROIOutput::handleIncomingData()
{
  if (ZMQConnection::hasMessage(&m_dataOutput)) {
    m_dataOutput.handleIncomingData();
  }
  if (ZMQConnection::hasMessage(&m_roiOutput)) {
    m_roiOutput.handleIncomingData();
  }
}

bool ZMQDataAndROIOutput::isReady() const
{
  return m_dataOutput.isReady() and m_roiOutput.isReady();
}

std::vector<zmq::socket_t*> ZMQDataAndROIOutput::getSockets() const
{
  return {m_dataOutput.getSockets()[0], m_roiOutput.getSockets()[0]};
}

std::string ZMQDataAndROIOutput::getMonitoringJSON() const
{
  return m_dataOutput.getMonitoringJSON();
}

std::string ZMQDataAndROIOutput::getROIMonitoringJSON() const
{
  return m_roiOutput.getMonitoringJSON();
}
