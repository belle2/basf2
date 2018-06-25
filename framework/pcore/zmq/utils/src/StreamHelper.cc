/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pcore/zmq/utils/StreamHelper.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/core/Environment.h>

#include <TSystem.h>

using namespace Belle2;

void StreamHelper::initialize(int compressionLevel, bool handleMergeable)
{
  gSystem->Load("libdataobjects");
  m_streamer = std::make_unique<DataStoreStreamer>(compressionLevel, handleMergeable);

  if ((Environment::Instance().getStreamingObjects()).size() > 0) {
    m_streamer->setStreamingObjects(Environment::Instance().getStreamingObjects());
    B2INFO("Tx: Streaming objects limited : " << (Environment::Instance().getStreamingObjects()).size() << " objects");
  }
}

std::unique_ptr<EvtMessage> StreamHelper::stream() const
{
  return std::unique_ptr<EvtMessage>(m_streamer->streamDataStore(true, true));
}

void StreamHelper::read(const std::unique_ptr<ZMQNoIdMessage>& message, const StoreObjPtr<RandomGenerator>& randomGenerator)
{
  message->toDataStore(m_streamer, randomGenerator);
}