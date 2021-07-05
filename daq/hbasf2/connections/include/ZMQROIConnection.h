/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/connections/ZMQRawConnection.h>
#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <daq/roisend/h2m.h>

#include <zmq.hpp>

namespace Belle2 {

  /**
   * Dedicated output to send ROI messages to the PXD ONSEN system.
   *
   * Behaves exactly the same as a RawOutput (it is implemented with one)
   * but adds the PXD-specific headers and trailers around the message
   * when sending.
   */
  class ZMQROIOutput : public ZMQConnection {
  public:
    /// Create a new ROI output basically initializing the raw output member
    ZMQROIOutput(const std::string& outputAddress, const std::shared_ptr<ZMQParent>& parent);

    /// Send a message and add the PXD specific header and trailer
    void handleEvent(zmq::message_t message);
    /// Copy the functionality from the raw output
    std::string getMonitoringJSON() const final;
    /// Copy the functionality from the raw output
    void handleIncomingData();
    /// Copy the functionality from the raw output
    bool isReady() const final;
    /// Copy the functionality from the raw output
    std::vector<zmq::socket_t*> getSockets() const final;
    /// Return the connection string
    std::string getEndPoint() const { return m_rawOutput.getEndPoint(); }
  private:
    /// Size of the header in bytes
    static constexpr const unsigned int HEADER_SIZE = sizeof(struct h2m_header_t);
    /// Size of the trailer in bytes
    static constexpr const unsigned int TRAILER_SIZE = sizeof(struct h2m_footer_t);

    /// The used raw output
    ZMQRawOutput m_rawOutput;

    /// Helper function to add the header with the given message size
    void addHeader(char* buffer, unsigned int size);
    /// Helper function to add the trailer
    void addTrailer(char* buffer);
  };

  /**
   * Helper connection hosting both a normal raw and a ROI output and sending to both at the same time.
   *
   * The normal message will be sent to the raw connection whereas the additional
   * message will be sent to the roi connection.
   *
   * As two connections are hosted also two monitoring JSON can be requested.
   */
  class ZMQDataAndROIOutput : public ZMQConnection {
  public:
    /// Initialize the raw and roi connection
    ZMQDataAndROIOutput(const std::string& dataAddress, const std::string& roiAddress, bool addEventSize,
                        const std::shared_ptr<ZMQParent>& parent);

    /// Get the monitoring JSON from the raw connection
    std::string getMonitoringJSON() const final;
    /// Get the monitoring JSON from the ROI connection
    std::string getROIMonitoringJSON() const;

    /// Send the normal data message to raw and the additional message to ROI
    void handleEvent(std::unique_ptr<ZMQNoIdMessage> message);
    /// Handle both a socket (dis)connect by raw or ROI
    void handleIncomingData();
    /// Ready only when both sockets are ready
    bool isReady() const final;
    /// Return both sockets for polling
    std::vector<zmq::socket_t*> getSockets() const final;
    /// Return the connection string
    std::string getEndPoint() const { return m_dataOutput.getEndPoint() + ";" + m_roiOutput.getEndPoint(); }
  private:
    /// The used raw connection
    ZMQRawOutput m_dataOutput;
    /// The used ROI connection
    ZMQRawOutput m_roiOutput;
  };
}
