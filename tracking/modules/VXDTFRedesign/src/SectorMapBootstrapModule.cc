/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZTemp.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DNormed.h"
#include "tracking/trackFindingVXD/TwoHitFilters/SlopeRZ.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance1DZSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance2DXYSquared.h"
#include "tracking/trackFindingVXD/TwoHitFilters/Distance3DSquared.h"


#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"
#include "tracking/trackFindingVXD/FilterTools/Observer.h"

#include "tracking/trackFindingVXD/sectorMapTools/SectorMap.h"
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include "tracking/modules/VXDTFRedesign/SectorMapBootstrapModule.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/dataobjects/VXDTFSecMap.h"
#include "tracking/dataobjects/FilterID.h"
#include "tracking/dataobjects/SectorMapConfig.h"
#include "framework/gearbox/Const.h"
#include "framework/datastore/StoreObjPtr.h"

#include <tracking/spacePointCreation/SpacePoint.h>

#include <TFile.h>
#include <TTree.h>

#include <math.h>
#include <algorithm>

using namespace Belle2;
using namespace std;

REG_MODULE(SectorMapBootstrap);

SectorMapBootstrapModule::SectorMapBootstrapModule() : Module()
{
  setDescription("Create an empty sector map to be trained by the following module"
                );
}

void
SectorMapBootstrapModule::initialize()
{
  StoreObjPtr< SectorMap<SpacePoint> > sectorMap("", DataStore::c_Persistent);
  sectorMap.registerInDataStore(DataStore::c_DontWriteOut);
  sectorMap.create();
  bootstrapSectorMap();

}

void
SectorMapBootstrapModule::beginRun()
{


}


void
SectorMapBootstrapModule::event()
{
}

void
SectorMapBootstrapModule::bootstrapSectorMap(void)
{
  /// TODO nice interface for creating SectorMapConfig:
  SectorMapConfig config1;
//   config1.pTmin = 0.02;
//   config1.pTmax = 0.08;
  config1.pTmin = 0.02; // minimal relevant version
  config1.pTmax = 0.15; // minimal relevant version
  config1.pTSmear = 0.;
  config1.allowedLayers = {0, 3, 4, 5, 6}; // TODO -> convert to vector containing all layerNumbers to be used; e.g.: {0, 3, 4, 5, 6};
//   config1.uSectorDivider = { .15, .5, .85, 1.};
//   config1.vSectorDivider = { .1, .3, .5, .7, .9, 1.};
  config1.uSectorDivider = { .3, .7, 1.}; // standard relevant version
  config1.vSectorDivider = { .3, .7, 1.}; // standard relevant version
//   config1.uSectorDivider = { .5, 1.}; // small relevant version
//   config1.vSectorDivider = { .5, 1.}; // small relevant version
//   config1.uSectorDivider = { 1.}; // minimal relevant version
//   config1.vSectorDivider = { 1.}; // minimal relevant version
  config1.pdgCodesAllowed = {};
  config1.seedMaxDist2IPXY = 23.5;
  config1.seedMaxDist2IPZ = 23.5;
  config1.nHitsMin = 3;
  config1.vIP = B2Vector3D(0, 0, 0);
  config1.secMapName = "lowTestRedesign";
  config1.twoHitFilters = { "Distance3DSquared"/*, "Distance2DXYSquared", "SlopeRZ", "CircleDist2IPHighOccupancy"*/};
  config1.threeHitFilters = { "Angle3DSimple"/*, "DeltaSlopeRZ"*/};
  config1.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config1.mField = 1.5;
  config1.rarenessThreshold = 0.001;
  config1.quantiles = {0.005, 1. - 0.005};
  // TODO: still missing: minimal sample-size, quantiles for smaller samplesizes, threshshold small <-> big sampleSize.
  bootstrapSectorMap(config1);


  SectorMapConfig config2;
//   config2.pTCuts = {0.075, 0.300};
  config2.pTmin = 0.075;
  config2.pTmax = 0.300;
  config2.pTSmear = 0.;
  config2.allowedLayers = {0, 3, 4, 5, 6};
  config2.uSectorDivider = { .15, .5, .85, 1.};
  config2.vSectorDivider = { .1, .3, .5, .7, .9, 1.};
  config2.pdgCodesAllowed = {};
  config2.seedMaxDist2IPXY = 23.5;
  config2.seedMaxDist2IPZ = 23.5;
  config2.nHitsMin = 3;
  config2.vIP = B2Vector3D(0, 0, 0);
  config2.secMapName = "medTestRedesign";
  config2.twoHitFilters = { "Distance3DSquared"/*, "Distance2DXYSquared", "SlopeRZ", "CircleDist2IPHighOccupancy"*/};
  config2.threeHitFilters = { "Angle3DSimple"/*, "DeltaSlopeRZ"*/};
  config2.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config2.mField = 1.5;
  config2.rarenessThreshold = 0.001;
  config2.quantiles = {0.005, 1. - 0.005};
  bootstrapSectorMap(config2);

  SectorMapConfig config3;
//   config3.pTCuts = {0.290, 3.5};
  config3.pTmin = 0.290;
  config3.pTmax = 3.5;
  config3.pTSmear = 0.;
  config3.allowedLayers = {0, 3, 4, 5, 6};
  config3.uSectorDivider = { .15, .5, .85, 1.};
  config3.vSectorDivider = { .1, .3, .5, .7, .9, 1.};
  config3.pdgCodesAllowed = {};
  config3.seedMaxDist2IPXY = 23.5;
  config3.seedMaxDist2IPZ = 23.5;
  config3.nHitsMin = 3;
  config3.vIP = B2Vector3D(0, 0, 0);
  config3.secMapName = "highTestRedesign";
  config3.twoHitFilters = { "Distance3DSquared"/*, "Distance2DXYSquared", "SlopeRZ"*/};
  config3.threeHitFilters = { "Angle3DSimple"/*, "DeltaCircleRadiusHighOccupancy"*/};
  config3.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config3.mField = 1.5;
  config3.rarenessThreshold = 0.001;
  config3.quantiles = {0.005, 1. - 0.005};
  bootstrapSectorMap(config3);

}

void
SectorMapBootstrapModule::bootstrapSectorMap(const SectorMapConfig& config)
{

  StoreObjPtr< SectorMap<SpacePoint> > newSectorMap("", DataStore::c_Persistent);
  VXDTFFilters<SpacePoint>* segmentFilters = new VXDTFFilters<SpacePoint>();
  segmentFilters->setConfig(config);

  CompactSecIDs compactSecIds;
  vector<int> layers  = { 1, 2, 3, 4, 5, 6};
  vector<int> ladders = { 8, 12, 7, 10, 12, 16};
  vector<int> sensors = { 2, 2, 2, 3, 4, 5};


//   vector< double > uSup;
// //   uSup.resize(config.uDirectionCuts.size() - 2);
//   for (unsigned int i = 1; i < config.uDirectionCuts.size() ; i++)
//  { uSup.push_back(config.uDirectionCuts.at(i)); }
// //   { uSup.at(i - 1) = config.uDirectionCuts.at(i); }
//
//   vector< double > vSup;
//
// //   vSup.resize(config.vDirectionCuts.size() - 2);
//   for (unsigned int i = 1; i < config.vDirectionCuts.size() ; i++)
//  { vSup.push_back(config.vDirectionCuts.at(i)); }
// //   { vSup.at(i - 1) = config.vDirectionCuts.at(i); }


  // Jakob: temporal solution, I propose to fix addSectorsOnSensor instead (explanations: see tracking/dataobjects/sectorMapConfig.h for more details).
  vector< double > uDividersMinusLastOne = config.uSectorDivider;
  uDividersMinusLastOne.pop_back();
  vector< double > vDividersMinusLastOne = config.vSectorDivider;
  vDividersMinusLastOne.pop_back();


  vector< vector< FullSecID > > sectors;

//   sectors.resize(uSup.size() + 1);
  sectors.resize(config.uSectorDivider.size());
  unsigned nSectorsInU = config.uSectorDivider.size(),
           nSectorsInV = config.vSectorDivider.size();


  for (auto layer : layers)
    for (int ladder = 1 ; ladder <= ladders.at(layer - 1) ; ladder++) {
      for (int sensor = 1 ; sensor <=  sensors.at(layer - 1) ; sensor++) {
        int counter = 0;
        for (unsigned int i = 0; i < nSectorsInU; i++) {
          sectors.at(i).resize(nSectorsInV);
          for (unsigned int j = 0; j < nSectorsInV ; j++) {
            sectors.at(i).at(j) = FullSecID(VxdID(layer, ladder , sensor),
                                            false, counter);
            counter ++;
          }
        }
        segmentFilters->addSectorsOnSensor(uDividersMinusLastOne , vDividersMinusLastOne, sectors) ;
      }
    }

  newSectorMap->assignFilters(config.secMapName, segmentFilters);

}



void
SectorMapBootstrapModule::endRun()
{

}
