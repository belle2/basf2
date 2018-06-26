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

  class StreamHelper {
  public:
    void initialize(int compressionLevel, bool handleMergeable);
    std::unique_ptr<EvtMessage> stream() const;
    void read(std::unique_ptr<ZMQNoIdMessage> message, const StoreObjPtr<RandomGenerator>& randomGenerator);

  private:
    /// The data store streamer to use
    std::unique_ptr<DataStoreStreamer> m_streamer;
  };
}