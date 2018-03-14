/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *    Jan Strube (jan.strube@pnnl.gov)                                    *
 *    Sam Cunliffe (samuel.cunliffe@pnnl.gov)                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/modules/TOPChannelMasker/TOPChannelMaskerModule.h>

#include <top/dataobjects/TOPDigit.h>       // data Cherenkov hits
#include <framework/datastore/StoreArray.h> // data store framework
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
    StoreArray<TOPDigit> digits;
    digits.isRequired();
    return;
  }

  void TOPChannelMaskerModule::event()
  {
    // are masks are available?
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
    StoreArray<TOPDigit> digits;
    for (auto& digit : digits) {
      if (!m_channelMask->isActive(digit.getModuleID(), digit.getChannel())) {
        digit.setHitQuality(TOPDigit::c_Junk);
      }
    }
    return;
  }

} // end Belle2 namespace

