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

  PXDError::PXDErrorFlags defaulterrormask;
  defaulterrormask[c_nrEVENT_STRUCT ] = 1;
  defaulterrormask[c_nrFRAME_TNR_MM ] = 1;
  defaulterrormask[c_nrMETA_MM ] = 1;
  defaulterrormask[c_nrONSEN_TRG_FIRST ] = 1;
  //
  defaulterrormask[c_nrDHC_END_MISS ] = 1;
  defaulterrormask[c_nrNR_FRAMES_TO_SMALL ] = 1;
  // defaulterrormask[c_nrROI_PACKET_INV_SIZE ] = 1; // does not affect pixel data
  defaulterrormask[c_nrDATA_OUTSIDE ] = 1;
  //
  defaulterrormask[c_nrDHC_START_SECOND ] = 1;
  // defaulterrormask[c_nrDHE_WRONG_ID_SEQ ] = 1; // until this is fixed in FW, we have to live with this
  defaulterrormask[c_nrFIX_SIZE ] = 1;
  defaulterrormask[c_nrDHE_CRC ] = 1;
  //
  defaulterrormask[c_nrDHC_UNKNOWN ] = 1;
  defaulterrormask[c_nrHEADERTYPE_INV ] = 1;
  defaulterrormask[c_nrPACKET_SIZE ] = 1;
  defaulterrormask[c_nrMAGIC ] = 1;
  //
  defaulterrormask[c_nrFRAME_NR ] = 1;
  defaulterrormask[c_nrFRAME_SIZE ] = 1;
  defaulterrormask[c_nrHLTROI_MAGIC ] = 1;
  defaulterrormask[c_nrMERGER_TRIGNR ] = 1;
  //
  defaulterrormask[c_nrDHP_SIZE ] = 1;
  defaulterrormask[c_nrDHE_DHP_DHEID ] = 1;
  defaulterrormask[c_nrDHE_DHP_PORT ] = 1;
  // defaulterrormask[c_nrDHP_PIX_WO_ROW ] = 1; // FIXME this should not be needed
  //
  defaulterrormask[c_nrDHE_START_END_ID ] = 1;
  defaulterrormask[c_nrDHE_START_ID ] = 1;
  defaulterrormask[c_nrDHE_START_WO_END ] = 1;
  // defaulterrormask[c_nrNO_PXD ] = 1; // THEN we anyway have no data
  //
  // defaulterrormask[c_nrNO_DATCON ] = 1;  // does not affect pixel data
  // defaulterrormask[c_nrFAKE_NO_DATA_TRIG ] = 1; // this will trigger always!!!!
  defaulterrormask[c_nrDHE_ACTIVE ] = 1;
  // defaulterrormask[c_nrDHP_ACTIVE ] = 1; // GHOST problem ... bit always set
  //
  defaulterrormask[c_nrSENDALL_TYPE ] = 1;
  defaulterrormask[c_nrNOTSENDALL_TYPE ] = 1;
  defaulterrormask[c_nrDHP_DBL_HEADER ] = 1;
  defaulterrormask[c_nrHEADER_ERR ] = 1;
  //
  defaulterrormask[c_nrHEADER_ERR_GHOST ] = 1;
  defaulterrormask[c_nrSUSP_PADDING ] = 1;
  defaulterrormask[c_nrDHC_WIE ] = 1;
  defaulterrormask[c_nrDHE_WIE ] = 1;
  //
  defaulterrormask[c_nrROW_OVERFLOW ] = 1;
  // defaulterrormask[c_nrDHP_NOT_CONT ] = 1; // GHOST problem
  defaulterrormask[c_nrDHP_DHP_FRAME_DIFFER ] = 1;
  defaulterrormask[c_nrDHP_DHE_FRAME_DIFFER ] = 1;
  //
  defaulterrormask[c_nrDHE_ID_INVALID ] = 1;
  defaulterrormask[c_nrDHC_DHCID_START_END_MM ] = 1;
  defaulterrormask[c_nrDHE_END_WO_START ] = 1;
  defaulterrormask[c_nrDHC_END_DBL ] = 1;
  //
  defaulterrormask[c_nrMETA_MM_DHC ] = 1;
  defaulterrormask[c_nrMETA_MM_DHE ] = 1;
  // defaulterrormask[c_nrCOL_OVERFLOW ] = 1; // we might get that for unconnected lines -> "DHPT fix"
  defaulterrormask[c_nrUNEXPECTED_FRAME_TYPE ] = 1;
  //
  defaulterrormask[c_nrMETA_MM_DHC_ERS ] = 1;
  // defaulterrormask[c_nrMETA_MM_DHC_TT ] = 1; // time tag is not set correctly in EvtMeta
  defaulterrormask[c_nrMETA_MM_ONS_HLT ] = 1;
  // defaulterrormask[c_nrMETA_MM_ONS_DC ] = 1; // problem with NO-DATCON
  //
  // defaulterrormask[c_nrEVT_TRG_GATE_DIFFER ] = 1; // still a bug in DHE FW
  // defaulterrormask[c_nrEVT_TRG_FRM_NR_DIFFER ] = 1; // still a bug in DHE FW
  // defaulterrormask[c_nrDHP_ROW_WO_PIX ] = 1; // still a bug in DHE FW?
  defaulterrormask[c_nrDHE_START_THIRD ] = 1;
  //
  defaulterrormask[c_nrFAKE_NO_FAKE_DATA ] = 1;
  // other bits not used yet

  addParam("PXDDAQEvtStatsName", m_PXDDAQEvtStatsName, "The name of the StoreObjPtr of input PXDDAQEvtStats", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of input PXDRawHits", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of input PXDRawAdcs", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of input PXDRawROIs", std::string(""));

  int crit = 0;
  addParam("CriticalErrorMask", crit, "Set error mask for which data is removed (0=none, 1=default, other undefined yet)", 1);
  switch (crit) {
    case 0:
      m_criticalErrorMask = 0;
      B2INFO("Disabling critical error mask");
      break;
    case 1:
      m_criticalErrorMask = defaulterrormask;
      break;
    default:
      B2FATAL("Undefined value for criticalErrorMaskSet");
      break;
  }
  // B2DEBUG(25, "The default error mask is $" << std::hex << defaulterrormask);

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

  B2DEBUG(25, "The set error mask is $" << std::hex << m_criticalErrorMask);
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
  std::map <int, int> found_dhe;
  PXDErrorFlags mask = EPXDErrMask::c_NO_ERROR;
  B2DEBUG(25, "Iterate PXD Packets for this Event");
  for (auto& pkt : *m_storeDAQEvtStats) {
    B2DEBUG(25, "Iterate DHC in Pkt " << pkt.getPktIndex());
    for (auto& dhc : pkt) {
      B2DEBUG(25, "Iterate DHE in DHC " << dhc.getDHCID());
      for (auto& dhe : dhc) {
        B2DEBUG(25, "Iterate DHP in DHE " << dhe.getDHEID() << " TrigGate " << dhe.getTriggerGate() << " FrameNr " << dhe.getFrameNr());
        found_dhe[dhe.getDHEID()]++;
        if (had_dhe) {
          if (dhe.getTriggerGate() != triggergate) {
            if (!m_ignoreTriggerGate) B2WARNING("Trigger Gate of DHEs not identical" << LogVar("Triggergate 1",
                                                  triggergate) << LogVar("TriggerGate 2", dhe.getTriggerGate()));
            mask [c_nrEVT_TRG_GATE_DIFFER] = true;
          }
          if (dhe.getFrameNr() != dheframenr) {
            if (!m_ignoreDHPFrame) B2WARNING("Frame Nr of DHEs not identical" << LogVar("FrameNr 1", dheframenr) << LogVar("FrameNr 2",
                                               dhe.getFrameNr()));
            mask [c_nrEVT_TRG_FRM_NR_DIFFER] = true;
          }
        } else {
          triggergate = dhe.getTriggerGate();
          dheframenr = dhe.getFrameNr();
          had_dhe = true;
        }
        for (auto& dhp : dhe) {
          B2DEBUG(25, "DHP " << dhp.getChipID() << " Framenr " << dhp.getFrameNr());
          // TODO check against other DHP (full bits) and DHE (limited bits)
          // TODO We know that this will fail with current firmware and most likely will not be fixed...
        }
        bool error = false;
        for (auto it = dhe.cm_begin(); it < dhe.cm_end(); ++it)  {
          if (std::get<2>(*it) == 63) {
            // TODO Check that we dont have CM=63 indicating fifo overflow, check and set bits
            // mask |= c_DHH_MISC_ERROR; // unpacker should set this already, anyway we would want it set only on the DHP/DHE level...
            B2DEBUG(25, "DHP data loss (CM=63) in " << LogVar("DHE", dhe.getDHEID()) << LogVar("DHP", int(std::get<0>(*it))) << LogVar("Row",
                    std::get<1>(*it)));
            error = true;
          }
        }
        if (error) B2WARNING("DHP data loss (CM=63) in " << LogVar("DHE", dhe.getDHEID()));
      }
    }
  }

  /// the following does only work with the overlapping trigger firmware.
  /// TODO make this check depending on firmware version database object (not yet existing)
  for (auto& a : found_dhe) {
    if (a.second > 1) B2WARNING("More than one packet for same DHE ID " << a.first);
  }
  /// the following checks would require the (not yet existing) database object which marks the available modules
//  for (auto& a : found_dhe) {
//    if (!m_dhe_expected[a.first]) B2ERROR("This DHE ID was not expected " << a.first);
//  }
//  for (auto& a : m_dhe_expected) {
//    if (a.second) {
//      if (found_dhe[a.first] == 0) B2ERROR("DHE packet missing for DHE ID " << a.first);
//    } else {
//      if (found_dhe[a.first] > 0) B2ERROR("This DHE ID was not expected " << a.first);
//    }
//  }
  m_storeDAQEvtStats->addErrorMask(mask);
  m_storeDAQEvtStats->setCritErrorMask(m_criticalErrorMask);
  m_storeDAQEvtStats->Decide();
  if (!m_storeDAQEvtStats->isUsable()) {
    // Clear all PXD related data but Raw and DaqEvtStats!
    m_storeRawHits.clear();
    m_storeROIs.clear();
    m_storeRawAdc.clear();
    setReturnValue(false); // allows special processing in case
  } else {
    // setReturnValue(true); // default, it is not needed
  }
}
