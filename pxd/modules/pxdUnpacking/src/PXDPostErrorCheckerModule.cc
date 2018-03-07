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
  setDescription("Check Post Unpacking DAQ errors");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void PXDPostErrorCheckerModule::initialize()
{
  m_storeDAQEvtStats.isRequired();
}

void PXDPostErrorCheckerModule::event()
{
  /**
   * The Following Checks can be done only here:
   * For all DHEs in all DHCs in all Packets
   * - Check that TriggerGate is identical
   * - Check that DHE FrameNr is identical
   * - Check that no DHP frameNr differs by more than xxx - TODO
   * - Check that we got data for all modules (database) - TODO
   * Be aware of overflows (mask valid bit ranges) for any counter!
   * -> do not use plain min() max(), this will fail!
   * */
  bool had_dhe = false;
  unsigned short triggergate = 0;
  unsigned short dheframenr = 0;
  B2DEBUG(20, "Iterate PXD DAQ Status");
  auto evt = *m_storeDAQEvtStats;
  PXDErrorFlags mask = EPXDErrMask::c_NO_ERROR;
  B2DEBUG(20, "Iterate PXD Packets");
  for (auto& pkt : evt) {
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
  evt.addErrorMask(mask);
}
