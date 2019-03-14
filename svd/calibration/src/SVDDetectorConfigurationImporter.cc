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
#include <svd/calibration/SVDDetectorConfigurationImporter.h>

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
#include <svd/calibration/SVDDetectorConfiguration.h>
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



void SVDDetectorConfigurationImporter::importSVDGlobalConfigParametersFromXML(const std::string& xmlFileName, bool errorTollerant)
{
  // This is the property tree
  ptree pt;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xmlFileName, pt);

  //auxilairy variables to store the XML file values
  int  maskFilter = 0;
  float  zeroSuppression = 0;
  float  latency = 0;
  std::string systemClock = "";

  for (ptree::value_type const& cfgDocumentChild :
       pt.get_child("cfg_document")) {

    if (cfgDocumentChild.first == "noise_run") {
      //      std::cout << "Masking bitmap is an attribute of the node <noise_run>!" << endl;
      maskFilter = cfgDocumentChild.second.get<int>("<xmlattr>.mask") ;
      std::cout << " masking bitmap    = " << maskFilter << endl;

    }
    if (cfgDocumentChild.first == "hardware_run") {
      //      std::cout << "Zero suppression is an attribute of the node <hardware_run>!" << endl;
      zeroSuppression = cfgDocumentChild.second.get<float>("<xmlattr>.zs_cut") ;
      std::cout << " zero suppression cut    = " << zeroSuppression << endl;

    }

    if (cfgDocumentChild.first == "i2c") {
      //std::cout << "Latency is an attribute of the node <i2c>!" << endl;
      latency = cfgDocumentChild.second.get<float>("<xmlattr>.lat") ;
      std::cout << " latency  = " << latency << endl;

    }

    if (cfgDocumentChild.first == "fadc_ctrl") {
      //          std::cout << "Zero suppression is an attribute of the node <fadc_ctrl>!"<<endl;
      systemClock = cfgDocumentChild.second.get<std::string>("<xmlattr>.system_clock") ;
      std::cout << " APV clock units = " << systemClock << endl;

    }
  }

  DBImportObjPtr<SVDDetectorConfiguration::t_svdGlobalConfig_payload> svdGlobalConfig(SVDDetectorConfiguration::svdGlobalConfig_name);

  svdGlobalConfig.construct();

  svdGlobalConfig->setZeroSuppression(zeroSuppression);
  svdGlobalConfig->setLatency(latency);
  svdGlobalConfig->setMaskFilter(maskFilter);
  svdGlobalConfig->setAPVClockInRFCUnits(systemClock);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdGlobalConfig.import(iov);
  B2RESULT("SVDGlobalConfigParameters imported to database.");

}

void SVDDetectorConfigurationImporter::importSVDLocalConfigParametersFromXML(const std::string& xmlfileName, bool errorTollerant)
{

  // This is the property tree
  ptree pt;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xmlfileName, pt);

  //auxilairy variables to store the XML file values
  std::string calPeakUnits = "";
  std::string calibTimeUnits = "";
  std::string calibDate = "";

  for (ptree::value_type const& apvChild :
       pt.get_child("cfg_document.back_end_layout.fadc.adc.apv25")) {


    if (apvChild.first == "cal_peaks") {
      calPeakUnits = apvChild.second.get<std::string>("<xmlattr>.units") ;
      std::cout << " calPeak units   = " << calPeakUnits << endl;

    }

    if (apvChild.first == "cal_peak_time") {
      calibTimeUnits = apvChild.second.get<std::string>("<xmlattr>.units") ;
      std::cout << " calibration time units   = " << calibTimeUnits << endl;

    }
  }
  for (ptree::value_type const& latestRunChild :
       pt.get_child("cfg_document.latest_runs")) {

    if (latestRunChild.first == "Noise") {
      //      std::cout << "Masking bitmap is an attribute of the node <noise_run>!" << endl;
      calibDate = latestRunChild.second.get<std::string>("<xmlattr>.end_of_run") ;
      std::cout << " calibration date    = " << calibDate << endl;

    }
  }

  DBImportObjPtr<SVDDetectorConfiguration::t_svdLocalConfig_payload> svdLocalConfig(SVDDetectorConfiguration::svdLocalConfig_name);

  svdLocalConfig.construct();

  svdLocalConfig->setCalPeakUnits(calPeakUnits);
  svdLocalConfig->setCalibrationTimeInRFCUnits(calibTimeUnits);
  svdLocalConfig->setCalibDate(calibDate);
  /* Injected charge set is HARDCODED here, by default 22500 electrons
   */
  svdLocalConfig->setInjectedCharge(22500);


  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdLocalConfig.import(iov);
  B2RESULT("SVDLocalConfigParameters imported to database.");

}








