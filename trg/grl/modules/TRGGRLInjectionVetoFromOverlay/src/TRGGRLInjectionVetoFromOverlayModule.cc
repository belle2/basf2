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
  addParam("TRGGRLInfoName", m_TRGGRLInfoName, "Name of the TRGGRLInfo object", std::string("TRGGRLObjects"));
  addParam("extensionName", m_extensionName,
           "Name added to default branch names", std::string("_beamBG"));
}

void TRGGRLInjectionVetoFromOverlayModule::initialize()
{
  m_TRGGRLInfoFromSimulation.isRequired(m_TRGGRLInfoName);
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
    for (const std::string& inputBitName : m_inputBitNames) {
      // Set the relevant input lines according to what is written in the BGO event
      const unsigned int inputBitNumber = m_TRGInputBits->getinbitnum(inputBitName.c_str());
      const bool inputBitAnswer = m_TRGSummaryFromOverlay->testInput(inputBitName.c_str());
      m_TRGGRLInfoFromSimulation->setInputBits(inputBitNumber, inputBitAnswer);
    }
  } catch (const std::exception&) {
    // TRGSummary methods can throw out_of_range, runtime_error and invalid_argument exceptions
    // Also: the methods already print a warning or an error before throwing the exception,
    // so it's not necessary to report anything here: let's simply return
    return;
  }
}
