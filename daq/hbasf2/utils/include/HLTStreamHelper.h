/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once
#include <framework/pcore/zmq/utils/StreamHelper.h>
#include <framework/pcore/zmq/messages/ZMQNoIdMessage.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawTRG.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <tracking/dataobjects/ROIpayload.h>
#include <tracking/dataobjects/ROIid.h>
#include <framework/dataobjects/EventMetaData.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>
#include <hlt/softwaretrigger/dataobjects/SoftwareTriggerVariables.h>
#include <mdst/dataobjects/TRGSummary.h>

#include <string>
#include <memory>

namespace Belle2 {

  /// Helper class for data store serialization
  class HLTStreamHelper {
  public:
    /// Initialize this class. Call this e.g. in the first event.
    void initialize();

    /// Stream the data store into an event message. Add ROI as additional message (if valid).
    std::unique_ptr<ZMQNoIdMessage> stream(bool addPersistentDurability, bool streamTransientObjects);

    /// Stream the data store into an event message and add SendHeader and SendTrailer around the message. Add ROI as additional message (if valid).
    std::unique_ptr<ZMQNoIdMessage> streamRaw();

    /// Stream all objects derived from TH1 into a message. Only the last subfolder is streamed by prefixing the histogram names with "<subfolder>/".
    std::unique_ptr<ZMQNoIdMessage> streamHistograms(bool compressed = true);

    /// Read in a ZMQ message and rebuilt the data store from it.
    void read(std::unique_ptr<ZMQNoIdMessage> message);

    /// Register all needed store objects, either only the raw data, ROIs and event meta data (for HLT) or additional objects (for express reco).
    void registerStoreObjects(bool addExpressRecoObjects);

  private:
    /// We use the framework stream helper
    StreamHelper m_streamHelper;

    /// Store Objects for HLT use
    StoreObjPtr<EventMetaData> m_eventMetaData;
    /// Store Objects for HLT use
    StoreArray<RawSVD> m_rawSVDs;
    /// Store Objects for HLT use
    StoreArray<RawCDC> m_rawCDCs;
    /// Store Objects for HLT use
    StoreArray<RawTOP> m_rawTOPs;
    /// Store Objects for HLT use
    StoreArray<RawARICH> m_rawARICHs;
    /// Store Objects for HLT use
    StoreArray<RawECL> m_rawECLs;
    /// Store Objects for HLT use
    StoreArray<RawKLM> m_rawKLMs;
    /// Store Objects for HLT use
    StoreArray<RawTRG> m_rawTRGs;
    /// Store Objects for HLT use
    StoreArray<RawFTSW> m_rawFTSWs;
    /// Store Objects for HLT use
    StoreObjPtr<ROIpayload> m_roiPayload;

    /// Additional Store Objects for ExpressReco use
    StoreObjPtr<RandomGenerator> m_randomGenerator;
    /// Additional Store Objects for ExpressReco use
    StoreObjPtr<SoftwareTriggerResult> m_softwareTriggerResult;
    /// Additional Store Objects for ExpressReco use
    StoreObjPtr<SoftwareTrigger::SoftwareTriggerVariables> m_softwareTriggerVariables;
    /// Additional Store Objects for ExpressReco use
    StoreObjPtr<TRGSummary> m_triggerSummary;
    /// Additional Store Objects for ExpressReco use
    StoreArray<RawPXD> m_rawPXDs;
    /// Additional Store Objects for ExpressReco use
    StoreArray<ROIid> m_rois;

    /// Temporary buffer for storing the compressed result
    std::vector<char> m_outputBuffer;
    /// Maximal size of the compression buffer
    unsigned int m_maximalCompressedSize = 100'000'000;

    /// If the ROI payload data storobject is filled, write out the roi message (otherwise an empty message)
    zmq::message_t getROIMessageIfViable() const;
  };
}