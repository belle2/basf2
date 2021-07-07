/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <svd/calibration/SVDDetectorConfigurationImporter.h>

// Map from the online world (FADC id, ADC id, APV ch id)
// to the offline world (layer, ladder, sensor, view, cell)
//#include <svd/online/SVDOnlineToOfflineMap.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

#include <framework/utilities/FileSystem.h>

// DB objects
#include <svd/dbobjects/SVDLocalConfigParameters.h>
#include <svd/dbobjects/SVDGlobalConfigParameters.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace Belle2;
using boost::property_tree::ptree;


void SVDDetectorConfigurationImporter::importSVDGlobalXMLFile(const std::string& fileName)
{

  IntervalOfValidity iov(m_firstExperiment, m_firstRun, m_lastExperiment, m_lastRun);
  const std::string filename = FileSystem::findFile(fileName);
  B2INFO("Importing the global run configuration xml file " << fileName << "\n");

  const std::string payloadname = "SVDGlobalXMLFile.xml";
  if (Database::Instance().addPayload(payloadname, filename, iov))
    B2INFO("Success!");
  else
    B2INFO("Failure :( ua uaa uaa uaa uaaaa)");
}

void SVDDetectorConfigurationImporter::importSVDGlobalConfigParametersFromXML(const std::string& xmlFileName)
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
  float hv = 0;
  int relativeTimeShift = 0;
  int nrFrames = 0;

  for (ptree::value_type const& cfgDocumentChild :
       pt.get_child("cfg_document")) {

    if (cfgDocumentChild.first == "noise_run") {
      maskFilter = cfgDocumentChild.second.get<int>("<xmlattr>.mask") ;
      B2INFO(" masking bitmap    = " << maskFilter);

    }
    if (cfgDocumentChild.first == "hardware_run") {
      zeroSuppression = cfgDocumentChild.second.get<float>("<xmlattr>.zs_cut") ;
      B2INFO(" zero suppression cut    = " << zeroSuppression);

    }

    if (cfgDocumentChild.first == "i2c") {
      latency = cfgDocumentChild.second.get<float>("<xmlattr>.lat") ;
      B2INFO(" latency  = " << latency);

    }

    if (cfgDocumentChild.first == "fadc_ctrl") {
      systemClock = cfgDocumentChild.second.get<std::string>("<xmlattr>.system_clock") ;
      B2INFO(" APV clock units = " << systemClock);
      nrFrames = cfgDocumentChild.second.get<int>("<xmlattr>.nr_frames") ;
      B2INFO(" Number of Frames = " << nrFrames);

    }

    if (cfgDocumentChild.first == "controller") {
      relativeTimeShift = cfgDocumentChild.second.get<int>("<xmlattr>.mix_trg_delay") ;
      B2INFO(" delay of 3-sample VS 6-sample in units of APV clock /4 = " << relativeTimeShift);
      if ((relativeTimeShift < 0) || (relativeTimeShift > 15))
        B2FATAL("OOPS!! the relative time shift = " << relativeTimeShift <<
                " is not allowed! It must be an int between 0 and 15 included. Please check the global xml. For the moment we set it to 0");
    }
  }

  for (ptree::value_type const& cfgDocumentChild :
       pt.get_child("cfg_document.ps_setup.hv_config")) {
    if (cfgDocumentChild.first == "config") {
      hv = cfgDocumentChild.second.get<float>("<xmlattr>.v_conf") ;
      B2INFO(" HV    = " << hv);
    }

  }

  DBImportObjPtr<SVDGlobalConfigParameters> svdGlobalConfig("SVDGlobalConfigParameters");

  svdGlobalConfig.construct(xmlFileName);

  svdGlobalConfig->setZeroSuppression(zeroSuppression);
  svdGlobalConfig->setLatency(latency);
  svdGlobalConfig->setMaskFilter(maskFilter);
  svdGlobalConfig->setAPVClockInRFCUnits(systemClock);
  svdGlobalConfig->setHV(hv);
  svdGlobalConfig->setRelativeTimeShift(relativeTimeShift);
  svdGlobalConfig->setNrFrames(nrFrames);

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdGlobalConfig.import(iov);
  B2RESULT("SVDGlobalConfigParameters imported to database.");

}

void SVDDetectorConfigurationImporter::importSVDLocalConfigParametersFromXML(const std::string& xmlFileName)
{

  // This is the property tree
  ptree pt;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xmlFileName, pt);

  //auxilairy variables to store the XML file values
  // TODO: calInjectedCharge is not used! Check if it can be removed.
  std::string calInjectedCharge;
  std::string calibTimeUnits = "";
  std::string calibDate = "";

  for (ptree::value_type const& apvChild :
       pt.get_child("cfg_document.back_end_layout.fadc.adc.apv25")) {


    if (apvChild.first == "cal_peaks") {
      calInjectedCharge = apvChild.second.get<std::string>("<xmlattr>.units");
      B2INFO(" injected charge from XML  = " << calInjectedCharge << ", but actually set to 22500, hardcoded");
    }

    if (apvChild.first == "cal_peak_time") {
      calibTimeUnits = apvChild.second.get<std::string>("<xmlattr>.units");
      B2INFO(" calibration time units   = " << calibTimeUnits);

    }
  }
  for (ptree::value_type const& latestRunChild :
       pt.get_child("cfg_document.latest_runs")) {

    if (latestRunChild.first == "Noise") {
      calibDate = latestRunChild.second.get<std::string>("<xmlattr>.end_of_run");
      B2INFO(" calibration date    = " << calibDate);

    }
  }

  DBImportObjPtr<SVDLocalConfigParameters> svdLocalConfig("SVDLocalConfigParameters");

  svdLocalConfig.construct(xmlFileName);

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








