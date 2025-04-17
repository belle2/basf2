/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/pcore/zmq/utils/StreamHelper.h>
#include <framework/pcore/zmq/connections/ZMQRawConnection.h>

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
   * The sending is handled via a raw connection (output in this case),
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
  class HLTDs2ZMQRawModule : public Module {
  public:
    /// Register the module parameters
    HLTDs2ZMQRawModule();

    /// Module initialization, only applied when the initial m_firstEvent is false.
    void initialize() override;
    /// On the first event, initialize the streamer and the connection. Serializes and sends out the data store content.
    void event() override;

  private:
    /// Module parameter: the ZMQ address of the connected application (to receive the messages)
    std::string m_param_output;
    /// Module paremeter: send out raw data with send header and trailer around the evtmessage instead of just the evtmessage.
    bool m_param_raw;
    /// Module parameter: add event size in the output data or not
    bool m_param_addEventSize;
    /// Module parameters for streamHelper
    bool m_param_addPersistentDurability, m_param_streamTransientObjects;

    /// ZMQ Parent needed for the connections
    std::shared_ptr<ZMQParent> m_parent;
    /// Raw output connection to the ZMQ application
    std::unique_ptr<ZMQRawOutput> m_output;

    /// Original stream helper
    StreamHelper m_streamHelper;
    /// Are we still in the first event? Please note the special handling of the first event in the HLTEventProcessor (therefore we do not stream the first event)
    bool m_firstEvent = false;

    /// Message helper to create zmq::message_t from EventMetaData and EvtMessage
    ZMQMessageHelper m_messageHelper;
  };
}
