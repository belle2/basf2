/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>

#include <memory>

namespace Belle2 {

  /// Helper class for data store serialization
  class StreamHelper {
  public:
    /// Initialize this class. Call this e.g. in the first event.
    void initialize(int compressionLevel, bool handleMergeable);
    /// Stream the data store into an event message
    std::unique_ptr<EvtMessage> stream(bool addPersistentDurability = true, bool streamTransientObjects = true);
    /// Read in a ZMQ message and rebuilt the data store from it.
    void read(std::unique_ptr<ZMQNoIdMessage> message);

  private:
    /// The data store streamer to use
    std::unique_ptr<DataStoreStreamer> m_streamer;
    /// The random generator object in the data store that we need to transport also
    StoreObjPtr<RandomGenerator> m_randomGenerator;
  };
}
