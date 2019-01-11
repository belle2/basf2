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
#include <svd/calibration/SVDDefaultCalibrationsImporter.h>

// Map from the online world (FADC id, ADC id, APV ch id)
// to the offline world (layer, ladder, sensor, view, cell)
#include <svd/online/SVDOnlineToOfflineMap.h>

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
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPedestalCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <svd/dbobjects/SVDLocalRunBadStrips.h>
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <vxd/dataobjects/VxdID.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

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
using boost::property_tree::ptree;

void SVDDefaultCalibrationsImporter::importSVDChannelMapping(const std::string& fileName)
{

  IntervalOfValidity iov(m_firstExperiment, m_firstRun, m_lastExperiment, m_lastRun);
  const std::string filename = FileSystem::findFile(fileName); //phase 3 xmlMapping
  B2INFO("Importing the svd online -> offline map " << fileName << "\n");
  //  const std::string filename = FileSystem::findFile("testbeam/vxd/data/2017_svd_mapping.xml");
  const std::string payloadname = "SVDChannelMapping.xml";
  if (Database::Instance().addPayload(payloadname, filename, iov))
    B2INFO("Success!");
  else
    B2INFO("Failure :( ua uaa uaa uaa uaaaa)");
}


void SVDDefaultCalibrationsImporter::importSVDTimeShiftCorrections()
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
//only for Phase2 Geometry!

void SVDDefaultCalibrationsImporter::importSVDFADCMaskedStrips()
{
  DBImportObjPtr<SVDFADCMaskedStrips::t_payload > svdFADCMasked(SVDFADCMaskedStrips::name);

  svdFADCMasked.construct(25);

  m_firstExperiment = 1;
  m_firstRun = 0;
  m_lastExperiment = 5;
  m_lastRun = -1;

  B2INFO("importing default values: all strips unmasked");
  Bool_t isMasked = false;

  /********************Loop for filling default values*********/


  //  unsigned int laddersOnLayer[] = { 0, 0, 0, 8, 11, 13, 17 };
  unsigned int laddersOnLayer[] = { 0, 0, 0, 2, 2, 2, 2 };
  for (unsigned int layer = 0 ; layer < 7 ; layer ++) {
    unsigned int sensorsOnLadder[] = {0, 0, 0, 3, 4, 5, 6};
    for (unsigned int ladder = 1; ladder < laddersOnLayer[layer]; ladder ++) {
      for (unsigned int sensor = 1; sensor < sensorsOnLadder[layer]; sensor ++) {

        B2INFO("layer: " << layer << ", ladder: " << ladder << " sensor: " << sensor);
        Bool_t side = 1;

        for (int strip = 0; strip < 768; strip++)
          svdFADCMasked->set(layer, ladder, sensor, side, strip, isMasked);


        side = 0;
        int maxStripNumber = 512;
        if (layer == 3) maxStripNumber = 768;

        for (int strip = 0; strip < maxStripNumber; strip++)
          svdFADCMasked->set(layer, ladder, sensor, side, strip, isMasked);

      }
    }
  }

  /*****************end of the Loop*****************/

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdFADCMasked.import(iov);
  B2RESULT("SVDFADCMaskedStrips imported to database.");


}


void SVDDefaultCalibrationsImporter::importSVDPulseShapeCalibrations()
{

  DBImportObjPtr<SVDPulseShapeCalibrations::t_calAmp_payload > svdPulseShapeCal(SVDPulseShapeCalibrations::calAmp_name);

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

  // Possible value of experimentType:
  // 0 : default for phase3
  // 1 : test beam run 111
  // 2 : test beam run 400 (with magnetic field)

  int experimentType = 2; // doing the import to localDB of run 400
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

        char s[8]; sprintf(s, "%u.%u.%u", layer, ladder, sensor);

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

        TVectorF* tmpSensorGain = (TVectorF*)(finTestBeam.Get(uSideGain.Data()));
        TVectorF* tmpSensorTime = (TVectorF*)(finTestBeam.Get(uSideTime.Data()));
        TVectorF* tmpSensorWidth = (TVectorF*)(finTestBeam.Get(uSideWidth.Data()));


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

void SVDDefaultCalibrationsImporter::importSVDNoiseCalibrations()
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
        char s[8]; sprintf(s, "%u.%u.%u", layer, ladder, sensor);
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

void SVDDefaultCalibrationsImporter::importSVDLocalRunBadStrips(/*std::string fileName*/)
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

/*+++++++++++++++++++++-----Print methods---------+++++++++++++++++++++*/

void SVDDefaultCalibrationsImporter::printSVDPulseShapeCalibrations()
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


void SVDDefaultCalibrationsImporter::printSVDNoiseCalibrations()
{

  SVDNoiseCalibrations svdNoiseCal;
  std::cout << "Noise L3_1_1 side V strip 0: "
            <<  svdNoiseCal.getNoise(VxdID(3, 1, 1), false, 0)
            << " . " << std::endl;

  std::cout << "Noise L3_1_1 V side V strip 1: "
            <<  svdNoiseCal.getNoise(VxdID(3, 1, 1), false, 1)
            << " . " << std::endl;


}


void SVDDefaultCalibrationsImporter::printSVDLocalRunBadStrips()
{

  DBObjPtr<SVDLocalRunBadStrips> svdBadstrips;
  // float adc = static_cast<float> ( svdBadstrips->getADCFromCharge(22500.));

  std::cout << "isBad: "  <<  svdBadstrips->isBad(1, true, 1) << " . " << std::endl;

}


