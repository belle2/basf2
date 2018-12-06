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

//only for Phase2 Geometry!
void SVDDatabaseImporter::importSVDHotStripsCalibrations()
{
  DBImportObjPtr<SVDHotStripsCalibrations::t_payload > svdHotStripsCal(SVDHotStripsCalibrations::name);

  svdHotStripsCal.construct(25);

  m_firstExperiment = 1;
  m_firstRun = 0;
  m_lastExperiment = 5;
  m_lastRun = -1;

  B2INFO("importing default values: all strips unmasked");
  Bool_t isHot = false;

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
          svdHotStripsCal->set(layer, ladder, sensor, side, strip, isHot);


        side = 0;
        int maxStripNumber = 512;
        if (layer == 3) maxStripNumber = 768;

        for (int strip = 0; strip < maxStripNumber; strip++)
          svdHotStripsCal->set(layer, ladder, sensor, side, strip, isHot);

      }
    }
  }

  /*****************end of the Loop*****************/

  IntervalOfValidity iov(m_firstExperiment, m_firstRun,
                         m_lastExperiment, m_lastRun);

  svdHotStripsCal.import(iov);
  B2RESULT("SVDHotStripsCalibrations imported to database.");


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
