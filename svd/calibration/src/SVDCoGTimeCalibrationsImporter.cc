/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <svd/calibration/SVDCoGTimeCalibrationsImporter.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/PayloadFile.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <framework/utilities/FileSystem.h>

// wrapper objects
#include <svd/calibration/SVDCoGTimeCalibrations.h>
//db object
#include <svd/dbobjects/SVDCoGCalibrationFunction.h>

#include <vxd/dataobjects/VxdID.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <TFile.h>
#include <TVectorF.h>

using namespace std;
using namespace Belle2;
using boost::property_tree::ptree;

void SVDCoGTimeCalibrationsImporter::importNULLCoGTimeCalibrations()
{

  DBImportObjPtr< SVDCoGTimeCalibrations::t_payload > svdcogtimecal(SVDCoGTimeCalibrations::name);

  svdcogtimecal.construct(SVDCoGCalibrationFunction(), "");

  m_firstExperiment = 0;
  m_firstRun = 0;
  m_lastExperiment = -1;
  m_lastRun = -1;

  B2INFO("importing default values for IoV 0,0,-1,-1");

  /***********************start filling loop**************/

  SVDCoGCalibrationFunction timeCal;
  timeCal.set_shift(0, 0, 0, 0);
  timeCal.set_scale(1, 1, 1, 1);
  timeCal.print_par();
  B2INFO("beta = " << timeCal.calibratedValue(0, 0));
  B2INFO("alfa + beta = " << timeCal.calibratedValue(1, 0));

  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  for (unsigned int layer = 1 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {

        int NstripsV = 512;
        if (layer == 3)
          NstripsV = 768;

        /** Setting slope = 1 and shift = 0 for all sensors*/
        for (int strip = 0; strip < 768; strip ++) {
          svdcogtimecal->set(layer, ladder, sensor, 1, strip, timeCal);
          if (strip < NstripsV)
            svdcogtimecal->set(layer, ladder, sensor, 0, strip, timeCal);
        }
      }
    }
  }

  timeCal.print_par();
  /************************end of filling*********************/

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdcogtimecal.import(iov);

  B2RESULT("SVDCoGTimeCalibrations imported to database.");

}

void SVDCoGTimeCalibrationsImporter::printSVDCoGTimeCalibrations(int layer, int ladder, int sensor, bool isUside)
{

  SVDCoGTimeCalibrations svdCoGTimeCal;
  float beta = svdCoGTimeCal.getCorrectedTime(VxdID(layer, ladder, sensor), isUside, 0, 0, 0); //t_RAW = 0
  float alfa = svdCoGTimeCal.getCorrectedTime(VxdID(layer, ladder, sensor), isUside, 0, 1, 0) - beta; //t_RAW = 1 - beta

  std::cout << "CoGTime corrections for L3_1_1 side V strip 0: "
            << " beta = " << beta << "ns " << std::endl;
  std::cout << " alfa = " << alfa << "ns " << std::endl;
}
