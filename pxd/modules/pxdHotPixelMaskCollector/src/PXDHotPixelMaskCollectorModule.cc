/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdHotPixelMaskCollector/PXDHotPixelMaskCollectorModule.h>

#include <TH1I.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDHotPixelMaskCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDHotPixelMaskCollectorModule::PXDHotPixelMaskCollectorModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Calibration Collector Module for PXD hot pixel masking");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("FilterSensorName", m_filterSensorName, "Name of sensor", std::string("1.1.1"));
}

void PXDHotPixelMaskCollectorModule::prepare() // Do your initialise() stuff here
{
  m_pxdDigits.isRequired();

  // Data object creation --------------------------------------------------
  auto hhitmap = new TH1I("hhitmap",
                          "Pixel hits from PXDDigits histogram, distribution parameters found by PXDHotPixelMaskCollectorModule", 250 * 768, 0, 250 * 768);

  auto hnevents = new TH1I("hnevents",
                           "Number of events used for masking, distribution parameters found by PXDHotPixelMaskCollectorModule", 1, 0, 1);

  // Data object registration ----------------------------------------------
  registerObject<TH1I>("hitmap", hhitmap); // Does the registerInDatastore for you
  registerObject<TH1I>("nevents", hnevents);

}

void PXDHotPixelMaskCollectorModule::collect() // Do your event() stuff here
{
  // Data object access and filling ----------------------------------------
  TH1I* collector_nevents = getObjectPtr<TH1I>("nevents");
  collector_nevents->Fill(0);

  TH1I* collector_hitmap = getObjectPtr<TH1I>("hitmap");

  for (auto& digit :  m_pxdDigits) {
    unsigned int pixelID = digit.getUCellID() * 768 + digit.getVCellID();
    collector_hitmap->Fill(pixelID, 1);
  }
}
