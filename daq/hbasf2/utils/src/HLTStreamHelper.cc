/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/hbasf2/utils/HLTStreamHelper.h>
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

using namespace Belle2;

namespace {
  void streamHistogramImpl(TDirectory* curdir, Belle2::MsgHandler& msg, const std::string& dirName = "")
  {
    TList* keylist = curdir->GetList();

    TIter nextkey(keylist);
    TKey* key = nullptr;

    while ((key = (TKey*)nextkey())) {
      TObject* obj = curdir->Get(key->GetName());
      TClass* objectClass = obj->IsA();
      std::string objectName = dirName;
      if (not objectName.empty()) {
        objectName += "/";
      }
      objectName += obj->GetName();

      if (objectClass->InheritsFrom(TH1::Class())) {
        auto* h1 = dynamic_cast<TH1*>(obj);
        msg.add(h1, objectName);
      } else if (objectClass->InheritsFrom(TDirectory::Class())) {
        auto* tdir = dynamic_cast<TDirectory*>(obj);
        // FIXME: Currently the dqm server does not understand multi-layer directory structures
        // therefore I break this down to only show the last directory
        streamHistogramImpl(tdir, msg, obj->GetName());
      }
    }
  }
}

void HLTStreamHelper::initialize()
{
  m_streamHelper.initialize(0, true);
}

void HLTStreamHelper::registerStoreObjects(bool addExpressRecoObjects)
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

  if (addExpressRecoObjects) {
    m_randomGenerator.registerInDataStore(DataStore::c_DontWriteOut);
    m_softwareTriggerResult.registerInDataStore();
    m_softwareTriggerVariables.registerInDataStore();
    m_triggerSummary.registerInDataStore();
    m_rawPXDs.registerInDataStore();
    m_roiPayload.registerInDataStore();
    m_rois.registerInDataStore("ROIs");
  }
}

std::unique_ptr<ZMQNoIdMessage> HLTStreamHelper::streamRaw()
{
  const auto eventMessage = m_streamHelper.stream(false, false);

  // Fill Header and Trailer
  SendHeader hdr;
  SendTrailer trl;

  // Number of total words
  int msgsize = (eventMessage->size() - 1) / sizeof(int) + 1;
  int total_nwrds = msgsize +  hdr.GetHdrNwords() + trl.GetTrlNwords();

  // Fill header and trailer
  hdr.SetNwords(total_nwrds);
  hdr.SetNumEventsinPacket(1);
  hdr.SetNumNodesinPacket(1);
  hdr.SetEventNumber(m_eventMetaData->getEvent());
  //  hdr.SetExpRunWord(evtmeta->getRun());
  hdr.SetSubRunNum(m_eventMetaData->getSubrun());     // modified on Apr. 20, 2016 by SY
  hdr.SetRunNum(m_eventMetaData->getRun());
  hdr.SetExpNum(m_eventMetaData->getExperiment());
  hdr.SetNodeID(300);

  zmq::message_t rawMessage(total_nwrds * sizeof(int));
  int* buffer = rawMessage.data<int>();

  // Fill header
  memcpy(buffer, hdr.GetBuffer(), hdr.GetHdrNwords()*sizeof(int));

  // Fill EvtMessage
  memcpy(buffer + hdr.GetHdrNwords(), eventMessage->buffer(), eventMessage->size());

  // Fill trailer
  memcpy(buffer + hdr.GetHdrNwords() + msgsize, trl.GetBuffer(),
         trl.GetTrlNwords()*sizeof(int));

  zmq::message_t roiMessage = getROIMessageIfViable();
  auto zmqMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_rawDataMessage, std::move(rawMessage), std::move(roiMessage));

  return zmqMessage;
}

std::unique_ptr<ZMQNoIdMessage> HLTStreamHelper::stream(bool addPersistentDurability, bool streamTransientObjects)
{
  const auto eventMessage = m_streamHelper.stream(addPersistentDurability, streamTransientObjects);

  zmq::message_t roiMessage = getROIMessageIfViable();
  auto zmqMessage = ZMQMessageFactory::createMessage(EMessageTypes::c_eventMessage, eventMessage, std::move(roiMessage));
  return zmqMessage;
}

zmq::message_t HLTStreamHelper::getROIMessageIfViable() const
{
  zmq::message_t roiMessage;

  if (m_roiPayload.isValid()) {
    const size_t length = m_roiPayload->getPacketLengthByte();
    const char* data = reinterpret_cast<const char*>(m_roiPayload->getRootdata());

    roiMessage = zmq::message_t(length);
    char* tbuffer = roiMessage.data<char>();
    memcpy(tbuffer, data, length);
  }

  return roiMessage;
}

std::unique_ptr<ZMQNoIdMessage> HLTStreamHelper::streamHistograms(bool compressed)
{
  B2ASSERT("Event Meta Data not set!", m_eventMetaData.isValid());

  Belle2::MsgHandler msgHandler;
  streamHistogramImpl(gDirectory, msgHandler);

  auto evtMessage = std::unique_ptr<EvtMessage>(msgHandler.encode_msg(Belle2::ERecordType::MSG_EVENT));

  EventMetaData& eventMetaData = *m_eventMetaData;
  auto eventInformationString = TBufferJSON::ToJSON(&eventMetaData);
  zmq::message_t additionalEventMessage(eventInformationString.Data(), eventInformationString.Length());

  if (not compressed) {
    return Belle2::ZMQMessageFactory::createMessage(Belle2::EMessageTypes::c_rawDataMessage, evtMessage,
                                                    std::move(additionalEventMessage));
  }

  // TODO: do I want to use compression everywhere? That is probably not worth it! -> test this!
  if (m_outputBuffer.empty()) {
    m_outputBuffer.resize(m_maximalCompressedSize, 0);
  }

  B2DEBUG(10, "Size before compression " << evtMessage->size());
  int size = m_maximalCompressedSize;
  size = LZ4_compress_default(evtMessage->buffer(), &m_outputBuffer[0], evtMessage->size(), size);
  B2ASSERT("Compression failed", size > 0);
  B2DEBUG(10, "Size after compression " << size);

  zmq::message_t message(&m_outputBuffer[0], size);

  return Belle2::ZMQMessageFactory::createMessage(Belle2::EMessageTypes::c_compressedDataMessage,
                                                  std::move(message), std::move(additionalEventMessage));
}

void HLTStreamHelper::read(std::unique_ptr<ZMQNoIdMessage> message)
{
  if (message->isMessage(EMessageTypes::c_eventMessage)) {
    m_streamHelper.read(std::move(message));
  } else if (message->isMessage(EMessageTypes::c_rawDataMessage)) {
    int* eventBuffer = message->getMessagePart<1>().data<int>();

    SendHeader sndhdr;
    sndhdr.SetBuffer(eventBuffer);
    int npackedevts = sndhdr.GetNumEventsinPacket();
    if (npackedevts != 1) {
      B2WARNING("Strange SendHeader : ");
      //    for (int i = 0; i < sndhdr.SENDHDR_NWORDS; i++) {
      for (int i = 0; i < 10; i++) {
        B2WARNING(std::hex << * (sndhdr.GetBuffer() + i));
      }

      B2WARNING("Raw2DsModule::number of events in packet is not 1. This process gets stuck here. Please ABORT the system. (Please see discussion of daqcore channel in https://b2rc.kek.jp/ on 2017. Nov. 30. about why this is not FATAL message.");
      sleep(86400);
    }
    int ncprs = sndhdr.GetNumNodesinPacket();
    int nwords = sndhdr.GetTotalNwords() - SendHeader::SENDHDR_NWORDS - SendTrailer::SENDTRL_NWORDS;

    // Get buffer header
    int* bufbody = eventBuffer + SendHeader::SENDHDR_NWORDS;

    // Unpack buffer
    RawDataBlock tempdblk;
    tempdblk.SetBuffer(bufbody, nwords, false, npackedevts, ncprs);

    unsigned int utime = 0;
    unsigned int ctime = 0;
    unsigned long long int mtime = 0;

    int store_time_flag = 0;
    unsigned int error_flag = 0;

    // Store data contents in Corresponding RawXXXX
    for (int cprid = 0; cprid < ncprs * npackedevts; cprid++) {
      // Pick up one COPPER and copy data in a temporary buffer
      int nwds_buf = tempdblk.GetBlockNwords(cprid);
      int* cprbuf = new int[nwds_buf];
      memcpy(cprbuf, tempdblk.GetBuffer(cprid), nwds_buf * 4);

      // Check FTSW
      if (tempdblk.CheckFTSWID(cprid)) {
        RawFTSW* ftsw = m_rawFTSWs.appendNew();
        ftsw->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);

        // Tentative for DESY TB 2017
        utime = (unsigned int)(ftsw->GetTTUtime(0));
        ctime = (unsigned int)(ftsw->GetTTCtime(0));
        mtime = 1000000000 * (unsigned long long int)utime + (unsigned long long int)(std::round(ctime / 0.127216));
        store_time_flag = 1;
        continue;
      } else if (store_time_flag == 0) {
        // Tentative until RawFTSW data stream is established. 2018.5.28
        // Not store RawCOPPER here. 2018.11.23
        RawCOPPER tempcpr_time;
        tempcpr_time.SetBuffer(cprbuf, nwds_buf, false, 1, 1);
        utime = (unsigned int)(tempcpr_time.GetTTUtime(0));
        ctime = (unsigned int)(tempcpr_time.GetTTCtime(0));
        mtime = 1000000000 * (unsigned long long int)utime + (unsigned long long int)(std::round(ctime / 0.127216));
        store_time_flag = 1;
      }

      RawCOPPER tempcpr;
      tempcpr.SetBuffer(cprbuf, nwds_buf, false, 1, 1);
      int subsysid = tempcpr.GetNodeID(0);
      error_flag |= (unsigned int)(tempcpr.GetDataType(0));

      // Switch to each detector and register RawXXX
      if ((subsysid & DETECTOR_MASK) == CDC_ID) {
        (m_rawCDCs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == SVD_ID) {
        (m_rawSVDs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == BECL_ID) {
        (m_rawECLs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == EECL_ID) {
        (m_rawECLs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == TOP_ID) {
        (m_rawTOPs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == ARICH_ID) {
        (m_rawARICHs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == BKLM_ID) {
        (m_rawKLMs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if ((subsysid & DETECTOR_MASK) == EKLM_ID) {
        (m_rawKLMs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else if (((subsysid & DETECTOR_MASK) & 0xF0000000) == TRGDATA_ID) {
        (m_rawTRGs.appendNew())->SetBuffer(cprbuf, nwds_buf, 1, 1, 1);
      } else {
        // Do not store Unknown RawCOPPER object. 2018.11.25
        B2WARNING("Unknown COPPER ID : ");
        for (int i = 0; i < 12; i++) {
          B2WARNING(std::hex << cprbuf[i]);
        }
        B2FATAL("Unknown COPPER ID is found. CPRID = " << std::hex << subsysid << " Please check. Exiting...");
      }
    }

    if (store_time_flag != 1) {
      B2FATAL("No time information could be extracted from Data. That should not happen. Exiting...");
    }

    m_eventMetaData.create();
    m_eventMetaData->setExperiment(sndhdr.GetExpNum());
    m_eventMetaData->setRun(sndhdr.GetRunNum());
    m_eventMetaData->setSubrun(sndhdr.GetSubRunNum());
    m_eventMetaData->setEvent(sndhdr.GetEventNumber());
    m_eventMetaData->setTime(mtime);

    if (error_flag) {
      if (error_flag & RawHeader_latest::B2LINK_PACKET_CRC_ERROR) {
        m_eventMetaData->addErrorFlag(EventMetaData::c_B2LinkPacketCRCError);
        B2WARNING("Raw2Ds: c_B2LinkPacketCRCError flag was set in EventMetaData.");
      }
      if (error_flag & RawHeader_latest::B2LINK_EVENT_CRC_ERROR) {
        m_eventMetaData->addErrorFlag(EventMetaData::c_B2LinkEventCRCError);
        B2WARNING("Raw2Ds: c_B2LinkEventCRCError flag was set in EventMetaData.");
      }
    }
  }
}
