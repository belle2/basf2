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
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/dbobjects/SVDLocalRunBadStrips.h>

#include <vxd/dataobjects/VxdID.h>

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

  DBImportObjPtr<SVDPulseShapeCalibrations::t_payload > svdPulseShapeCal(SVDPulseShapeCalibrations::name);
  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  SVDStripCalAmp defaultCalAmp;
  defaultCalAmp.gain = 60. / 22500;
  defaultCalAmp.peakTime = 75.;
  defaultCalAmp.pulseWidth = 120.;
  svdPulseShapeCal.construct(defaultCalAmp);

  SVDStripCalAmp funnyCalAmp;
  funnyCalAmp.gain = 60. / 22500;
  funnyCalAmp.peakTime = .75;
  funnyCalAmp.pulseWidth = 1.20;
  /********************Loop for filling default values*********/


  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  for (unsigned int layer = 0 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {

        B2INFO("layer: " << layer << "ladder: " << ladder << " sensor: " << sensor);
        Bool_t side = 1;
        for (int strip = 0; strip < 768; strip++)  svdPulseShapeCal->set(layer, ladder, sensor, side, strip, defaultCalAmp);
        side = 0;
        int maxStripNumber = 512;
        if (layer == 3) maxStripNumber = 768;

        for (int strip = 0; strip < maxStripNumber; strip++) svdPulseShapeCal->set(layer, ladder, sensor, side, strip, defaultCalAmp);


      }
    }
  }

  /*****************end of the Loop*****************/
  //svdPulseShapeCal->set(3, 1, 1, 1, 0, funnyCalAmp);

  svdPulseShapeCal.import(iov);
  B2RESULT("SVDPulseShapeCalibrations imported to database.");

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

  DBImportObjPtr< SVDNoiseCalibrations::t_payload > svdnoisecal(SVDNoiseCalibrations::name);

  //stream.close();
  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdnoisecal.construct(3.33);

  // Here we write a different noise


  float defaultUsideNoiseSlanted = 900. / 375.;
  float defaultVsideNoiseSlanted = 700. / 375.;
  float defaultUsideNoiseBarrel = 1100. / 375.;
  float defaultVsideNoiseBarrel = 650. / 375.;
  float defaultUsideNoiseBwd = 950. / 375.;
  float defaultVsideNoiseBwd = 750. / 375.;
  float defaultUsideNoiseLayer3 = 1100. / 375.;
  float defaultVsideNoiseLayer3 = 800. / 375.;
  float valueToFill = -99;


  /***********************start filling loop**************/

  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  for (unsigned int layer = 1 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {
        /** Setting the correct noise value to fill*/
        Bool_t side = 1;

        if (layer == 3) valueToFill = defaultUsideNoiseLayer3;
        else {
          valueToFill = defaultUsideNoiseBarrel;
          if (sensor == 1) /** First sensor on ladders is Slanted */      valueToFill = defaultUsideNoiseSlanted;
          if (sensor == sensorsOnLadder[layer] - 1)
            valueToFill = defaultUsideNoiseBwd;
        }


        for (int strip = 0; strip < 768; strip++)
          svdnoisecal->set(layer, ladder, sensor, side, strip, valueToFill);
        B2INFO("Filled side U!");

        side = 0;

        int maxStripNumber = 512;
        if (layer == 3) {
          maxStripNumber = 768;
          valueToFill = defaultVsideNoiseLayer3;
        } else {
          valueToFill = defaultVsideNoiseBarrel;
          if (sensor == 1) /** First sensor on ladders is Slanted */      valueToFill = defaultVsideNoiseSlanted;
          if (sensor == sensorsOnLadder[layer] - 1)
            valueToFill = defaultVsideNoiseBwd;
        }
        for (int strip = 0; strip < maxStripNumber; strip++) svdnoisecal->set(layer, ladder, sensor, side, strip, valueToFill);

      }
    }
  }
  /************************end of filling*********************/

  svdnoisecal.import(iov);

  B2RESULT("SVDNoiseCalibrations imported to database.");

}

void SVDDatabaseImporter::importSVDLocalRunBadStrips(/*std::string fileName*/)
{
  /* std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");
  */

  DBImportObjPtr<SVDLocalRunBadStrips> svdbadstrips;

  //stream.close();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  svdbadstrips.construct();
  svdbadstrips.import(iov);


  B2RESULT("SVDLocalRunBadStrips imported to database.");

}


void SVDDatabaseImporter::printSVDPulseShapeCalibrations()
{
  SVDPulseShapeCalibrations svdPulseShapeCalibrations;
  SVDNoiseCalibrations svdNoiseCal;
  std::cout << "PeakTime L3_1_1 side V strip 0: "
            <<  svdPulseShapeCalibrations.getPeakTime(VxdID(3, 1, 1), false, 0)
            << " . " << std::endl;

  std::cout << "PeakTime L3_1_1 V side V strup 1: "
            <<  svdPulseShapeCalibrations.getPeakTime(VxdID(3, 1, 1), false, 1)
            << " . " << std::endl;

}


void SVDDatabaseImporter::printSVDNoiseCalibrations()
{

  SVDNoiseCalibrations svdNoiseCal;
  std::cout << "Noise L3_1_1 side V strip 0: "
            <<  svdNoiseCal.getNoise(VxdID(3, 1, 1), false, 0)
            << " . " << std::endl;

  std::cout << "Noise L3_1_1 V side V strup 1: "
            <<  svdNoiseCal.getNoise(VxdID(3, 1, 1), false, 1)
            << " . " << std::endl;


}


void SVDDatabaseImporter::printSVDLocalRunBadStrips()
{

  DBObjPtr<SVDLocalRunBadStrips> svdBadstrips;
  // float adc = static_cast<float> ( svdBadstrips->getADCFromCharge(22500.));

  std::cout << "isBad: "  <<  svdBadstrips->isBad(1, true, 1) << " . " << std::endl;

}

