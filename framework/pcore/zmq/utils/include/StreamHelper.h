/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>

#include <string>
#include <memory>

namespace Belle2 {

  /// Helper class for data store serialization
  class StreamHelper {
  public:
    /// Initialize this class. Call this e.g. in the first event.
    void initialize(int compressionLevel, bool handleMergeable);
    /// Stream the data store into an event message
    std::unique_ptr<EvtMessage> stream() const;
    /// Read in a ZMQ message and rebuilt the data store from it.
    void read(std::unique_ptr<ZMQNoIdMessage> message);

  private:
    /// The data store streamer to use
    std::unique_ptr<DataStoreStreamer> m_streamer;
  };
}