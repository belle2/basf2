/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/gdl/modules/TRGGDLInjectionVetoFromOverlay/TRGGDLInjectionVetoFromOverlayModule.h>

using namespace Belle2;

REG_MODULE(TRGGDLInjectionVetoFromOverlay);

TRGGDLInjectionVetoFromOverlayModule::TRGGDLInjectionVetoFromOverlayModule() : Module()
{
  setDescription("Module for adding to MC samples the information about the TRG active veto from "
                 "beam background overlay files.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("extensionName", m_extensionName,
           "Name added to default branch names", std::string("_beamBG"));
}

void TRGGDLInjectionVetoFromOverlayModule::initialize()
{
  m_TRGSummaryFromSimulation.isRequired();
  m_TRGSummaryFromOverlay.isOptional(std::string("TRGSummary") + m_extensionName);
}

void TRGGDLInjectionVetoFromOverlayModule::event()
{
  if (!m_TRGSummaryFromSimulation.isValid() or !m_TRGSummaryFromOverlay.isValid())
    return;
  // Check if the corresponing BGO event falls into the TRG veto:
  // if yes, let's set the current MC event as falling into the TRG veto as well
  try {
    if (m_TRGSummaryFromOverlay->testInput("passive_veto")) {
      const unsigned int passive_veto = m_TRGSummaryFromSimulation->getInputBitNumber("passive_veto");
      m_TRGSummaryFromSimulation->setInputBits(passive_veto, 1);
      // Set also the cdcecl_veto input line according to what written in the BGO event
      const unsigned int cdcecl_veto = m_TRGSummaryFromSimulation->getInputBitNumber("cdcecl_veto");
      m_TRGSummaryFromSimulation->setInputBits(cdcecl_veto, m_TRGSummaryFromOverlay->testInput("cdcecl_veto"));
    }
  } catch (const std::exception&) {
    // TRGSummary methods can throw out_of_range, runtime_error and invalid_argument exceptions
    // Also: the methods already print a warning before throwing the exception, so it's not necessary
    // to report anything here: let's simply return
    return;
  }
}
