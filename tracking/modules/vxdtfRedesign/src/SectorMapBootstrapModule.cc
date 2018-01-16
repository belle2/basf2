/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni, Thomas Lueck                                  *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#include <iostream>

#include <tracking/trackFindingVXD/filterMap/map/FiltersContainer.h>
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include "tracking/modules/vxdtfRedesign/SectorMapBootstrapModule.h"
#include "tracking/vxdCaTracking/PassData.h"
#include "tracking/dataobjects/VXDTFSecMap.h"
#include "tracking/dataobjects/FilterID.h"
#include "tracking/dataobjects/SectorMapConfig.h"
#include <tracking/spacePointCreation/SpacePoint.h>

#include "framework/gearbox/Const.h"
#include "framework/datastore/StoreObjPtr.h"

// needed for complicated parameter types to not get an undefined reference error
#include <framework/core/ModuleParam.templateDetails.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>

#include <math.h>
#include <algorithm>
#include <fstream>


using namespace Belle2;
using namespace std;

REG_MODULE(SectorMapBootstrap);

SectorMapBootstrapModule::SectorMapBootstrapModule() : Module()
{

  setPropertyFlags(c_ParallelProcessingCertified);

  setDescription("Create the VXDTF SectorMap for the following modules."
                );

  addParam("SectorMapsInputFile", m_sectorMapsInputFile,
           "File from which the SectorMaps will be retrieved if\
 ReadSectorMap is set to true", m_sectorMapsInputFile);

  addParam("SectorMapsOutputFile", m_sectorMapsOutputFile,
           "File into which the SectorMaps will be written if\
 WriteSectorMap is set to true", m_sectorMapsOutputFile);

  addParam("ReadSectorMap", m_readSectorMap, "If set to true \
retrieve the SectorMaps from SectorMapsInputFile during initialize.", m_readSectorMap);

  addParam("WriteSectorMap", m_writeSectorMap, "If set to true \
at endRun write the SectorMaps to SectorMapsOutputFile.", m_writeSectorMap);

  addParam("SetupToRead", m_setupToRead, "If non empty only the setup with the given name will be read"
           " from the from the root file. All other will be ignored. If empty \"\" (default) all setups are read. Will "
           "only used if sectormap is retrieved from root file. Case will be ignored!",
           std::string(""));

  addParam("ReadSecMapFromDB", m_readSecMapFromDB, "If set to true the sector map will be read from the Data Base. NOTE: this will "
           "override the parameter ReadSectorMap (reading sector map from file)!!!", m_readSecMapFromDB);


  // dummy vector needed to get the current structure of the filter
  std::vector< std::pair<char, void*> > dummyVector = {};

  VXDTFFilters<SpacePoint>::twoHitFilter_t empty2HitFilter;
  // the structure is the same for all specializations of the template
  std::string structure2HitFilter = empty2HitFilter.getNameAndReference(&dummyVector);
  dummyVector.clear();
  addParam("twoHitFilterAdjustFunctions", m_twoHitFilterAdjustFunctions,
           "Vector of vectors containing expressions used to "
           "alter the 2-hit filters. The inner vector should contain exactly two strings. The first entry is interpreted as index (integer). "
           "The second entry is interpreted as function used to create a TF1. The variable to be altered will be assumed to be called \"x\" "
           "and in addition one can use \"[0]\" can be used which will be interpreted as FullSecID of the static sector the filter is attached to. "
           "No other parameter is allowed. The structure of the 2-hit filter is as follows:       " + structure2HitFilter +
           "    Example: [(1, \"12\"), (3, \"sin(x)\"), (4, \"x + [0]\")]    PS: use this feature only if you know what you are doing!",
           m_twoHitFilterAdjustFunctions);

  VXDTFFilters<SpacePoint>::threeHitFilter_t empty3HitFilter;
  // the structure is the same for all specializations of the template
  std::string structure3HitFilter = empty3HitFilter.getNameAndReference(&dummyVector);
  dummyVector.clear();
  addParam("threeHitFilterAdjustFunctions", m_threeHitFilterAdjustFunctions,
           "Vector of vectors containing expressions used to "
           "alter the 3-hit filters. The inner vector should contain exactly two strings. The first entry is interpreted as index (integer). "
           "The second entry is interpreted as function used to create a TF1. The variable to be altered will be assumed to be called \"x\" "
           "and in addition \"[0]\" can be used which will be interpreted as FullSecID of the static sector the filter is attached to. No other "
           "parameter is allowd. The structure of the 2-hit filter is as follows:     " + structure3HitFilter +
           "    Example: [(1, \"12\"), (3, \"sin(x)\"), (4, \"x + [0]\")]    PS: use this feature only if you know what you are doing!",
           m_twoHitFilterAdjustFunctions);
}

void
SectorMapBootstrapModule::initialize()
{

  // in case sector map is read from the DB one needs to set the DB pointer
  if (m_readSecMapFromDB) {
    B2DEBUG(1, "SectorMapBootstrapModule: Retrieving sectormap from DB. Filename: " << m_sectorMapsInputFile.c_str());
    m_ptrDBObjPtr = new DBObjPtr<PayloadFile>(m_sectorMapsInputFile.c_str());
    if (m_ptrDBObjPtr == nullptr) B2FATAL("SectorMapBootstrapModule: the DBObjPtr is not initialized");
    // add a callback function so that the sectormap is updated each time the DBObj changes
    m_ptrDBObjPtr->addCallback(this,  &SectorMapBootstrapModule::retrieveSectorMap);
  }
  // retrieve the SectorMap or create an empty one
  if (m_readSectorMap || m_readSecMapFromDB)
    retrieveSectorMap();
  else
    bootstrapSectorMap();

  // security measurement: test if output file exists so that existing sector maps are not overwritten
  if (m_writeSectorMap) {
    if (std::ifstream(m_sectorMapsOutputFile.c_str())) {
      B2FATAL("Detected existing output file! Please delete or move before proceeding! File name: " << m_sectorMapsOutputFile);
    } else {
      B2DEBUG(1, "Checked that output file does not exist!");
    }
  }



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

  // TODO: find a better way to put the configs into the framework

  // WARNING: chose the names of the configs in that way that they are not contained in each other!
  //       E.g. having two configs with names "BobTheGreat" and "Bob" is not allowed as it will cause problems in some modules!


  // for now declare this as default config for SVD only tracking!
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
  config1.secMapName = "SVDOnlyDefault"; // has been: "lowTestRedesign";
  config1.mField = 1.5;
  config1.rarenessThreshold = 0.; //0.001;
  config1.quantiles = {0., 1.};  //{0.005, 1. - 0.005};
  // TODO: still missing: minimal sample-size, quantiles for smaller samplesizes, threshshold small <-> big sampleSize.
  bootstrapSectorMap(config1);


  // same as config1 but allows the PXD layers
  // default for VXD tracking (SVD+PXD)
  SectorMapConfig config1point1;
  config1point1.pTmin = 0.02; // minimal relevant version
  config1point1.pTmax = 3.15; // minimal relevant version // Feb18-onePass-Test
  config1point1.pTSmear = 0.;
  config1point1.allowedLayers = {0, 1, 2, 3, 4, 5, 6};
  config1point1.uSectorDivider = { .3, .7, 1.}; // standard relevant version
  config1point1.vSectorDivider = { .3, .7, 1.}; // standard relevant version
  config1point1.pdgCodesAllowed = {};
  config1point1.seedMaxDist2IPXY = 23.5;
  config1point1.seedMaxDist2IPZ = 23.5;
  config1point1.nHitsMin = 3;
  config1point1.vIP = B2Vector3D(0, 0, 0);
  config1point1.secMapName = "SVDPXDDefault"; // has been: "lowTestSVDPXD";
  config1point1.mField = 1.5;
  config1point1.rarenessThreshold = 0.; //0.001;
  config1point1.quantiles = {0., 1.};  //{0.005, 1. - 0.005};
  bootstrapSectorMap(config1point1);

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
  config4.mField = 1.5;
  config4.rarenessThreshold = 0.001;
  config4.quantiles = {0.005, 1. - 0.005};


  for (double stress = .1; stress < 1.; stress += .1) {
    config4.uSectorDivider.push_back(stress);
    config4.vSectorDivider.push_back(stress);
  }

  bootstrapSectorMap(config4);


  SectorMapConfig configTB;
  configTB.pTmin = 1.0; // minimal relevant version
  configTB.pTmax = 8.0; // minimal relevant version // Feb18-onePass-Test
  configTB.pTSmear = 0.;
  configTB.allowedLayers = {0, 3, 4, 5, 6};
  configTB.uSectorDivider = { 1.}; // standard relevant version
  configTB.vSectorDivider = { 1.}; // standard relevant version
  configTB.pdgCodesAllowed = { -11, 11};
  configTB.seedMaxDist2IPXY = 23.5;
  configTB.seedMaxDist2IPZ = 23.5;
  configTB.nHitsMin = 3;
  configTB.vIP = B2Vector3D(-100, 0, 0); // should be the same as for the MC generation!
  configTB.secMapName = "testbeamTEST";
  configTB.mField = 1.;
  configTB.rarenessThreshold = 0.; //0.001;
  configTB.quantiles = {0., 1.};  //{0.005, 1. - 0.005};
  // TODO: still missing: minimal sample-size, quantiles for smaller samplesizes, threshshold small <-> big sampleSize.
  bootstrapSectorMap(configTB);



}

void
SectorMapBootstrapModule::endRun()
{
  if (m_writeSectorMap)
    persistSectorMap();
}

void
SectorMapBootstrapModule::bootstrapSectorMap(const SectorMapConfig& config)
{

  VXDTFFilters<SpacePoint>* segmentFilters = new VXDTFFilters<SpacePoint>();
  segmentFilters->setConfig(config);

  // TO DO: All these informations must be retrieved from the geometry
  CompactSecIDs compactSecIds;

  vector< double > uDividersMinusLastOne = config.uSectorDivider;
  uDividersMinusLastOne.pop_back();
  vector< double > vDividersMinusLastOne = config.vSectorDivider;
  vDividersMinusLastOne.pop_back();


  vector< vector< FullSecID > > sectors;

//   sectors.resize(uSup.size() + 1);
  sectors.resize(config.uSectorDivider.size());
  unsigned nSectorsInU = config.uSectorDivider.size(),
           nSectorsInV = config.vSectorDivider.size();

  // retrieve the full list of sensors from the geometry
  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  std::vector<VxdID> listOfSensors = geometry.getListOfSensors();
  for (VxdID aSensorId : listOfSensors) {

    // filter only those sensors on layers which are specified in the config
    if (std::find(config.allowedLayers.begin(), config.allowedLayers.end(),
                  aSensorId.getLayerNumber()) == config.allowedLayers.end()) continue;

    // for testbeams there might be other sensors in the geometry so filter for SVD and PXD only, as the CompactSecID dont like those!
    VXD::SensorInfoBase::SensorType type = geometry.getSensorInfo(aSensorId).getType();
    if (type != VXD::SensorInfoBase::SVD && type != VXD::SensorInfoBase::PXD) {
      B2WARNING("Found sensor which is not PXD or SVD with VxdID: " << aSensorId << " ! Will skip that sensor ");
      continue;
    }

    int counter = 0;
    for (unsigned int i = 0; i < nSectorsInU; i++) {
      sectors.at(i).resize(nSectorsInV);
      for (unsigned int j = 0; j < nSectorsInV ; j++) {
        sectors.at(i).at(j) = FullSecID(aSensorId, false, counter);
        counter ++;
      }
    }
    segmentFilters->addSectorsOnSensor(uDividersMinusLastOne ,
                                       vDividersMinusLastOne,
                                       sectors) ;
  }//end loop over sensors


  // if layer 0 is specified in the config then the virtual IP is added
  if (std::find(config.allowedLayers.begin(), config.allowedLayers.end(), 0) != config.allowedLayers.end()) {
    std::vector<double> uCuts4vIP = {}, vCuts4vIP = {};
    sectors.clear();
    sectors = {{0}};
    segmentFilters->addSectorsOnSensor(uCuts4vIP, vCuts4vIP, sectors);
  }

  // put config into the container
  FiltersContainer<SpacePoint>::getInstance().assignFilters(config.secMapName, segmentFilters);

}


/// Persist the whole sector map on a root file
void
SectorMapBootstrapModule::persistSectorMap(void)
{

  // the "CREATE" option results in the root file not being opened if it already exists (to prevent overwriting existing sectormaps)
  TFile rootFile(m_sectorMapsOutputFile.c_str() , "CREATE");
  if (!rootFile.IsOpen()) B2FATAL("Unable to open rootfile! This could be caused by an already existing file of the same name: "
                                    << m_sectorMapsOutputFile.c_str());

  TTree* tree = new TTree(c_setupKeyNameTTreeName.c_str(),
                          c_setupKeyNameTTreeName.c_str());

  TString setupKeyName;

  tree->Branch(c_setupKeyNameBranchName.c_str(),
               & setupKeyName);

  auto allSetupsFilters = FiltersContainer<SpacePoint>::getInstance().getAllSetups();
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


}


/// Retrieve the whole sector map from a root file
void
SectorMapBootstrapModule::retrieveSectorMap(void)
{

  std::string rootFileName = m_sectorMapsInputFile;
  // if reading from the DB get the root file name from the DB object ptr
  if (m_readSecMapFromDB) {
    if (m_ptrDBObjPtr == nullptr) B2FATAL("SectorMapBootstrapModule: the pointer to the DB payload is not set!");
    if (!(*m_ptrDBObjPtr).isValid()) B2FATAL("SectorMapBootstrapModule the DB object is not valid!");

    rootFileName = (*m_ptrDBObjPtr)->getFileName();
  }

  B2DEBUG(1, "SectorMapBootstrapModule: retrieving new SectorMap. New file name: " << rootFileName);
  TFile rootFile(rootFileName.c_str());

  // some cross check that the file is open
  if (!rootFile.IsOpen()) B2FATAL("The root file: " << rootFileName << " was not found.");

  TTree* tree = nullptr;
  rootFile.GetObject(c_setupKeyNameTTreeName.c_str(), tree);
  if (tree == nullptr) B2FATAL("SectorMapBootstrapModule: tree not found! tree name: " << c_setupKeyNameTTreeName.c_str());

  TString* setupKeyName = nullptr;
  tree->SetBranchAddress(c_setupKeyNameBranchName.c_str(),
                         & setupKeyName);
  if (setupKeyName == nullptr) B2FATAL("SectorMapBootstrapModule: setupKeyName not found");

  // ignore case, so only upper case
  TString setupToRead_upper = m_setupToRead;
  setupToRead_upper.ToUpper();

  // to monitor if anything was read from the root files
  bool read_something = false;

  FiltersContainer<SpacePoint>& filtersContainer = FiltersContainer<SpacePoint>::getInstance();
  auto nEntries = tree->GetEntriesFast();
  for (int i = 0;  i < nEntries ; i++) {
    tree->GetEntry(i);

    // if a setup name is specified only read that one
    if (setupToRead_upper != "") {
      TString buff = setupKeyName->Data();
      buff.ToUpper();
      if (buff != setupToRead_upper) continue;
    }

    rootFile.cd(setupKeyName->Data());

    B2DEBUG(1, "Retrieving SectorMap with name " << setupKeyName->Data());

    VXDTFFilters<SpacePoint>* segmentFilters = new VXDTFFilters<SpacePoint>();

    string setupKeyNameStd = string(setupKeyName->Data());
    segmentFilters->retrieveFromRootFile(setupKeyName);

    // if the m_twoHitFilterAdjustFunctions m_threeHitFilterAdjustFunctions are non empty filters will be altered
    if (m_twoHitFilterAdjustFunctions.size() > 0) {
      B2WARNING("The 2-hit filters will be altered from the default!");
      B2INFO("The following set of indizes and functions will be used to alter the 2-hit filters:");
      for (auto& entry : m_twoHitFilterAdjustFunctions) {
        B2INFO("index=" << std::get<0>(entry) << " function=" << std::get<1>(entry));
      }
      segmentFilters->modify2SPFilters(m_twoHitFilterAdjustFunctions);
    }
    if (m_threeHitFilterAdjustFunctions.size() > 0) {
      B2WARNING("The 3-hit filters will be altered from the default!");
      B2INFO("The following set of indizes and functions will be used to alter the 3-hit filters:");
      for (auto& entry : m_threeHitFilterAdjustFunctions) {
        B2INFO("index=" << std::get<0>(entry) << " function=" << std::get<1>(entry));
      }
      segmentFilters->modify3SPFilters(m_threeHitFilterAdjustFunctions);
    }

    // locks all functions that can modify the filters
    segmentFilters->lockFilters();

    B2DEBUG(1, "Retrieved map with name: " << setupKeyNameStd << " from rootfie.");
    filtersContainer.assignFilters(setupKeyNameStd, segmentFilters);

    rootFile.cd("..");

    setupKeyName->Clear();

    read_something = true;
  }

  if (!read_something) B2FATAL("SectorMapBootstrapModule: No setup was read from the root file! " <<
                                 "The requested setup name was: " << m_setupToRead);

  rootFile.Close();

  // delete the TString which was allocated by ROOT but not cleaned up
  if (setupKeyName != nullptr) {
    delete setupKeyName;
  }

}


