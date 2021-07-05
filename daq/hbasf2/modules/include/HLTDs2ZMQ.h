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
#include <framework/pcore/zmq/connections/ZMQConfirmedConnection.h>

#include <framework/pcore/zmq/utils/ZMQParent.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <memory>
#include <zmq.hpp>

namespace Belle2 {
  /**
   * On every event, serialize the data store and send the binary data out to
   * the connected ZMQ application (most likely a collector or final collector).
   *
   * The sending is handled via a confirmed connection (output in this case),
   * so all the typical behaviour applies. Also sends out end run and termination
   * messages. Depending on the module setting, can send out events in
   * raw format (with send header and trailer and a serialized event message as buffer)
   * or only as normal ROOT serialized stream (evt message).
   * The former is the typical use case when talking with e.g. storage, the
   * latter can be used for local tests or when sending full events e.g. to the event display.
   * Please note that the environment setting of the stream objects heavily
   * influences the time spent in this module (because serialization needs time).
   *
   * This module is only useful in the HLT context or for testing it and it optimized to be used
   * together with the HLTEventProcessor. Please note the special handling of the first event in the
   * HLTEventProcessor (therefore we do not stream the first event)
   */
  class HLTDs2ZMQModule : public Module {
  public:
    /// Register the module parameters
    HLTDs2ZMQModule();

    /// On the first event, initialize the streamer and the connection. Serializes and sends out the data store content.
    void event() override;
    /// Send out a run stop message.
    void endRun() override;
    /// Send out a terminate message.
    void terminate() override;

  private:
    /// Module parameter: the ZMQ address of the connected application (to receive the messages)
    std::string m_param_output;
    /// Module paremeter: send out raw data with send header and trailer around the evtmessage instead of just the evtmessage.
    bool m_param_raw;

    /// ZMQ Parent needed for the connections
    std::shared_ptr<ZMQParent> m_parent;
    /// Confirmed connection to the ZMQ application
    std::unique_ptr<ZMQConfirmedOutput> m_output;

    /// Utility class used for serialization
    HLTStreamHelper m_streamHelper;
    /// Are we still in the first event? Please note the special handling of the first event in the HLTEventProcessor (therefore we do not stream the first event)
    bool m_firstEvent = true;
  };
}