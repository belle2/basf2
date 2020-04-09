/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Markus Prim                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/pcore/zmq/connections/ZMQRawConnection.h>
#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <daq/roisend/h2m.h>

#include <zmq.hpp>

namespace Belle2 {

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

  private:
    /// The used raw connection
    ZMQRawOutput m_dataOutput;
    /// The used ROI connection
    ZMQRawOutput m_roiOutput;
  };
}
