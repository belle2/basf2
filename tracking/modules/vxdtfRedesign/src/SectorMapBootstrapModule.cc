/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#include <iostream>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance1DZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance3DNormed.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/SlopeRZ.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance1DZSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance2DXYSquared.h>
#include <tracking/trackFindingVXD/sectorMap/twoHitVariables/Distance3DSquared.h>


#include <tracking/trackFindingVXD/sectorMap/filterFramework/Shortcuts.h>
#include <tracking/trackFindingVXD/sectorMap/filterFramework/Observer.h>

#include <tracking/trackFindingVXD/sectorMap/map/SectorMap.h>
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include "tracking/modules/vxdtfRedesign/SectorMapBootstrapModule.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/dataobjects/VXDTFSecMap.h"
#include "tracking/dataobjects/FilterID.h"
#include "tracking/dataobjects/SectorMapConfig.h"
#include "framework/gearbox/Const.h"
#include "framework/datastore/StoreObjPtr.h"

#include <tracking/spacePointCreation/SpacePoint.h>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include <math.h>
#include <algorithm>


using namespace Belle2;
using namespace std;

REG_MODULE(SectorMapBootstrap);

SectorMapBootstrapModule::SectorMapBootstrapModule() : Module()
{
  setDescription("Create the VXDTF SectorMap for the following modules."
                );
}

void
SectorMapBootstrapModule::initialize()
{
  StoreObjPtr< SectorMap<SpacePoint> > sectorMap("", DataStore::c_Persistent);
  sectorMap.registerInDataStore(DataStore::c_DontWriteOut);
  sectorMap.create();
  bootstrapSectorMap();
  //Thomas : is that file used for observer only! Shouldnt it be then optional?
  m_tfile = new TFile("observeTheSecMap.root", "RECREATE");
  m_tfile->cd();
  TTree* newTree = new TTree("twoHitTree", "reallyWeWantToHaveThatTTreeNow");

  // take care of two-hit-filters:
//   auto outerHit = new SpacePoint();
//   auto innerHit = new SpacePoint();

  VXDTFFilters<SpacePoint>::twoHitFilter_t aFilter;
  initializeObservers(aFilter, newTree/*, outerHit, innerHit*/);
//   ObserverCheckMCPurity::initialize< CircleRadius<SpacePoint>, ClosedRange<double, double>>(CircleRadius<SpacePoint>(), ClosedRange<double, double>());
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


  // TODO: Most of these informations are not used at all.
  //        It seems to me (EP) that only the SectorDividers are used.

  SectorMapConfig config1;
//   config1.pTmin = 0.02;
//   config1.pTmax = 0.08;
  config1.pTmin = 0.02; // minimal relevant version
//   config1.pTmax = 0.15; // minimal relevant version
  config1.pTmax = 3.15; // minimal relevant version // Feb18-onePass-Test
  config1.pTSmear = 0.;
  config1.allowedLayers = {0, 3, 4, 5, 6};
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
  config1.twoHitFilters = { "Distance3DSquared", "Distance2DXYSquared", "Distance1DZ", "SlopeRZ", "Distance3DNormed"};
  config1.threeHitFilters = { "Angle3DSimple", "CosAngleXY", "AngleRZSimple", "CircleDist2IP", "DeltaSlopeRZ", "DeltaSlopeZoverS", "DeltaSoverZ", "HelixParameterFit", "Pt", "CircleRadius"};
  config1.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config1.mField = 1.5;
  config1.rarenessThreshold = 0.; //0.001;
  config1.quantiles = {0., 1.};  //{0.005, 1. - 0.005};
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


  SectorMapConfig config4;

  config4.pTmin = 0.290;
  config4.pTmax = 3.5;
  config4.pTSmear = 0.;
  config4.allowedLayers = {0, 3, 4, 5, 6};
  config4.uSectorDivider = {};
  config4.vSectorDivider = {};
  config4.pdgCodesAllowed = {};
  config4.seedMaxDist2IPXY = 23.5;
  config4.seedMaxDist2IPZ = 23.5;
  config4.nHitsMin = 3;
  config4.vIP = B2Vector3D(0, 0, 0);

  config4.secMapName = "STRESS";

  config4.twoHitFilters = { "Distance3DSquared"/*, "Distance2DXYSquared", "SlopeRZ"*/};
  config4.threeHitFilters = { "Angle3DSimple"/*, "DeltaCircleRadiusHighOccupancy"*/};
  config4.fourHitFilters = { "DeltaDistCircleCenter", "DeltaCircleRadius"};
  config4.mField = 1.5;
  config4.rarenessThreshold = 0.001;
  config4.quantiles = {0.005, 1. - 0.005};


  for (double stress = .1; stress < 1.; stress += .1) {
    config4.uSectorDivider.push_back(stress);
    config4.vSectorDivider.push_back(stress);
  }

  bootstrapSectorMap(config4);
}

void
SectorMapBootstrapModule::endRun()
{
  persistSectorMap();
}

void
SectorMapBootstrapModule::bootstrapSectorMap(const SectorMapConfig& config)
{

  StoreObjPtr< SectorMap<SpacePoint> > newSectorMap("", DataStore::c_Persistent);
  VXDTFFilters<SpacePoint>* segmentFilters = new VXDTFFilters<SpacePoint>();
  segmentFilters->setConfig(config);

  // TO DO: All these informations must be retrieved from the geometry
  CompactSecIDs compactSecIds;
  vector<int> layers  = { 1, 2, 3, 4, 5, 6};
  vector<int> ladders = { 8, 12, 7, 10, 12, 16};
  vector<int> sensors = { 2, 2, 2, 3, 4, 5};

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
        segmentFilters->addSectorsOnSensor(uDividersMinusLastOne ,
                                           vDividersMinusLastOne,
                                           sectors) ;
      }
    }

  newSectorMap->assignFilters(config.secMapName, segmentFilters);

}


/// Persist the whole sector map on a root file
void
SectorMapBootstrapModule::persistSectorMap(void)
{

  StoreObjPtr< SectorMap<SpacePoint> > theSectorMap("", DataStore::c_Persistent);
  const char* rootFileName = "testSectorMap.root";
  TFile rootFile(rootFileName , "RECREATE");

  TTree* tree = new TTree(c_setupKeyNameTTreeName.c_str(),
                          c_setupKeyNameTTreeName.c_str());

  TString setupKeyName;

  tree->Branch(c_setupKeyNameBranchName.c_str(),
               & setupKeyName);

  auto allSetupsFilters = theSectorMap->getAllSetups();
  for (auto singleSetupFilters : allSetupsFilters) {

    setupKeyName = TString(singleSetupFilters.first.c_str());

    tree->Fill();

    rootFile.mkdir(setupKeyName, setupKeyName);
    rootFile.cd(setupKeyName);

    singleSetupFilters.second->persistOnRootFile();

    rootFile.cd("..");

  }

  rootFile.Write();
  rootFile.Close();

  retrieveSectorMap();
}


/// Persist the whole sector map on a root file
void
SectorMapBootstrapModule::retrieveSectorMap(void)
{

  StoreObjPtr< SectorMap<SpacePoint> > theSectorMap("", DataStore::c_Persistent);
  const char* rootFileName = "testSectorMap.root";
  TFile rootFile(rootFileName);

  TTree* tree ;
  rootFile.GetObject(c_setupKeyNameTTreeName.c_str(), tree);

  TString* setupKeyName = NULL;
  tree->SetBranchAddress(c_setupKeyNameBranchName.c_str(),
                         & setupKeyName);

  auto nEntries = tree->GetEntriesFast();
  for (int i = 0;  i < nEntries ; i++) {
    tree->GetEntry(i);
    rootFile.cd(setupKeyName->Data());

    VXDTFFilters<SpacePoint>* segmentFilters = new VXDTFFilters<SpacePoint>();
    segmentFilters->retrieveFromRootFile();

    string setupKeyNameStd = string(setupKeyName->Data());

    theSectorMap->assignFilters(setupKeyNameStd, segmentFilters);

    rootFile.cd("..");

    setupKeyName->Clear();

  }


  rootFile.Close();


}


void
SectorMapBootstrapModule::terminate()
{
  m_tfile->Write();
  m_tfile->Close();
  delete m_tfile;
}

