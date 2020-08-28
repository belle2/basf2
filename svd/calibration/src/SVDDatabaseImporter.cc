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
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// wrapper objects
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>

#include <boost/property_tree/ptree.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace Belle2;
using boost::property_tree::ptree;


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
