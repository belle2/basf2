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


  for (ptree::value_type const& cfgDocumentChild :
       pt.get_child("cfg_document")) {

    if (cfgDocumentChild.first == "noise_run") {
      std::cout << "Masking bitmap is an attribute of the node <noise_run>!" << endl;
      int  maskFilter = cfgDocumentChild.second.get<int>("<xmlattr>.mask") ;
      std::cout << " masking bitmap    = " << maskFilter << endl;

    }
    if (cfgDocumentChild.first == "hardware_run") {
      std::cout << "Zero suppression is an attribute of the node <hardware_run>!" << endl;
      float  zeroSuppression = cfgDocumentChild.second.get<int>("<xmlattr>.zs_cut") ;
      std::cout << " zero suppression cut    = " << zeroSuppression << endl;

    }

    if (cfgDocumentChild.first == "i2c") {
      std::cout << "Latency is an attribute of the node <i2c>!" << endl;
      float  latency = cfgDocumentChild.second.get<int>("<xmlattr>.lat") ;
      std::cout << " latency  = " << latency << endl;

    }
  }


}

void SVDDetectorConfigurationImporter::importSVDLocalConfigParametersFromXML(const std::string& xmlfileName, bool errorTollerant)
{


}



template< class SVDcalibration >
void SVDDetectorConfigurationImporter::importSVDCalibrationsFromXML(const std::string& condDbname,
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
                  B2ERROR("Inconsistency among maps: xml files tells \n" <<
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








