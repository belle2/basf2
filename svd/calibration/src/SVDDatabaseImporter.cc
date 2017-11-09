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

// wrapper objects
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/dbobjects/SVDLocalRunBadStrips.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <vxd/dataobjects/VxdID.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <TFile.h>
#include <TVectorF.h>
/*
#include <fstream>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
*/

using namespace std;
using namespace Belle2;

void SVDDatabaseImporter::importSVDTimeShiftCorrections()
{

  DBImportObjPtr<SVDPulseShapeCalibrations::t_time_payload > svdTimeShiftCal(SVDPulseShapeCalibrations::time_name);

  svdTimeShiftCal.construct(25);

  m_firstExperiment = 3;
  //  m_firstRun = 111;
  m_firstRun = 400;
  m_lastExperiment = 3;
  //  m_lastRun = 111;
  m_lastRun = 400;

  B2INFO("importing values for run 111 of test beam (evaluated on 10k events of run111");

  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  for (unsigned int layer = 0 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {

        //U side
        bool side = 1 ;

        /*  //run111
              float valueToFill = 31.5; //all layers except L6
        //RMS = 6 for all layers
        if(layer == 6)
          valueToFill = 25.5;
        */

        float valueToFill = 42; //all layers except L6
        //RMS = 6 for all layers
        if (layer == 6)
          valueToFill = 34.5;

        for (int strip = 0; strip < 768; strip++)
          svdTimeShiftCal->set(layer, ladder, sensor, side, strip, valueToFill);


        //V side
        side = 0;
        valueToFill = 30.5;
        //RMS = 5 for all layers

        int maxStripNumber = 512;
        if (layer == 3) maxStripNumber = 768;

        for (int strip = 0; strip < maxStripNumber; strip++)
          svdTimeShiftCal->set(layer, ladder, sensor, side, strip, valueToFill);

      }
    }
  }

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdTimeShiftCal.import(iov);

  B2RESULT("SVDTimeShiftCorrections imported to database.");
}


void SVDDatabaseImporter::importSVDPulseShapeCalibrations()
{

  DBImportObjPtr<SVDPulseShapeCalibrations::t_payload > svdPulseShapeCal(SVDPulseShapeCalibrations::name);

  /*
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
  */

  int experimentType =
    -99; //Possible value: 0, 1, 2. experimentType = 0: default for phase3, experimentType = 1 : test beam run 111 ; experimentType = 2 : test beam run 400 (with magnetic field)

  experimentType = 2; // doing the import to localDB of run 400
  std::string fileName = "";
  if (experimentType == 1) {
    m_firstExperiment = 3;
    m_firstRun = 111;
    m_lastExperiment = 3;
    m_lastRun = 111;

    B2INFO("importing default values for run 111 of test beam");

    fileName = "/home/belle2/zani/run111_testbeam/SVDShaperProperties.root";

  }

  if (experimentType == 2) {
    m_firstExperiment = 3;
    m_firstRun = 400;
    m_lastExperiment = 3;
    m_lastRun = 400;

    B2INFO("importing default values for run 400 of test beam");

    fileName = "/home/belle2/zani/run400_testbeam/SVDShaperProperties.root";

  }

  TFile finTestBeam(fileName.c_str());
  finTestBeam.Print();
  SVDStripCalAmp defaultCalAmp;
  defaultCalAmp.gain = -99; // 60. / 22500;
  defaultCalAmp.peakTime = -99; // 75.;
  defaultCalAmp.pulseWidth = -99; // 120.;
  svdPulseShapeCal.construct(defaultCalAmp);

  /********************Loop for filling default values*********/


  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  for (unsigned int layer = 0 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {

        //        B2INFO("layer: " << layer << "ladder: " << ladder << " sensor: " << sensor);
        Bool_t side = 1;

        char s[8]; sprintf(s, "%d.%d.%d", layer, ladder, sensor);

        TString uSideGain(s);
        uSideGain = uSideGain + ".u_peak";
        TString uSideTime(s);
        uSideTime = uSideTime + ".u_time";
        TString uSideWidth(s);
        uSideWidth = uSideWidth + ".u_fwhm";

        TString vSideGain(s);
        vSideGain = vSideGain + ".v_peak";
        TString vSideTime(s);
        vSideTime = vSideTime + ".v_time";
        TString vSideWidth(s);
        vSideWidth = vSideWidth + ".v_fwhm";

        TVectorF* tmpSensorGain = NULL;
        tmpSensorGain = (TVectorF*)(finTestBeam.Get(uSideGain.Data()));
        TVectorF* tmpSensorTime = NULL;
        tmpSensorTime = (TVectorF*)(finTestBeam.Get(uSideTime.Data()));
        TVectorF* tmpSensorWidth = NULL;
        tmpSensorWidth = (TVectorF*)(finTestBeam.Get(uSideWidth.Data()));


        for (int strip = 0; strip < 768; strip++) {
          if (tmpSensorGain) defaultCalAmp.gain = ((*tmpSensorGain))[strip] / 22500.;
          else defaultCalAmp.gain = NAN;
          if (tmpSensorTime) defaultCalAmp.peakTime = ((*tmpSensorTime))[strip];
          else defaultCalAmp.peakTime = NAN;
          if (tmpSensorWidth) defaultCalAmp.pulseWidth = ((*tmpSensorWidth))[strip] * 31.44 / 8.;
          else defaultCalAmp.pulseWidth = NAN;
          svdPulseShapeCal->set(layer, ladder, sensor, side, strip, defaultCalAmp);
        }

        side = 0;
        int maxStripNumber = 512;
        if (layer == 3) maxStripNumber = 768;

        tmpSensorGain = (TVectorF*)(finTestBeam.Get(vSideGain.Data()));
        tmpSensorTime = (TVectorF*)(finTestBeam.Get(vSideTime.Data()));
        tmpSensorWidth = (TVectorF*)(finTestBeam.Get(vSideWidth.Data()));
        for (int strip = 0; strip < maxStripNumber; strip++) {
          if (tmpSensorGain) defaultCalAmp.gain = ((*tmpSensorGain))[strip] / 22500.;
          else defaultCalAmp.gain = NAN;
          if (tmpSensorTime) defaultCalAmp.peakTime = ((*tmpSensorTime))[strip];
          else defaultCalAmp.peakTime = NAN;
          if (tmpSensorWidth) defaultCalAmp.pulseWidth = ((*tmpSensorWidth))[strip] * 31.44 / 8.;
          else  defaultCalAmp.pulseWidth = NAN;
          svdPulseShapeCal->set(layer, ladder, sensor, side, strip, defaultCalAmp);
        }

      }
    }
  }

  /*****************end of the Loop*****************/
  //svdPulseShapeCal->set(3, 1, 1, 1, 0, funnyCalAmp);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdPulseShapeCal.import(iov);
  B2RESULT("SVDPulseShapeCalibrations imported to database.");


}


void SVDDatabaseImporter::importSVDNoiseCalibrations()
{

  DBImportObjPtr< SVDNoiseCalibrations::t_payload > svdnoisecal(SVDNoiseCalibrations::name);

  svdnoisecal.construct(3.33);

  int experimentType =
    -99; //Possible value: 0, 1, 2. experimentType = 0: default for phase3, experimentType = 1 : test beam run 111 ; experimentType = 2 : test beam run 400 (with magnetic field)

  experimentType = 2; // doing the import to localDB of run 400
  std::string fileName = "";

  if (experimentType == 1) {
    m_firstExperiment = 3;
    m_firstRun = 111;
    m_lastExperiment = 3;
    m_lastRun = 111;

    B2INFO("importing default values for run 111 of test beam");

    fileName = "/home/belle2/zani/run111_testbeam/SVDNoises.root";

  }

  if (experimentType == 2) {
    m_firstExperiment = 3;
    m_firstRun = 400;
    m_lastExperiment = 3;
    m_lastRun = 400;

    B2INFO("importing default values for run 400 of test beam");

    fileName = "/home/belle2/zani/run400_testbeam/SVDNoises.root";

  }

  // Here we write a different noise, for a different interval of validity

  TFile finTestBeam(fileName.c_str());
  finTestBeam.Print();


  // these were default values for phase 3

  /*
    if (! experimentType.compare("phase3") ) // compare rerturn 0 if strings are equal
    {
    float defaultUsideNoiseSlanted = 900. / 375.;
    float defaultVsideNoiseSlanted = 700. / 375.;
    float defaultUsideNoiseBarrel = 1100. / 375.;
    float defaultVsideNoiseBarrel = 650. / 375.;
    float defaultUsideNoiseBwd = 950. / 375.;
    float defaultVsideNoiseBwd = 750. / 375.;
    float defaultUsideNoiseLayer3 = 1100. / 375.;
    float defaultVsideNoiseLayer3 = 800. / 375.;
    float valueToFill = -99;
  */

  /***********************start filling loop**************/

  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  for (unsigned int layer = 1 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {

        /** Setting the correct noise value to fill*/
        Bool_t side = 1;
        char s[8]; sprintf(s, "%d.%d.%d", layer, ladder, sensor);
        TString uSide(s);
        uSide = uSide + ".u";

        TString vSide(s);
        vSide = vSide + ".v";
        float valueToFill = -99;

        /* if (layer == 3) valueToFill = defaultUsideNoiseLayer3;
           else {
           valueToFill = defaultUsideNoiseBarrel;
           if (sensor == 1) // First sensor on ladders is Slanted
           valueToFill = defaultUsideNoiseSlanted;
           if (sensor == sensorsOnLadder[layer] - 1)
           valueToFill = defaultUsideNoiseBwd;
           }
        */
        TVectorF* tmp_sensor = NULL;
        tmp_sensor = (TVectorF*)(finTestBeam.Get(uSide.Data()));

        for (int strip = 0; strip < 768; strip++) {
          if (tmp_sensor) valueToFill = ((*tmp_sensor))[strip];
          else valueToFill = NAN;
          svdnoisecal->set(layer, ladder, sensor, side, strip, valueToFill);
        }
        B2INFO("Filled side U!");

        side = 0;
        tmp_sensor = (TVectorF*)(finTestBeam.Get(vSide.Data()));

        int maxStripNumber = 512;
        /*
        // old loop for filling n side default values for phase3
        if (layer == 3) {
          maxStripNumber = 768;
          valueToFill = defaultVsideNoiseLayer3;
        } else {
        valueToFill = defaultVsideNoiseBarrel;
        if (sensor == 1) // First sensor on ladders is Slanted
          valueToFill = defaultVsideNoiseSlanted;
        if (sensor == sensorsOnLadder[layer] - 1)
          valueToFill = defaultVsideNoiseBwd;
        }
        */
        if (layer == 3) maxStripNumber = 768;
        for (int strip = 0; strip < maxStripNumber; strip++) {
          if (tmp_sensor) valueToFill = ((*tmp_sensor))[strip];
          else valueToFill = NAN;
          svdnoisecal->set(layer, ladder, sensor, side, strip, valueToFill);
        }
      }
    }
  }

  //}//end if("phase3")
  /************************end of filling*********************/

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

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

void SVDDatabaseImporter::importSVDHitTimeNeuralNetwork(string fileName, bool threeSamples)
{
  ifstream xml(fileName);
  if (!xml.good()) {
    B2RESULT("ERROR: File not found.\nNeural network from " << fileName << " could not be imported.");
    return;
  }
  string label("SVDTimeNet_6samples");
  if (threeSamples)
    label = "SVDTimeNet_3samples";
  stringstream buffer;
  buffer << xml.rdbuf();
  DBImportObjPtr<DatabaseRepresentationOfWeightfile> importObj(label);
  importObj.construct();
  importObj->m_data = buffer.str();

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);
  importObj.import(iov);
  B2RESULT("Neural network from " << fileName << " successfully imported.");
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

  std::cout << "Noise L3_1_1 V side V strip 1: "
            <<  svdNoiseCal.getNoise(VxdID(3, 1, 1), false, 1)
            << " . " << std::endl;


}


void SVDDatabaseImporter::printSVDLocalRunBadStrips()
{

  DBObjPtr<SVDLocalRunBadStrips> svdBadstrips;
  // float adc = static_cast<float> ( svdBadstrips->getADCFromCharge(22500.));

  std::cout << "isBad: "  <<  svdBadstrips->isBad(1, true, 1) << " . " << std::endl;

}


