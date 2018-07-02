/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdUnpacking/PXDPostErrorCheckerModule.h>

using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDPostErrorChecker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDPostErrorCheckerModule::PXDPostErrorCheckerModule() : Module()
{
  //Set module properties
  setDescription("PXD: Check Post Unpacking for DAQ errors");
  setPropertyFlags(c_ParallelProcessingCertified);

  constexpr uint64_t defaulterrormask =
//         c_XXXXTB_IDS | // unused
    c_FRAME_TNR_MM |
    c_META_MM |
    c_ONSEN_TRG_FIRST |
    //
    c_DHC_END_MISS |
    c_NR_FRAMES_TO_SMALL |
//         c_ROI_PACKET_INV_SIZE | // does not affect pixel data
    c_DATA_OUTSIDE |
    //
    c_DHC_START_SECOND |
    c_DHE_WRONG_ID_SEQ |
    c_FIX_SIZE |
    c_DHE_CRC |
    //
    c_DHC_UNKNOWN |
//         c_XXXMERGER_CRC | unused
    c_PACKET_SIZE |
    c_MAGIC |
    //
    c_FRAME_NR |
    c_FRAME_SIZE |
    c_HLTROI_MAGIC |
    c_MERGER_TRIGNR |
    //
    c_DHP_SIZE |
    c_DHE_DHP_DHEID |
    c_DHE_DHP_PORT |
    c_DHP_PIX_WO_ROW |
    //
    c_DHE_START_END_ID |
    c_DHE_START_ID |
    c_DHE_START_WO_END |
    c_NO_PXD |
    //
//         c_NO_DATCON |  // does not affect pixel data
    c_FAKE_NO_DATA_TRIG |
    c_DHE_ACTIVE |
//         c_DHP_ACTIVE | // GHOST problem ... bit always set
    //
    c_SENDALL_TYPE |
    c_NOTSENDALL_TYPE |
    c_DHP_DBL_HEADER |
    c_HEADER_ERR |
    //
    c_HEADER_ERR_GHOST |
    c_SUSP_PADDING |
    c_DHC_WIE |
    c_DHE_WIE |
    //
    c_ROW_OVERFLOW |
//         c_DHP_NOT_CONT | // GHOST problem
    c_DHP_DHP_FRAME_DIFFER |
    c_DHP_DHE_FRAME_DIFFER |
    //
    c_DHE_ID_INVALID |
    c_DHC_DHCID_START_END_MM |
    c_DHE_END_WO_START |
    c_DHC_END_DBL |
    //
    c_META_MM_DHC |
    c_META_MM_DHE |
//         c_COL_OVERFLOW | // we might get that for unconnected lines -> "DHPT fix"
    c_UNEXPECTED_FRAME_TYPE |
    //
    c_META_MM_DHC_ERS |
//         c_META_MM_DHC_TT | // time tag is not set correctly in EvtMeta
    c_META_MM_ONS_HLT |
    c_META_MM_ONS_DC |
    //
//         c_EVT_TRG_GATE_DIFFER | // still a bug in DHE FW
//         c_EVT_TRG_FRM_NR_DIFFER | // still a bug in DHE FW
//         c_DHP_ROW_WO_PIX |
    c_DHE_START_THIRD ;

  // other bits not used yet

  addParam("PXDDAQEvtStatsName", m_PXDDAQEvtStatsName, "The name of the StoreObjPtr of input PXDDAQEvtStats", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of input PXDRawHits", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of input PXDRawAdcs", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of input PXDRawROIs", std::string(""));
  addParam("ClusterName", m_RawClusterName, "The name of the StoreArray of input PXDClusters", std::string(""));

  addParam("CriticalErrorMask", m_criticalErrorMask, "Set error mask for which data is removed", defaulterrormask);

  addParam("IgnoreTriggerGate", m_ignoreTriggerGate, "Ignore different triggergate between DHEs", true);
  addParam("IgnoreDHPFrame", m_ignoreDHPFrame, "Ignore different dhp frame between DHEs", true);


}

void PXDPostErrorCheckerModule::initialize()
{
  m_storeDAQEvtStats.isRequired();

  // Needed if we need to Clear them on detected Error
  m_storeRawHits.isOptional(m_PXDRawHitsName);
  m_storeRawAdc.isOptional(m_PXDRawAdcsName);
  m_storeROIs.isOptional(m_PXDRawROIsName);
  m_storeRawCluster.isOptional(m_RawClusterName);
}

void PXDPostErrorCheckerModule::event()
{
  /**
   * The Following Checks can be done only here:
   * For all DHEs in all DHCs in all Packets
   * - Check that TriggerGate is identical
   * - Check that DHE FrameNr is identical
   * - Check that no DHP frameNr differs by more than xxx - TODO
   * - Decide if Error Mask etc allows for flagging as good data TODO
   * - Check that we got data for all modules (database) - TODO
   * Be aware of overflows (mask valid bit ranges) for any counter!
   * -> do not use plain min() max(), this will fail!
   * */
  bool had_dhe = false;
  unsigned short triggergate = 0;
  unsigned short dheframenr = 0;
  PXDErrorFlags mask = EPXDErrMask::c_NO_ERROR;
  B2DEBUG(20, "Iterate PXD Packets for this Event");
  for (auto& pkt : *m_storeDAQEvtStats) {
    B2DEBUG(20, "Iterate DHC in Pkt " << pkt.getPktIndex());
    for (auto& dhc : pkt) {
      B2DEBUG(20, "Iterate DHE in DHC " << dhc.getDHCID());
      for (auto& dhe : dhc) {
        B2DEBUG(20, "Iterate DHP in DHE " << dhe.getDHEID() << " TrigGate " << dhe.getTriggerGate() << " FrameNr " << dhe.getFrameNr());
        if (had_dhe) {
          if (dhe.getTriggerGate() != triggergate) {
            if (!m_ignoreTriggerGate) B2ERROR("Trigger Gate of DHEs not identical" << triggergate << " != " << dhe.getTriggerGate());
            mask |= EPXDErrMask::c_EVT_TRG_GATE_DIFFER;
          }
          if (dhe.getFrameNr() != dheframenr) {
            if (!m_ignoreDHPFrame) B2ERROR("Frame Nr of DHEs not identical" << dheframenr << " != " << dhe.getFrameNr());
            mask |= EPXDErrMask::c_EVT_TRG_FRM_NR_DIFFER;
          }
        } else {
          triggergate = dhe.getTriggerGate();
          dheframenr = dhe.getFrameNr();
          had_dhe = true;
        }
        for (auto& dhp : dhe) {
          B2DEBUG(20, "DHP " << dhp.getChipID() << " Framenr " << dhp.getFrameNr());
          // TODO check against other DHP (full bits) and DHE (limited bits)
          // TODO We know that this will fail with current firmware and most likely will not be fixed...
        }
      }
    }
  }
  m_storeDAQEvtStats->addErrorMask(mask);
  m_storeDAQEvtStats->setCritErrorMask(m_criticalErrorMask);
  m_storeDAQEvtStats->Decide();
  if (!m_storeDAQEvtStats->isUsable()) {
    // Clear all PXD related data but Raw and DaqEvtStats!
    m_storeRawHits.clear();
    m_storeROIs.clear();
    m_storeRawAdc.clear();
    m_storeRawCluster.clear();
    setReturnValue(false); // allows special processing in case
  } else {
    // setReturnValue(true); // default, it is not needed
  }
}
