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
#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingVXD/sectorMapTools/Sector.h>

// #include <geometry/bfieldmap/BFieldMap.h>
// #include <tracking/dataobjects/VXDTFSecMap.h>
// #include <vxd/geometry/GeoCache.h>
// #include <vxd/geometry/SensorInfoBase.h>
// #include <framework/datastore/StoreObjPtr.h>
// #include <framework/dataobjects/EventMetaData.h>

// #include <TTree.h>
// #include <TFile.h>




using namespace std;
using namespace Belle2;


void SectorMapFactory::initialize()
{
  bool wasInitializedBefore = DataStore::Instance().getInitializeActive();

  if (wasInitializedBefore == false) DataStore::Instance().setInitializeActive(true);

  StoreArray<Sector>::registerPersistent(m_sectorMapName);

  if (wasInitializedBefore == false) DataStore::Instance().setInitializeActive(false);
}


void SectorMapFactory::importRootMap()
{
  const VXDTFSecMap* newMap = NULL;
  string directory = "/Detector/Tracking/CATFParameters/" + m_sectorMapName;

  try {
    newMap = dynamic_cast<const VXDTFSecMap*>(Gearbox::getInstance().getTObject(directory.c_str()));
  } catch (exception& e) {
    B2FATAL("SectorMapFactory::importRootMap: could not load sectorMap. Reason: exception thrown:\n"
            << e.what() << "," <<
            "\nthis means you have to check whether the sectorMaps stored in " <<
            "\n ../tracking/data/VXDTFindex.xml and/or in ../testbeam/vxd/data/VXDTFindexTF.xml " <<
            "\n are uncommented and locally unpacked and available!")
  }
  B2INFO("size is " << newMap->getSectorMap().size())

  StoreArray<Sector> sectorArray(m_sectorMapName);

  if (!sectorArray.isValid())  sectorArray.create();

  //direct construction
//         EventMetaData* newobj = evtData.appendNew();
//         newobj->setEvent(10 + i);
  for (auto & rawSectorData : newMap->getSectorMap()) {
    Sector* aSector = sectorArray.appendNew(rawSectorData.first);
    aSector->setDistance(0.1);
  }

  /** this shall be a sorted sectorSet by sectorID (independently of what the sortingParameter says).
   * the reason for this is that the searching in the map is done by the sectorID
   * and therefore we do not use the overloading comparison operator to be sure that everything works as expected
   */
//   sort(v.begin(), v.end(),
//      [](const pair<int, int>& lhs, const pair<int, int>& rhs) -> bool {
//              if (lhs.second == 0)
//                  return true;
//              return lhs.second < rhs.second; } );
}
