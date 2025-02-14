/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <trg/grl/modules/TRGGRLInjectionVetoFromOverlay/TRGGRLInjectionVetoFromOverlayModule.h>

using namespace Belle2;

REG_MODULE(TRGGRLInjectionVetoFromOverlay);

TRGGRLInjectionVetoFromOverlayModule::TRGGRLInjectionVetoFromOverlayModule() : Module()
{
  setDescription("Module for adding to MC samples the information about the TRG injection veto from "
                 "beam background overlay files.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("extensionName", m_extensionName,
           "Name added to default branch names", std::string("_beamBG"));
}

void TRGGRLInjectionVetoFromOverlayModule::initialize()
{
  m_TRGGRLInfoFromSimulation.isRequired("TRGGRLObjects");
  m_TRGSummaryFromOverlay.isOptional(std::string("TRGSummary") + m_extensionName);
}

void TRGGRLInjectionVetoFromOverlayModule::beginRun()
{
  if (not m_TRGInputBits.isValid())
    B2FATAL("TRGGDLInputBits database object is not available");
}

void TRGGRLInjectionVetoFromOverlayModule::event()
{
  if (!m_TRGGRLInfoFromSimulation.isValid() or !m_TRGSummaryFromOverlay.isValid())
    return;
  try {
    // Set the passive_veto and cdcecl_veto input lines according to what is written in the BGO event
    const unsigned int passive_vetoBit = m_TRGInputBits->getinbitnum("passive_veto");
    const bool passive_vetoAnswer = m_TRGSummaryFromOverlay->testInput("passive_veto");
    m_TRGGRLInfoFromSimulation->setInputBits(passive_vetoBit, passive_vetoAnswer);
    const unsigned int cdcecl_vetoBit = m_TRGInputBits->getinbitnum("cdcecl_veto");
    const bool cdcecl_vetoAnswer = m_TRGSummaryFromOverlay->testInput("cdcecl_veto");
    m_TRGGRLInfoFromSimulation->setInputBits(cdcecl_vetoBit, cdcecl_vetoAnswer);
  } catch (const std::exception&) {
    // TRGSummary methods can throw out_of_range, runtime_error and invalid_argument exceptions
    // Also: the methods already print a warning or an error before throwing the exception,
    // so it's not necessary to report anything here: let's simply return
    return;
  }
}
