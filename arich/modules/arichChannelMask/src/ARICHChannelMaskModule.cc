/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <arich/modules/arichChannelMask/ARICHChannelMaskModule.h>

#include <TH2F.h>


using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ARICHChannelMask)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ARICHChannelMaskModule::ARICHChannelMaskModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Collector for ARICH channel mask production in CAF");
}

void ARICHChannelMaskModule::prepare()
{

  auto hist = new TH2F("ch_occupancy", "HAPD channel occupancy in bits", 420 * 144 + 1, -0.5, 420 * 144 + 0.5, 4, -0.5, 3.5);
  registerObject<TH2F>("ch_occupancy", hist);
  m_ARICHDigits.isRequired();
}


void ARICHChannelMaskModule::collect()
{
  auto hist = getObjectPtr<TH2F>("ch_occupancy");
  hist->Fill(420 * 144, 0); // evnt count

  for (const auto& digit : m_ARICHDigits) {
    uint8_t bits = digit.getBitmap();
    for (int i = 0; i < 4; i++) {
      if ((bits & (1 << i)) && !(bits & ~(1 << i))) {
        hist->Fill((digit.getModuleID() - 1) * 144 + digit.getChannelID(), i);
      }
    }
  }
}
