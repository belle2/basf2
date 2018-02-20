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

  // Data object registration ----------------------------------------------
  registerObject<TH1I>("hitmap_cal", hhitmap); // Does the registerInDatastore for you

}

void PXDHotPixelMaskCollectorModule::collect() // Do your event() stuff here
{

  //VxdID filterID(m_filterSensorName);

  // Data object access and filling ----------------------------------------
  TH1I* collector_data = getObjectPtr<TH1I>("hitmap_cal");
  for (auto& digit :  m_pxdDigits) {
    unsigned int pixelID = digit.getUCellID() * 768 + digit.getVCellID();
    collector_data->Fill(pixelID, 1);
  }
}
