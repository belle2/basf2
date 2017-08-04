/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 * WARNING: Do not try to fry it with water. Use only olive oil.          *
 **************************************************************************/

// Own include
#include <svd/calibration/SVDDatabaseImporter.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DB objects
#include <svd/dbobjects/SVDPulseShapeCalibrations.h>
#include <svd/dbobjects/SVDNoiseCalibrations.h>

#include <iostream>
/*#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
*/

using namespace std;
using namespace Belle2;

void SVDDatabaseImporter::importSVDPulseShapeCalibrations(/*std::string fileName*/)
{
  /* std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");
  */

  DBImportObjPtr<SVDPulseShapeCalibrations> svdpulsecal;

  //stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  svdpulsecal.construct();
  svdpulsecal.import(iov);

  B2RESULT("SVDPulseShapeCalibrations imported to database.");

}


void SVDDatabaseImporter::importSVDNoiseCalibrations(/*std::string fileName*/)
{
  /* std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");
  */

  DBImportObjPtr<SVDNoiseCalibrations> svdnoisecal;

  //stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  svdnoisecal.construct();
  svdnoisecal.import(iov);

  B2RESULT("SVDNoiseCalibrations imported to database.");

}


void SVDDatabaseImporter::printSVDPulseShapeCalibrations()
{

  DBObjPtr<SVDPulseShapeCalibrations> svdPulseShapeCal;
  // float adc = static_cast<float> ( svdPulseShapeCal->getADCFromCharge(22500.));

  std::cout << "Charge from ADC:  "  <<  svdPulseShapeCal->getChargeFromADC(1, true, 1,
            1) << ", ADC from Charge:  " << svdPulseShapeCal->getADCFromCharge(1, true, 1,
                22500.) << ", peaking time [ns]: " << svdPulseShapeCal->getPeakTime(1, true,
                    1) << ", pulse width [ns]: " << svdPulseShapeCal->getWidth(1, true, 1) << " . " << std::endl;

}


void SVDDatabaseImporter::printSVDNoiseCalibrations()
{

  DBObjPtr<SVDNoiseCalibrations> svdNoiseCal;
  // float adc = static_cast<float> ( svdNoiseCal->getADCFromCharge(22500.));

  std::cout << "Noise: "  <<  svdNoiseCal->getNoise(1, true, 1) << " . " << std::endl;

}

