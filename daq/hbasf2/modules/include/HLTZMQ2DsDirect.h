/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <daq/hbasf2/utils/HLTStreamHelper.h>
#include <framework/pcore/zmq/connections/ZMQLoadBalancedConnection.h>

#include <framework/pcore/zmq/utils/ZMQParent.h>

#include <memory>
#include <string>

namespace Belle2 {
  /**
   * Special ZMQ2Ds module without the HLT-specific handling of initialization and begin/end run.
   * Basically just receives and event from ZMQ and stores it to the datastore without further processing.
   * Similar to the former Rx module.
   */
  class HLTZMQ2DsDirectModule : public Module {
  public:
    /// Register the module parameters
    HLTZMQ2DsDirectModule();

    /// Receive the first event and initialize the data store with it
    void initialize() override;
    /// If not in the first event, receive an event and store in the DS
    void event() override;

  private:
    /// ZMQ Parent needed for the connections
    std::shared_ptr<ZMQParent> m_parent;
    /// Load balanced connection to the previous ZMQ application
    std::unique_ptr<ZMQLoadBalancedInput> m_input;

    /// Utility class for deserialization
    HLTStreamHelper m_streamHelper;

    /// Are we still in the first real event?
    bool m_firstEvent = true;

    /// Module parameter: ZMQ address of the input ZMQ application
    std::string m_param_input;
    /// Module parameter: how many events should be kept in flight. Has an impact on the stopping time as well as the rate stability
    unsigned int m_param_bufferSize = 20;

    void readEvent();
  };
}