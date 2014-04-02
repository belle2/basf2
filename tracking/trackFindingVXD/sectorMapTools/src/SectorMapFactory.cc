/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/sectorMapTools/SectorMapFactory.h"

#include "tracking/dataobjects/VXDTFSecMap.h"

#include <framework/logging/Logger.h>
// #include <framework/gearbox/Const.h>
#include <framework/gearbox/GearDir.h> // needed for reading xml-files, Gearbox::getInstance...
// #include <framework/datastore/StoreArray.h>
//
// #include <geometry/bfieldmap/BFieldMap.h>
// #include <tracking/dataobjects/VXDTFSecMap.h>
// #include <vxd/geometry/GeoCache.h>
// #include <vxd/geometry/SensorInfoBase.h>
// #include <framework/datastore/StoreObjPtr.h>
// #include <framework/dataobjects/EventMetaData.h>

// #include <TTree.h>
// #include <TFile.h>


#include <string>

using namespace std;
using namespace Belle2;


void importRootMap()
{
  string chosenSetup = "std";
  const VXDTFSecMap* newMap = NULL;
  string directory = "/Detector/Tracking/CATFParameters/" + chosenSetup;

  try {
    newMap = dynamic_cast<const VXDTFSecMap*>(Gearbox::getInstance().getTObject(directory.c_str()));
  } catch (exception& e) {
    B2FATAL("VXDTFModule::initialize: could not load sectorMap. Reason: exception thrown: " << e.what() << ", this means you have to check whether the sectorMaps stored in ../tracking/data/VXDTFindex.xml and/or ../testbeam/vxd/data/VXDTFindexTF.xml are uncommented and locally unpacked and available!")
  }
}
