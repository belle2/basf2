/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <daq/hbasf2/connections/ZMQROIConnection.h>
#include <arpa/inet.h>

using namespace Belle2;

ZMQROIOutput::ZMQROIOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent) : m_rawOutput(outputAddress,
      false,
      parent) {}

void ZMQROIOutput::handleEvent(zmq::message_t message)
{
  const auto dataSize = message.size();

  zmq::message_t wrappedMessage(HEADER_SIZE + dataSize + TRAILER_SIZE);
  char* buffer = wrappedMessage.data<char>();

  addHeader(&buffer[0], wrappedMessage.size());
  memcpy(&buffer[HEADER_SIZE], message.data<char>(), dataSize);
  addTrailer(&buffer[HEADER_SIZE + dataSize]);

  m_rawOutput.handleEvent(std::move(wrappedMessage));
}

std::string ZMQROIOutput::getMonitoringJSON() const
{
  return m_rawOutput.getMonitoringJSON();
}

void ZMQROIOutput::handleIncomingData()
{
  m_rawOutput.handleIncomingData();
}

bool ZMQROIOutput::isReady() const
{
  return m_rawOutput.isReady();
}

std::vector<zmq::socket_t*> ZMQROIOutput::getSockets() const
{
  return m_rawOutput.getSockets();
}

void ZMQROIOutput::addHeader(char* buffer, unsigned int size)
{
  auto* header = reinterpret_cast<struct h2m_header_t*>(buffer);
  size_t sizeInWords = size / sizeof(unsigned int);

  header->h_n_words = htonl(sizeInWords);
  header->h_n_words_in_header = htonl(sizeof(struct h2m_header_t) / sizeof(unsigned int));
  header->h_reserved[0] = 0x02701144;
  // TODO: what is this??? header->h_reserved[1] = 0x02410835;
  header->h_marker = 0x5f5f5f5f;
}

void ZMQROIOutput::addTrailer(char* buffer)
{
  auto* trailer = reinterpret_cast<struct h2m_footer_t*>(buffer);
  trailer->f_reserved[0] = 0x02701144;
  trailer->f_reserved[1] = 0x02410835;
  trailer->f_checksum = 0;
  trailer->f_marker = 0xa0a0a0a0;
}

ZMQDataAndROIOutput::ZMQDataAndROIOutput(const std::string& dataAddress, const std::string& roiAddress,
                                         bool addEventSize, const std::shared_ptr<ZMQParent>& parent) :
  m_dataOutput(dataAddress, addEventSize, parent), m_roiOutput(roiAddress, parent) {}

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