/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Seokhee Park, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <daq/hbasf2/utils/StorageStreamHelper.h>
#include <framework/pcore/zmq/messages/ZMQMessageFactory.h>
#include <framework/pcore/MsgHandler.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#include <TH1F.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TKey.h>
#include <TClass.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TBufferJSON.h>

#include <lz4.h>
#include <zmq.hpp>

#include <daq/storage/ONSENBinData.h>

using namespace Belle2;

void StorageStreamHelper::initialize()
{
  //m_streamHelper.initialize(0, true);

  m_buf = new int [10000000];
  m_data.setBuffer(m_buf);
  m_data_hlt.setBuffer(NULL);
  m_data_pxd.setBuffer(NULL);
}

void StorageStreamHelper::registerStoreObjects(bool addExpressRecoObjects)
{
  m_eventMetaData.registerInDataStore();
  m_rawSVDs.registerInDataStore();
  m_rawCDCs.registerInDataStore();
  m_rawTOPs.registerInDataStore();
  m_rawARICHs.registerInDataStore();
  m_rawECLs.registerInDataStore();
  m_rawKLMs.registerInDataStore();
  m_rawTRGs.registerInDataStore();
  m_rawFTSWs.registerInDataStore();

  //m_eventT0.registerInDataStore();
  m_onlineEventT0.registerInDataStore();
  //m_eventT0.isRequired();

  if (addExpressRecoObjects) {
    m_randomGenerator.registerInDataStore(DataStore::c_DontWriteOut);
    m_softwareTriggerResult.registerInDataStore();
    m_softwareTriggerVariables.registerInDataStore();
    m_triggerSummary.registerInDataStore();
    m_rawPXDs.registerInDataStore();
    m_roiPayload.registerInDataStore();
    m_rois.registerInDataStore("ROIs");
  }

  m_streamer = new DataStoreStreamer();
}

void StorageStreamHelper::read(std::unique_ptr<ZMQNoIdMessage> message)
{
  if (message->isMessage(EMessageTypes::c_eventMessage)) {
    B2DEBUG(10, "EMessageTypes::c_eventMessage: begin");
    m_streamHelper.read(std::move(message));
    B2DEBUG(10, "EMessageTypes::c_eventMessage: end");
  } else if (message->isMessage(EMessageTypes::c_rawDataMessage)) {
    B2DEBUG(10, "EMessageTypes::c_rawDataMessage");
    int* eventBuffer = message->getMessagePart<1>().data<int>();

    m_data.setBuffer(eventBuffer);

    int nboard = m_data.getNBoard();
    /*
    if (m_eb2 == 0 && nboard == 1) {
      m_data_hlt.setBuffer(m_data.getBuffer());
    } else if (m_eb2 > 0 || nboard > 1) {
      m_data_hlt.setBuffer(m_data.getBody());
    }
    */

    m_data_hlt.setBuffer(m_data.getBody());

    EvtMessage* msg = new EvtMessage((char*)m_data_hlt.getBody());
    if (msg->type() == MSG_TERMINATE) {
      B2INFO("Got Termination message");
      delete msg;
      return;
    }
    m_streamer->restoreDataStore(msg);
    delete msg;

    if (nboard > 1) {
      unsigned int offset = m_data_hlt.getWordSize() + m_data.getHeaderWordSize();
      for (int i = 0; i < nboard - 1; i++) {
        m_data_pxd.setBuffer(m_data.getBuffer() + offset);
        offset += m_data_pxd.getWordSize();
        if (m_data_pxd.getBody()[0] != ONSENBinData::MAGIC) {
          B2FATAL("Bad ONSEN magic for PXD = " << m_data_pxd.getTrailerMagic());
          return;
        } else if (m_data_pxd.getTrailerMagic() != BinData::TRAILER_MAGIC) {
          B2FATAL("Bad tarailer magic for PXD = " << m_data_pxd.getTrailerMagic());
          return;
        }
        if (m_data_pxd.getBuffer() != NULL) {
          m_rawPXDs.appendNew((int*)m_data_pxd.getBody(), m_data_pxd.getBodyByteSize());
        }
      }
    } else {
      m_data_pxd.setBuffer(NULL);
    }

    // m_eventMetaData.create();
    m_eventMetaData->setExperiment(m_data_hlt.getExpNumber());
    m_eventMetaData->setRun(m_data_hlt.getRunNumber());
    m_eventMetaData->setSubrun(m_data_hlt.getSubNumber());
    m_eventMetaData->setEvent(m_data_hlt.getEventNumber());
  }
}