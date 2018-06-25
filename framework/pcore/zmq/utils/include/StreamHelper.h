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

#include <string>
#include <memory>

namespace Belle2 {

  class StreamHelper {
  public:
    void initialize(int compressionLevel, bool handleMergeable);
    std::unique_ptr<ZMQNoIdMessage> stream() const;

  private:
    /// The data store streamer to use
    std::unique_ptr<DataStoreStreamer> m_streamer;
  };
}