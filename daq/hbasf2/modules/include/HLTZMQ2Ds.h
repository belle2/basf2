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
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/core/RandomGenerator.h>

#include <memory>
#include <zmq.hpp>

namespace Belle2 {
  /**
   * Input module in the ZMQ reconstruction path receiving events via ZMQ
   * and deserializing the to the data store. The connection to the previous ZMQ application
   * (most likely a distributor or collector) is handled via a load balanced connection
   * (input in this case). The buffer size for the load balanced connection can be
   * controlled via a module parameter.
   * This module only works in the context of the HLT when using the HLTEventProcessor,
   * due to the special form the first event as well as beginRun and endRun are handled.
   * Please read the overall description in the HLTEventProcessor for an overview.
   *
   * Before the first real event is received (which is the first time the event function
   * is called by the HLTEventProcessor, but before the forking), the
   * event meta data is initialized with a predefined experiment and run number (set via
   * module parameters) so make module initialization in all other modules possible.
   * However, no event function should be called for other modules in this event
   * (as the data store is invalid). In the first real event after the forking,
   * the connection and streamer is initialized. Then, normal event messages
   * are deserialized and written to data store. End run or terminate messages are
   * handled by setting a special flag of the EventMetaData. Also in this case
   * the remaining modules should not process this event via an event function
   * (assured by the HLTEventProcessor).
   *
   * TODO: currently, this module is also used in the ExressReco use case, which
   * is good as it makes things more uniform. THe downside is, that we also have to define
   * the store arrays, which are not so trivial in this case (and needs to be kep in sync).
   * We might change this in the future.
   */
  class HLTZMQ2DsModule : public Module {
  public:
    /// Register the module parameters
    HLTZMQ2DsModule();

    /// Register the needed store arrays. In case of the HLT, this are only the raw data objects, for ExpressReco, the streamer defines an additional list
    void initialize() override;
    /// Handle the cases (a) before first event, (b) first event and (c) normal event as described in the class documentation.
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
    /// Are we still before the first real event = before the modules are initialized = before the forking?
    bool m_inInitialize = true;

    /// Module parameter: ZMQ address of the input ZMQ application
    std::string m_param_input;
    /// Module parameter: how many events should be kept in flight. Has an impact on the stopping time as well as the rate stability
    unsigned int m_param_bufferSize = 2;
    /// Module parameter: additional to the raw data, also register the data store objects needed for express reco. TODO: this might change
    bool m_param_addExpressRecoObjects = false;

    /// Default experiment number to be set during initialization/run end to have something to load the geometry. Module parameter but will be updated
    unsigned int m_lastExperiment = 42;
    /// Default run number to be set during initialization/run end to have something to load the geometry. Module parameter but will be updated
    unsigned int m_lastRun = 8;

    /// Reference to the event meta data to set numbers and flags according to the state and received messages
    StoreObjPtr<EventMetaData> m_eventMetaData;
  };
}
