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
#include "tracking/trackFindingVXD/sectorMapTools/TrainerConfigData.h"
#include "framework/gearbox/Const.h"
#include "framework/datastore/StoreObjPtr.h"

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
  StoreObjPtr< SectorMap > sectorMap("", DataStore::c_Persistent);
  sectorMap.registerInDataStore(DataStore::c_DontWriteOut);
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
  /// TODO nice interface for creating TrainerConfigData:
  TrainerConfigData config1;
  config1.pTCuts = {0.02, 0.08};
  config1.pTSmear = 0.;
  config1.minMaxLayer = {3, 6};
  config1.uDirectionCuts = {0., .15, .5, .85, 1.};
  config1.vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};
  config1.pdgCodesAllowed = {};
  config1.seedMaxDist2IPXY = 23.5;
  config1.seedMaxDist2IPZ = 23.5;
  config1.nHitsMin = 3;
  config1.vIP = B2Vector3D(0, 0, 0);
  config1.secMapName = "lowTestRedesign";
  config1.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "SlopeRZ", "CircleDist2IPHighOccupancy"};
  config1.threeHitFilters = { "Angle3DSimple", "DeltaSlopeRZ"};
  config1.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config1.mField = 1.5;
  config1.rarenessThreshold = 0.001;
  config1.quantiles = {0.005, 0.005};
  bootstrapSectorMap(config1);


  TrainerConfigData config2;
  config2.pTCuts = {0.075, 0.300};
  config2.pTSmear = 0.;
  config2.minMaxLayer = {3, 6};
  config2.uDirectionCuts = {0., .15, .5, .85, 1.};
  config2.vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};
  config2.pdgCodesAllowed = {};
  config2.seedMaxDist2IPXY = 23.5;
  config2.seedMaxDist2IPZ = 23.5;
  config2.nHitsMin = 3;
  config2.vIP = B2Vector3D(0, 0, 0);
  config2.secMapName = "medTestRedesign";
  config2.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "SlopeRZ", "CircleDist2IPHighOccupancy"};
  config2.threeHitFilters = { "Angle3DSimple", "DeltaSlopeRZ"};
  config2.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config2.mField = 1.5;
  config2.rarenessThreshold = 0.001;
  config2.quantiles = {0.005, 0.005};
  bootstrapSectorMap(config2);

  TrainerConfigData config3;
  config3.pTCuts = {0.290, 3.5};
  config3.pTSmear = 0.;
  config3.minMaxLayer = {3, 6};
  config3.uDirectionCuts = {0., .15, .5, .85, 1.};
  config3.vDirectionCuts = {0., .1, .3, .5, .7, .9, 1.};
  config3.pdgCodesAllowed = {};
  config3.seedMaxDist2IPXY = 23.5;
  config3.seedMaxDist2IPZ = 23.5;
  config3.nHitsMin = 3;
  config3.vIP = B2Vector3D(0, 0, 0);
  config3.secMapName = "highTestRedesign";
  config3.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "SlopeRZ"};
  config3.threeHitFilters = { "Angle3DSimple", "DeltaCircleRadiusHighOccupancy"};
  config3.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config3.mField = 1.5;
  config3.rarenessThreshold = 0.001;
  config3.quantiles = {0.005, 0.005};
  bootstrapSectorMap(config3);

}

void
SectorMapBootstrapModule::bootstrapSectorMap(const TrainerConfigData& config)
{

  StoreObjPtr< SectorMap > newSectorMap("", DataStore::c_Persistent);
  VXDTFFilters* segmentFilters = new VXDTFFilters();
  segmentFilters->setConfig(config);

  CompactSecIDs compactSecIds;
  vector<int> layers  = { 1, 2, 3, 4, 5, 6};
  vector<int> ladders = { 8, 12, 7, 10, 12, 16};
  vector<int> sensors = { 2, 2, 2, 3, 4, 5};

  vector< float > uSup;
  uSup.resize(config.uDirectionCuts.size() - 2);
  for (unsigned int i = 1; i < config.uDirectionCuts.size() ; i++)
    uSup[i - 1] = config.uDirectionCuts[i];

  vector< float > vSup;
  vSup.resize(config.vDirectionCuts.size() - 2);
  for (unsigned int i = 1; i < config.vDirectionCuts.size() ; i++)
    vSup[i - 1] = config.uDirectionCuts[i];

  vector< vector< FullSecID > > sectors;

  sectors.resize(uSup.size() + 1);


  for (auto layer : layers)
    for (int ladder = 1 ; ladder <= ladders[layer - 1] ; ladder++)
      for (int sensor = 1 ; sensor <=  sensors[layer - 1] ; sensor++) {
        int counter = 0;
        for (unsigned int i = 0; i < uSup.size() + 1; i++) {
          sectors[i].resize(vSup.size() + 1);
          for (unsigned int j = 0; j < vSup.size() + 1 ; j++) {
            sectors[i][j] = FullSecID(VxdID(layer, ladder , sensor),
                                      false, counter);
            counter ++;
          }
        }
        segmentFilters->addSectorsOnSensor(uSup , vSup, sectors) ;
      }

  newSectorMap->assignFilters(config.secMapName, segmentFilters);

}



void
SectorMapBootstrapModule::endRun()
{

}

