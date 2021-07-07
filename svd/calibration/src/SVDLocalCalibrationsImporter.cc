/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <svd/calibration/SVDLocalCalibrationsImporter.h>

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
#include <framework/logging/Logger.h>

#include <framework/utilities/FileSystem.h>

// wrapper objects
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPedestalCalibrations.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDHotStripsCalibrations.h>
#include <svd/calibration/SVDFADCMaskedStrips.h>

#include <vxd/dataobjects/VxdID.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

using namespace std;
using namespace Belle2;
using boost::property_tree::ptree;

void SVDLocalCalibrationsImporter::importSVDChannelMapping(const std::string& fileName)
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


void SVDLocalCalibrationsImporter::importSVDNoiseCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant)
{
  // We do initialize the noise to a negative value so that
  // the SNR for not properly initialized channels is negative
  // CAVEAT EMPTOR: if thou will disable an APV25 chip and will
  // not update the noise.... the hits on the previously disabled
  // chips will be discarded by the clusterizer.
  // That is: please please pleaseeee do not upload calibrations
  // with incomplete setup.
  importSVDCalibrationsFromXML< SVDNoiseCalibrations::t_payload  >(SVDNoiseCalibrations::name,
      xmlFileName, "noises",
      -1.0, errorTollerant);
}

void SVDLocalCalibrationsImporter::importSVDPedestalCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant)
{
  importSVDCalibrationsFromXML< SVDPedestalCalibrations::t_payload  >(SVDPedestalCalibrations::name,
      xmlFileName, "pedestals",
      -1.0, errorTollerant);
}

void SVDLocalCalibrationsImporter::importSVDHotStripsCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant)
{
  importSVDCalibrationsFromXML< SVDHotStripsCalibrations::t_payload  >(SVDHotStripsCalibrations::name,
      xmlFileName, "hot_strips",
      false, errorTollerant);
}

void SVDLocalCalibrationsImporter::importSVDFADCMaskedStripsFromXML(const std::string& xmlFileName, bool errorTollerant)
{
  importSVDCalibrationsFromXML< SVDFADCMaskedStrips::t_payload  >(SVDFADCMaskedStrips::name,
      xmlFileName, "masks",
      false, errorTollerant);
}




template< class SVDcalibration >
void SVDLocalCalibrationsImporter::importSVDCalibrationsFromXML(const std::string& condDbname,
    const std::string& xmlFileName,
    const std::string& xmlTag,
    typename SVDcalibration::t_perSideContainer::calibrationType defaultValue,
    bool errorTollerant)
{
  DBImportObjPtr< SVDcalibration> payload(condDbname);

  DBObjPtr<PayloadFile> OnlineToOfflineMapFileName("SVDChannelMapping.xml");

  OnlineToOfflineMapFileName.hasChanged();

  unique_ptr<SVDOnlineToOfflineMap> map =
    make_unique<SVDOnlineToOfflineMap>(OnlineToOfflineMapFileName->getFileName());

  payload.construct(defaultValue , xmlFileName);

  // This is the property tree
  ptree pt;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xmlFileName, pt);

  for (ptree::value_type const& backEndLayoutChild :
       pt.get_child("cfg_document.back_end_layout")) {

    if (backEndLayoutChild.first == "fadc") {
      int FADCid(0);
      string FADCidString = backEndLayoutChild.second.get<string>("<xmlattr>.id");
      stringstream ss;
      ss << std::hex << FADCidString;
      ss >> FADCid;

      for (ptree::value_type const& fadcChild : backEndLayoutChild.second.get_child("")) {
        if (fadcChild.first == "adc") {
          int ADCid = fadcChild.second.get<int>("<xmlattr>.id") ;
          B2DEBUG(1, "  ADC id    = " << ADCid);

          int layerId = fadcChild.second.get<int>("<xmlattr>.layer_id");
          B2DEBUG(1, "  layer_id  = " << layerId);

          int ladderId = fadcChild.second.get<int>("<xmlattr>.ladder_id") ;
          B2DEBUG(1, "  ladder_id = " << ladderId);

          int hybridId = fadcChild.second.get<int>("<xmlattr>.hybrid_id");
          B2DEBUG(1, "  hybrid_id = " << hybridId);

          B2DEBUG(1, "  delay25   = " <<
                  fadcChild.second.get<int>("<xmlattr>.delay25"));

          for (ptree::value_type const& apvChild : fadcChild.second.get_child("")) {
            if (apvChild.first == "apv25") {
              int apv25ADCid = apvChild.second.get<int>("<xmlattr>.id");
              string valuesString = apvChild.second.get<string>(xmlTag) ;
              B2DEBUG(10, xmlTag << " APV25ID" << apv25ADCid << " "
                      << valuesString << "\n~~~~~~~~\n");

              stringstream ssn;
              ssn << valuesString;
              double value;
              for (int apvChannel  = 0 ; apvChannel < 128; apvChannel ++) {
                ssn >> value;
                const SVDOnlineToOfflineMap::SensorInfo& info =
                  map->getSensorInfo(FADCid, ADCid * 6 + apv25ADCid);

                short strip = map->getStripNumber(apvChannel, info);
                int side = info.m_uSide ?
                           SVDcalibration::Uindex :
                           SVDcalibration::Vindex ;
                int layer = info.m_sensorID.getLayerNumber();
                int ladder = info.m_sensorID.getLadderNumber();
                int sensor = info.m_sensorID.getSensorNumber();
                if (apvChannel % 127 == 0)
                  B2DEBUG(100, layer << "_"  << ladder << "_" <<
                          sensor << "_" << side << "_" << strip << "( " <<
                          apvChannel << ") " << value);
                if (errorTollerant || layer != layerId || ladder != ladderId
                    // ||
                    // test on the sensor != f( hybrid) anr apv perhaps
                   )
                  B2FATAL("Inconsistency among maps: xml files tells \n" <<
                          "layer " << layerId << " ladder " << ladderId <<
                          " hybridID " << hybridId << "\n" <<
                          "while the BASF2 map tells \n" <<
                          "layer " << layer << " ladder " << ladder <<
                          " sensor " << sensor << "\n");


                payload->set(layer, ladder, sensor, side , strip, value);
              }
            }
          }
        }
      }
    }
  }



  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  payload.import(iov);

  B2RESULT("Imported to database.");

}

/****
 * HERE!
 */
void SVDLocalCalibrationsImporter::importSVDCalAmpCalibrationsFromXML(const std::string& xmlFileName, bool errorTollerant)
{

  DBImportObjPtr< typename SVDPulseShapeCalibrations::t_calAmp_payload > pulseShapes(SVDPulseShapeCalibrations::calAmp_name);

  DBObjPtr<PayloadFile> OnlineToOfflineMapFileName("SVDChannelMapping.xml");

  OnlineToOfflineMapFileName.hasChanged();

  unique_ptr<SVDOnlineToOfflineMap> map =
    make_unique<SVDOnlineToOfflineMap>(OnlineToOfflineMapFileName->getFileName());

  pulseShapes.construct(SVDStripCalAmp() , xmlFileName);

  // This is the property tree
  ptree pt;

  // Load the XML file into the property tree. If reading fails
  // (cannot open file, parse error), an exception is thrown.
  read_xml(xmlFileName, pt);

  for (ptree::value_type const& backEndLayoutChild :
       pt.get_child("cfg_document.back_end_layout")) {

    if (backEndLayoutChild.first == "fadc") {
      int FADCid(0);
      string FADCidString = backEndLayoutChild.second.get<string>("<xmlattr>.id");
      stringstream ss;
      ss << std::hex << FADCidString;
      ss >> FADCid;

      for (ptree::value_type const& fadcChild : backEndLayoutChild.second.get_child("")) {
        if (fadcChild.first == "adc") {
          int ADCid = fadcChild.second.get<int>("<xmlattr>.id") ;
          B2DEBUG(1, "  ADC id    = " << ADCid);

          int layerId = fadcChild.second.get<int>("<xmlattr>.layer_id");
          B2DEBUG(1, "  layer_id  = " << layerId);

          int ladderId = fadcChild.second.get<int>("<xmlattr>.ladder_id") ;
          B2DEBUG(1, "  ladder_id = " << ladderId);

          int hybridId = fadcChild.second.get<int>("<xmlattr>.hybrid_id");
          B2DEBUG(1, "  hybrid_id = " << hybridId);

          B2DEBUG(1, "  delay25   = " << fadcChild.second.get<int>("<xmlattr>.delay25"));

          for (ptree::value_type const& apvChild : fadcChild.second.get_child("")) {
            if (apvChild.first == "apv25") {
              int apv25ADCid = apvChild.second.get<int>("<xmlattr>.id");
              string ampString = apvChild.second.get<string>("cal_peaks") ;
              string widthString = apvChild.second.get<string>("cal_width") ;
              string peakTimeString = apvChild.second.get<string>("cal_peak_time") ;

              stringstream ssAmp;
              ssAmp << ampString;

              stringstream ssWidth;
              ssWidth << widthString;

              stringstream ssPeak;
              ssPeak << peakTimeString;

              double amp, width, peakTime;
              for (int apvChannel  = 0 ; apvChannel < 128; apvChannel ++) {
                ssAmp >> amp;
                ssWidth >> width;
                ssPeak >> peakTime;

                const SVDOnlineToOfflineMap::SensorInfo& info = map->getSensorInfo(FADCid, ADCid * 6 + apv25ADCid);

                short strip = map->getStripNumber(apvChannel, info);
                int side = info.m_uSide ?
                           SVDPulseShapeCalibrations::t_calAmp_payload::Uindex :
                           SVDPulseShapeCalibrations::t_calAmp_payload::Vindex;

                int layer = info.m_sensorID.getLayerNumber();
                int ladder = info.m_sensorID.getLadderNumber();
                int sensor = info.m_sensorID.getSensorNumber();
                if (errorTollerant || layer != layerId || ladder != ladderId   // ||
                    // test on the sensor != f( hybrid) anr apv perhaps
                   )
                  B2ERROR("Inconsistency among maps: xml files tels \n" <<
                          "layer " << layerId << " ladder " << ladderId << " hybridID " << hybridId << "\n" <<
                          "while the BASF2 map tels \n" <<
                          "layer " << layer << " ladder " << ladder << " sensor " << sensor << "\n");

                SVDStripCalAmp stripCalAmp;
                // UGLY: 22500. 31.44/8 should be written somewhere in the XML file provided by Hao
                stripCalAmp.gain = amp / 22500.;
                stripCalAmp.peakTime = peakTime * 31.44 / 8;
                stripCalAmp.pulseWidth = width * 31.44 / 8 ;
                pulseShapes->set(layer, ladder, sensor, side , strip, stripCalAmp);

              }
            }
          }
        }
      }
    }
  }



  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  pulseShapes.import(iov);

  B2RESULT("Imported to database.");


}



