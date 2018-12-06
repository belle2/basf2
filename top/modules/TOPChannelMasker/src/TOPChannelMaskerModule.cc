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
  }

  void TOPChannelMaskerModule::initialize()
  {
    // register data objects
    m_digits.isRequired();

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

    // if channel masks have changed then pass the masking to the FORTRAN
    // reconstruction code to exclude from the pdf
    if (m_channelMask.hasChanged()) {
      TOPreco::setChannelMask(m_channelMask);
    }

    // now flag actual data Cherenkov hits as coming from bad channels
    for (auto& digit : m_digits) {
      if (!m_channelMask->isActive(digit.getModuleID(), digit.getChannel())) {
        digit.setHitQuality(TOPDigit::c_Junk);
      }
    }

  }

} // end Belle2 namespace

