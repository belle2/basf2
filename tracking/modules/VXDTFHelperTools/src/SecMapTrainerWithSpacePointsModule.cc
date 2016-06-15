/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <tracking/modules/VXDTFHelperTools/SecMapTrainerWithSpacePointsModule.h>

#include <tracking/dataobjects/SecMapVector.h> // needed for rootExport
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/trackFindingVXD/sectorMapTools/SectorTools.h>


#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Const.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include "tracking/vxdCaTracking/SharedFunctions.h"

#include <TObject.h>

// #include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <math.h>
#include <TRandom.h>
using namespace std;
using namespace Belle2;
using namespace SecMapTrainerWithSPNames;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SecMapTrainerWithSpacePoints)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


/// /// /// /// /// /// /// /// CONSTRUCTOR /// /// /// /// /// /// /// ///
SecMapTrainerWithSpacePointsModule::SecMapTrainerWithSpacePointsModule() : Module()
{
  InitializeVariables();

  //Set module properties
  setDescription("this module analyzes a big number of events (pGun or evtGen) to create raw sectorMaps which are needed for the VXDTF. This information can be exported via ascii files (not recommended) and root files (default, recommended).");
//   setPropertyFlags(c_ParallelProcessingCertified);

  std::vector<double> defaultConfigU = {0.0, 0.5, 1.0};
  std::vector<double> defaultConfigV = {0.0, 0.33, 0.67, 1.0};
  std::vector<float> defaultpTcuts = { 0.035};
  std::vector<std::string> defaultSecNames = {"fullRange"};
  std::vector<float> originVec = {0, 0, 0};
  std::vector<float> acceptedRegionForSensorsVec = { -1, -1};
  std::vector<std::string> rootFileNameVals = {"SecMapTrainerWithSpacePointsResults", "UPDATE"};
  std::vector<std::string> supportedDetectors = {"SVD"};

  addParam("exportSectorCoords", m_PARAMexportSectorCoords, "set true if you want to export coordinates of the sectors too",
           bool(true));

  addParam("sectorSetupFileName", m_PARAMsectorSetupFileName,
           "enables personal sector setups (can be loaded by the vxd track finder)", string("genericSectorMap"));

  addParam("spTCarrayName", m_PARAMspTCarrayName,
           "the name of the storeArray containing the SpacePointTrackCands used for the secMap-generation", string(""));

  addParam("pTcuts", m_PARAMpTcuts,
           "minimal number of entries is 1. first entry defines lower threshold for pT in GeV/c. Each further value defines a momentum range for a new sectorMap",
           defaultpTcuts);

  addParam("pTCutSmearer", m_PARAMpTCutSmearer,
           "has to be 100. > x >= 0, if > 0, the tracks near a pT cut are sorted into several secMaps. this parameter defines the border-area in percent",
           float(0.));

  addParam("detectorType", m_PARAMdetectorType,
           "defines which detector type has to be exported. Like geometry, simply add the detector types you want to include in the track candidates. Currently supported: PXD, SVD, VXD and TEL",
           supportedDetectors);

  addParam("maxXYvertexDistance", m_PARAMmaxXYvertexDistance,
           "allows to abort particles having their production vertex too far away from the origin (xy-plane) - WARNING for testbeam cases, this is a typical source for strange results!",
           float(0.5));

  addParam("maxZvertexDistance", m_PARAMmaxZvertexDistance,
           "allows to abort particles having their production vertex too far away from the origin (z-dist) - WARNING for testbeam cases, this is a typical source for strange results",
           float(2.0));

  addParam("setOrigin", m_PARAMsetOrigin,
           "standard origin is (0,0,0). If you want to have the map calculated for another origin, set here(x,y,z) - WARNING for testbeam cases, this is a typical source for strange results",
           originVec);

  addParam("testBeam", m_PARAMtestBeam,
           "if normal mode (0) does not produce a full sectormap, try setting it to testBeam-mode = 1 (testbeam 1 does not assume that the IP is at the origin and ignores curler) or even testBeam-mode = 2 (next to mode 1 features it ignores tracks jumping e.g. from layer 1 to 7 (telescopes), should only be used with care since some bad cases can not be caught that way)",
           int(0));

  addParam("multiHitsAllowed", m_PARAMmultiHitsAllowed,
           "if this parameter is true, the SecMapTrainerWithSpacePointsModule ignores tracks which have more than one hit on the same sensor. If false, these tracks get filtered. There will be a warning, if parameter 'testBeam' is != 0 and this parameter is true, since there curlers shouldn't be possible",
           bool(false));

  addParam("acceptedRegionForSensors", m_PARAMacceptedRegionForSensors,
           " accepts pair of input values. first one defines minimal distance for sectors to the origin and second one defines maximum accepted distance for sectors. If anyone of these values is above 0, sectors will be sorted using their distance2Origin parameter, not their layerID",
           acceptedRegionForSensorsVec);

  addParam("magneticFieldStrength", m_PARAMmagneticFieldStrength, "set strength of magnetic field in Tesla, standard is 1.5T",
           float(1.5));

  addParam("analysisWriteToRoot", m_PARAManalysisWriteToRoot,
           " if true, analysis data is stored to root file with file name chosen by 'rootFileName'", bool(false));

  addParam("secMapWriteToRoot", m_PARAMsecMapWriteToRoot,
           " if true, analysis data is stored to root file with file name chosen by 'rootFileName'", bool(false));

  addParam("secMapWriteToAscii", m_PARAMsecMapWriteToAscii, " if true, analysis data is stored to ascii files (standard setting)",
           bool(true));

  addParam("rootFileName", m_PARAMrootFileName,
           " only two entries accepted, first one is the root filename, second one is 'RECREATE' or 'UPDATE' which is the write mode for the root file, parameter is used only if 'writeToRoot'=true  ",
           rootFileNameVals);

  addParam("sectorConfigU", m_PARAMsectorConfigU,
           "allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0",
           defaultConfigU);

  addParam("sectorConfigV", m_PARAMsectorConfigV,
           "allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0",
           defaultConfigV);

  addParam("trackErrorTracks", m_PARAMtrackErrorTracks, "track tracks which cause strange results", bool(false));

  addParam("highestAllowedLayer", m_PARAMhighestAllowedLayer, "set value below 6 if you want to exclude outer layers (standard is 6)",
           unsigned(6));

  addParam("uniSigma", m_PARAMuniSigma,
           "standard value is 1/sqrt(12). Change this value for sharper or more diffuse hits (coupled with 'smearHits')",
           float(1 / sqrt(12.)));

  addParam("smearHits", m_PARAMsmearHits, "if True, hits get smeared by pitch/uniSigma", bool(false));

  addParam("noCurler", m_PARAMnoCurler,
           "if True, curling tracks get reduced to first part of trajectory before starting to curl back", bool(false));

  addParam("minTrackletLength", m_PARAMminTrackletLength, "defines the number of hits needed to be stored as a tracklet",
           unsigned(3));

  addParam("filterCharges", m_filterCharges,
           "this value can be set to: 1: allow only particles with positive charges, 0: allow all particles, -1: allow only particles with negative charges - standard is 0",
           short(0));


  //2 hit filters:
  addParam("logDistanceXY", m_PARAMlogDistanceXY, "set 'true' if you want to log distances (XY) between trackHits", bool(true));
  addParam("logDistanceZ", m_PARAMlogDistanceZ, "set 'true' if you want to log distances (Z) between trackHits", bool(true));
  addParam("logDistance3D", m_PARAMlogDistance3D, "set 'true' if you want to log distances (3D) between trackHits", bool(true));
  addParam("logNormedDistance3D", m_PARAMlogNormedDistance3D, " set 'true' to log improved 3D distance between trackHits",
           bool(true));
  addParam("logSlopeRZ", m_PARAMlogSlopeRZ, " set 'true' to log slope in r-z-plane for line of 2 trackHits", bool(true));

  //3 hit filters;
  addParam("logAngles3D", m_PARAMlogAngles3D, "set 'true' if you want to log angles3D between segments", bool(true));
  addParam("logAnglesXY", m_PARAMlogAnglesXY, "set 'true' if you want to log anglesXY between segments", bool(true));
  addParam("logAnglesRZ", m_PARAMlogAnglesRZ, "set 'true' if you want to log anglesRZ between segments", bool(true));
  addParam("logDeltaSlopeRZ", m_PARAMlogDeltaSlopeRZ, "set 'true' if you want to log delta slopes in r-z-plane between segments",
           bool(true));
  addParam("logPt", m_PARAMlogPt, "set 'true' if you want to log Pt between segments", bool(true));
  addParam("logTRadius2IPDistance", m_PARAMlogTRadiustoIPDistance,
           " set 'true' to log the difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP",
           bool(true));
  addParam("logHelixParameterFit", m_PARAMlogHelixParameterFit,
           "set 'true' if you want to log delta ((helix-circle-segment-angle) / deltaZ)", bool(true));
  addParam("logDeltaSOverZ", m_PARAMlogDeltaSOverZ, "set 'true' if you want to log Delta S (arc-length) over Z", bool(true));
  addParam("logDeltaSlopeZOverS", m_PARAMlogDeltaSlopeZOverS, "set 'true' if you want to log Delta of the slope Z over S",
           bool(true));

  // 2+1 hit high occupancy filters:
  addParam("logAnglesHighOccupancy3D", m_PARAMlogAnglesHighOccupancy3D,
           "set 'true' if you want to log anglesHighOccupancy3D between segments", bool(true));
  addParam("logAnglesHighOccupancyXY", m_PARAMlogAnglesHighOccupancyXY,
           "set 'true' if you want to log anglesHighOccupancyXY between segments", bool(true));
  addParam("logAnglesHighOccupancyRZ", m_PARAMlogAnglesHighOccupancyRZ,
           "set 'true' if you want to log anglesHighOccupancyRZ between segments", bool(true));
  addParam("logDeltaSlopeHighOccupancyRZ", m_PARAMlogDeltaSlopeHighOccupancyRZ,
           "set 'true' if you want to log HighOccupancy delta slopes in r-z-plane between segments", bool(true));
  addParam("logPtHighOccupancy", m_PARAMlogPtHighOccupancy, "set 'true' if you want to log PtHighOccupancy between segments",
           bool(true));
  addParam("logTRadiusHighOccupancy2IPDistance", m_PARAMlogTRadiusHighOccupancytoIPDistance,
           " set 'true' to log the HighOccupancy difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP",
           bool(true));
  addParam("logHelixParameterHighOccupancyFit", m_PARAMlogHelixParameterHighOccupancyFit,
           "set 'true' if you want to log HighOccupancy delta ((helix-circle-segment-angle) / deltaZ)", bool(true));

  //4 hit filters;
  addParam("logDeltaPt", m_PARAMlogDeltaPt, "set 'true' if you want to log delta Pt between segments", bool(true));
  addParam("logDeltaDistCircleCenter", m_PARAMlogDeltaDistCircleCenter,
           "set 'true' to compare the distance of the calculated centers of track circles", bool(true));

  //3+1 hit high occupancy filters:
  addParam("logDeltaPtHighOccupancy", m_PARAMlogDeltaPtHighOccupancy,
           "set 'true' if you want to log delta Pt HighOccupancy between segments", bool(true));
  addParam("logDeltaDistCircleCenterHighOccupancy", m_PARAMlogDeltaDistCircleCenterHighOccupancy,
           "set 'true' to compare the HighOccupancy distance of the calculated centers of track circles", bool(true));

  // debug filters, should only be logged if they shall be used for testing!
  addParam("logAlwaysTrue2Hit", m_PARAMlogAlwaysTrue2Hit,
           "debug filters: set 'true' if you want to log AlwaysTrue2Hit between segments", bool(false));
  addParam("logAlwaysFalse2Hit", m_PARAMlogAlwaysFalse2Hit,
           "debug filters: set 'true' if you want to log AlwaysFalse2Hit between segments", bool(false));
  addParam("logRandom2Hit", m_PARAMlogRandom2Hit,
           "debug filters: set 'true' if you want to log Random2Hit between segments", bool(false));
  addParam("logAlwaysTrue3Hit", m_PARAMlogAlwaysTrue3Hit,
           "debug filters: set 'true' if you want to log AlwaysTrue3Hit between segments", bool(false));
  addParam("logAlwaysFalse3Hit", m_PARAMlogAlwaysFalse3Hit,
           "debug filters: set 'true' if you want to log AlwaysFalse3Hit between segments", bool(false));
  addParam("logRandom3Hit", m_PARAMlogRandom3Hit,
           "debug filters: set 'true' if you want to log Random3Hit between segments", bool(false));
  addParam("logAlwaysTrue4Hit", m_PARAMlogAlwaysTrue4Hit,
           "debug filters: set 'true' if you want to log AlwaysTrue4Hit between segments", bool(false));
  addParam("logAlwaysFalse4Hit", m_PARAMlogAlwaysFalse4Hit,
           "debug filters: set 'true' if you want to log AlwaysFalse4Hit between segments", bool(false));
  addParam("logRandom4Hit", m_PARAMlogRandom4Hit,
           "debug filters: set 'true' if you want to log Random4Hit between segments", bool(false));
  addParam("logZigZagXY", m_PARAMlogZigZagXY,
           "WARNING ZigZagXY is not a debug filter but since it does not work using cutoffs, this Filter shall not be logged, since it only collects random data (this logging is needed to bypass a design-flaw, which discards sectorMaps which only carry Filters which do not need cutoffs)",
           bool(false));
  addParam("logZigZagXYWithSigmas", m_PARAMlogZigZagXYWithSigmas,
           "WARNING ZigZagXYWithSigmas is not a debug filter but since it does not work using cutoffs, this Filter shall not be logged, since it only collects random data (this logging is needed to bypass a design-flaw, which discards sectorMaps which only carry Filters which do not need cutoffs)",
           bool(false));
  addParam("logZigZagRZ", m_PARAMlogZigZagRZ,
           "WARNING ZigZagRZ is not a debug filter but since it does not work using cutoffs, this Filter shall not be logged, since it only collects random data (this logging is needed to bypass a design-flaw, which discards sectorMaps which only carry Filters which do not need cutoffs)",
           bool(false));

  addParam("useOldSecCalc", m_PARAMuseOldSecCalc, "WARNING DEBUG - if true, old way to calc secID is used April15th-2014",
           bool(false));
}



/// /// /// /// /// /// /// /// DESTRUCTOR /// /// /// /// /// /// /// ///
SecMapTrainerWithSpacePointsModule::~SecMapTrainerWithSpacePointsModule()
{
}



/// /// /// /// /// /// /// /// INITIALIZE /// /// /// /// /// /// /// ///
void SecMapTrainerWithSpacePointsModule::initialize()
{
  if (int(m_PARAMsetOrigin.size()) != 3) {
    B2WARNING("SecMapTrainerWithSpacePointsModule::initialize: origin is set wrong, please set only 3 values (x,y,z). Rejecting user defined value and reset to (0,0,0)!");
    m_PARAMsetOrigin.clear();
    m_PARAMsetOrigin.assign(3, 0);
  }
  m_origin.SetXYZ(m_PARAMsetOrigin.at(0), m_PARAMsetOrigin.at(1), m_PARAMsetOrigin.at(2));

  if (m_PARAMminTrackletLength < 2) {
    B2ERROR("SecMapTrainerWithSpacePointsModule::initialize: minTrackletLength is set to " << m_PARAMminTrackletLength <<
            " which can't produce any usefull results, setting value to 2, but consider setting the parameter to 3 which is the recommended minimal length");
    m_PARAMminTrackletLength = 2;
  }

  if (int(m_PARAMacceptedRegionForSensors.size()) != 2) {
    B2ERROR("SecMapTrainerWithSpacePointsModule::initialize: acceptedRegionForSensor-parameter has got " <<
            m_PARAMacceptedRegionForSensors.size()
            << " which is not allowed. Setting to standard value. If you do not know the correct choice, please type 'basf2 -m VXDTF' and read the description.");
    m_PARAMacceptedRegionForSensors.clear();
    m_PARAMacceptedRegionForSensors.assign(2, -1);
  }

  if (m_PARAMtestBeam < 0 or m_PARAMtestBeam > 2) {
    B2ERROR("SecMapTrainerWithSpacePointsModule::initialize: testbeam-parameter set to " << m_PARAMtestBeam <<
            " which is not allowed. Setting to 0. If you do not know the correct choice, please type 'basf2 -m VXDTF' and read the description.");
    m_PARAMtestBeam = 0;
  }
  if (m_PARAMmultiHitsAllowed == true and m_PARAMtestBeam > 0) { B2WARNING("SecMapTrainerWithSpacePointsModule::Initialize: parameter 'multiHitsAllowed' is true although 'testBeamm' is " << m_PARAMtestBeam << "! Is this on purpose? Please check!");}


  if (m_PARAManalysisWriteToRoot == true) { // preparing output of analysis data:
    if ((m_PARAMrootFileName.size()) != 2) {
      string output;
      for (string entry : m_PARAMrootFileName) {
        output += "'" + entry + "' ";
      }
      B2FATAL("SecMapTrainerWithSpacePointsModule::initialize: rootFileName is set wrong, although parameter 'writeToRoot' is enabled! Actual entries are: "
              << output);
    }
  }

  if (m_filterCharges != -1 and m_filterCharges != 0 and m_filterCharges != 1) {
    B2ERROR("SecMapTrainerWithSpacePointsModule::initialize: parameter filterCharges is set to " << m_filterCharges <<
            ", which is invalid, setting to standard (0)!");
    m_filterCharges = 0;
  }


  m_threeHitFilterBox.resetMagneticField(m_PARAMmagneticFieldStrength);
  m_fourHitFilterBox.resetMagneticField(m_PARAMmagneticFieldStrength);

  B2INFO("~~~~~~~~~~~SecMapTrainerWithSpacePointsModule - initialize ~~~~~~~~~~");


  m_numOfLayers = 0;
  stringstream detectorNames; // same as above but a little bit more readable for B2XY-output...
  if (std::find(m_PARAMdetectorType.begin(), m_PARAMdetectorType.end(), "TEL") != m_PARAMdetectorType.end()) {
    m_numOfLayers += 6; /// WARNING hardcoded values, is there a way to get this information automatically?
    m_detectorName += "TEL";
    detectorNames << "TEL ";
    m_useTEL = true;
  }
  if (std::find(m_PARAMdetectorType.begin(), m_PARAMdetectorType.end(), "PXD") != m_PARAMdetectorType.end()) {
    m_numOfLayers += 2; /// WARNING hardcoded values, is there a way to get this information automatically?
    m_detectorName += "PXD";
    detectorNames << "PXD ";
    m_usePXD = true;
  }
  if (std::find(m_PARAMdetectorType.begin(), m_PARAMdetectorType.end(), "SVD") != m_PARAMdetectorType.end()) {
    m_numOfLayers += 4; /// WARNING hardcoded values, is there a way to get this information automatically?
    m_detectorName += "SVD";
    detectorNames << "SVD ";
    m_useSVD = true;
  }

  m_trackletLengthCounter.assign(m_numOfLayers * 2 + 1, 0);

  B2INFO("chosen detectorTypes: " << detectorNames.str() << ", highestAllowedLayer: " << m_PARAMhighestAllowedLayer << ", smearHits: "
         << m_PARAMsmearHits << ", noCurler: " << m_PARAMnoCurler << ", uniSigma: " << m_PARAMuniSigma);
  B2INFO("origin is set to: (x,y,z) (" << m_PARAMsetOrigin.at(0) << "," << m_PARAMsetOrigin.at(1) << "," << m_PARAMsetOrigin.at(
           2) << "), testBeam-mode is " << m_PARAMtestBeam << ", magnetic field set to " << m_PARAMmagneticFieldStrength << "T");

  if (m_PARAMpTCutSmearer > 100 or m_PARAMpTCutSmearer < 0) { B2ERROR("pTCutSmearer was set to " << m_PARAMpTCutSmearer << ", which is not allowed, set to 0 now..."); m_PARAMpTCutSmearer = 0; }

  m_spacePointTrackCands.isRequired(m_PARAMspTCarrayName);


  int numCuts = m_PARAMpTcuts.size();
  Sector newCenterSector(FullSecID().getFullSecString(), {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0);

  for (int i = 0; i < numCuts - 1; ++i) {
    string secMapName = (boost::format("%1%$%2%to%3%MeV_%4%") % m_PARAMsectorSetupFileName % int(m_PARAMpTcuts.at(i) * 1000.) % int(
                           m_PARAMpTcuts.at(i + 1) * 1000.) % m_detectorName).str();
    for (int j = 0; j < int(secMapName.length()); ++j) {
      switch (secMapName.at(j)) {
        case '$':
          secMapName.at(j) = '-';
        case '.':
          secMapName.at(j) = '-';
      }
    }
    m_PARAMsecMapNames.push_back(secMapName);

    InternalRawSectorMap* secMap = new InternalRawSectorMap(secMapName,
                                                            m_PARAMpTCutSmearer * 0.01,
                                                            m_PARAMpTcuts.at(i),
                                                            m_PARAMpTcuts.at(i + 1),
                                                            m_PARAMhighestAllowedLayer,
                                                            m_PARAMacceptedRegionForSensors.at(0),
                                                            m_PARAMacceptedRegionForSensors.at(1),
                                                            m_usePXD,
                                                            m_useSVD,
                                                            m_useTEL);
    secMap->insert({FullSecID().getFullSecString(), newCenterSector});

    B2INFO("SecMapTrainerWithSpacePointsModule-start: will use " << secMapName << " for storing cutoffs in range " <<
           secMap->getPtCuts().first <<
           " - " << secMap->getPtCuts().second << " GeV/c");
    m_sectorMaps.push_back(secMap);
  }

  // adding last secMap
  string secMapName = (boost::format("%1%$moreThan%2%MeV_%3%") % m_PARAMsectorSetupFileName % int(m_PARAMpTcuts.at(
                         numCuts - 1) * 1000)  % m_detectorName).str();
  for (int i = 0; i < int(secMapName.length()); ++i) {
    switch (secMapName.at(i)) {
      case '$':
        secMapName.at(i) = '-';
      case '.':
        secMapName.at(i) = '-';
    }
  }
  m_PARAMsecMapNames.push_back(secMapName);

  InternalRawSectorMap* secMap = new InternalRawSectorMap(secMapName,
                                                          m_PARAMpTCutSmearer * 0.01,
                                                          m_PARAMpTcuts.at(numCuts - 1),
                                                          -1,
                                                          m_PARAMhighestAllowedLayer,
                                                          m_PARAMacceptedRegionForSensors.at(0),
                                                          m_PARAMacceptedRegionForSensors.at(1),
                                                          m_usePXD,
                                                          m_useSVD,
                                                          m_useTEL);
  secMap->insert({FullSecID().getFullSecString(), newCenterSector});

  B2INFO("SecMapTrainerWithSpacePointsModule-start: will use " << secMapName << " for storing cutoffs in range " <<
         secMap->getPtCuts().first <<
         " - " << secMap->getPtCuts().second << " GeV/c");
  m_sectorMaps.push_back(secMap);


  m_trackletMomentumCounter.resize(numCuts, 0);



  if (m_PARAManalysisWriteToRoot == true) { // preparing output of analysis data:
    string fileNameOnly = m_PARAMrootFileName.at(0) + "Analysis.root";
    m_rootFilePtr = new TFile(fileNameOnly.c_str(), m_PARAMrootFileName.at(1).c_str());

    m_treeEventWisePtr = dynamic_cast<TTree*>(m_rootFilePtr->Get("m_treeEventWisePtr")); // trying to open existing TTree in file
    if (m_treeEventWisePtr == NULL/* and m_PARAMstoreExtraAnalysis == true*/) {  // did not exist
      B2INFO(" m_treeEventWisePtr did not exist, creating new tree...");
      m_treeEventWisePtr = new TTree("m_treeEventWisePtr", "anEventWiseTree");
      m_treeEventWisePtr->Branch("pTValuesInLayer1", &m_rootpTValuesInLayer1);
      m_treeEventWisePtr->Branch("momValuesInLayer1", &m_rootmomValuesInLayer1);
    } else { /*if (m_PARAMstoreExtraAnalysis == true)*/
      B2INFO(" m_treeEventWisePtr did exist, reopen old tree... Print: ");
      m_rootpTValuesInLayer1Ptr = &m_rootpTValuesInLayer1;
      m_rootmomValuesInLayer1Ptr = &m_rootmomValuesInLayer1;
      m_treeEventWisePtr->SetBranchAddress("pTValuesInLayer1", &m_rootpTValuesInLayer1Ptr);
      m_treeEventWisePtr->SetBranchAddress("momValuesInLayer1", &m_rootmomValuesInLayer1Ptr);
    }

  } else {
    m_rootFilePtr = NULL;
    m_treeEventWisePtr = NULL;
  }



}



/// /// /// /// /// /// /// /// BEGIN RUN /// /// /// /// /// /// /// ///
void SecMapTrainerWithSpacePointsModule::beginRun()
{
  B2INFO("~~~~~~~~~~~SecMapTrainerWithSpacePointsModule - beginRun ~~~~~~~~~~");

//   InitializeVariables();
}



/// /// /// /// /// /// /// /// EVENT /// /// /// /// /// /// /// ///
void SecMapTrainerWithSpacePointsModule::event()
{
  //get the data
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(5, "~~~~~~~~~~~SecMapTrainerWithSpacePointsModule - event " << m_eventCounter << " ~~~~~~~~~~");

  //simulated particles and hits
  unsigned nSPTCs = m_spacePointTrackCands.getEntries();

  if (nSPTCs == 0) {
    B2INFO("event " << m_eventCounter << ": there is no SpacePointTrackCandidate!");
    return;
  }

  m_rootmomValuesInLayer1.clear();
  m_rootpTValuesInLayer1.clear();

  B2DEBUG(5, "SecMapTrainerWithSpacePointsModule, event " << m_eventCounter << ": size of array nSpacePointTrackCands: " << nSPTCs);


  vector<VXDTrack> trackletsOfEvent; // tracks cut into bite-sized pieces for the filtering part

  /** nVXDTracks >= nSPTCs.
   * Reason:
   * for each sptc: find fitting secMap. if near border, nSecMaps fitting > 1.
   * in this case the VXDTrack will be cloned for each relevant secMap.
   */
  for (unsigned iTC = 0; iTC not_eq nSPTCs; ++ iTC) {
    const SpacePointTrackCand* currentTC = m_spacePointTrackCands[iTC];
    B2DEBUG(10, "current SPTC " << currentTC->getArrayIndex() <<
            " has got " << currentTC->getNHits() << " hits stored");
    unsigned chosenMap = 0;
    for (auto* secMap : m_sectorMaps) {
      chosenMap++;
      /// can be accepted by several secMaps, because of momentum range or whatever:
      bool accepted = checkAcceptanceOfSecMap(secMap, currentTC);

      B2DEBUG(15, "current SPTC " << currentTC->getArrayIndex() <<
              " with " << currentTC->getNHits() <<
              " hits stored and pT of " << currentTC->getMomSeed().Perp() <<
              "GeV/c was " << (accepted ? string("accepted") : string("rejected")) <<
              " by secMap " << secMap->getName() <<
              " having momCuts: " << secMap->getPtCuts().first <<
              "/" << secMap->getPtCuts().second);
      if (!accepted) continue;
      m_trackletMomentumCounter.at(chosenMap - 1)++;

      /// converts to internal data structure and attaches it to given secMap
      VXDTrack newTrack = convertSPTC2VXDTrack(secMap, currentTC, iTC);

      trackletsOfEvent.push_back(std::move(newTrack)); // nTracklets >= nSPTCs
    }
  }


  TVector3 hitGlobal;
  int nTotalHits = 0;
  float pMaxInMeV = 100000., pMinInMeV = 10.; /// WARNING hardcoded values!
  for (auto& tracklet : trackletsOfEvent) {
    nTotalHits += tracklet.getTrack().size();
  }
  B2DEBUG(5, "finished tracklet-generation. " << trackletsOfEvent.size() << " tracklets and " << nSPTCs <<
          " original tracks found having mean of " << float(nTotalHits) / float(trackletsOfEvent.size()) << " hits per tracklet");
  string currentSector, friendSector;
//4hit-variables:
  TVector3 hitG, moHitG, graMoHitG, greGraMoHitG;
  float deltaDistCircleCenter, deltapT;
// 3hit-variables:
  TVector3 motherHitGlobal, grandMotherHitGlobal;
  float dist2IP, angles3D, anglesXY, anglesRZ, helixParameterFit, deltaSlopeZOverS, deltaSOverZ , deltaSlopeRZ, pT;
// 2hit-variables:
  TVector3 segmentVector;
  float distanceXY, distanceZ, distance3D, normedDistance3D, slopeRZ;

  for (int i = 0; i < int(trackletsOfEvent.size()); i++) {

    bool firstrun = true, secondrun = true, thirdrun = true, lastRun = false;
    list<VXDHit> aTracklet = trackletsOfEvent.at(i).getTrack();
    InternalRawSectorMap::MapOfSectors* thisSecMap = trackletsOfEvent.at(i).getSecMap();
    list<VXDHit>::iterator iter = aTracklet.begin(); // main iterator looping through the whole tracklet, "innermost Hit"
    list<VXDHit>::iterator it2HitsFilter =
      aTracklet.begin(); //important for 2hit-Filters: points to current hit of 2-hit-processes " next to innermost hit"
    list<VXDHit>::iterator it3HitsFilter = aTracklet.begin(); // -"- 3hit-Filters...m_PARAMlogTRadiusHighOccupancytoIPDistance
    list<VXDHit>::iterator it4HitsFilter = aTracklet.begin(); // -"- 4hit-Filters... "outermost Hit"

    stringstream tcSectors;
    for (list<VXDHit>::iterator it = aTracklet.begin() ; it != aTracklet.end(); ++it) {
      currentSector = it->getSectorID();
      tcSectors << currentSector << " ";
//       B2DEBUG(20, "tracklet has a hit in the following sector: " << currentSector)
    }
    B2DEBUG(20, "executing " << i + 1 << "th tracklet with size " << aTracklet.size() << " and hits in the sectors:\n" <<
            tcSectors.str() << endl);

    InternalRawSectorMap::MapOfSectors::iterator thisSectorPos;
    for (; iter != aTracklet.end(); ++iter) {
      // moving from outermost hit inwards. iter points at the innermost hit of the current line of hits (means in the first iteration it points at the outermost hit, in the 2nd iteration it points at the inner hit next to the outermost hit and it2HitsFilter points to the outermost hit) with every iteration, iter leads the row of iterators inwards.
      list<VXDHit>::iterator tempIter = iter; ++tempIter;
      if (tempIter == aTracklet.end()) {
        lastRun = true;
      }

      if (firstrun == false) {
        B2DEBUG(25, "calculating 2-hit-filters");
        if (secondrun == false) {
          B2DEBUG(25, "calculating 3-hit-filters");
          if (thirdrun == false) {
            B2DEBUG(25, "calculating 4-hit-filters");
            currentSector = it4HitsFilter->getSectorID();
            friendSector = it3HitsFilter->getSectorID();
            thisSectorPos = thisSecMap->find(currentSector);

            hitG = it4HitsFilter->getHitPosition();
//             hitG.SetZ(0.);
            moHitG = it3HitsFilter->getHitPosition();
//             moHitG.SetZ(0.);
            graMoHitG = it2HitsFilter->getHitPosition();
//             graMoHitG.SetZ(0.);
            greGraMoHitG = iter->getHitPosition();
//             greGraMoHitG.SetZ(0.);
            B2DEBUG(50, "4-hit: outer -> innerHit Perp and SecID \n" << hitG.Perp() << "/" << currentSector << " " << moHitG.Perp() << "/" <<
                    friendSector << " " << graMoHitG.Perp() << "/" << it2HitsFilter->getSectorID() << " " << greGraMoHitG.Perp() << "/" <<
                    iter->getSectorID());
            m_fourHitFilterBox.resetValues(hitG, moHitG, graMoHitG, greGraMoHitG); // outerhit, centerhit, innerhit

            if (m_PARAMlogDeltaPt == true) {
              bool succeeded = true;
              deltapT = 0;
              try {
                deltapT = m_fourHitFilterBox.deltapT();
              } catch (FilterExceptions::Straight_Line& exception) {
                B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltapT threw an exception: " << exception.what() <<
                          ", value discarded...");
                succeeded = false;
              } catch (FilterExceptions::Circle_too_small& exception) {
                B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltapT threw an exception: " << exception.what() <<
                          ", value discarded...");
                succeeded = false;
              }
              if (std::isnan(deltapT) == false) {
                if (succeeded == true) {
                  B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltapT-Value: " << deltapT << " gets stored in sector " <<
                          thisSectorPos->second.getSectorID());
                  thisSectorPos->second.addValue(friendSector, FilterID::deltapT, deltapT);
                } else {
                  B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltapT-value got discarded");
                }
              } else {
                m_badFilterValueCtr++;
                B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltapT-Value: " << deltapT << " is 'nan'! currentSec: " <<
                          currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
                hitG.Print(); moHitG.Print(); graMoHitG.Print(); greGraMoHitG.Print();
              }
            }

            if (m_PARAMlogDeltaDistCircleCenter == true) {
              bool succeeded = true;
              deltaDistCircleCenter = 0;
              try {
                deltaDistCircleCenter =  m_fourHitFilterBox.deltaDistCircleCenter();
              } catch (FilterExceptions::Straight_Line& exception) {
                B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltaDistCircleCenter threw an exception: " <<
                          exception.what() << ", value discarded...");
                succeeded = false;
              } catch (FilterExceptions::Circle_too_small& exception) {
                B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltaDistCircleCenter threw an exception: " <<
                          exception.what() << ", value discarded...");
                succeeded = false;
              }
              if (std::isnan(deltaDistCircleCenter) == false) {
                if (succeeded == true) {
                  B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenter-Value: " << deltaDistCircleCenter <<
                          " gets stored in sector " << thisSectorPos->second.getSectorID());
                  thisSectorPos->second.addValue(friendSector, FilterID::deltaDistance2IP, deltaDistCircleCenter);
                } else {
                  B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenter-value got discarded");
                }
              } else {
                m_badFilterValueCtr++;
                B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenter-Value: " << deltaDistCircleCenter <<
                          " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
                hitG.Print(); moHitG.Print(); graMoHitG.Print(); greGraMoHitG.Print();
              }
            }

            //#### debugFilters:
            if (m_PARAMlogAlwaysTrue4Hit == true) {
              B2DEBUG(50, "4-hit-filterDEBUG in event " << m_eventCounter << ": calculated alwaysTrue4Hit-Value: " << 1 <<
                      " gets stored in sector " << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::alwaysTrue4Hit, 1);
            }
            if (m_PARAMlogAlwaysFalse4Hit == true) {
              B2DEBUG(50, "4-hit-filterDEBUG in event " << m_eventCounter << ": calculated alwaysFalse4Hit-Value: " << 0 <<
                      " gets stored in sector " << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::alwaysFalse4Hit, 0);
            }
            if (m_PARAMlogRandom4Hit == true) {
              B2DEBUG(50, "4-hit-filterDEBUG in event " << m_eventCounter << ": calculated random4Hit-Value: " << 0.5 << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::random4Hit, 0.5);
            }
            if (m_PARAMlogZigZagXY == true) {
              B2DEBUG(50, "4-hit-filterDEBUG in event " << m_eventCounter << ": calculated ziggZaggXY-Value: " << 1 << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::ziggZaggXY, 1);
            }
            if (m_PARAMlogAlwaysTrue4Hit == true) {
              B2DEBUG(50, "4-hit-filterDEBUG in event " << m_eventCounter << ": calculated ziggZaggXYWithSigma-Value: " << 1 <<
                      " gets stored in sector " << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::ziggZaggXYWithSigma, 1);
            }
            if (m_PARAMlogAlwaysTrue4Hit == true) {
              B2DEBUG(50, "4-hit-filterDEBUG in event " << m_eventCounter << ": calculated ziggZaggRZ-Value: " << 1 << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::ziggZaggRZ, 1);
            }

            ++it4HitsFilter; /// -"- 4hit-Filters...
          } else { thirdrun = false; }

          currentSector = it3HitsFilter->getSectorID();
          friendSector = it2HitsFilter->getSectorID();
          thisSectorPos = thisSecMap->find(currentSector);

          hitGlobal = it3HitsFilter->getHitPosition();
          motherHitGlobal = it2HitsFilter->getHitPosition();
          grandMotherHitGlobal = iter->getHitPosition();

          /// high occupancy mode for 3+1 hits
          m_fourHitFilterBox.resetValues(hitGlobal, motherHitGlobal, grandMotherHitGlobal, m_origin); // outerhit, centerhit, innerhit
          B2DEBUG(50, "3-hit and hioc 3+1: outer -> innerHit Perp and SecID \n" << hitGlobal.Perp() << "/" << currentSector << " " <<
                  motherHitGlobal.Perp() << "/" << friendSector << " " << grandMotherHitGlobal.Perp() << "/" << iter->getSectorID() << " " <<
                  m_origin.Perp());
          if (m_PARAMlogDeltaPtHighOccupancy == true and lastRun == false) {
            bool succeeded = true;
            deltapT = 0;
            try {
              deltapT = m_fourHitFilterBox.deltapT();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltapTHighOccupancy threw an exception: " <<
                        exception.what() << ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltapTHighOccupancy threw an exception: " <<
                        exception.what() << ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(deltapT) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltapTHighOccupancy-Value: " << deltapT <<
                        " gets stored in sector " << thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::deltapTHighOccupancy, deltapT);
              } else {
                B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltapTHighOccupancy-value got discarded");
              }
            } else {
              m_badFilterValueCtr++;
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltapTHighOccupancy-Value: " << deltapT <<
                        " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print(); m_origin.Print();
            }
          }
          if (m_PARAMlogDeltaDistCircleCenterHighOccupancy == true and lastRun == false) {
            bool succeeded = true;
            deltaDistCircleCenter = 0;
            try {
              deltaDistCircleCenter =  m_fourHitFilterBox.deltaDistCircleCenter();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltaDistCircleCenterHighOccupancy threw an exception: " <<
                        exception.what() << ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltaDistCircleCenterHighOccupancy threw an exception: " <<
                        exception.what() << ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(deltaDistCircleCenter) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenterHighOccupancy-Value: " <<
                        deltaDistCircleCenter << " gets stored in sector " << thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::deltaDistanceHighOccupancy2IP, deltaDistCircleCenter);
              } else {
                B2DEBUG(50, "4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenterHighOccupancy-value got discarded");
              }
            } else {
              m_badFilterValueCtr++;
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenterHighOccupancy-Value: " <<
                        deltaDistCircleCenter << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector <<
                        ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print(); m_origin.Print();
            }
          }
          /// high occupancy mode - end

          m_threeHitFilterBox.resetValues(hitGlobal, motherHitGlobal, grandMotherHitGlobal); // outerhit, centerhit, innerhit

          if (m_PARAMlogTRadiustoIPDistance == true) {
            bool succeeded = true;
            dist2IP = 0;
            try {
              dist2IP = m_threeHitFilterBox.calcCircleDist2IP();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating dist2IP threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating dist2IP threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(dist2IP) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated dist2IP-Value: " << dist2IP << " gets stored in sector " <<
                        thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::distance2IP, dist2IP);
              } else {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated dist2IP-value got discarded");
              }
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated dist2IP-Value: " << dist2IP << " is 'nan'! currentSec: " <<
                        currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print(); m_origin.Print();
            }
          }

          if (m_PARAMlogPt == true) {
            bool succeeded = true;
            pT = 0;
            try {
              pT = m_threeHitFilterBox.calcPt();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating pT threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating pT threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(pT) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated pT-Value: " << pT << " gets stored in sector " <<
                        thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::pT, pT);
              } else {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated pT-value got discarded");
              }
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated pT-Value: " << pT << " is 'nan'! currentSec: " <<
                        currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
            if (m_PARAMtrackErrorTracks == true) {
              if (pT > 5.0 * pMaxInMeV * 0.001 || pT < 0.2 * pMinInMeV * 0.001) {
                B2WARNING(" at event " << m_eventCounter << " there is a problem with filter pt: " << pT << "GeV/c");
              }
            }
          }

          if (m_PARAMlogDeltaSlopeRZ == true) {
            deltaSlopeRZ = m_threeHitFilterBox.calcDeltaSlopeRZ();
            if (std::isnan(deltaSlopeRZ) == false) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeRZ-Value: " << deltaSlopeRZ <<
                      " gets stored in sector " << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::deltaSlopeRZ, deltaSlopeRZ);
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeRZ-Value: " << deltaSlopeRZ <<
                        " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          if (m_PARAMlogAngles3D == true) {
            angles3D = m_threeHitFilterBox.calcAngle3D();
            if (std::isnan(angles3D) == false) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated angles3D-Value: " << angles3D << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::angles3D, angles3D);
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated angles3D-Value: " << angles3D << " is 'nan'! currentSec: " <<
                        currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          if (m_PARAMlogAnglesXY == true) {
            anglesXY = m_threeHitFilterBox.calcAngleXY();
            if (std::isnan(anglesXY) == false) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated anglesXY-Value: " << anglesXY << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::anglesXY, anglesXY);
            }  else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesXY-Value: " << anglesXY << " is 'nan'! currentSec: " <<
                        currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          if (m_PARAMlogAnglesRZ == true) {
            anglesRZ = m_threeHitFilterBox.calcAngleRZ();
            if (std::isnan(anglesRZ) == false) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated anglesRZ-Value: " << anglesRZ << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::anglesRZ, anglesRZ);
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesRZ-Value: " << anglesRZ << " is 'nan'! currentSec: " <<
                        currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          if (m_PARAMlogHelixParameterFit == true) {
            bool succeeded = true;
            helixParameterFit = 0;
            try {
              helixParameterFit = m_threeHitFilterBox.calcHelixParameterFit();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating helixParameterFit threw an exception: " << exception.what()
                        << ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating helixParameterFit threw an exception: " << exception.what()
                        << ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(helixParameterFit) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated helixParameterFit-Value: " << helixParameterFit <<
                        " gets stored in sector " << thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::helixParameterFit, helixParameterFit);
              } else {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated helixParameterFit-value got discarded");
              }
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated helixParameterFit-Value: " << helixParameterFit <<
                        " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          if (m_PARAMlogDeltaSOverZ == true) {
            bool succeeded = true;
            deltaSOverZ = 0;
            try {
              deltaSOverZ = m_threeHitFilterBox.calcDeltaSOverZ();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating deltaSOverZ threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltaSOverZ threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(deltaSOverZ) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSOverZ-Value: " << deltaSOverZ <<
                        " gets stored in sector " << thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::deltaSOverZ, deltaSOverZ);
              } else {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSOverZ-value got discarded");
              }
            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSOverZ-Value: " << deltaSOverZ <<
                        " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          if (m_PARAMlogDeltaSlopeZOverS == true) {
            bool succeeded = true;
            deltaSlopeZOverS = 0;
            try {
              deltaSlopeZOverS = m_threeHitFilterBox.calcDeltaSlopeZOverS();
            } catch (FilterExceptions::Straight_Line& exception) {
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating deltaSlopeZOverS threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            } catch (FilterExceptions::Circle_too_small& exception) {
              B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating deltaSlopeZOverS threw an exception: " << exception.what() <<
                        ", value discarded...");
              succeeded = false;
            }
            if (std::isnan(deltaSlopeZOverS) == false) {
              if (succeeded == true) {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeZOverS-Value: " << deltaSlopeZOverS <<
                        " gets stored in sector " << thisSectorPos->second.getSectorID());
                thisSectorPos->second.addValue(friendSector, FilterID::deltaSlopeZOverS, deltaSlopeZOverS);
              } else {
                B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeZOverS-value got discarded");
              }

            } else {
              m_badFilterValueCtr++;
              B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeZOverS-Value: " << deltaSlopeZOverS <<
                        " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          //#### debugFilters:
          if (m_PARAMlogAlwaysTrue3Hit == true) {
            B2DEBUG(50, "3-hit-filterDEBUG in event " << m_eventCounter << ": calculated alwaysTrue3Hit-Value: " << 1 <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::alwaysTrue3Hit, 1);
          }
          if (m_PARAMlogAlwaysFalse3Hit == true) {
            B2DEBUG(50, "3-hit-filterDEBUG in event " << m_eventCounter << ": calculated alwaysFalse3Hit-Value: " << 0 <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::alwaysFalse3Hit, 0);
          }
          if (m_PARAMlogRandom3Hit == true) {
            B2DEBUG(50, "3-hit-filterDEBUG in event " << m_eventCounter << ": calculated random3Hit-Value: " << 0.5 << " gets stored in sector "
                    << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::random3Hit, 0.5);
          }

          ++it3HitsFilter; /// -"- 3hit-Filters...

        } else { secondrun = false; }

        currentSector = it2HitsFilter->getSectorID();
        friendSector = iter->getSectorID();
        thisSectorPos = thisSecMap->find(currentSector);

        if (thisSectorPos == thisSecMap->end()) { B2ERROR(" sector " << currentSector << " not found...");} /// WARNING TODO WTF?!?

        if (typeid(string).name() != typeid(friendSector).name()) {
          B2WARNING("SecMapTrainerWithSpacePointsModule event " << m_eventCounter <<
                    ": type of friendSector is no string, aborting tracklet...");
          continue;
        }

        hitGlobal = it2HitsFilter->getHitPosition();
        motherHitGlobal = iter->getHitPosition();
        B2DEBUG(50, "2-hit and hioc 2+1: outer -> innerHit Perp and SecID \n" << hitGlobal.Perp() << "/" << currentSector << " " <<
                motherHitGlobal.Perp() << "/" << friendSector << " " << m_origin.Perp());

        /// high occupancy mode for 2+1 hits
        m_threeHitFilterBox.resetValues(hitGlobal, motherHitGlobal, m_origin); // outerhit, centerhit, innerhit
        if (m_PARAMlogTRadiusHighOccupancytoIPDistance == true and lastRun == false) {
          bool succeeded = true;
          dist2IP = 0;
          try {
            dist2IP = m_threeHitFilterBox.calcCircleDist2IP();
          } catch (FilterExceptions::Straight_Line& exception) {
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating distHighOccupancy2IP threw an exception: " <<
                      exception.what() << ", value discarded...");
            succeeded = false;
          } catch (FilterExceptions::Circle_too_small& exception) {
            B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating distHighOccupancy2IP threw an exception: " <<
                      exception.what() << ", value discarded...");
            succeeded = false;
          }
          if (std::isnan(dist2IP) == false) {
            if (succeeded == true) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated distHighOccupancy2IP-Value(FilterID " <<
                      FilterID::distanceHighOccupancy2IP << "): " << dist2IP << " for sector " << FullSecID(friendSector) << " gets stored in sector " <<
                      thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::distanceHighOccupancy2IP, dist2IP);
            } else {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated distHighOccupancy2IP-value got discarded");
            }
          } else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated distHighOccupancy2IP-Value: " << dist2IP <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }
        if (m_PARAMlogPtHighOccupancy == true and lastRun == false) {
          bool succeeded = true;
          pT = 0;
          try {
            pT = m_threeHitFilterBox.calcPt();
          } catch (FilterExceptions::Straight_Line& exception) {
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating pTHighOccupancy threw an exception: " << exception.what() <<
                      ", value discarded...");
            succeeded = false;
          } catch (FilterExceptions::Circle_too_small& exception) {
            B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating pTHighOccupancy threw an exception: " << exception.what() <<
                      ", value discarded...");
            succeeded = false;
          }
          if (std::isnan(pT) == false) {
            if (succeeded == true) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated pTHighOccupancy-Value: " << pT << " gets stored in sector "
                      << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::pTHighOccupancy, pT);
            } else {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated distHighOccupancy2IP-value got discarded");
            }
          } else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated pTHighOccupancy-Value: " << pT << " is 'nan'! currentSec: " <<
                      currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }
        if (m_PARAMlogDeltaSlopeHighOccupancyRZ == true and lastRun == false) {
          deltaSlopeRZ = m_threeHitFilterBox.calcDeltaSlopeRZ();
          if (std::isnan(deltaSlopeRZ) == false) {
            B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeHighOccupancyRZ-Value: " << deltaSlopeRZ <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::deltaSlopeHighOccupancyRZ, deltaSlopeRZ);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeHighOccupancyRZ-Value: " << deltaSlopeRZ <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }
        if (m_PARAMlogAnglesHighOccupancy3D == true and lastRun == false) {
          angles3D = m_threeHitFilterBox.calcAngle3D();
          if (std::isnan(angles3D) == false) {
            B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancy3D-Value: " << angles3D <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::anglesHighOccupancy3D, angles3D);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancy3D-Value: " << angles3D <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }
        if (m_PARAMlogAnglesHighOccupancyXY == true and lastRun == false) {
          anglesXY = m_threeHitFilterBox.calcAngleXY();
          if (std::isnan(anglesXY) == false) {
            B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyXY-Value: " << anglesXY <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::anglesHighOccupancyXY, anglesXY);
          }  else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyXY-Value: " << anglesXY <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }
        if (m_PARAMlogAnglesHighOccupancyRZ == true and lastRun == false) {
          anglesRZ = m_threeHitFilterBox.calcAngleRZ();
          if (std::isnan(anglesRZ) == false) {
            B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyRZ-Value: " << anglesRZ <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::anglesHighOccupancyRZ, anglesRZ);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyRZ-Value: " << anglesRZ <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }
        if (m_PARAMlogHelixParameterHighOccupancyFit == true and lastRun == false) {
          bool succeeded = true;
          helixParameterFit = 0;
          try {
            helixParameterFit = m_threeHitFilterBox.calcHelixParameterFit();
          } catch (FilterExceptions::Straight_Line& exception) {
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculating helixParameterHighOccupancyFit threw an exception: " <<
                      exception.what() << ", value discarded...");
            succeeded = false;
          } catch (FilterExceptions::Circle_too_small& exception) {
            B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculating helixParameterHighOccupancyFit threw an exception: " <<
                      exception.what() << ", value discarded...");
            succeeded = false;
          }
          if (std::isnan(helixParameterFit) == false) {
            if (succeeded == true) {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated helixParameterHighOccupancyFit-Value: " << helixParameterFit
                      << " gets stored in sector " << thisSectorPos->second.getSectorID());
              thisSectorPos->second.addValue(friendSector, FilterID::helixParameterHighOccupancyFit, helixParameterFit);
            } else {
              B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated helixParameterHighOccupancyFit-value got discarded");
            }
          } else {
            m_badFilterValueCtr++;
            B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated helixHighOccupancyFit-Value: " << helixParameterFit <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print(); m_origin.Print();
          }
        }

//         if (m_PARAMlogDeltaSOverZHighOccupancy == true and lastRun == false) {
//             deltaSOverZ = m_threeHitFilterBox.calcDeltaSOverZ();
//             if (std::isnan(deltaSOverZ) == false) {
//               B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSOverZ-Value: " << deltaSOverZ << " gets stored in sector " << thisSectorPos->second.getSectorID())
//               thisSectorPos->second.addValue(friendSector, FilterID::deltaSOverZ, deltaSOverZ);
//             } else {
//               m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSOverZ-Value: " << deltaSOverZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
//               hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
//             }
//           }
//
//           if (m_PARAMlogDeltaSlopeZOverS == true) {
//             deltaSlopeZOverS = m_threeHitFilterBox.calcDeltaSlopeZOverS();
//             if (std::isnan(deltaSlopeZOverS) == false) {
//               B2DEBUG(50, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeZOverS-Value: " << deltaSlopeZOverS << " gets stored in sector " << thisSectorPos->second.getSectorID())
//               thisSectorPos->second.addValue(friendSector, FilterID::deltaSlopeZOverS, deltaSlopeZOverS);
//             } else {
//               m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeZOverS-Value: " << deltaSlopeZOverS << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
//               hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
//             }
//           }
        /// high occupancy mode - end

        m_twoHitFilterBox.resetValues(hitGlobal, motherHitGlobal); // outerhit, innerhit

        if (m_PARAMlogDistanceXY == true) {
          distanceXY = m_twoHitFilterBox.calcDistXY();
          if (std::isnan(distanceXY) == false) {
            B2DEBUG(50, "2-hit-filter in event " << m_eventCounter << ": calculated distanceXY-Value: " << distanceXY <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::distanceXY, distanceXY);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated distanceXY-Value: " << distanceXY << " is 'nan'! currentSec: "
                      << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogDistanceZ == true) {
          distanceZ = m_twoHitFilterBox.calcDistZ();
          if (std::isnan(distanceZ) == false) {
            B2DEBUG(50, "2-hit-filter in event " << m_eventCounter << ": calculated distanceZ-Value: " << distanceZ << " gets stored in sector "
                    << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::distanceZ, distanceZ);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated distanceZ-Value: " << distanceZ << " is 'nan'! currentSec: "
                      << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogDistance3D == true) {
          distance3D = m_twoHitFilterBox.calcDist3D();
          if (std::isnan(distance3D) == false) {
            B2DEBUG(50, "2-hit-filter in event " << m_eventCounter << ": calculated distance3D-Value: " << distance3D <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::distance3D, distance3D);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated distance3D-Value: " << distance3D << " is 'nan'! currentSec: "
                      << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogNormedDistance3D == true) {
          normedDistance3D = m_twoHitFilterBox.calcNormedDist3D();
          if (std::isnan(normedDistance3D) == false) {
            B2DEBUG(50, "2-hit-filter in event " << m_eventCounter << ": calculated normedDistance3D-Value: " << normedDistance3D <<
                    " gets stored in sector " << thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::normedDistance3D, normedDistance3D);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated normedDistance3D-Value: " << normedDistance3D <<
                      " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogSlopeRZ == true) {
          slopeRZ = m_twoHitFilterBox.calcSlopeRZ();
          if (std::isnan(slopeRZ) == false) {
            B2DEBUG(50, "2-hit-filter in event " << m_eventCounter << ": calculated slopeRZ-Value: " << slopeRZ << " gets stored in sector " <<
                    thisSectorPos->second.getSectorID());
            thisSectorPos->second.addValue(friendSector, FilterID::slopeRZ, slopeRZ);
          } else {
            m_badFilterValueCtr++;
            B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated slopeRZ-Value: " << slopeRZ << " is 'nan'! currentSec: " <<
                      currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ");
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        //#### debugFilters:
        if (m_PARAMlogAlwaysTrue2Hit == true) {
          B2DEBUG(50, "2-hit-filterDEBUG in event " << m_eventCounter << ": calculated alwaysTrue2Hit-Value: " << 1 <<
                  " gets stored in sector " << thisSectorPos->second.getSectorID());
          thisSectorPos->second.addValue(friendSector, FilterID::alwaysTrue2Hit, 1);
        }
        if (m_PARAMlogAlwaysFalse2Hit == true) {
          B2DEBUG(50, "2-hit-filterDEBUG in event " << m_eventCounter << ": calculated alwaysFalse2Hit-Value: " << 0 <<
                  " gets stored in sector " << thisSectorPos->second.getSectorID());
          thisSectorPos->second.addValue(friendSector, FilterID::alwaysFalse2Hit, 0);
        }
        if (m_PARAMlogRandom2Hit == true) {
          B2DEBUG(50, "2-hit-filterDEBUG in event " << m_eventCounter << ": calculated random2Hit-Value: " << 0.5 << " gets stored in sector "
                  << thisSectorPos->second.getSectorID());
          thisSectorPos->second.addValue(friendSector, FilterID::random2Hit, 0.5);
        }
        ++it2HitsFilter; //important for 2hit-Filters: points to current hit of 2-hit-processes

      } else { firstrun = false; }
    } // looping through current tracklet
  } // looping through all tracklets

  if (m_PARAManalysisWriteToRoot == true /*and m_PARAMstoreExtraAnalysis == true */) {
    m_rootFilePtr->cd();
    m_treeEventWisePtr->Fill();
  }
  B2DEBUG(5, "SecMapTrainerWithSpacePointsModule - event " << m_eventCounter << ", calculations done!");
}



/// /// /// /// /// /// /// /// END RUN /// /// /// /// /// /// /// ///
void SecMapTrainerWithSpacePointsModule::endRun()
{
  B2INFO("~~~~~~~~~~~SecMapTrainerWithSpacePointsModule - end of endRun ~~~~~~~~~~");
}



/// /// /// /// /// /// /// /// TERMINATE /// /// /// /// /// /// /// ///
void SecMapTrainerWithSpacePointsModule::terminate()
{
  int totalTrackletCounter = 0;
  int totalHitCounter = 0;
  m_eventCounter++;
  B2INFO("~~~~~~~~~~~SecMapTrainerWithSpacePointsModule - terminate ~~~~~~~~~~");
  for (int i = 0; i < int(m_PARAMsecMapNames.size()); ++i) {
    B2INFO(" within " << m_eventCounter << " events we got " << m_trackletMomentumCounter.at(i) << " tracklets in the " <<
           m_PARAMsecMapNames.at(i) << " setup");
  }
  for (int i = 0; i < int(m_trackletLengthCounter.size()); i++) {
//    if ( m_trackletLengthCounter.at(i) == 0 ) { continue; }
    B2INFO(" within " << m_eventCounter << " events we got " << m_trackletLengthCounter.at(i) << " tracklets containing " << i + 1 <<
           " hits");
    totalTrackletCounter += m_trackletLengthCounter.at(i);
    totalHitCounter += m_trackletLengthCounter.at(i) * (i + 1);
  }
  B2INFO(m_badHitsCounter << " hits had to be discarded because of double impact in same sensor having same direction of flight");
  B2INFO(m_badTrackletCounter <<
         " tracklets had to be discarded because of crazy flight (forward and backward movement all the time)");

  B2INFO(" there were " << float(totalTrackletCounter) / float(m_eventCounter) << "/" << float(m_telHitCounter) / float(
           m_eventCounter) << "/" << float(m_pxdHitCounter) / float(m_eventCounter) << "/" << float(m_svdHitCounter) / float(
           m_eventCounter) << " tracklets/telHits/pxdHits/svdHits per event and " << m_totalHitCounter << " hits total");
  B2INFO(" there were " << m_longTrackCounter << " Tracks having more than " << m_numOfLayers * 2 << " hits...");
  B2INFO(" there were " << m_longTrackletCounter << " Tracklets having more than " << m_numOfLayers * 2 << " hits!!!");
  B2INFO(m_badFilterValueCtr << " times, a filter produced invalid results ('nan')");


  /// ~~~~~~~~~~~~~~~~~~~ exporting secMaps ~~~~~~~~~~~~~~~ ///


  int ctr = 0, smCtr = 0;

  SecMapVector rawSectorMapVector;

  for (InternalRawSectorMap* thisMap : m_sectorMaps) {
    int secMapSize = thisMap->size();
    ctr = 0;
    B2INFO("writing " << secMapSize << " entries of secmap " << m_PARAMsecMapNames.at(smCtr));

    VXDTFRawSecMapTypedef::StrippedRawSecMap rootSecMap;
    VXDTFRawSecMapTypedef::SectorDistancesMap
    distanceOfSectorsMap; // stores the secID in .first and the value for the distances in .second
    InternalRawSectorMap::MapOfSectors* secMap =  &thisMap->getSecMap();

    for (InternalRawSectorMap::SecMapEntry& thisEntry : *secMap) {
      if (secMapSize > 10) {
        if ((ctr % int(0.1 * float(secMapSize))) == 0 && secMapSize > 0) { // this check produces segfault if secMapSize < 10
          B2INFO("writing entry " << ctr << ": " << thisEntry.first);
        }
      } else { B2INFO("writing entry " << ctr << ": " << thisEntry.first); }

      // doing typeCheck: if (Class* check = dynamic_cast<Class*>(aPtr)) != NULL) then aPtr isOfType Class*
//      if ((Sector* derived = dynamic_cast<Sector*>(&thisEntry.second)) != NULL)
      if ((dynamic_cast<Sector*>(&thisEntry.second)) != NULL) {

        if (m_PARAMsecMapWriteToAscii == true) {
          thisEntry.second.exportFriends(m_PARAMsecMapNames.at(smCtr));
        }

        if (m_PARAMsecMapWriteToRoot == true) { /* stores all Sectors and a raw version of the data (no calculated cutoffs yet)*/
          rootSecMap.push_back({FullSecID(thisEntry.first).getFullSecID(), thisEntry.second.exportFriendsRoot()});
        }

        distanceOfSectorsMap.push_back({FullSecID(thisEntry.first).getFullSecID(), thisEntry.second.getDistance2Origin()});
        thisEntry.second.clearFriends();
        ctr++;
      } else {
        B2WARNING(" sector " << thisEntry.first << " is no sector!");
      }
    }

    stringstream sectorDistances;

    for (VXDTFRawSecMapTypedef::SectorDistance aValue : distanceOfSectorsMap) {
      sectorDistances << FullSecID(aValue.first) << "/" << aValue.second << "\n";
    }
    B2DEBUG(1, "the following sectors had the following distances to the chosen origin:\n" << sectorDistances.str());

    if (m_PARAMsecMapWriteToRoot == true and rootSecMap.size() != 0) {

      for (auto& aSector : rootSecMap) {  // looping over sectors (VXDTFRawSecMapTypedef::StrippedRawSecMap)
        B2DEBUG(10, "In Sector " << FullSecID(aSector.first) << " there are the following friends:");
        for (auto& afriend : aSector.second) {  // looping over friends
          B2DEBUG(10, "..." << FullSecID(afriend.first) << " with " << afriend.second.size() << " cutoffTypes");
        }
      }
      // fill in data:
      VXDTFRawSecMap newTemporarySecMap;
      newTemporarySecMap.addSectorMap(rootSecMap);
      newTemporarySecMap.setMapName(m_PARAMsecMapNames.at(smCtr));
      newTemporarySecMap.setDetectorType(m_detectorName);
      newTemporarySecMap.setSectorConfigU(m_PARAMsectorConfigU);
      newTemporarySecMap.setSectorConfigV(m_PARAMsectorConfigV);
      newTemporarySecMap.setOrigin(m_origin);
      newTemporarySecMap.setMagneticFieldStrength(m_PARAMmagneticFieldStrength);
      newTemporarySecMap.setLowerMomentumThreshold(m_PARAMpTcuts.at(smCtr));
      newTemporarySecMap.addDistances(distanceOfSectorsMap);
      newTemporarySecMap.setMinDistance2origin(m_PARAMacceptedRegionForSensors.at(0));
      newTemporarySecMap.setMaxDistance2origin(m_PARAMacceptedRegionForSensors.at(1));
//    newTemporarySecMap.setRareSectorCombinations({m_PARMfilterRareCombinations, m_PARAMrarenessFilter}); /// TODO, implement here, check if correctly used
      if (smCtr + 1 > int(m_PARAMpTcuts.size())) {
        newTemporarySecMap.setHigherMomentumThreshold(std::numeric_limits<float>::max());
      } else {
        newTemporarySecMap.setHigherMomentumThreshold(m_PARAMpTcuts.at(smCtr));
      }

      SecMapVector::MapPack newMapPack = {m_PARAMsecMapNames.at(smCtr), newTemporarySecMap};
      rawSectorMapVector.push_back(newMapPack);

      stringstream info;
      info << " there are " << rawSectorMapVector.size() <<
           " sectorMaps stored in the rootOutput-container. These maps have got (sectors/friends/values) ";
      for (auto aMap : rawSectorMapVector.getFullVector()) {
        info << aMap.second.size() << "/" <<  aMap.second.getNumOfFriends() << "/" << aMap.second.getNumOfValues() << ", ";
      }
      B2INFO(info.str() << " Entries.");
    }

    thisMap->clear();
    smCtr++;
  }
  m_sectorMaps.clear();

  if (m_PARAManalysisWriteToRoot == true and m_treeEventWisePtr != NULL) {

    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module

    m_treeEventWisePtr->Write("",
                              TObject::kOverwrite); // WARNING TODO to check whether kOverwrite does make sense (use analysisWriteToRoot = true, then check, if number of entries are growing when executed several times with same parameters)!

    m_rootFilePtr->Close();
  }

  if (m_PARAMsecMapWriteToRoot == true) {
    string fileNameOnly = m_PARAMrootFileName.at(0) + "SecMap.root";
    TFile secMapFile(fileNameOnly.c_str(), m_PARAMrootFileName.at(1).c_str());
    rawSectorMapVector.Write();
    secMapFile.Close();
    B2INFO(" SecMapTrainerWithSpacePointsModule::endRun: exporting secMaps via " << fileNameOnly);
  }



  for (InternalRawSectorMap* secMap : m_sectorMaps) {  //secMaps can be deleted
    delete secMap;
  }
  m_sectorMaps.clear();
  B2INFO(" SecMapTrainerWithSpacePointsModule, everything is done. Terminating.");
}





bool SecMapTrainerWithSpacePointsModule::acceptHit(const SpacePoint* aSP, SecMapTrainerWithSPNames::InternalRawSectorMap* secMap)
{
  // catch hits on layers too high
  if (VxdID(aSP->getVxdID()).getLayerNumber() > secMap->getHighestAllowedLayer()) {
    B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was rejected due to being on a Layer too high: " << VxdID(
              aSP->getVxdID()).getLayerNumber());
    return false;
  }

  // catch hits which are on wrong detector
  Belle2::VXD::SensorInfoBase::SensorType detectorType = aSP->getType();
  if (detectorType == Belle2::VXD::SensorInfoBase::SensorType::PXD and secMap->usePXD() == false) {
    B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was rejected due to bad detector type PXD");
    return false;
  }
  if (detectorType == Belle2::VXD::SensorInfoBase::SensorType::SVD and secMap->useSVD() == false) {
    B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was rejected due to bad detector type SVD");
    return false;
  }
  if (detectorType == Belle2::VXD::SensorInfoBase::SensorType::TEL and secMap->useTEL() == false) {
    B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was rejected due to bad detector type TEL");
    return false;
  }

  // catch hits which are out of range
  if (secMap->getAcceptedRegionForSensors().first > 0) {
    if ((aSP->getPosition() - m_origin).Mag() < secMap->getAcceptedRegionForSensors().first) {
      B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was rejected due to bad hit position (below lower threshold)");
      return false;
    }
  }
  if (secMap->getAcceptedRegionForSensors().second > 0) {
    if ((aSP->getPosition() - m_origin).Mag() > secMap->getAcceptedRegionForSensors().second) {
      B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was rejected due to bad hit position (above upper threshold)");
      return false;
    }
  }
  // passed all tests
  B2DEBUG(25, "acceptHit: SP " << aSP->getArrayIndex() << " was accepted by secMap " << secMap->getName());
  return true;
} /**< for given hit and sectorMap, the function returns true, if hit is accepted and false if not */





bool SecMapTrainerWithSpacePointsModule::checkAcceptanceOfSecMap(SecMapTrainerWithSPNames::InternalRawSectorMap* secMap,
    const SpacePointTrackCand* currentTC)
{

  // catch wrong pT-range
  if (secMap->acceptPt(currentTC->getMomSeed().Perp()) == false) {
    B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " (with nhits " <<
            currentTC->getNHits() << ") was rejected due to wrong pT");
    return false;
  }

  // catch wrong charge
  if (m_filterCharges != 0) {
    if (std::abs(currentTC->getPdgCode()) > 10 and std::abs(currentTC->getPdgCode()) < 16) { // catch leptons
      if (boost::math::sign(currentTC->getPdgCode()) == boost::math::sign(m_filterCharges)) {
        B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was rejected due to wrong lepton charge");
        return false;
      }
    } else {
      if (boost::math::sign(currentTC->getPdgCode()) != boost::math::sign(m_filterCharges)) {
        B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was rejected due to wrong hadron charge");
        return false;
      }
    }
  }

  // catch TCs where more than one hit was on the same sensor
  if (currentTC->hasHitsOnSameSensor()) {
    B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was rejected due having hits on same sensor.");
    return false;
  }

  // catch tracks which start too far away from orign
  if (currentTC->getPosSeed().Perp() > m_PARAMmaxXYvertexDistance
      or
      std::abs(currentTC->getPosSeed().Z()) > m_PARAMmaxZvertexDistance) {
    B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was rejected due to bad seed position");
    return false;
  }

  // catch tracks which are too short in any case
  if (currentTC->getNHits() < m_PARAMminTrackletLength) {
    B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was rejected due to having not enough hits (" <<
            currentTC->getNHits() << ")");
    return false;
  }

  // catch stuff which depends on single hits
  unsigned int nGoodHits = 0;
  for (const SpacePoint* aSP : currentTC->getHits()) {
    if (acceptHit(aSP, secMap)) nGoodHits++; // pass all tests
  }

  // catch tracks which are too short because of hit-specific cuts
  if (nGoodHits < m_PARAMminTrackletLength) {
    B2DEBUG(20, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was rejected due to having not enough _good_ hits ("
            <<
            nGoodHits << ")");
    return false;
  }

  // pass all tests
  B2DEBUG(25, "checkAcceptanceOfSecMap: TC " << currentTC->getArrayIndex() << " was accepted by secMap " << secMap->getName());
  return true;
} /**< can be accepted by several secMaps, because of momentum range or whatever  */





SecMapTrainerWithSPNames::VXDTrack SecMapTrainerWithSpacePointsModule::convertSPTC2VXDTrack(
  SecMapTrainerWithSPNames::InternalRawSectorMap* secMap,
  const SpacePointTrackCand*  currentTC,
  unsigned tcID)
{
  VXDTrack newTrack(tcID, currentTC->getMomSeed().Perp(), &secMap->getSecMap());

  // collect hits which fullfill all given tests
  std::vector<const SpacePoint*> goodSPs;
  for (const SpacePoint* aSP : currentTC->getHits()) {
    if (!acceptHit(aSP, secMap)) continue;
    goodSPs.push_back(aSP);
  }

  // want to have hits going from outer to inner ones
  if (currentTC->isOutgoing()) std::reverse(goodSPs.begin(), goodSPs.end());

  for (const SpacePoint* aSP : goodSPs) {

    std::string fullSecID = calcSecID(aSP, secMap);

    if (fullSecID == string("-1")) { B2ERROR("a secID for spacePoint not found!"); continue; }

    newTrack.addHit(
      VXDHit(aSP->getType(),
             fullSecID,
             aSP->getVxdID(),
             aSP->getPosition(),
             TVector3(0, 0, 0),
             currentTC->getPdgCode(),
             getOrigin()
            )
    );

    if (m_PARAManalysisWriteToRoot == true /*and m_PARAMstoreExtraAnalysis == true*/) {
      if (FullSecID(fullSecID).getLayerID() == 1) {
        m_rootmomValuesInLayer1.push_back(currentTC->getMomSeed().Mag());
        m_rootpTValuesInLayer1.push_back(currentTC->getMomSeed().Perp());
      } // optional TODO here for the other layers too, if anyone wants to analyze that
    }
  }

  if (newTrack.size() > m_numOfLayers * 2 + 1) { m_longTrackletCounter++; } else { m_trackletLengthCounter.at(newTrack.size() - 1)++; }

  // add vertex (but without real vertexPosition, since origin is assumed)
  VXDHit newVirtualHit(Const::IR, FullSecID().getFullSecString(), 0, getOrigin(), currentTC->getMomSeed(), currentTC->getPdgCode(),
                       getOrigin(), 0);
  newVirtualHit.setVertex();

  newTrack.addHit(newVirtualHit);

  return newTrack;
} /**< converts to internal data structure and attaches it to given secMap */





std::string SecMapTrainerWithSpacePointsModule::calcSecID(const SpacePoint* aSP,
                                                          SecMapTrainerWithSPNames::InternalRawSectorMap* secMap)
{
  VxdID thisSensor = aSP->getVxdID();
  const VXD::SensorInfoBase& aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(thisSensor);
  std::string aSectorName;
  unsigned int aSecID = 0;
  float dist2Origin = 0;
  VxdID::baseType aUniID = thisSensor.getID();
  int aLayerID = thisSensor.getLayerNumber();
  SectorTools::NormCoords aCoorNormalized = {aSP->getNormalizedLocalU(), aSP->getNormalizedLocalV()};
  SectorTools::NormCoords aCornerCoordinate;
  SectorTools::NormCoords aRelCoordCorner1 = {0, 0};
  SectorTools::NormCoords aRelCoordCorner2 = {0, 1};
  SectorTools::NormCoords aRelCoordCorner3 = {1, 0};
  SectorTools::NormCoords aRelCoordCorner4 = {1, 1};
  SectorTools::NormCoords aRelCoordCenter = {0.5, 0.5};

  // Calculate the SectorID (SectorTool-Object)
  aSecID = SectorTools::calcSecID(m_PARAMsectorConfigU, m_PARAMsectorConfigV, aCoorNormalized);

  if (aSecID == std::numeric_limits<unsigned short>::max()) { return "-1"; } // equals to false

  aSectorName = (boost::format("%1%_%2%_%3%") % aLayerID % aUniID % aSecID).str();


  if (secMap->find(aSectorName) == secMap->end()) {
    // 1. Corner Calculate
    aCornerCoordinate = SectorTools::calcNormalizedSectorPoint(m_PARAMsectorConfigU, m_PARAMsectorConfigV, aSecID, aRelCoordCorner1);
    SectorTools::NormCoords localCorner00 = SpacePoint::convertNormalizedToLocalCoordinates(aCornerCoordinate, aUniID);

    // 2. Corner Calculate
    aCornerCoordinate = SectorTools::calcNormalizedSectorPoint(m_PARAMsectorConfigU, m_PARAMsectorConfigV, aSecID, aRelCoordCorner2);
    SectorTools::NormCoords localCorner01 = SpacePoint::convertNormalizedToLocalCoordinates(aCornerCoordinate, aUniID);

    // 3. Corner Calculate
    aCornerCoordinate = SectorTools::calcNormalizedSectorPoint(m_PARAMsectorConfigU, m_PARAMsectorConfigV, aSecID, aRelCoordCorner3);
    SectorTools::NormCoords localCorner10 = SpacePoint::convertNormalizedToLocalCoordinates(aCornerCoordinate, aUniID);

    // 4. Corner Calculate
    aCornerCoordinate = SectorTools::calcNormalizedSectorPoint(m_PARAMsectorConfigU, m_PARAMsectorConfigV, aSecID, aRelCoordCorner4);
    SectorTools::NormCoords localCorner11 = SpacePoint::convertNormalizedToLocalCoordinates(aCornerCoordinate, aUniID);

    // Center
    aCornerCoordinate = SectorTools::calcNormalizedSectorPoint(m_PARAMsectorConfigU, m_PARAMsectorConfigV, aSecID, aRelCoordCenter);
    SectorTools::NormCoords aCenterOfSector = SpacePoint::convertNormalizedToLocalCoordinates(aCornerCoordinate, aUniID);

    B2DEBUG(75, "OOO SIZE: " << aSensorInfo.getUSize() << ", " << aSensorInfo.getVSize());
    B2DEBUG(75, "OOO Center normalized: " << aCornerCoordinate.first << ", " << aCornerCoordinate.second);
    B2DEBUG(75, "OOO Center real: " << localCorner11.first << ", " << localCorner11.second);

    B2DEBUG(75, "OOO I have found a SecID: " << aSectorName << " with centerU/V: " << aCenterOfSector.first << "/" <<
            aCenterOfSector.second << " for hit " << aCoorNormalized.first << "/" << aCoorNormalized.second);
    B2DEBUG(100, "OOO Sector " << aSectorName << " - edges: O(" << localCorner00.first << "," << localCorner00.second << "), U(" <<
            localCorner01.first << "," << localCorner01.second << "), V(" << localCorner10.first << "," << localCorner10.second << "), UV(" <<
            localCorner11.first << "," << localCorner11.second << "), centerU/V: " << aCenterOfSector.first << "/" << aCenterOfSector.second);


    dist2Origin = (aSensorInfo.pointToGlobal(TVector3(aCenterOfSector.first, aCenterOfSector.second, 0.)) - m_origin).Mag();

    Sector newSector(aSectorName, localCorner00, localCorner10, localCorner01, localCorner11, dist2Origin);

    secMap->insert({aSectorName, newSector});
  } else {
    secMap->find(aSectorName)->second.increaseCounter();
  }

  return aSectorName;
} /**< for given hit and secMap, the correct secID is calculated */
