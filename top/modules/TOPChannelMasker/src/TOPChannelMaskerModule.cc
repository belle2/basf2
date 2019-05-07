/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Jan Strube (jan.strube@pnnl.gov)                                    *
 *    Sam Cunliffe (sam.cunliffe@desy.de)                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPChannelMasker/TOPChannelMaskerModule.h>
#include <top/reconstruction/TOPreco.h>     // reconstruction wrapper
#include <top/reconstruction/TOPconfigure.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
  REG_MODULE(TOPChannelMasker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  TOPChannelMaskerModule::TOPChannelMaskerModule() : Module()
  {
    // Set module properties
    setDescription("Masks dead PMs from the reconstruction");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("printMask", m_printMask,
             "if true print channel mask as set in reconstruction", false);

  }

  void TOPChannelMaskerModule::initialize()
  {
    // register data objects
    m_digits.isRequired();
    m_eventAsicMask.isOptional();

    // Configure TOP detector in FORTRAN code
    TOPconfigure config;
  }

  void TOPChannelMaskerModule::event()
  {
    // if changed then pass pixel relative efficiencies to the FORTRAN reconstructon code
    if (m_pmtInstalled.hasChanged() or m_pmtQEData.hasChanged() or
        m_channelRQE.hasChanged() or m_thresholdEff.hasChanged()) {
      TOPreco::setChannelEffi();
      // reset others to prevent passing the same effi. again in the next couple of events
      m_pmtInstalled.hasChanged();
      m_pmtQEData.hasChanged();
      m_channelRQE.hasChanged();
      m_thresholdEff.hasChanged();
    }

    // are masks available?
    if (!m_channelMask.isValid()) {
      B2ERROR("channel mask not available, not masking any channels");
      return;
    }

    // have asic masks changed?
    bool asicMasksChanged = false;
    if (m_eventAsicMask.isValid()) {
      if (m_eventAsicMask->get() != m_savedAsicMask.get()) {
        m_savedAsicMask.set(m_eventAsicMask->get());
        asicMasksChanged = true;
      }
    }

    // if channel masks have changed then pass the masking to the FORTRAN
    // reconstruction code to exclude from the pdf
    if (m_channelMask.hasChanged() or asicMasksChanged) {
      TOPreco::setChannelMask(m_channelMask, m_savedAsicMask, m_printMask);
    }

    // now flag actual data Cherenkov hits as coming from bad channels
    for (auto& digit : m_digits) {
      if (!m_channelMask->isActive(digit.getModuleID(), digit.getChannel())) {
        digit.setHitQuality(TOPDigit::c_Junk);
      }
    }

  }

} // end Belle2 namespace

