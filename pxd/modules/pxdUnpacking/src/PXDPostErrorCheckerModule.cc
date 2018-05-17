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

  addParam("PXDDAQEvtStatsName", m_PXDDAQEvtStatsName, "The name of the StoreObjPtr of input PXDDAQEvtStats", std::string(""));
  addParam("PXDRawHitsName", m_PXDRawHitsName, "The name of the StoreArray of input PXDRawHits", std::string(""));
  addParam("PXDRawAdcsName", m_PXDRawAdcsName, "The name of the StoreArray of input PXDRawAdcs", std::string(""));
  addParam("PXDRawROIsName", m_PXDRawROIsName, "The name of the StoreArray of input PXDRawROIs", std::string(""));
  addParam("ClusterName", m_RawClusterName, "The name of the StoreArray of input PXDClusters", std::string(""));

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
            B2ERROR("Trigger Gate of DHEs not identical" << triggergate << " != " << dhe.getTriggerGate());
            mask |= EPXDErrMask::c_EVT_TRG_GATE_DIFFER;
          }
          if (dhe.getFrameNr() != dheframenr) {
            B2ERROR("Frame Nr of DHEs not identical" << dheframenr << " != " << dhe.getFrameNr());
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
        }
      }
    }
  }
  m_storeDAQEvtStats->addErrorMask(mask);
  m_storeDAQEvtStats->Decide();
  if (!m_storeDAQEvtStats->isUsable()) {
    // Clear all PXD related data but Raw and DaqEvtStats!
    m_storeRawHits.clear();
    m_storeROIs.clear();
    m_storeRawAdc.clear();
    m_storeRawCluster.clear();
  }
}
