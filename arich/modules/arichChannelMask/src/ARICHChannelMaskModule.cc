/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/modules/arichChannelMask/ARICHChannelMaskModule.h>

#include <string>

#include <TTree.h>
#include <TH1F.h>
#include <TRandom.h>


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
  setDescription("Test Module for saving big data in CAF");
  // Parameter definitions
  //  addParam("entriesPerEvent", m_entriesPerEvent,
  //         "Number of entries that we fill into the saved tree per event. As we increase this we start storing larger amonuts of data in a smaller number of events to test the limits.",
  //        int(10));
}

void ARICHChannelMaskModule::prepare()
{

  auto hist = new TH1F("ch_occupancy", "HAPD channel occupancy", 420 * 144, -0.5, 420 * 144 - 1);
  registerObject<TH1F>("ch_occupancy", hist);
  m_ARICHDigits.isRequired();
}


void ARICHChannelMaskModule::collect()
{
  auto hist = getObjectPtr<TH1F>("ch_occupancy");

  for (const auto& digit : m_ARICHDigits) {
    hist->Fill((digit.getModuleID() - 1) * 420 + digit.getChannelID());
  }
}

