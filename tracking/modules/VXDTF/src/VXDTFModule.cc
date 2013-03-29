/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/VXDTF/VXDTFModule.h"

// framework
#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Const.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <tracking/gfbfield/GFGeant4Field.h>


//genfit

#include <GFTrack.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFAbsTrackRep.h>
#include <RKTrackRep.h>
#include <GFFieldManager.h>
#include <GFConstField.h>
#include <GFTools.h>
#include <GFKalman.h>
#include <GFMaterialEffects.h>
#include <GFTGeoMaterialInterface.h>


//root packages:
#include <TMatrixT.h>
#include <TMatrixD.h>
#include <TGeoManager.h>
#include <TMath.h>

//STL or C/C++-packages;
#include <algorithm>
#include <functional>
#include <utility>
#include <math.h>
#include <time.h>

//Boost-packages:
#include <boost/foreach.hpp>
#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp> // needed for is_any_of
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/sign.hpp>
// #include <boost/chrono/chrono_io.hpp>





using namespace std;
using namespace Belle2;
using boost::format;
using boost::lexical_cast;
using boost::math::sign;
using boost::bind;
// using boost::chrono::high_resolution_clock;


// random generator function:
ptrdiff_t rngWrapper(ptrdiff_t i) { return gRandom->Integer(i); }
// pointer object to it:
ptrdiff_t (*p_rngWrapper)(ptrdiff_t) = rngWrapper;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDTF)


VXDTFModule::VXDTFModule() : Module()
{
  /// setting standard values for steering parameters
  std::vector<double> defaultConfigU; // sector sizes
  std::vector<double> defaultConfigV;
  defaultConfigU.push_back(0.0);
  defaultConfigU.push_back(0.5);
  defaultConfigU.push_back(1.0);
  defaultConfigV.push_back(0.0);
  defaultConfigV.push_back(0.33);
  defaultConfigV.push_back(0.67);
  defaultConfigV.push_back(1.0);

  std::vector<std::string> sectorSetup;
  sectorSetup.push_back("std");

  vector<int> detectorType, highestAllowedLayer, minLayer, minState;
  detectorType.push_back(1); // TODO does not work with Const::SVD/PXD yet since there is no Const::VXD which would be needed for the TF
  highestAllowedLayer.push_back(6);
  minLayer.push_back(4);
  minState.push_back(2);

  vector<double> setupWeigh;
  setupWeigh.push_back(0.0);

  vector<bool> activateDistance3D, activateDistanceXY, activateDistanceZ, activateSlopeRZ, activateNormedDistance3D, activateAngles3D, activateAnglesXY, activateAnglesRZ, activateDeltaSlopeRZ, activateDistance2IP, activatePT, activateHelixFit, activateAngles3DHioC, activateAnglesXYHioC, activateAnglesRZHioC, activateDeltaSlopeRZHioC, activateDistance2IPHioC, activatePTHioC, activateHelixFitHioC, activateZigZagXY, activateZigZagRZ, activateDeltaPt, activateDeltaDistance2IP, activateCircleFit, activateDeltaPtHioC, activateDeltaDistance2IPHioC;
  activateDistance3D.push_back(true);
  activateDistanceXY.push_back(true);
  activateDistanceZ.push_back(true);
  activateSlopeRZ.push_back(true);
  activateNormedDistance3D.push_back(true);
  activateAngles3D.push_back(true);
  activateAnglesXY.push_back(true);
  activateAnglesRZ.push_back(true);
  activateDeltaSlopeRZ.push_back(true);
  activatePT.push_back(true);
  activateHelixFit.push_back(true);
  activateDistance2IP.push_back(true);
  activateZigZagXY.push_back(true);
  activateZigZagRZ.push_back(true);
  activateDeltaPt.push_back(true);
  activateDeltaDistance2IP.push_back(true);
  activateCircleFit.push_back(true);
  activateAngles3DHioC.push_back(true);
  activateAnglesXYHioC.push_back(true);
  activateAnglesRZHioC.push_back(true);
  activateDeltaSlopeRZHioC.push_back(true);
  activatePTHioC.push_back(true);
  activateHelixFitHioC.push_back(true);
  activateDistance2IPHioC.push_back(true);
  activateDeltaPtHioC.push_back(true);
  activateDeltaDistance2IPHioC.push_back(true);
  vector<double> tuneDistance3D, tuneDistanceXY, tuneDistanceZ, tuneSlopeRZ, tuneNormedDistance3D, tuneAngles3D, tuneAnglesXY, tuneAnglesRZ, tuneDeltaSlopeRZ, tuneDistance2IP, tunePT, tuneHelixFit, tuneAngles3DHioC, tuneAnglesXYHioC, tuneAnglesRZHioC, tuneDeltaSlopeRZHioC, tuneDistance2IPHioC, tunePTHioC, tuneHelixFitHioC, tuneZigZagXY, tuneZigZagRZ, tuneDeltaPt, tuneDeltaDistance2IP, tuneCircleFit, tuneDeltaPtHioC, tuneDeltaDistance2IPHioC;
  tuneDistance3D.push_back(0);
  tuneDistanceXY.push_back(0);
  tuneDistanceZ.push_back(0);
  tuneSlopeRZ.push_back(0);
  tuneNormedDistance3D.push_back(0);
  tuneAngles3D.push_back(0);
  tuneAnglesXY.push_back(0);
  tuneAnglesRZ.push_back(0);
  tuneDeltaSlopeRZ.push_back(0);
  tuneDistance2IP.push_back(0);
  tunePT.push_back(0);
  tuneHelixFit.push_back(0);
  tuneZigZagXY.push_back(0);
  tuneZigZagRZ.push_back(0);
  tuneDeltaPt.push_back(0);
  tuneDeltaDistance2IP.push_back(0);
  tuneCircleFit.push_back(50); // chi2-threshold
  tuneAngles3DHioC.push_back(0);
  tuneAnglesXYHioC.push_back(0);
  tuneAnglesRZHioC.push_back(0);
  tuneDeltaSlopeRZHioC.push_back(0);
  tuneDistance2IPHioC.push_back(0);
  tunePTHioC.push_back(0);
  tuneHelixFitHioC.push_back(0);
  tuneDeltaPtHioC.push_back(0);
  tuneDeltaDistance2IPHioC.push_back(0);

  //Set module properties
  setDescription(" trackfinder for the SVD using cellular automaton techniques, kalman filter (genfit) and a hopfield network as well.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  ///Steering parameter import

  addParam("debugMode", m_PARAMDebugMode, "some code will only be executed if this mode is enabled", bool(false));
  addParam("sectorConfigU", m_PARAMsectorConfigU, "allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigU);
  addParam("sectorConfigV", m_PARAMsectorConfigV, "allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigV);

  addParam("detectorType", m_PARAMdetectorType, "defines which detector type has to be exported. VXD: -1, PXD: 0, SVD: 1", detectorType);
  addParam("sectorSetup", m_PARAMsectorSetup, "lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row", sectorSetup);

  addParam("tuneCutoffs", m_PARAMtuneCutoffs, "for rapid changes of all cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -99% < x < +1000%", double(0.0));
  addParam("GFTrackCandidatesColName", m_PARAMgfTrackCandsColName, "Name of collection holding the GFTrackCandidates (output)", string(""));

  addParam("setupWeigh", m_PARAMsetupWeigh, " allows to individually weigh (level of surpression) each setup/pass. Please choose value between 0 and 100 (%), higher momentum setups are more reliable, therefore should be weighed best", setupWeigh);

  addParam("activateDistance3D", m_PARAMactivateDistance3D, " set True/False for each setup individually", activateDistance3D);
  addParam("activateDistanceXY", m_PARAMactivateDistanceXY, " set True/False for each setup individually", activateDistanceXY);
  addParam("activateDistanceZ", m_PARAMactivateDistanceZ, " set True/False for each setup individually", activateDistanceZ);
  addParam("activateSlopeRZ", m_PARAMactivateSlopeRZ, " set True/False for each setup individually", activateSlopeRZ);
  addParam("activateNormedDistance3D", m_PARAMactivateNormedDistance3D, " set True/False for each setup individually", activateNormedDistance3D);
  addParam("activateAngles3DHioC", m_PARAMactivateAngles3DHioC, " set True/False for each setup individually", activateAngles3DHioC);
  addParam("activateAnglesXYHioC", m_PARAMactivateAnglesXYHioC, " set True/False for each setup individually", activateAnglesXYHioC);
  addParam("activateAnglesRZHioC", m_PARAMactivateAnglesRZHioC, " set True/False for each setup individually", activateAnglesRZHioC);
  addParam("activateDeltaSlopeRZHioC", m_PARAMactivateDeltaSlopeRZHioC, " set True/False for each setup individually", activateDeltaSlopeRZHioC);
  addParam("activateDistance2IPHioC", m_PARAMactivateDistance2IPHioC, " set True/False for each setup individually", activateDistance2IPHioC);
  addParam("activatePTHioC", m_PARAMactivatePTHioC, " set True/False for each setup individually", activatePTHioC);
  addParam("activateHelixFitHioC", m_PARAMactivateHelixFitHioC, " set True/False for each setup individually", activateHelixFitHioC);
  addParam("tuneAngles3DHioC", m_PARAMtuneAngles3DHioC, " tune for each setup individually, in %", tuneAngles3DHioC);
  addParam("tuneAnglesXYHioC", m_PARAMtuneAnglesXYHioC, " tune for each setup individually, in %", tuneAnglesXYHioC);
  addParam("tuneAnglesRZHioC", m_PARAMtuneAnglesRZHioC, " tune for each setup individually, in %", tuneAnglesRZHioC);
  addParam("tuneDeltaSlopeRZHioC", m_PARAMtuneDeltaSlopeRZHioC, " tune for each setup individually, in %", tuneDeltaSlopeRZHioC);
  addParam("tuneDistance2IPHioC", m_PARAMtuneDistance2IPHioC, " tune for each setup individually, in %", tuneDistance2IPHioC);
  addParam("tunePTHioC", m_PARAMtunePTHioC, " tune for each setup individually, in %", tunePTHioC);
  addParam("tuneHelixFitHioC", m_PARAMtuneHelixFitHioC, " tune for each setup individually, in %", tuneHelixFitHioC);
  addParam("activateDeltaPtHioC", m_PARAMactivateDeltaPtHioC, " set True/False for each setup individually", activateDeltaPtHioC);
  addParam("activateDeltaDistance2IPHioC", m_PARAMactivateDeltaDistance2IPHioC, " set True/False for each setup individually", activateDeltaDistance2IPHioC);
  addParam("tuneDeltaPtHioC", m_PARAMtuneDeltaPtHioC, " tune for each setup individually, in %", tuneDeltaPtHioC);
  addParam("tuneDeltaDistance2IPHioC", m_PARAMtuneDeltaDistance2IPHioC, " tune for each setup individually, in %", tuneDeltaDistance2IPHioC);


  addParam("activateAngles3D", m_PARAMactivateAngles3D, " set True/False for each setup individually", activateAngles3D);
  addParam("activateAnglesXY", m_PARAMactivateAnglesXY, " set True/False for each setup individually", activateAnglesXY);
  addParam("activateAnglesRZ", m_PARAMactivateAnglesRZ, " set True/False for each setup individually", activateAnglesRZ);
  addParam("activateDeltaSlopeRZ", m_PARAMactivateDeltaSlopeRZ, " set True/False for each setup individually", activateDeltaSlopeRZ);
  addParam("activateDistance2IP", m_PARAMactivateDistance2IP, " set True/False for each setup individually", activateDistance2IP);
  addParam("activatePT", m_PARAMactivatePT, " set True/False for each setup individually", activatePT);
  addParam("activateHelixFit", m_PARAMactivateHelixFit, " set True/False for each setup individually", activateHelixFit);

  addParam("activateZigZagXY", m_PARAMactivateZigZagXY, " set True/False for each setup individually", activateZigZagXY);
  addParam("activateZigZagRZ", m_PARAMactivateZigZagRZ, " set True/False for each setup individually", activateZigZagRZ);
  addParam("activateDeltaPt", m_PARAMactivateDeltaPt, " set True/False for each setup individually", activateDeltaPt);
  addParam("activateDeltaDistance2IP", m_PARAMactivateDeltaDistance2IP, " set True/False for each setup individually", activateDeltaDistance2IP);
  addParam("activateCircleFit", m_PARAMactivateCircleFit, " set True/False for each setup individually", activateCircleFit);



  addParam("tuneDistance3D", m_PARAMtuneDistance3D, " tune for each setup individually in %", tuneDistance3D);
  addParam("tuneDistanceXY", m_PARAMtuneDistanceXY, " tune for each setup individually, in %", tuneDistanceXY);
  addParam("tuneDistanceZ", m_PARAMtuneDistanceZ, " tune for each setup individually, in %", tuneDistanceZ);
  addParam("tuneSlopeRZ", m_PARAMtuneSlopeRZ, " tune for each setup individually, in %", tuneSlopeRZ);
  addParam("tuneNormedDistance3D", m_PARAMtuneNormedDistance3D, " tune for each setup individually, in %", tuneNormedDistance3D);

  addParam("tuneAngles3D", m_PARAMtuneAngles3D, " tune for each setup individually, in %", tuneAngles3D);
  addParam("tuneAnglesXY", m_PARAMtuneAnglesXY, " tune for each setup individually, in %", tuneAnglesXY);
  addParam("tuneAnglesRZ", m_PARAMtuneAnglesRZ, " tune for each setup individually, in %", tuneAnglesRZ);
  addParam("tuneDeltaSlopeRZ", m_PARAMtuneDeltaSlopeRZ, " tune for each setup individually, in %", tuneDeltaSlopeRZ);
  addParam("tuneDistance2IP", m_PARAMtuneDistance2IP, " tune for each setup individually, in %", tuneDistance2IP);
  addParam("tunePT", m_PARAMtunePT, " tune for each setup individually, in %", tunePT);
  addParam("tuneHelixFit", m_PARAMtuneHelixFit, " tune for each setup individually, in %", tuneHelixFit);

  addParam("tuneZigZagXY", m_PARAMtuneZigZagXY, " currently not in use, only here for symmetrical reasons", tuneZigZagXY);
  addParam("tuneZigZagRZ", m_PARAMtuneZigZagRZ, " currently not in use, only here for symmetrical reasons", tuneZigZagRZ);
  addParam("tuneDeltaPt", m_PARAMtuneDeltaPt, " tune for each setup individually, in %", tuneDeltaPt);
  addParam("tuneDeltaDistance2IP", m_PARAMtuneDeltaDistance2IP, " tune for each setup individually, in %", tuneDeltaDistance2IP);
  addParam("tuneCircleFit", m_PARAMtuneCircleFit, " set threshold for each setup individually", tuneCircleFit);


  addParam("highOccupancyThreshold", m_PARAMhighOccupancyThreshold, "", int(17));
  addParam("tccMinLayer", m_PARAMminLayer, "determines lowest layer considered by track candidate collector", minLayer);
  addParam("tccMinState", m_PARAMminState, "determines lowest state of cells considered by track candidate collector", minState);
  addParam("omega", m_PARAMomega, "tuning parameter for the hopfield network", double(0.5));

  //for testing purposes:
  addParam("highestAllowedLayer", m_PARAMhighestAllowedLayer, "set value below 6 if you want to exclude outer layers (standard is 6)", highestAllowedLayer);
  addParam("standardPdgCode", m_PARAMpdGCode, "standard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons)", int(211));

  addParam("cleanOverlappingSet", m_PARAMcleanOverlappingSet, "when true, TCs which are found more than once (possible because of multipass) will get filtered", bool(true));
  addParam("useHopfield", m_PARAMuseHopfield, "allows to deactivate hopfield, so overlapping TCs are exported", bool(true));

  addParam("qiSmear", m_PARAMqiSmear, " set True if you want to smear QI's of TCs (needed when no Kalman filter activated) ", bool(true));
  addParam("smearMean", m_PARAMsmearMean, " when qiSmear = True, bias of perturbation can be set here", double(0.0));
  addParam("smearSigma", m_PARAMsmearSigma, " when qiSmear = True, degree of perturbation can be set here", double(0.0001));

  addParam("calcQIType", m_PARAMcalcQIType, "allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength', 'circleFit'", string("kalman"));
  addParam("storeBrokenQI", m_PARAMstoreBrokenQI, "if true, TC survives QI-calculation-process even if fit was not possible", bool(true));
  addParam("KFBackwardFilter", m_KFBackwardFilter, "determines whether the kalman filter moves inwards or backwards, 'True' means inwards", bool(false));
  addParam("TESTERexpandedTestingRoutines", m_TESTERexpandedTestingRoutines, "set true if you want to export expanded infos of TCs for further analysis", bool(true));


  /// temporarily disabled (maybe used later)
  //  addParam("activateQQQMode", m_activateQQQMode, " set True to calc QQQ-values for TCs", bool(false));
}


VXDTFModule::~VXDTFModule()
{

}

void VXDTFModule::initialize()
{
  m_littleHelperBox.resetValues(m_PARAMsmearMean, m_PARAMsmearSigma);
  /// constants:
  m_CONSTaThird = 1. / 3.;
  if (m_PARAMhighOccupancyThreshold < 0) { m_PARAMhighOccupancyThreshold = 0; }

  if (m_PARAMpdGCode > 10 and m_PARAMpdGCode < 18) { // in this case, its a lepton. since leptons with positive sign have got negative codes, this must taken into notice
    m_chargeSignFactor = 1;
  } else { m_chargeSignFactor = -1; }

  // check misusage of parameters:
  if (m_PARAMtuneCutoffs <= -99.0 or m_PARAMtuneCutoffs > 1000.0) {
    B2WARNING("VXDTF: chosen value for parameter 'tuneCutoffs' is invalid, reseting value to standard (=0.0)...")
    m_PARAMtuneCutoffs = 0.;
  } else {
    m_PARAMtuneCutoffs = m_PARAMtuneCutoffs * 0.01; // reformatting to faster calculation
  }

  /// GFTrackCandidate
  StoreArray<GFTrackCand>::registerPersistent(m_PARAMgfTrackCandsColName);

  if (gGeoManager == NULL) {
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
  }
  GFMaterialEffects::getInstance()->setMscModel("Highland");

/// TODO: further checks for validity needed!

  /// temporary members for testing purposes (minimal testing routines)
  if (m_TESTERexpandedTestingRoutines == true) {
    StoreArray<VXDTFInfoBoard>::registerPersistent();
  }

  StoreArray<PXDCluster>::optional();
  StoreArray<SVDCluster>::optional();

  m_TESTERtriggeredZigZagXY = 0;
  m_TESTERtriggeredZigZagRZ = 0;
  m_TESTERtriggeredDpT = 0;
  m_TESTERapprovedByTCC = 0;
  m_TESTERcountTotalTCsAfterTCC = 0;
  m_TESTERcountTotalTCsAfterTCCFilter = 0;
  m_TESTERcountTotalTCsFinal = 0;
  m_TESTERbadHopfieldCtr = 0;
  m_TESTERHopfieldLetsOverbookedTCsAliveCtr = 0;
  m_TESTERfilteredOverlapsQI = 0;
  m_TESTERNotFilteredOverlapsQI = 0;
  m_TESTERfilteredOverlapsQICtr = 0;
  m_TESTERcleanOverlappingSetStartedCtr = 0;
  m_TESTERgoodFitsCtr = 0;
  m_TESTERbadFitsCtr = 0;
  m_TESTERbrokenEventsCtr = 0;
  m_TESTERfilteredBadSeedTCs = 0;
  m_TESTERdistortedHitCtr = 0;
  m_TESTERtotalsegmentsSFCtr = 0;
  m_TESTERtotalsegmentsNFCtr = 0;
  m_TESTERdiscardedSegmentsSFCtr = 0;
  m_TESTERdiscardedSegmentsNFCtr = 0;
  m_TESTERbrokenCaRound = 0;
  m_TESTERkalmanSkipped = 0;

}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *******************************+  beginRun +******************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void VXDTFModule::beginRun()
{
  B2DEBUG(1, "################## entering VXD CA track finder - beginRun ######################");
  B2DEBUG(50, "##### be careful, current TF status does not support more than one run per initialization! #####");


  /// for each setup, fill parameters, calc numTotalLayers... TODO: failsafe implementation (no protection against bad user imputs) lacks of style, longterm goal, export that procedure into a function
  int totalFriendCounter;
  m_usePXDorSVDorVXDhits = 0; /// needed, since in theory some passes could use SVD+PXD and some SVD-only. =0, when only PXDhits, = 1, when only svd-hits, =-1 when both cases occur
  m_numOfSectorSetups = m_PARAMsectorSetup.size();

  for (int i = 0; i < m_numOfSectorSetups; ++i) {

    CurrentPassData* newPass = new CurrentPassData;

    newPass->sectorSetup = m_PARAMsectorSetup[i];

    if (int (m_PARAMdetectorType.size()) < i + 1) {
      newPass->detectorType = m_PARAMdetectorType[m_PARAMdetectorType.size() - 1];
      B2WARNING("detectorType not set for each sectorMap, copying first choice")
    } else {
      newPass->detectorType = m_PARAMdetectorType[i];
    }
    if (int (m_PARAMsetupWeigh.size()) < i + 1) {
      B2WARNING("setupWeigh not set each sectorMap, copying first choice")
      newPass->setupWeigh = m_PARAMsetupWeigh[m_PARAMsetupWeigh.size() - 1] * 0.01;
    } else {
      newPass->setupWeigh = m_PARAMsetupWeigh[i] * 0.01;
      if (newPass->setupWeigh < 0) { newPass->setupWeigh = 0; } else if (newPass->setupWeigh > 1.) { newPass->setupWeigh = 1.; }
    }
    if (int (m_PARAMhighestAllowedLayer.size()) < i + 1) {
      B2WARNING("highestAllowedLayer not set each sectorMap, copying first choice")
      newPass->highestAllowedLayer = m_PARAMhighestAllowedLayer[m_PARAMhighestAllowedLayer.size() - 1];
    } else {
      newPass->highestAllowedLayer = m_PARAMhighestAllowedLayer[i];
    }
    if (int (m_PARAMminLayer.size()) < i + 1) {
      B2WARNING("minLayer not set each sectorMap, copying first choice")
      newPass->minLayer = m_PARAMminLayer[m_PARAMminLayer.size() - 1];
    } else {
      newPass->minLayer = m_PARAMminLayer[i];
    }
    if (int (m_PARAMminState.size()) < i + 1) {
      B2WARNING("minState not set each sectorMap, copying first choice")
      newPass->minState = m_PARAMminState[m_PARAMminState.size() - 1];
    } else {
      newPass->minState = m_PARAMminState[i];
    }


    // calc numtotalLayers and chosenDetectorType:
    if (newPass->detectorType == 0) {
      newPass->chosenDetectorType = "PXD";
      newPass->numTotalLayers = 2;
      newPass->minLayer = 2;
      newPass->minState = 1;
      if (m_usePXDorSVDorVXDhits == 1) { m_usePXDorSVDorVXDhits = -1; }   // cases 0,-1, don't do anything, since state is okay
    } else if (newPass->detectorType == 1) {
      newPass->chosenDetectorType = "SVD";
      newPass->numTotalLayers = 4;
      if (m_usePXDorSVDorVXDhits == 0) { m_usePXDorSVDorVXDhits = -1; }   // cases 1,-1, don't do anything, since state is okay
    } else if (newPass->detectorType == -1) {
      newPass->chosenDetectorType = "VXD";
      newPass->numTotalLayers = 6;
      if (m_usePXDorSVDorVXDhits not_eq - 1) { m_usePXDorSVDorVXDhits = -1; }  // cases -1, don't do anything, since state is okay
    } else {
      B2WARNING("Pass " << i << " with setting '" << m_PARAMsectorSetup[i] << "': chosen detectorType via param 'detectorType' is invalid, resetting value to standard (=VXD,-1)")
      newPass->chosenDetectorType = "VXD";
      newPass->detectorType = -1;
      newPass->numTotalLayers = 6;
    }
    newPass->numTotalLayers = newPass->numTotalLayers - (6 - newPass->highestAllowedLayer);
    if (newPass->numTotalLayers < 2) { newPass->numTotalLayers = 2; }   // to avoid division by zero in some cases
//    /*if (newPass->numtotalLayers > m_numtotalLayers ) { m_numtotalLayers = newPass->numtotalLayers; } // this value only stores the highest number of total layers*/
    B2INFO("Pass " << i << ": chosen detectorType: " << newPass->chosenDetectorType << " and chosen sectorSetup: " << newPass->sectorSetup)

    newPass->secConfigU = m_PARAMsectorConfigU; // currently same sectorConfig for each pass, future support of different setups prepared
    newPass->secConfigV = m_PARAMsectorConfigV; // currently same sectorConfig for each pass, future support of different setups prepared

    int sfCtr = 0, sfhoCtr = 0, nfCtr = 0, nfhoCtr = 0, tccfCtr = 0; // counting number of activated tests for each filter step
    ///sFinder:
    if (int (m_PARAMactivateDistance3D.size()) < i + 1) {
      newPass->distance3D.first = m_PARAMactivateDistance3D[m_PARAMactivateDistance3D.size() - 1];
    } else {
      newPass->distance3D.first = m_PARAMactivateDistance3D[i];
    }
    if (int (m_PARAMactivateDistanceXY.size()) < i + 1) {
      newPass->distanceXY.first = m_PARAMactivateDistanceXY[m_PARAMactivateDistanceXY.size() - 1];
    } else {
      newPass->distanceXY.first = m_PARAMactivateDistanceXY[i];;
    }
    if (int (m_PARAMactivateDistanceZ.size()) < i + 1) {
      newPass->distanceZ.first = m_PARAMactivateDistanceZ[m_PARAMactivateDistanceZ.size() - 1];
    } else {
      newPass->distanceZ.first = m_PARAMactivateDistanceZ[i];
    }
    if (int (m_PARAMactivateSlopeRZ.size()) < i + 1) {
      newPass->slopeRZ.first = m_PARAMactivateSlopeRZ[m_PARAMactivateSlopeRZ.size() - 1];
    } else {
      newPass->slopeRZ.first = m_PARAMactivateSlopeRZ[i];
    }
    if (int (m_PARAMactivateNormedDistance3D.size()) < i + 1) {
      newPass->normedDistance3D.first = m_PARAMactivateNormedDistance3D[m_PARAMactivateNormedDistance3D.size() - 1];
    } else {
      newPass->normedDistance3D.first = m_PARAMactivateNormedDistance3D[i];
    }
    if (newPass->distance3D.first == true) { sfCtr++; }
    if (newPass->distanceXY.first == true) { sfCtr++; }
    if (newPass->distanceZ.first == true) { sfCtr++; }
    if (newPass->slopeRZ.first == true) { sfCtr++; }
    if (newPass->normedDistance3D.first == true) { sfCtr++; }
    ///sFinder ho finder (2+1 hit)
    if (int (m_PARAMactivateAngles3DHioC.size()) < i + 1) {
      newPass->anglesHighOccupancy3D.first = m_PARAMactivateAngles3DHioC[m_PARAMactivateAngles3DHioC.size() - 1];
    } else {
      newPass->anglesHighOccupancy3D.first =  m_PARAMactivateAngles3DHioC[i];
    }
    if (int (m_PARAMactivateAnglesXYHioC.size()) < i + 1) {
      newPass->anglesHighOccupancyXY.first = m_PARAMactivateAnglesXYHioC[m_PARAMactivateAnglesXYHioC.size() - 1];
    } else {
      newPass->anglesHighOccupancyXY.first = m_PARAMactivateAnglesXYHioC[i];
    }
    if (int (m_PARAMactivateAnglesRZHioC.size()) < i + 1) {
      newPass->anglesHighOccupancyRZ.first = m_PARAMactivateAnglesRZHioC[m_PARAMactivateAnglesRZHioC.size() - 1];
    } else {
      newPass->anglesHighOccupancyRZ.first = m_PARAMactivateAnglesRZHioC[i];
    }
    if (int (m_PARAMactivateDeltaSlopeRZHioC.size()) < i + 1) {
      newPass->deltaSlopeHighOccupancyRZ.first = m_PARAMactivateDeltaSlopeRZHioC[m_PARAMactivateDeltaSlopeRZHioC.size() - 1];
    } else {
      newPass->deltaSlopeHighOccupancyRZ.first = m_PARAMactivateDeltaSlopeRZHioC[i];
    }
    if (int (m_PARAMactivateDistance2IPHioC.size()) < i + 1) {
      newPass->distanceHighOccupancy2IP.first = m_PARAMactivateDistance2IPHioC[m_PARAMactivateDistance2IPHioC.size() - 1];
    } else {
      newPass->distanceHighOccupancy2IP.first =  m_PARAMactivateDistance2IPHioC[i];
    }
    if (int (m_PARAMactivatePTHioC.size()) < i) {
      newPass->pTHighOccupancy.first = m_PARAMactivatePTHioC[m_PARAMactivatePTHioC.size() - 1];
    } else {
      newPass->pTHighOccupancy.first =  m_PARAMactivatePTHioC[i];
    }
    if (int (m_PARAMactivateHelixFitHioC.size()) < i + 1) {
      newPass->helixHighOccupancyFit.first = m_PARAMactivateHelixFitHioC[m_PARAMactivateHelixFitHioC.size() - 1];
    } else {
      newPass->helixFit.first =  m_PARAMactivateHelixFitHioC[i];
    }
    if (newPass->anglesHighOccupancy3D.first == true) { sfhoCtr++; }
    if (newPass->anglesHighOccupancyXY.first == true) { sfhoCtr++; }
    if (newPass->anglesHighOccupancyRZ.first == true) { sfhoCtr++; }
    if (newPass->deltaSlopeHighOccupancyRZ.first == true) { sfhoCtr++; }
    if (newPass->distanceHighOccupancy2IP.first == true) { sfhoCtr++; }
    if (newPass->pTHighOccupancy.first == true) { sfhoCtr++; }
    if (newPass->helixHighOccupancyFit.first == true) { sfhoCtr++; }

    ///nbFinder:
    if (int (m_PARAMactivateAngles3D.size()) < i + 1) {
      newPass->angles3D.first = m_PARAMactivateAngles3D[m_PARAMactivateAngles3D.size() - 1];
    } else {
      newPass->angles3D.first =  m_PARAMactivateAngles3D[i];
    }
    if (int (m_PARAMactivateAnglesXY.size()) < i + 1) {
      newPass->anglesXY.first = m_PARAMactivateAnglesXY[m_PARAMactivateAnglesXY.size() - 1];
    } else {
      newPass->anglesXY.first = m_PARAMactivateAnglesXY[i];
    }
    if (int (m_PARAMactivateAnglesRZ.size()) < i + 1) {
      newPass->anglesRZ.first = m_PARAMactivateAnglesRZ[m_PARAMactivateAnglesRZ.size() - 1];
    } else {
      newPass->anglesRZ.first = m_PARAMactivateAnglesRZ[i];
    }
    if (int (m_PARAMactivateDeltaSlopeRZ.size()) < i + 1) {
      newPass->deltaSlopeRZ.first = m_PARAMactivateDeltaSlopeRZ[m_PARAMactivateDeltaSlopeRZ.size() - 1];
    } else {
      newPass->deltaSlopeRZ.first = m_PARAMactivateDeltaSlopeRZ[i];
    }
    if (int (m_PARAMactivateDistance2IP.size()) < i + 1) {
      newPass->distance2IP.first = m_PARAMactivateDistance2IP[m_PARAMactivateDistance2IP.size() - 1];
    } else {
      newPass->distance2IP.first =  m_PARAMactivateDistance2IP[i];
    }
    if (int (m_PARAMactivatePT.size()) < i + 1) {
      newPass->pT.first = m_PARAMactivatePT[m_PARAMactivatePT.size() - 1];
    } else {
      newPass->pT.first =  m_PARAMactivatePT[i];
    }
    if (int (m_PARAMactivateHelixFit.size()) < i + 1) {
      newPass->helixFit.first = m_PARAMactivateHelixFit[m_PARAMactivateHelixFit.size() - 1];
    } else {
      newPass->helixFit.first =  m_PARAMactivateHelixFit[i];
    }
    if (newPass->angles3D.first == true) { nfCtr++; }
    if (newPass->anglesXY.first == true) { nfCtr++; }
    if (newPass->anglesRZ.first == true) { nfCtr++; }
    if (newPass->deltaSlopeRZ.first == true) { nfCtr++; }
    if (newPass->distance2IP.first == true) { nfCtr++; }
    if (newPass->pT.first == true) { nfCtr++; }
    if (newPass->helixFit.first == true) { nfCtr++; }
    ///nFinder ho finder (3+1 hit)
    if (int (m_PARAMactivateDeltaPtHioC.size()) < i + 1) {
      newPass->deltaPtHighOccupancy.first = m_PARAMactivateDeltaPtHioC[m_PARAMactivateDeltaPtHioC.size() - 1];
    } else {
      newPass->deltaPtHighOccupancy.first = m_PARAMactivateDeltaPtHioC[i];
    }
    if (int (m_PARAMactivateDeltaDistance2IPHioC.size()) < i + 1) {
      newPass->deltaDistanceHighOccupancy2IP.first = m_PARAMactivateDeltaDistance2IPHioC[m_PARAMactivateDeltaDistance2IPHioC.size() - 1];
    } else {
      newPass->deltaDistanceHighOccupancy2IP.first = m_PARAMactivateDeltaDistance2IPHioC[i];
    }
    if (newPass->deltaPtHighOccupancy.first == true) { nfhoCtr++; }
    if (newPass->deltaDistanceHighOccupancy2IP.first == true) { nfhoCtr++; }

    /// post-TCC-filter:
    if (int (m_PARAMactivateZigZagXY.size()) < i + 1) {
      newPass->zigzagXY.first = m_PARAMactivateZigZagXY[m_PARAMactivateZigZagXY.size() - 1];
    } else {
      newPass->zigzagXY.first = m_PARAMactivateZigZagXY[i];
    }
    if (int (m_PARAMactivateZigZagRZ.size()) < i + 1) {
      newPass->zigzagRZ.first = m_PARAMactivateZigZagRZ[m_PARAMactivateZigZagRZ.size() - 1];
    } else {
      newPass->zigzagRZ.first = m_PARAMactivateZigZagRZ[i];
    }
    if (int (m_PARAMactivateDeltaPt.size()) < i + 1) {
      newPass->deltaPt.first = m_PARAMactivateDeltaPt[m_PARAMactivateDeltaPt.size() - 1];
    } else {
      newPass->deltaPt.first = m_PARAMactivateDeltaPt[i];
    }
    if (int (m_PARAMactivateCircleFit.size()) < i + 1) {
      newPass->circleFit.first = m_PARAMactivateCircleFit[m_PARAMactivateCircleFit.size() - 1];
    } else {
      newPass->circleFit.first = m_PARAMactivateCircleFit[i];
    }
    if (int (m_PARAMactivateDeltaDistance2IP.size()) < i + 1) {
      newPass->deltaDistance2IP.first = m_PARAMactivateDeltaDistance2IP[m_PARAMactivateDeltaDistance2IP.size() - 1];
    } else {
      newPass->deltaDistance2IP.first = m_PARAMactivateDeltaDistance2IP[i];
    }
    if (newPass->zigzagXY.first == true) { tccfCtr++; }
    if (newPass->zigzagRZ.first == true) { tccfCtr++; }
    if (newPass->deltaPt.first == true) { tccfCtr++; }
    if (newPass->circleFit.first == true) { tccfCtr++; }
    if (newPass->deltaDistance2IP.first == true) { tccfCtr++; }

    newPass->activatedSegFinderTests = sfCtr;
    newPass->activatedHighOccupancySegFinderTests = sfhoCtr;
    newPass->activatedNbFinderTests = nfCtr;
    newPass->activatedHighOccupancyNbFinderTests = nfhoCtr;
    newPass->activatedTccFilterTests = tccfCtr;
    B2INFO("Pass " << i << " VXD Track finder: " << sfCtr << " segFinder tests, " << nfCtr << " friendFinder tests and " << tccfCtr << " TCC filter tests are enabled. HighOccupancy-mode activated for > " << m_PARAMhighOccupancyThreshold << " hits per sensor");

    if (int (m_PARAMtuneDistance3D.size()) < i + 1) {
      newPass->distance3D.second = m_PARAMtuneDistance3D[m_PARAMtuneDistance3D.size() - 1];
    } else {
      newPass->distance3D.second = m_PARAMtuneDistance3D[i];
    }
    if (int (m_PARAMtuneDistanceXY.size()) < i + 1) {
      newPass->distanceXY.second = m_PARAMtuneDistanceXY[m_PARAMtuneDistanceXY.size() - 1];
    } else {
      newPass->distanceXY.second = m_PARAMtuneDistanceXY[i];;
    }
    if (int (m_PARAMtuneDistanceZ.size()) < i + 1) {
      newPass->distanceZ.second = m_PARAMtuneDistanceZ[m_PARAMtuneDistanceZ.size() - 1];
    } else {
      newPass->distanceZ.second = m_PARAMtuneDistanceZ[i];
    }
    if (int (m_PARAMtuneSlopeRZ.size()) < i + 1) {
      newPass->slopeRZ.second = m_PARAMtuneSlopeRZ[m_PARAMtuneSlopeRZ.size() - 1];
    } else {
      newPass->slopeRZ.second = m_PARAMtuneSlopeRZ[i];
    }
    if (int (m_PARAMtuneNormedDistance3D.size()) < i + 1) {
      newPass->normedDistance3D.second = m_PARAMtuneNormedDistance3D[m_PARAMtuneNormedDistance3D.size() - 1];
    } else {
      newPass->normedDistance3D.second = m_PARAMtuneNormedDistance3D[i];
    }
    if (int (m_PARAMtuneAngles3D.size()) < i + 1) {
      newPass->angles3D.second = m_PARAMtuneAngles3D[m_PARAMtuneAngles3D.size() - 1];
    } else {
      newPass->angles3D.second =  m_PARAMtuneAngles3D[i];
    }
    if (int (m_PARAMtuneAnglesXY.size()) < i + 1) {
      newPass->anglesXY.second = m_PARAMtuneAnglesXY[m_PARAMtuneAnglesXY.size() - 1];
    } else {
      newPass->anglesXY.second = m_PARAMtuneAnglesXY[i];
    }
    if (int (m_PARAMtuneAnglesRZ.size()) < i + 1) {
      newPass->anglesRZ.second = m_PARAMtuneAnglesRZ[m_PARAMtuneAnglesRZ.size() - 1];
    } else {
      newPass->anglesRZ.second = m_PARAMtuneAnglesRZ[i];
    }
    if (int (m_PARAMtuneDeltaSlopeRZ.size()) < i + 1) {
      newPass->deltaSlopeRZ.second = m_PARAMtuneDeltaSlopeRZ[m_PARAMtuneDeltaSlopeRZ.size() - 1];
    } else {
      newPass->deltaSlopeRZ.second = m_PARAMtuneDeltaSlopeRZ[i];
    }
    if (int (m_PARAMtuneDistance2IP.size()) < i + 1) {
      newPass->distance2IP.second = m_PARAMtuneDistance2IP[m_PARAMtuneDistance2IP.size() - 1];
    } else {
      newPass->distance2IP.second =  m_PARAMtuneDistance2IP[i];
    }
    if (int (m_PARAMtunePT.size()) < i + 1) {
      newPass->pT.second = m_PARAMtunePT[m_PARAMtunePT.size() - 1];
    } else {
      newPass->pT.second =  m_PARAMtunePT[i];
    }
    if (int (m_PARAMtuneHelixFit.size()) < i + 1) {
      newPass->helixFit.second = m_PARAMtuneHelixFit[m_PARAMtuneHelixFit.size() - 1];
    } else {
      newPass->helixFit.second =  m_PARAMtuneHelixFit[i];
    }
    if (int (m_PARAMtuneZigZagXY.size()) < i + 1) {
      newPass->zigzagXY.second = m_PARAMtuneZigZagXY[m_PARAMtuneZigZagXY.size() - 1];
    } else {
      newPass->zigzagXY.second = m_PARAMtuneZigZagXY[i];
    }
    if (int (m_PARAMtuneZigZagRZ.size()) < i + 1) {
      newPass->zigzagRZ.second = m_PARAMtuneZigZagRZ[m_PARAMtuneZigZagRZ.size() - 1];
    } else {
      newPass->zigzagRZ.second = m_PARAMtuneZigZagRZ[i];
    }
    if (int (m_PARAMtuneDeltaPt.size()) < i + 1) {
      newPass->deltaPt.second = m_PARAMtuneDeltaPt[m_PARAMtuneDeltaPt.size() - 1];
    } else {
      newPass->deltaPt.second = m_PARAMtuneDeltaPt[i];
    }
    if (int (m_PARAMtuneDeltaDistance2IP.size()) < i + 1) {
      newPass->deltaDistance2IP.second = m_PARAMtuneDeltaDistance2IP[m_PARAMtuneDeltaDistance2IP.size() - 1];
    } else {
      newPass->deltaDistance2IP.second = m_PARAMtuneDeltaDistance2IP[i];
    }
    if (int (m_PARAMtuneCircleFit.size()) < i + 1) {
      newPass->circleFit.second = m_PARAMtuneCircleFit[m_PARAMtuneCircleFit.size() - 1];
    } else {
      newPass->circleFit.second = m_PARAMtuneCircleFit[i];
    }
    // high occupancy cases:
    if (int (m_PARAMtuneAngles3DHioC.size()) < i + 1) {
      newPass->anglesHighOccupancy3D.second = m_PARAMtuneAngles3DHioC[m_PARAMtuneAngles3DHioC.size() - 1];
    } else {
      newPass->anglesHighOccupancy3D.second =  m_PARAMtuneAngles3DHioC[i];
    }
    if (int (m_PARAMtuneAnglesXYHioC.size()) < i + 1) {
      newPass->anglesHighOccupancyXY.second = m_PARAMtuneAnglesXYHioC[m_PARAMtuneAnglesXYHioC.size() - 1];
    } else {
      newPass->anglesHighOccupancyXY.second = m_PARAMtuneAnglesXYHioC[i];
    }
    if (int (m_PARAMtuneAnglesRZHioC.size()) < i + 1) {
      newPass->anglesHighOccupancyRZ.second = m_PARAMtuneAnglesRZHioC[m_PARAMtuneAnglesRZHioC.size() - 1];
    } else {
      newPass->anglesHighOccupancyRZ.second = m_PARAMtuneAnglesRZHioC[i];
    }
    if (int (m_PARAMtuneDeltaSlopeRZHioC.size()) < i + 1) {
      newPass->deltaSlopeHighOccupancyRZ.second = m_PARAMtuneDeltaSlopeRZHioC[m_PARAMtuneDeltaSlopeRZHioC.size() - 1];
    } else {
      newPass->deltaSlopeHighOccupancyRZ.second = m_PARAMtuneDeltaSlopeRZHioC[i];
    }
    if (int (m_PARAMtuneDistance2IPHioC.size()) < i + 1) {
      newPass->distanceHighOccupancy2IP.second = m_PARAMtuneDistance2IPHioC[m_PARAMtuneDistance2IPHioC.size() - 1];
    } else {
      newPass->distanceHighOccupancy2IP.second =  m_PARAMtuneDistance2IPHioC[i];
    }
    if (int (m_PARAMtunePTHioC.size()) < i + 1) {
      newPass->pTHighOccupancy.second = m_PARAMtunePTHioC[m_PARAMtunePTHioC.size() - 1];
    } else {
      newPass->pTHighOccupancy.second =  m_PARAMtunePTHioC[i];
    }
    if (int (m_PARAMtuneHelixFitHioC.size()) < i + 1) {
      newPass->helixHighOccupancyFit.second = m_PARAMtuneHelixFitHioC[m_PARAMtuneHelixFitHioC.size() - 1];
    } else {
      newPass->helixHighOccupancyFit.second =  m_PARAMtuneHelixFitHioC[i];
    }
    if (int (m_PARAMtuneDeltaPtHioC.size()) < i + 1) {
      newPass->deltaPtHighOccupancy.second = m_PARAMtuneDeltaPtHioC[m_PARAMtuneDeltaPtHioC.size() - 1];
    } else {
      newPass->deltaPtHighOccupancy.second = m_PARAMtuneDeltaPtHioC[i];
    }
    if (int (m_PARAMtuneDeltaDistance2IPHioC.size()) < i + 1) {
      newPass->deltaDistanceHighOccupancy2IP.second = m_PARAMtuneDeltaDistance2IPHioC[m_PARAMtuneDeltaDistance2IPHioC.size() - 1];
    } else {
      newPass->deltaDistanceHighOccupancy2IP.second = m_PARAMtuneDeltaDistance2IPHioC[i];
    }


    /// importing sectorMap including friend Information and friend specific cutoffs
    newPass->sectorMap.clear();
    totalFriendCounter = 0;

    GearDir belleDir("/Detector/Tracking/CATFParameters/");

    string chosenSetup = (boost::format("sectorList_%1%_%2%") % newPass->sectorSetup  % newPass->chosenDetectorType).str();

    GearDir sectorList(belleDir, chosenSetup);

    B2INFO("importing sectors, using " << chosenSetup << " > " << sectorList.getNumberNodes("aSector") << " sectors found");

    if (sectorList.getNumberNodes("aSector") == 0) {
      B2FATAL("Failed to import sector map " << chosenSetup << "! No track finding possible. Please check ../tracking/data/VXDTFinxex.xml whether your chosen sector maps are uncommented and recompile if you change entries...")
    }
    double cutoffMinValue, cutoffMaxValue;
    string aSectorName, aFriendName, aFilterName;
    pair<double, double> cutoff;
    BOOST_FOREACH(const GearDir & aSector, sectorList.getNodes("aSector")) {

      aSectorName = aSector.getString("sectorOfInterest");

      GearDir friendList(aSector, "friendList/");

      B2DEBUG(1000, " > > importing sector: " << aSectorName << " including " << friendList.getNumberNodes("aFriend") << " friends. ");
      totalFriendCounter += friendList.getNumberNodes("aFriend");
      VXDSector* pSector = new VXDSector(aSectorName);

      BOOST_FOREACH(const GearDir & aFriend, friendList.getNodes("aFriend")) {

        aFriendName = aFriend.getString("FriendOfInterest");

        GearDir filterList(aFriend, "filterList/");

        BOOST_FOREACH(const GearDir & aFilter, filterList.getNodes("aFilter")) {

          aFilterName = aFilter.getString("FilterOfInterest");
          GearDir quantiles(aFilter, "quantiles/");
          cutoffMinValue = 0., cutoffMaxValue = 0.;
          string min = "Min", max = "Max";
          // now, for each filter will be checked, whether it shall be stored or not and whether the cutoffs shall be modified:
          if (aFilterName == m_filterNameBox.nameDistance3D && newPass->distance3D.first == true) {   // first: activateDistance3D, second: tuneDistance3D
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->distance3D.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->distance3D.second);

          } else if (aFilterName == m_filterNameBox.nameDistanceXY &&  newPass->distanceXY.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->distanceXY.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->distanceXY.second);

          } else if (aFilterName == m_filterNameBox.nameDistanceZ &&  newPass->distanceZ.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->distanceZ.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->distanceZ.second);

          } else if (aFilterName == m_filterNameBox.nameSlopeRZ &&  newPass->slopeRZ.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->slopeRZ.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->slopeRZ.second);

          } else if (aFilterName == m_filterNameBox.nameNormedDistance3D &&  newPass->normedDistance3D.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->normedDistance3D.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->normedDistance3D.second);

          } else if (aFilterName == m_filterNameBox.nameAngles3D &&  newPass->angles3D.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->angles3D.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->angles3D.second);

          } else if (aFilterName == m_filterNameBox.nameAnglesXY &&  newPass->anglesXY.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->anglesXY.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->anglesXY.second);

          } else if (aFilterName == m_filterNameBox.nameAnglesRZ &&  newPass->anglesRZ.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->anglesRZ.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->anglesRZ.second);

          } else if (aFilterName == m_filterNameBox.namePT &&  newPass->pT.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->pT.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->pT.second);

          } else if (aFilterName == m_filterNameBox.nameHelixFit &&  newPass->helixFit.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->helixFit.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->helixFit.second);

          } else if (aFilterName == m_filterNameBox.nameDistance2IP &&  newPass->distance2IP.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->distance2IP.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->distance2IP.second);

          } else if (aFilterName == m_filterNameBox.nameDeltaSlopeRZ &&  newPass->deltaSlopeRZ.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->deltaSlopeRZ.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->deltaSlopeRZ.second);

          } else if (aFilterName == m_filterNameBox.nameDeltapT &&  newPass->deltaPt.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->deltaPt.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->deltaPt.second);

          } else if (aFilterName == m_filterNameBox.nameDeltaDistance2IP &&  newPass->deltaDistance2IP.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->deltaDistance2IP.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->deltaDistance2IP.second);

          } else if (aFilterName == m_filterNameBox.nameAnglesHighOccupancy3D &&  newPass->anglesHighOccupancy3D.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->anglesHighOccupancy3D.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->anglesHighOccupancy3D.second);

          } else if (aFilterName == m_filterNameBox.nameAnglesHighOccupancyXY &&  newPass->anglesHighOccupancyXY.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->anglesHighOccupancyXY.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->anglesHighOccupancyXY.second);

          } else if (aFilterName == m_filterNameBox.nameAnglesHighOccupancyRZ &&  newPass->anglesHighOccupancyRZ.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->anglesHighOccupancyRZ.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->anglesHighOccupancyRZ.second);

          } else if (aFilterName == m_filterNameBox.namePTHighOccupancy &&  newPass->pTHighOccupancy.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->pTHighOccupancy.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->pTHighOccupancy.second);

          } else if (aFilterName == m_filterNameBox.nameHelixHighOccupancyFit &&  newPass->helixHighOccupancyFit.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->helixHighOccupancyFit.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->helixHighOccupancyFit.second);

          } else if (aFilterName == m_filterNameBox.nameDistanceHighOccupancy2IP &&  newPass->distanceHighOccupancy2IP.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->distanceHighOccupancy2IP.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->distanceHighOccupancy2IP.second);

          } else if (aFilterName == m_filterNameBox.nameDeltaSlopeHighOccupancyRZ &&  newPass->deltaSlopeHighOccupancyRZ.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->deltaSlopeHighOccupancyRZ.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->deltaSlopeHighOccupancyRZ.second);

          } else if (aFilterName == m_filterNameBox.nameDeltapTHighOccupancy &&  newPass->deltaPtHighOccupancy.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->deltaPtHighOccupancy.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->deltaPtHighOccupancy.second);

          } else if (aFilterName == m_filterNameBox.nameDeltaDistanceHighOccupancy2IP &&  newPass->deltaDistanceHighOccupancy2IP.first == true) {
            cutoffMinValue = getXMLValue(quantiles, min, aFilterName);
            cutoffMaxValue = getXMLValue(quantiles, max, aFilterName);
            cutoffMinValue = addExtraGain(-1, cutoffMinValue, m_PARAMtuneCutoffs, newPass->deltaDistanceHighOccupancy2IP.second);
            cutoffMaxValue = addExtraGain(+1, cutoffMaxValue, m_PARAMtuneCutoffs, newPass->deltaDistanceHighOccupancy2IP.second);
          }

          cutoff = make_pair(cutoffMinValue, cutoffMaxValue);
          pSector->addCutoff(aFilterName, aFriendName, cutoff);
          /*      tempQuantile = make_pair("q0.01", cutoffMinValue);
           p Cutoff->addQuantile(tempQuantile);*/
        }
      }
      newPass->sectorMap.insert(make_pair(aSectorName, pSector)); /// TODO: add only if there is at least one cutoff added (implement by autocount within pSector->addCutoff and additional .size()- memberfunction)
    }

    //generating virtual sector (represents the region of the primary vertex)
    string centerSector = "00_00_0";
    VXDSector* pCenterSector = new VXDSector(centerSector);
    newPass->sectorMap.insert(make_pair(centerSector, pCenterSector));
    B2DEBUG(100, "Pass" << i << ": adding virtual centerSector with " << newPass->sectorMap.find(centerSector)->second->getFriends().size() << " friends.");

    B2INFO("Pass" << i << ": there are " << newPass->sectorMap.size() << " sectors in map and a total number of " << totalFriendCounter << " friends");

    /// for testing purposes:
    //     MapOfSectors::const_iterator mapIter = newPass.sectorMap.begin();
    //     int cutoffTypesCounter = 0;
    //     while (mapIter != newPass.sectorMap.end()) { // iterating through sectors
    //
    //      const vector<string>& currentFriends = mapIter->second->getFriends();
    //      int nFriends = currentFriends.size();
    //      B2DEBUG(200,"Opening sector " << mapIter->first << " which has got " << nFriends << " friends");
    //
    //      for (int f = 0; f < nFriends; ++f ) { // looping through friends
    //        B2DEBUG(250," > Opening sectorFriend " << currentFriends.at(f) << "..."); /// method is safer than [f]
    //        vector<string> currentCutoffTypes = mapIter->second->getSupportedCutoffs(currentFriends.at(f));
    //
    //        for (int j = 0; j < int(currentCutoffTypes.size()); ++j) {
    //          Cutoff* aCutoff = mapIter->second->getCutoff(currentCutoffTypes.at(j), currentFriends.at(f));
    //          B2DEBUG(250," cutoff is of type: " << currentCutoffTypes[j]
    //            << ", min: " << aCutoff->getMinValue() << ", max:" << aCutoff->getMaxValue());
    //          cutoffTypesCounter++;
    //        }
    //      }
    //      ++mapIter;
    //     }
    //     B2DEBUG(200,"Pass"<<i<<": there are a total of " << cutoffTypesCounter << " cutoffs stored in sectorMap");
    /// testing purposes end...

    m_passSetupVector.push_back(newPass); /// store pass for eternity (well until end of program)
  }

  /** Section 1 - end **/


  m_eventCounter = 0;
  m_badSectorRangeCounter = 0;
  m_TESTERbadSectorRangeCounterForClusters = 0;
  m_TESTERclustersPersSectorNotMatching = 0;
  m_badFriendCounter = 0;
  m_totalPXDClusters = 0;
  m_totalSVDClusters = 0;
  m_totalSVDClusterCombis = 0;
  m_TESTERhighOccupancyCtr = 0;

}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *********************************+ event +********************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void VXDTFModule::event()
{
  EventInfoPackage thisInfoPackage;
  thisInfoPackage.clear();
  boostClock::time_point beginEvent = boostClock::now();


  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  thisInfoPackage.evtNumber = m_eventCounter;
  B2DEBUG(1, "################## entering vxd CA track finder - event " << m_eventCounter << " ######################");
  /** cleaning will be done at the end of the event **/

  /** Section 3 - importing hits and find their papaSectors.**/

//   generating virtual Hit at position (0, 0, 0) - needed for virtual segment.
  TVector3 centerPosition; //(0., 0., 0.); // !!!
  string centerSector = "00_00_0";
  VxdID centerVxdID = VxdID(0, 0, 0); // dummy VxdID for virtual IP
  int passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    VXDTFHit* pTFHit = new VXDTFHit(centerPosition, passNumber, 0, 0, 0, -1, centerSector, centerVxdID, 0.0); // has no position in HitList, because it doesn't exist...

    currentPass->sectorMap.find(centerSector)->second->addHit(pTFHit);
    currentPass->hitVector.push_back(pTFHit);
    passNumber++;
  }

  vector<ClusterInfo> clustersOfEvent; /// contains info which tc uses which clusters
  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();

  // importing hits
  StoreArray<PXDCluster> aPxdClusterArray;
  int numOfPxdClusters = aPxdClusterArray.getEntries();
  StoreArray<SVDCluster> aSvdClusterArray;
  int numOfSvdClusters = aSvdClusterArray.getEntries();
  thisInfoPackage.numPXDCluster = numOfPxdClusters;
  thisInfoPackage.numSVDCluster = numOfSvdClusters;

  // preparing storearray for trackCandidates and fitted tracks
  StoreArray<GFTrackCand> finalTrackCandidates(m_PARAMgfTrackCandsColName);
  finalTrackCandidates.create();

  if (m_TESTERexpandedTestingRoutines == true) {
    StoreArray<VXDTFInfoBoard> extraInfo4GFTCs;
    extraInfo4GFTCs.create();
  }

  boostClock::time_point stopTimer = boostClock::now();
  m_TESTERtimeConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
  thisInfoPackage.sectionConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);

  B2DEBUG(1, "VXDTF event " << m_eventCounter << ": size of arrays, PXDCluster: " << numOfPxdClusters << ", SVDCLuster: " << numOfSvdClusters);

  TVector3 hitGlobal, hitLocal, transformedHitLocal, localSensorSize;
  double vSize, uSizeAtHit, uCoord, vCoord;
  string aSectorName;
  VxdID aVxdID;
  boostClock::time_point timeStamp = boostClock::now();
  int badSectorRangeCtr = 0, aLayerID;
  string checkString4badHits = "-", testString;
  if (m_usePXDorSVDorVXDhits not_eq 1) {   /// means: is true when at least one pass wants PXD hits
    for (int iPart = 0; iPart < numOfPxdClusters; ++iPart) {
      const PXDCluster* const aClusterPtr = aPxdClusterArray[iPart];

      hitLocal.SetXYZ(aClusterPtr->getU(), aClusterPtr->getV(), 0);

      aVxdID = aClusterPtr->getSensorID();
      aLayerID = aVxdID.getLayerNumber();
      VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
      hitGlobal = aSensorInfo.pointToGlobal(hitLocal);

      // local(0,0,0) is the _center_ of the sensorplane, not at the edge!
      vSize = 0.5 * aSensorInfo.getVSize();
      uSizeAtHit = 0.5 * aSensorInfo.getUSize(hitLocal[1]);

      uCoord = hitLocal[0] + uSizeAtHit; // *0,5 putting (0,0) from the center to the edge of the plane (considers the trapeziodal shape)
      vCoord = hitLocal[1] + vSize;
      transformedHitLocal.SetXYZ(uCoord, vCoord, 0);
      localSensorSize.SetXYZ(uSizeAtHit, vSize, 0);

//      B2DEBUG(1000,"Sensor edges: O("<<-uSize1<<","<<-vSize1<<"), U("<<uSize1<<","<<-vSize1<<"), V("<<-uSize2<<","<<vSize1<<"), UV("<<uSize2<<","<<vSize1<<")");
      B2DEBUG(1000, "local pxd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ")");

      passNumber = 0; // why not using for at this point since I need a iteration number anyway? foreach is faster than for classic (better access to entries)
      BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
        if (aLayerID > currentPass->highestAllowedLayer) { continue; }   // skip particle if True
        if (currentPass->detectorType not_eq 0 && currentPass->detectorType not_eq - 1) { continue; }  // PXD is included in 0 & -1

        pair<string, MapOfSectors::iterator> activatedSector = searchSector4Hit(aVxdID,
                                                               transformedHitLocal,
                                                               localSensorSize,
                                                               currentPass->sectorMap,
                                                               currentPass->secConfigU,
                                                               currentPass->secConfigV);
        aSectorName = activatedSector.first;
        MapOfSectors::iterator secMapIter =  activatedSector.second;

        if (aSectorName.size() > 0) {
          testString = aSectorName.at(0);
        } else {
          testString = checkString4badHits;
        }

        if (testString == checkString4badHits) {
          B2DEBUG(10, "VXDTF - event " << m_eventCounter << ": pxdhit out of sector range (setup " << currentPass->sectorSetup << ", type  " << currentPass->chosenDetectorType << "). (" << aSectorName << " does not exist) Discarding hit...");
          badSectorRangeCtr++;
          m_TESTERbadSensors.push_back(aSectorName);
          continue;
        }

        B2DEBUG(50, " PXDCluster: with posOfHit in StoreArray: " << iPart << " is found again within secID " << aSectorName << " using sectorSetup " << currentPass->sectorSetup);
        ClusterInfo aClusterUV(iPart);
        int numClusters = clustersOfEvent.size();
        clustersOfEvent.push_back(aClusterUV);
        VXDTFHit* pTFHit = new VXDTFHit(hitGlobal, passNumber, 0, 0, numClusters, 0, aSectorName, aVxdID, 0.0); // no timeInfo for PXDHits

        currentPass->hitVector.push_back(pTFHit);
        secMapIter->second->addHit(pTFHit);
        currentPass->sectorSequence.push_back(activatedSector);

        B2DEBUG(150, "size of sectorSequence: " << currentPass->sectorSequence.size() << "size of hitVector: " << currentPass->hitVector.size());
        passNumber++;
      }
    }
  }

  int numOfClusterCombis = 0;
  if (m_usePXDorSVDorVXDhits not_eq 0) {   /// means: is true when at least one pass wants SVD hits
    map<int, SensorStruct > activatedSensors;
    typedef pair<int, SensorStruct > mapEntry;
    vector<ClusterHit> clusterHitList;

    // store each cluster (as a clusterPtr) in a map(uniID, sensorStruct), where sensorStruct contains 2 vectors (uClusters and vClusters).
    // in the end a map containing illuminated sensors - and each cluster inhabiting them - exists.
    map<int, SensorStruct>::iterator sensorIter;
    for (int iPart = 0; iPart < numOfSvdClusters; ++iPart) {

      SVDCluster* aClusterPtr = aSvdClusterArray[iPart];

      aVxdID = aClusterPtr->getSensorID();
      int aUniID = aVxdID.getID();
      sensorIter = activatedSensors.find(aUniID);
      if (sensorIter == activatedSensors.end()) {
        SensorStruct newSensor;
        newSensor.layerID = aVxdID.getLayerNumber();
        sensorIter = activatedSensors.insert(sensorIter, mapEntry(aUniID, newSensor)); //activatedSensors.find(aUniID);
      }
      if (aClusterPtr->isUCluster() == true) {
        sensorIter->second.uClusters.push_back(make_pair(iPart, aClusterPtr));
      } else {
        sensorIter->second.vClusters.push_back(make_pair(iPart, aClusterPtr));
      }
    }
    B2DEBUG(20, activatedSensors.size() << " SVD sensors activated...")

    // iterate through map & combine each possible combination of clusters. Store them in a vector of structs, where each struct carries an u & a v cluster
    int occupancy = m_TESTERSVDOccupancy.size(), numHits = 0;
    BOOST_FOREACH(mapEntry aSensor, activatedSensors) {
      int numUclusters = aSensor.second.uClusters.size();
      int numVclusters = aSensor.second.vClusters.size();
      B2DEBUG(100, " sensor " << aSensor.first << " has got " << numUclusters << " uClusters and " << numVclusters << " vClusters")
      if (numUclusters == 0 || numVclusters == 0) {
        m_TESTERbadSectorRangeCounterForClusters++;
        B2WARNING("at event: " << m_eventCounter << " sensor " << aSensor.first << " at layer " << aSensor.second.layerID << " has got " << numUclusters << "/" << numVclusters << " u/vclusters!")
      }
      if (numUclusters != numVclusters) {
        m_TESTERclustersPersSectorNotMatching++;
        if (m_PARAMDebugMode == true) {
          B2WARNING("at event: " << m_eventCounter << " at sensor " << aSensor.first << " at layer " << aSensor.second.layerID << " number of clusters do not match: Has got " << numUclusters << "/" << numVclusters << " u/vclusters!")
        }
      }

      for (int uClNum = 0; uClNum < numUclusters; ++uClNum) {
        for (int vClNum = 0; vClNum < numVclusters; ++vClNum) {
          ClusterHit aHit;
          aHit.uCluster = aSensor.second.uClusters[uClNum].second;
          aHit.uClusterIndex = aSensor.second.uClusters[uClNum].first;
          aHit.vCluster = aSensor.second.vClusters[vClNum].second;
          aHit.vClusterIndex = aSensor.second.vClusters[vClNum].first;
          clusterHitList.push_back(aHit);
          ++numHits;
        }
      }

      // protocolling number of 2D-cluster-combinations per sensor
      if (numHits == 0) { continue; }
      if (occupancy < numHits) {
        m_TESTERSVDOccupancy.resize(numHits, 0);
        occupancy = numHits;
      }
      m_TESTERSVDOccupancy[numHits - 1] += 1;
      if (m_PARAMhighOccupancyThreshold < numHits) {
        m_highOccupancyCase = true;
        m_TESTERhighOccupancyCtr++;
      } else { m_highOccupancyCase = false; }

      numHits = 0;
    }
    numOfClusterCombis = clusterHitList.size();

    BOOST_FOREACH(ClusterHit aClusterCombi, clusterHitList) {
      const SVDCluster* const uClusterPtr = aClusterCombi.uCluster; // uCluster
      const SVDCluster* const vClusterPtr = aClusterCombi.vCluster; // vCluster
      int clusterIndexU = aClusterCombi.uClusterIndex;
      int clusterIndexV = aClusterCombi.vClusterIndex;

      float timeStampU = uClusterPtr->getClsTime();
      float timeStampV = vClusterPtr->getClsTime();

      aVxdID = uClusterPtr->getSensorID();
      aLayerID = aVxdID.getLayerNumber();
      VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
      if ((aSensorInfo.getBackwardWidth() > aSensorInfo.getForwardWidth()) == true) {   // isWedgeSensor
        hitLocal.SetX(aSensorInfo.getWidth(vClusterPtr->getPosition()) / aSensorInfo.getWidth(0) * uClusterPtr->getPosition());
      } else { // rectangular Sensor
        hitLocal.SetX(uClusterPtr->getPosition());
      }
      hitLocal.SetY(vClusterPtr->getPosition()); // always correct
      hitLocal.SetZ(0.);

      hitGlobal = aSensorInfo.pointToGlobal(hitLocal);

      // local(0,0,0) is the center of the sensorplane
      vSize = 0.5 * aSensorInfo.getVSize();
      uSizeAtHit = 0.5 * aSensorInfo.getUSize(hitLocal[1]);

      uCoord = hitLocal[0] + uSizeAtHit; // *0,5 putting (0,0) from the center to the edge of the plane (considers the trapeziodal shape)
      vCoord = hitLocal[1] + vSize;

      if (uCoord < 0.) {   /// if Hit is not in sensorRange, move it back to the range
        uCoord = 0. + 0.0001;
        m_TESTERdistortedHitCtr++;
      } else if (uCoord > uSizeAtHit * 2.) {
        uCoord = uSizeAtHit * 2. - 0.0001;
        m_TESTERdistortedHitCtr++;
      }
      if (vCoord < 0.) {
        vCoord = 0. + 0.0001;
        m_TESTERdistortedHitCtr++;
      } else if (vCoord > vSize * 2.) {
        vCoord = vSize * 2. - 0.0001;
        m_TESTERdistortedHitCtr++;
      }

      transformedHitLocal.SetXYZ(uCoord, vCoord, 0);
      localSensorSize.SetXYZ(uSizeAtHit, vSize, 0);

//      B2DEBUG(1000,"Sensor edges: O("<<-uSize1<<","<<-vSize1<<"), U("<<uSize1<<","<<-vSize1<<"), V("<<-uSize2<<","<<vSize1<<"), UV("<<uSize2<<","<<vSize1<<")");
      B2DEBUG(1000, "local svd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ")");

      passNumber = 0;
      BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
        if (aLayerID > currentPass->highestAllowedLayer) { continue; }   // skip particle if True
        if (currentPass->detectorType == 0) { continue; }   // SVD is included in 1 & -1, but not in 0

        pair<string, MapOfSectors::iterator> activatedSector = searchSector4Hit(aVxdID,
                                                               transformedHitLocal,
                                                               localSensorSize,
                                                               currentPass->sectorMap,
                                                               currentPass->secConfigU,
                                                               currentPass->secConfigV);
        aSectorName = activatedSector.first;
        MapOfSectors::iterator secMapIter =  activatedSector.second;

        if (aSectorName.size() > 0) {
          testString = aSectorName.at(0);
        } else {
          testString = checkString4badHits;
        }

        if (testString == checkString4badHits)  {
          B2DEBUG(10, "VXDTF - event " << m_eventCounter << ": svdhit out of sector range(setup " << currentPass->sectorSetup << ", type  " << currentPass->chosenDetectorType << "). (" << aSectorName << " does not exist) Discarding hit...");
          badSectorRangeCtr++;
          m_TESTERbadSensors.push_back(aSectorName);
          continue;
        }
        int numClusters = clustersOfEvent.size();
        ClusterInfo aClusterU(clusterIndexU);
        ClusterInfo aClusterV(clusterIndexV);
        clustersOfEvent.push_back(aClusterU);
        clustersOfEvent.push_back(aClusterV);

        B2DEBUG(50, "A SVDCluster is found again within secID " << aSectorName << " using sectorSetup " << currentPass->sectorSetup);
//         VXDTFHit* pTFHit = new VXDTFHit(hitGlobal, passNumber, clusterIndexU, clusterIndexV, 0, 1, aSectorName, aVxdID,  0.5 * (timeStampU + timeStampV));
        VXDTFHit* pTFHit = new VXDTFHit(hitGlobal, passNumber, numClusters, numClusters + 1, 0, 1, aSectorName, aVxdID,  0.5 * (timeStampU + timeStampV));

        currentPass->hitVector.push_back(pTFHit);
        secMapIter->second->addHit(pTFHit);
        currentPass->sectorSequence.push_back(activatedSector);

        int numPassHits = currentPass->hitVector.size();
        B2DEBUG(150, "size of sectorSequence: " << currentPass->sectorSequence.size() << "size of hitVector: " << numPassHits);
        passNumber++;
      }
    }
  }
  m_badSectorRangeCounter += badSectorRangeCtr;
  m_totalPXDClusters += numOfPxdClusters;
  m_totalSVDClusters += numOfSvdClusters;
  m_totalSVDClusterCombis += numOfClusterCombis;
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.hitSorting += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.hitSorting += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  B2DEBUG(1, "VXDTF- import hits: of " << numOfSvdClusters << " svdClusters, " << numOfClusterCombis << " svd2Dclusters and " << numOfPxdClusters  << " pxdClusters, " << badSectorRangeCtr << " hits had to be discarded because out of sector range")
  /** Section 3 - end **/


  /** Section 4 - SEGFINDER **/
  passNumber = 0;
  timeStamp = boostClock::now();
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    int numPassHits = currentPass->hitVector.size();
    thisInfoPackage.numSVDHits += numPassHits;
    B2DEBUG(50, "Pass " << passNumber << ": sectorSequence has got " << currentPass->sectorSequence.size() << " entries before applying unique & sort");

//    currentPass->sectorSequence.sort(compareSecSequence);
//    typedef pair<string,MapOfSectors::iterator> aPair;
//    B2WARNING("looping through activated sectors, before sorting: ")
//    BOOST_FOREACH(aPair thisPair, currentPass->sectorSequence) {
//      B2WARNING("sectorName: " << thisPair.first)
//    }
    currentPass->sectorSequence.sort(compareSecSequence);
    currentPass->sectorSequence.unique();
    currentPass->sectorSequence.reverse();

    B2DEBUG(5, "Pass " << passNumber << ": " << currentPass->sectorSequence.size() << " sectors activated, ");
    passNumber++;

  }

  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    B2DEBUG(10, "pass " << passNumber << ": starting segFinder...");
    int discardedSegments = segFinder(currentPass);                             /// calling funtion "segFinder"
    int activatedSegments = currentPass->activeCellList.size();
    m_TESTERtotalsegmentsSFCtr += activatedSegments;
    m_TESTERdiscardedSegmentsSFCtr += discardedSegments;
    B2DEBUG(1, "VXDTF-event " << m_eventCounter << ", pass" << passNumber << " @ segfinder - " << activatedSegments << " segments activated, " << discardedSegments << " discarded");
    thisInfoPackage.segFinderActivated += activatedSegments;
    thisInfoPackage.segFinderDiscarded += discardedSegments;
    passNumber++;
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.segFinder += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.segFinder += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 4 - end **/



  /** Section 5 - NEIGHBOURFINDER **/
  passNumber = 0;
  timeStamp = boostClock::now();
  int totalActiveCells = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    B2DEBUG(5, "pass " << passNumber << ": starting neighbourFinder...");
    int discardedSegments = neighbourFinder(currentPass);                       /// calling funtion "neighbourFinder"
    int activatedSegments = currentPass->activeCellList.size();
    m_TESTERtotalsegmentsNFCtr += activatedSegments;
    m_TESTERdiscardedSegmentsNFCtr += discardedSegments;
    B2DEBUG(1, "VXDTF-event " << m_eventCounter << ", pass" << passNumber << " @ nbfinder - " << activatedSegments << " segments activated, " << discardedSegments << " discarded");
    thisInfoPackage.nbFinderActivated += activatedSegments;
    thisInfoPackage.nbFinderDiscarded += discardedSegments;
    totalActiveCells += activatedSegments;
    passNumber++;
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.nbFinder += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.nbFinder += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 5 - end **/


  if (totalActiveCells > 5000) {
    B2ERROR("event " << m_eventCounter << ": total number of activated segments: " << totalActiveCells << ", termitating event!");
    m_TESTERbrokenEventsCtr++;

    /** cleaning part **/
    BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
      cleanEvent(currentPass, centerSector);
    }
    return;
  } /// WARNING: hardcoded filter for events containing huge number of segments



  /** Section 6 - Cellular Automaton**/
  passNumber = 0;
  timeStamp = boostClock::now();
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    int numRounds = cellularAutomaton(currentPass);
    B2DEBUG(1, "pass " << passNumber << ": cellular automaton finished in " << numRounds << " rounds");
    if (numRounds < 0) {
      B2ERROR("event " << m_eventCounter << ": cellular automaton entered an infinite loop, therefore aborted, terminating event!");
      m_TESTERbrokenEventsCtr++;

      /** cleaning part **/
      BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
        cleanEvent(currentPass, centerSector);
      }
      return;
    }
    passNumber++;
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.cellularAutomaton += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.cellularAutomaton += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 6 - end **/



  /** Section 7 - Track Candidate Collector (TCC) **/
  passNumber = 0;
  timeStamp = boostClock::now();
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    delFalseFriends(currentPass, centerPosition);
    tcCollector(currentPass);
    int survivingTCs = currentPass->tcVector.size();
    B2DEBUG(1, "pass " << passNumber << ": track candidate collector generated " << survivingTCs << " TCs");
    thisInfoPackage.numTCsAfterTCC += survivingTCs;
    passNumber++;
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.tcc += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.tcc += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 7 - end **/



  /** Section 8 - tcFilter **/
  passNumber = 0;
  timeStamp = boostClock::now();
  int survivingTCs = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    if (int(currentPass->tcVector.size()) != 0) {
      survivingTCs = tcFilter(currentPass, passNumber, clustersOfEvent);
      thisInfoPackage.numTCsAfterTCCfilter += survivingTCs;
      B2DEBUG(1, "pass " << passNumber << ": track candidate filter, " << survivingTCs << " TCs survived.");
    } else {
      B2DEBUG(1, "pass " << passNumber << " has no TCs therefore not starting tcFilter()");
    }
    passNumber++;
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.postCAFilter += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.postCAFilter += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 8 - end **/



  /** Section 9 - check overlap */
  timeStamp = boostClock::now();
  ClusterUsage totalClusterUsage;
  bool  isOB = false;
  int countOverbookedClusters = 0;
  /// each clusterInfo knows which TCs are using it, following loop therefore checks each for overlapping ones
  BOOST_FOREACH(ClusterInfo aCluster, clustersOfEvent) {
    isOB = aCluster.isOverbooked();
    if (isOB == true) { countOverbookedClusters++; }
  } // now each TC knows whether it is overbooked or not (aCluster.isOverbooked() implicitly checked this)
  B2DEBUG(1, "after checking overlaps: there are " << countOverbookedClusters << " clusters of " << clustersOfEvent.size() << " marked as 'overbooked'...")

  int countCurrentTCs = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    /// determine initial values for all track Candidates:
    calcInitialValues4TCs(currentPass->tcVector);                               /// calcInitialValues4TCs
    BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentPass->tcVector) {
      if (currentTC->getCondition() == false) { continue; }   // in this case, the TC is already filtered

      bool overbooked = currentTC->getOverlappingState(); /// TODO find out whether it is better to use the slower but safer function checkOverlappingState
      m_tcVector.push_back(currentTC);
      if (overbooked == true) {
        m_tcVectorOverlapped.push_back(currentTC);
      }
      countCurrentTCs++;
    }
  } ///collect TCs and separately store overlapping TCs for hopfield used
  int totalOverlaps = m_tcVectorOverlapped.size();

  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.checkOverlap += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.checkOverlap += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);

  B2DEBUG(1, "event " << m_eventCounter << ": " << totalOverlaps << " overlapping track candidates found within " << countCurrentTCs << " current TCs alive")
  /** Section 9 - end */

//  for (int iPart = 0; iPart < numOfSvdClusters; ++iPart) {
//
//       SVDCluster* aClusterPtr = aSvdClusterArray[iPart];
//
//       aVxdID = aClusterPtr->getSensorID();
//       int aUniID = aVxdID.getID();
//       sensorIter = activatedSensors.find(aUniID);
//       if (sensorIter == activatedSensors.end()) {
//         SensorStruct newSensor;
//        newSensor.layerID = aVxdID.getLayerNumber();
//         sensorIter = activatedSensors.insert(sensorIter, mapEntry(aUniID, newSensor)); //activatedSensors.find(aUniID);
//       }
//     }

/// testing purposes, can be deleted sometime soon...
// // // // //  vector<int> svdClusters;
// // // // //  BOOST_FOREACH( VXDTFTrackCandidate* aTC, m_tcVector) {
// // // // //    if ( aTC->getCondition() == false ) { continue; }
// // // // //    vector<int> svdHits = aTC->getSVDHitIndices();
// // // // //    BOOST_FOREACH(int index, svdHits) {
// // // // //      int clusterIndex = clustersOfEvent[index].getIndex();
// // // // //      svdClusters.push_back(clusterIndex);
// // // // //    }
// // // // //  }
// // // // //  std::sort(svdClusters.begin(), svdClusters.end());
// // // // //  std::unique(svdClusters.begin(), svdClusters.end());
// // // // //  int numTestClusters = svdClusters.size();
// // // // //  if ( numTestClusters not_eq 0 ) {
// // // // //    B2DEBUG(150, "event " << m_eventCounter << ": highest svdClusterIndex: " << numOfSvdClusters-1 << ", highest clusterInfoIndex: " << svdClusters[numTestClusters-1])
// // // // //  }

  /** Section 10 - calc QI for each TC */
  timeStamp = boostClock::now();
  /// since KF is rather slow, Kf will not be used when there are many overlapping TCs. In this case, the simplified QI-calculator will be used.
  bool allowKalman = false;
  if (m_PARAMcalcQIType == "kalman") { allowKalman = true; }
  if (totalOverlaps > 5000) {  /// WARNING: hardcoded value!
    B2ERROR("event " << m_eventCounter << ": total number of overlapping track candidates: " << totalOverlaps << ", termitating event!");
    m_TESTERbrokenEventsCtr++;

    /** cleaning part **/
    BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
      cleanEvent(currentPass, centerSector);
    }
    return;
  } else if (totalOverlaps > 500) {  /// WARNING: hardcoded value!
    B2WARNING("VXDTF event " << m_eventCounter << ": total number of overlapping TCs is " << totalOverlaps << " and therefore KF is too slow, will use simple QI calculation which produces worse results")
    allowKalman = false;
    m_TESTERkalmanSkipped++;
  }
  if (m_PARAMcalcQIType == "kalman" and allowKalman == true) {
//     calcQIbyKalman(m_tcVector, aPxdClusterArray, aSvdClusterArray, clustersOfEvent); /// calcQIbyKalman // old version, backup 13-03-29
    calcQIbyKalman(m_tcVector, aPxdClusterArray); /// calcQIbyKalman
  } else if (m_PARAMcalcQIType == "trackLength") {
    calcQIbyLength(m_tcVector, m_passSetupVector);                              /// calcQIbyLength
  } else { // if (m_PARAMcalcQIType == "circleFit") { // and if totalOverlaps > 500
    calcQIbyCircleFit(m_tcVector);                                              ///calcQIbyCircleFit
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.kalmanStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.kalmanStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 10 - end */



  /** Section 11 - cleanOverlappingSet */
  timeStamp = boostClock::now();
  if (totalOverlaps > 2 && m_PARAMcleanOverlappingSet == true) {
    int olSize = m_tcVectorOverlapped.size();
    int killedTCs = 1;
    int totalKilledTCs = 0, cleaningRepeatedCtr = 0;
    while (killedTCs != 0) {
      int beforeSize = m_tcVectorOverlapped.size();
      killedTCs = cleanOverlappingSet(m_tcVectorOverlapped); /// removes TCs which are found more than once completely
      totalOverlaps = m_tcVectorOverlapped.size();
      B2DEBUG(2, "out of funcCleanOverlappingSet: tcVectorBefore.size(): " << beforeSize << ", tcVectorAfter.size(): " << totalOverlaps << ", killed " << killedTCs << " TCs")

      totalKilledTCs += killedTCs;
      cleaningRepeatedCtr ++;
    }
    B2DEBUG(2, "out of funcCleanOverlappingSet: tcVectorBefore.size(): " << olSize << ", tcVectorAfter.size(): " << totalOverlaps << ", killed " << totalKilledTCs << " TCs within " << cleaningRepeatedCtr << " iterations")
    m_TESTERcleanOverlappingSetStartedCtr++;
    thisInfoPackage.numTCsKilledByCleanOverlap += totalKilledTCs;
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.cleanOverlap += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.cleanOverlap += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  /** Section 11 - end */



  /** testing purposes! */
  TCsOfEvent testTCVector;
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, m_tcVector) {
    if (currentTC->getCondition() == false) { continue; }
    if (currentTC->checkOverlappingState() == true) {
      testTCVector.push_back(currentTC);
    }
  }
  B2DEBUG(1, "size of tcVector: " << m_tcVector.size() << ", tcVectorOverlapped: " << m_tcVectorOverlapped.size() << " testTCVector: " << testTCVector.size() << ", totalOverlaps: " << totalOverlaps)

  isOB = false;
  countOverbookedClusters = 0;
  /// each clusterInfo knows which TCs are using it, following loop therefore checks each for overlapping ones
  BOOST_FOREACH(ClusterInfo aCluster, clustersOfEvent) {
    isOB = aCluster.isOverbooked();
    if (isOB == true) { countOverbookedClusters++; }
  } // now each TC knows whether it is overbooked or not (aCluster.isOverbooked() implicitly checked this)
  B2DEBUG(1, "after checking overlaps again: there are " << countOverbookedClusters << " clusters of " << clustersOfEvent.size() << " marked as 'overbooked'...")
  /** testing purposes - end */



  /** Section 12 - Hopfield */
  timeStamp = boostClock::now();
  if (m_PARAMuseHopfield == true) {
    /// checking overlapping TCs for best subset, if there are more than 2 different TC's
    if (totalOverlaps > 2) {
      hopfield(m_tcVectorOverlapped, m_PARAMomega);                             /// hopfield
    } else if (totalOverlaps == 2) {
      // for that easy situation we dont need the neuronal network, especially when the nn does sometimes chose the wrong one...
      if (m_tcVectorOverlapped[0]->getTrackQuality() > m_tcVectorOverlapped[1]->getTrackQuality()) {
        m_tcVectorOverlapped[1]->setCondition(false);
      } else { m_tcVectorOverlapped[0]->setCondition(false); }
    } else { B2DEBUG(10, " less than 2 overlapping Track Candidates found, no need for neuronal network") }
  }

  testTCVector.clear();
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, m_tcVector) {
    if (currentTC->getCondition() == false) { continue; }
    if (currentTC->checkOverlappingState() == true) {
      testTCVector.push_back(currentTC);
    }
  }
  int testTCs = testTCVector.size();
  B2DEBUG(1, "after Hopfield: size of tcVector: " << m_tcVector.size() << ", tcVectorOverlapped: " << m_tcVectorOverlapped.size() << " testTCVector: " << testTCs)
  if (testTCs != 0) {
    m_TESTERHopfieldLetsOverbookedTCsAliveCtr++;
    B2ERROR("event " << m_eventCounter << ": after Hopfield there are still some overlapping TCs! restarting hopfield...")
    /// checking overlapping TCs for best subset, if there are more than 2 different TC's
    if (testTCs > 2) {
      hopfield(testTCVector, m_PARAMomega);                             /// hopfield
    } else if (testTCs == 2) {
      // for that easy situation we dont need the neuronal network, especially when the nn does sometimes chose the wrong one...
      if (testTCVector[0]->getTrackQuality() > testTCVector[1]->getTrackQuality()) {
        testTCVector[1]->setCondition(false);
      } else { testTCVector[0]->setCondition(false); }
    } else { B2ERROR(" less 1 overlapping Track Candidate found after Hopfield, should not be possible!") }
  }
  isOB = false;
  countOverbookedClusters = 0;
  /// each clusterInfo knows which TCs are using it, following loop therefore checks each for overlapping ones
  BOOST_FOREACH(ClusterInfo aCluster, clustersOfEvent) {
    isOB = aCluster.isOverbooked();
    if (isOB == true) { countOverbookedClusters++; }
  } // now each TC knows whether it is overbooked or not (aCluster.isOverbooked() implicitly checked this)
  B2DEBUG(1, "after doing Hopfield again: there are " << countOverbookedClusters << " clusters of " << clustersOfEvent.size() << " marked as 'overbooked'...")



  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.neuronalStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.neuronalStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  B2DEBUG(1, "before exporting TCs, length of m_tcVector: " << m_tcVector.size() << ", m_tcVectorOverlapped: " << m_tcVectorOverlapped.size() << ", both include already dead TCs");
  /** Section 12 - end */



  /** Section 13 - generateGFTrackCand */
  timeStamp = boostClock::now();
  vector<int> totalIndices, tempIndices;
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, m_tcVector) {
    if (currentTC->getCondition() == false) { continue; }

    tempIndices = currentTC->getSVDHitIndices();
    totalIndices.insert(totalIndices.end(), tempIndices.begin(), tempIndices.end());


    GFTrackCand gfTC = generateGFTrackCand(currentTC, clustersOfEvent);                          /// generateGFTrackCand

    if (m_TESTERexpandedTestingRoutines == true) {
      VXDTFInfoBoard newBoard;
      StoreArray<VXDTFInfoBoard> extraInfo4GFTCs; // needed since I use it only within if-parenthesis

      int indexNumber = finalTrackCandidates.getEntries(); // valid for both, GFTrackCand and InfoBoard
      gfTC.setMcTrackId(indexNumber); // so the GFTrackCand knows which index of infoBoard is assigned to it

      newBoard.assignGFTC(indexNumber); // same number aDEBUGs for the GFTrackCand
      newBoard.fitIsPossible(currentTC->getFitSucceeded());
      newBoard.setProbValue(currentTC->getTrackQuality());
      extraInfo4GFTCs.appendNew(newBoard);
    }
    finalTrackCandidates.appendNew(gfTC);
  }
  int nTotalIndices = totalIndices.size();
  std::sort(totalIndices.begin(), totalIndices.end());
  std::unique(totalIndices.begin(), totalIndices.end());
  int nReducedIndices = totalIndices.size();

  int tcPos = 0, numOfFinalTCs = 0;
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, m_tcVector) {
    if (currentTC->getCondition() == false) { tcPos++; continue; }
    TVector3 momentum = currentTC->getInitialMomentum();
    double momValue = momentum.Mag();
    momentum.SetZ(0.);
    double pTValue = momentum.Mag();
    B2DEBUG(20, "event " << m_eventCounter << ": TC " << tcPos << " has got condition = true, means its a final TC (TC-output). Its total Momentum is" << momValue << ", its transverse momentum is: " << pTValue)
    numOfFinalTCs++;
    tcPos++;
  }
  B2DEBUG(1, "vxdtf - event " << m_eventCounter << ": " << finalTrackCandidates.getEntries() << " final track candidates determined! Having a total number of hit indices: "  << nTotalIndices << ", and after doube entry removal: " << nReducedIndices)
  m_TESTERcountTotalTCsFinal += finalTrackCandidates.getEntries();
  thisInfoPackage.numTCsfinal += finalTrackCandidates.getEntries();
  /** Section 13 - end */



  /** cleaning part **/
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    cleanEvent(currentPass, centerSector);
  }
  stopTimer = boostClock::now();
  m_TESTERtimeConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.sectionConsumption.intermediateStuff += boost::chrono::duration_cast<boostNsec>(stopTimer - timeStamp);
  thisInfoPackage.totalTime = boost::chrono::duration_cast<boostNsec>(stopTimer - beginEvent);
//  B2WARNING("event: " << m_eventCounter << ", duration : " << thisInfoPackage.totalTime.count());
  m_TESTERlogEvents.push_back(thisInfoPackage);
}


/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ********************************+  endRun +******************************** **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


void VXDTFModule::endRun()
{
  std::sort(m_TESTERbadSensors.begin(), m_TESTERbadSensors.end());
  std::unique(m_TESTERbadSensors.begin(), m_TESTERbadSensors.end());
  std::sort(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
  std::unique(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());
  std::reverse(m_TESTERlogEvents.begin(), m_TESTERlogEvents.end());

  B2INFO(" ############### VXDTF endRun ############### ")
  B2INFO("VXDTF, settings: number of passes: " << m_passSetupVector.size() << ", tuneCutoffs: " << m_PARAMtuneCutoffs << ", chosen settings: ")
  stringstream infoStuff2;
  string infoStuff;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    infoStuff + ", secUConfig: ";
    // std::string str = boost::lexical_cast<std::string>(dbl)
    BOOST_FOREACH(double limit, currentPass->secConfigU) { infoStuff + boost::lexical_cast<string>(limit) + " "; }
    infoStuff + ", secVConfig: ";
    BOOST_FOREACH(double limit, currentPass->secConfigV) { infoStuff + boost::lexical_cast<string>(limit) + " "; }
    B2INFO("VXDTF, passtype: " << currentPass->chosenDetectorType << ", setup: " << currentPass->sectorSetup << ", maxLayer: " << currentPass->highestAllowedLayer << ", minLayer: " << currentPass->minLayer << ", minState: " << currentPass->minState << " sfTests: " << currentPass->activatedSegFinderTests << ", nfTests: " << currentPass->activatedNbFinderTests << ", TccFilterTests: " << currentPass->activatedTccFilterTests << infoStuff)
  }

  infoStuff2 << ", following sensors had out of range hits: ";
  BOOST_FOREACH(string badSensor, m_TESTERbadSensors) {
    infoStuff2 << badSensor << " ";
  }
  infoStuff2 << endl;

  B2INFO("VXDTF, total number of PXDHits: " << m_totalPXDClusters << ", SVDClusters/Hits: " << m_totalSVDClusters << "/" << m_totalSVDClusters * 0.5 << ", SVDClusterCombinations: " << m_totalSVDClusterCombis << ", ghostHitRate: " << double(m_totalSVDClusterCombis) / (double(m_totalSVDClusters) * 0.5) << ". Discarded hits without sector: " << m_badSectorRangeCounter << ", without friends: " << m_badFriendCounter << ", distorted hits: " << m_TESTERdistortedHitCtr << " discarded Sensors bec. of missing second cluster: " << m_TESTERbadSectorRangeCounterForClusters << " mismathing number of u&vClusters per sensor: " << m_TESTERclustersPersSectorNotMatching << infoStuff2.str())
  B2INFO("VXDTF, segfinder segments activated/discarded: " << m_TESTERtotalsegmentsSFCtr << "/" << m_TESTERdiscardedSegmentsSFCtr << ", nbFinder segments activated/discarded: " << m_TESTERtotalsegmentsNFCtr << "/" << m_TESTERdiscardedSegmentsNFCtr)

  B2INFO("VXDTF, after " << m_eventCounter + 1 << " events, ZigZagXY triggered " << m_TESTERtriggeredZigZagXY << " times, ZigZagRZ triggered " << m_TESTERtriggeredZigZagRZ << " times, and dpT triggered " << m_TESTERtriggeredDpT << " times, TCC approved " << m_TESTERapprovedByTCC << " TCs, "  << m_TESTERbadHopfieldCtr << "/" << m_TESTERHopfieldLetsOverbookedTCsAliveCtr << " times, the Hopfield network had no survivors/accepted ovrelapping TCs!")
  B2INFO("VXDTF, total number of TCs after TCC: " << m_TESTERcountTotalTCsAfterTCC << ", after TCC-filter: " << m_TESTERcountTotalTCsAfterTCCFilter << ", final: " << m_TESTERcountTotalTCsFinal)
  B2INFO("VXDTF, numOfTimes calcInitialValues4TCs filtered TCs: " << m_TESTERfilteredBadSeedTCs << ", cleanOverlappingSet got activated:" << m_TESTERcleanOverlappingSetStartedCtr << ", cleanOverlappingSet killed numTCs: " << m_TESTERfilteredOverlapsQI << ", cleanOverlappingSet did/didn't filter TCs: " << m_TESTERfilteredOverlapsQICtr << "/" << m_TESTERNotFilteredOverlapsQI << ", QIfilterMode: " << m_PARAMcalcQIType)
  B2INFO("VXDTF, numOfTimes, where a kalman fit was possible: " << m_TESTERgoodFitsCtr << ", where it failed: " << m_TESTERbadFitsCtr << ", where the TF had to be terminated: " << m_TESTERbrokenEventsCtr << ", and " << m_TESTERbrokenCaRound << " CA rounds aborted, " << m_TESTERkalmanSkipped << " kalmanSkipped because of high occupancy, " << m_TESTERhighOccupancyCtr << "when highOccupancyMode was activated")

  string printOccupancy;
  for (int numOfHitsMinus1 = 0; numOfHitsMinus1 < int(m_TESTERSVDOccupancy.size()); ++ numOfHitsMinus1) {
    if (m_TESTERSVDOccupancy[numOfHitsMinus1] != 0) {
      printOccupancy = printOccupancy + " got " + boost::lexical_cast<string>(m_TESTERSVDOccupancy[numOfHitsMinus1]) + " times a sensor with " + boost::lexical_cast<string>(numOfHitsMinus1 + 1) + " hits\n";
    }
  }
  B2INFO(printOccupancy);
//  string printTime;

  stringstream printTime;
  printTime << "hitSorting took " << (m_TESTERtimeConsumption.hitSorting.count() * 0.001) << " microseconds\n";
  printTime << "segFinder took " << (m_TESTERtimeConsumption.segFinder.count() * 0.001) << " microseconds\n";
  printTime << "nbFinder took " << (m_TESTERtimeConsumption.nbFinder.count() * 0.001) << " microseconds\n";
  printTime << "cellularAutomaton took " << (m_TESTERtimeConsumption.cellularAutomaton.count() * 0.001) << " microseconds\n";
  printTime << "tcc took " << (m_TESTERtimeConsumption.tcc.count() * 0.001) << " microseconds\n";
  printTime << "postCAFilter took " << (m_TESTERtimeConsumption.postCAFilter.count() * 0.001) << " microseconds\n";
  printTime << "kalmanStuff took " << (m_TESTERtimeConsumption.kalmanStuff.count() * 0.001) << " microseconds\n";
  printTime << "checkOverlap took " << (m_TESTERtimeConsumption.checkOverlap.count() * 0.001) << " microseconds\n";
  printTime << "cleanOverlap took " << (m_TESTERtimeConsumption.cleanOverlap.count() * 0.001) << " microseconds\n";
  printTime << "neuronalStuff took " << (m_TESTERtimeConsumption.neuronalStuff.count() * 0.001) << " microseconds\n";
  printTime << "intermediateStuff took " << (m_TESTERtimeConsumption.intermediateStuff.count() * 0.001) << " microseconds\n";
  B2INFO(printTime.str());


  B2INFO(" ##### VXDTF extra analysis ##### ")
  int numLoggedEvents = m_TESTERlogEvents.size();
  int median = numLoggedEvents / 2;
  int q10 = numLoggedEvents / 10;
  int q90 = 9 * q10;
  B2INFO(" there were " << numLoggedEvents << " events recorded by the eventLogger, listing slowest, fastest, median q0.1 and q0.9 event:")
  int meanTimeConsumption = 0;
  BOOST_FOREACH(EventInfoPackage infoPackage, m_TESTERlogEvents) {
    meanTimeConsumption += infoPackage.totalTime.count();
  }
  B2INFO("slowest event: " << m_TESTERlogEvents[0].Print());
  B2INFO("q10 event: " << m_TESTERlogEvents[q10].Print());
  B2INFO("median event: " << m_TESTERlogEvents[median].Print());
  B2INFO("q90 event: " << m_TESTERlogEvents[q90].Print());
  B2INFO("fastest event: " << m_TESTERlogEvents[numLoggedEvents - 1].Print());
  B2INFO("manually calculated mean: " << meanTimeConsumption / numLoggedEvents << ", and median: " << m_TESTERlogEvents[median].totalTime.count() << " of time consumption per event");

  B2INFO(" ############### VXDTF endRun - end ############### ")
}



void VXDTFModule::terminate()
{
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    BOOST_FOREACH(secMapEntry aSector, currentPass->sectorMap) {
      delete aSector.second;
    }
    currentPass->sectorMap.clear();
    delete currentPass; // deleting struct itself
  }
  m_passSetupVector.clear();
}



/** ***** delFalseFriends ***** **/
///checks state of inner neighbours and removes incompatible and virtual ones
void VXDTFModule::delFalseFriends(CurrentPassData* currentPass, TVector3 primaryVertex)
{
  std::vector<VXDSegmentCell*> segmentsOfSector;
  OperationSequenceOfActivatedSectors::iterator secSequenceIter;
  for (secSequenceIter = currentPass->sectorSequence.begin(); secSequenceIter != currentPass->sectorSequence.end(); ++secSequenceIter) {
    MapOfSectors::const_iterator currentSectorIter = secSequenceIter->second;
    segmentsOfSector = currentSectorIter->second->getInnerSegmentCells(); // loading segments of sector
    BOOST_FOREACH(VXDSegmentCell * segment, segmentsOfSector) {
      segment->kickFalseFriends(primaryVertex);
    }
  }
}



/** ***** findTCs ***** **/
///Recursive CoreFunction of the Track Candidate Collector, stores every possible way starting at a Seed (VXDSegmentCell)
void VXDTFModule::findTCs(TCsOfEvent& tcList,  VXDTFTrackCandidate* currentTC, std::string maxLayer)
{
  VXDSegmentCell* pLastSeg = currentTC->getSegments().back(); // get last entry in segList of TC
  if (pLastSeg->getOuterHit()->getSectorName()[0] != maxLayer) { pLastSeg->setSeed(false); }

  const std::list<VXDSegmentCell*>& neighbours = pLastSeg->getAllInnerNeighbours();
  std::list<VXDSegmentCell*>::const_iterator nbIter = neighbours.begin();
  int nbSize = neighbours.size();

  if (nbSize == 0)  { // currentTC is complete
    tcList.push_back(currentTC);
  } else if (nbSize == 1) {
    VXDSegmentCell* pNextSeg =  *nbIter;
    currentTC->addSegments(pNextSeg);
    currentTC->addHits(pNextSeg->getInnerHit()); // automatically informs current Hit that it got owned by a TC

    findTCs(tcList, currentTC, maxLayer);

  } else { // nbSize > 1
    nbIter++; // iterator points to the second entry of the list of neighbours!
    for (; nbIter != neighbours.end(); nbIter++) {
      VXDSegmentCell* pNextSeg =  *nbIter;
      VXDTFTrackCandidate* pTCCopy = new VXDTFTrackCandidate(currentTC);   // make a clone of old TC
      pTCCopy->addSegments(pNextSeg);
      pTCCopy->addHits(pNextSeg->getInnerHit());

      findTCs(tcList, pTCCopy, maxLayer);
    } // makes clones of current TC for each neighbour (excluding the first one) and adds it to the TCs if neighbour fits in the scheme...

    //separate step for the first neighbour in line (has to be done after the cloning parts)...
    VXDSegmentCell* pNextSeg =  *(neighbours.begin());
    currentTC->addSegments(pNextSeg);
    currentTC->addHits(pNextSeg->getInnerHit());

    findTCs(tcList, currentTC, maxLayer);
  }
}



/** ***** calcQQQscore ***** **/
/// calculates integer score for current filter (all filters combined deliver the QQQ (normed to 0-1)) WARNING although currently not in use, maybe needed later...
int VXDTFModule::calcQQQscore(std::vector<std::pair<std::string, double> > quantiles, double currentValue, int numOfQuantiles)
{
  int score;
  int i = 0;
  double currentQuantile = quantiles[i].second;
  if (currentValue < currentQuantile) {
    /// is smaller than min value
    score = 0;
  } else if (currentValue > quantiles.back().second) {
    /// is bigger than max value
    score = 0;
  } else {
    do {
      i++;
      currentQuantile = quantiles[i].second;
      if (i >= int(quantiles.size())) { B2ERROR("VXDTFModule-CalcQQQScore: wrong index count!")}
    } while (currentValue > currentQuantile);
//      std::cout << " score " << currentValue << " liegt zwischen " << quantiles[i-1].second << " und " << quantiles[i].second << std::endl;
//      double marker = double(i);
    // < double(numOfQuantiles-1)/2.0 {
    int center = (numOfQuantiles + 1) / 2;
    if (i < center) {
      score = i;
    } else if (i > center) {
      score = center - (i - (center - 1));
    } else { score = center; }
//      std::cout << " score ist: " << score << " und i ist: " << i << std::endl;
  }
  return score;
}



/** ***** calcQQQscoreDeltas ***** **/
/// calculates integer score for current filter (all filters combined deliver the QQQ (normed to 0-1)), for normedDist3D, deltaPt... WARNING although currently not in use, maybe needed later...
int VXDTFModule::calcQQQscoreDeltas(std::vector<std::pair<std::string, double> > quantiles, double currentValue, int numOfQuantiles)
{
  int score;
  int center = (numOfQuantiles + 1) / 2;
  int i = 0;
  double currentQuantile = quantiles[i].second;
  if (currentValue < currentQuantile) {
    /// is smaller than min value
    score = center;
  } else if (currentValue > quantiles.back().second) {
    score = 0;
  } else {
    do {
      i++;
      currentQuantile = quantiles[i].second;
      if (i >= int(quantiles.size())) { B2ERROR("VXDTFModule-CalcQQQScoreDeltas: wrong index count!"); i = numOfQuantiles + 1; break;}
    } while (currentValue < currentQuantile);
    score = (numOfQuantiles + 1 - i) / 2;
  }
  return score;
}



/** ***** hopfield ***** **/
/// Neuronal network filtering overlapping Track Candidates by searching best subset of TC's
void VXDTFModule::hopfield(TCsOfEvent& tcVector, double omega)
{
  int numOfTCs = tcVector.size();

  // TMatrixD = MatrixT <double>
  TMatrixD W(numOfTCs, numOfTCs);  /// weight matrix, knows compatibility between each possible pair of TCs
  TMatrixD xMatrix(1, numOfTCs); //
  TMatrixD xMatrixOld(1, numOfTCs);
  TMatrixD actMatrix(1, numOfTCs);
  TMatrixD tempMatrix(1, numOfTCs);
  TMatrixD tempXMatrix(1, numOfTCs);

  int countCasesWhen2NeuronsAreCompatible = 0;
  double compatibleValue = (1.0 - omega) / double(numOfTCs - 1);
  for (int itrk = 0; itrk < numOfTCs; itrk++) {

    list<int>  hitsItrk = tcVector[itrk]->getHopfieldHitIndices();
    int numOfHitsItrk = hitsItrk.size();

    for (int jtrk = itrk + 1; jtrk < numOfTCs; jtrk++) {

      list<int> hitsBoth = hitsItrk;
      list<int> hitsJtrk = tcVector[jtrk]->getHopfieldHitIndices();
      int numOfHitsJtrk = hitsJtrk.size();

      hitsBoth.insert(hitsBoth.end(), hitsJtrk.begin(), hitsJtrk.end());
      hitsBoth.sort();
      hitsBoth.unique();

      int numOfHitsBoth = hitsBoth.size();

      if (numOfHitsBoth < (numOfHitsItrk + numOfHitsJtrk)) { // means they share hits if true
        W(itrk, jtrk) = -1.0;
        W(jtrk, itrk) = -1.0;
      } else {
        W(itrk, jtrk) = compatibleValue;
        W(jtrk, itrk) = compatibleValue;
        countCasesWhen2NeuronsAreCompatible++;
      }
    }
  } // checking compatibility of TCs (compatible if they dont share hits, not compatible if they share ones)

  if (countCasesWhen2NeuronsAreCompatible == 0) {
    B2DEBUG(1, "VXDTF event " << m_eventCounter << ": hopfield: no compatible neurons found, chosing TC by best QI...");
    int tempIndex = 0;
    double tempQI = tcVector[0]->getTrackQuality(), tempQI2 = 0;
    for (int i = 1; i < numOfTCs; i++) {
      tempQI2 = tcVector[i]->getTrackQuality();
      if (tempQI < tempQI2) { tempIndex = i; }
    }
    for (int i = 0; i < numOfTCs; i++) {
      if (i != tempIndex) {
        tcVector[i]->setCondition(false);
      }
    }
    return; // leaving hopfield after chosing the last man standing
  }

  stringstream printOut;
  printOut << " weight matrix W: " << endl << endl;
  for (int aussen = 0; aussen < numOfTCs; aussen++) {
    for (int innen = 0; innen < numOfTCs; innen++) {
      printOut << W(aussen, innen) << "\t";
    }
    printOut << endl;
  }
  printOut << endl;

  B2DEBUG(100, printOut);

  vector<int> sequenceVector(numOfTCs);
  double rNum;

  for (int i = 0; i < numOfTCs; i++) {
    rNum = gRandom->Uniform(1.0);
    xMatrix(0, i) = rNum;
    sequenceVector[i] = i;
  }

  double T = 3.1; // original: 3.1
  double Tmin = 0.1;
  double cmax = 0.05;
  double c = 1.0;
  double act = 0.0;

  int nNcounter = 0;

  while (c > cmax) {

    random_shuffle(sequenceVector.begin(), sequenceVector.end(), rngWrapper);

    xMatrixOld = xMatrix;

    BOOST_FOREACH(int i, sequenceVector) {
      double aTempVal = 0.0;
      for (int a = 0; a < numOfTCs; a++) { aTempVal = aTempVal + W(i, a) * xMatrix(0, a); } // doing it by hand...

      act = aTempVal + tcVector[i]->getTrackQuality(); //tempMatrix(0,0);

      xMatrix(0, i) = 0.5 * (1. + tanh(act / T));

      B2DEBUG(100, "tc, random number " << i << " -  old value: " << xMatrix(0, i))
    }

    T = 0.5 * (T + Tmin);
    nNcounter += 1;

    tempMatrix = (xMatrix - xMatrixOld);
    tempMatrix.Abs();
    c = tempMatrix.Max();
    B2DEBUG(100, " c value: " << c)

    xMatrixOld = xMatrix;
    nNcounter++;
  }

  B2DEBUG(1, "Hopfield network - found subset of TCs within " << nNcounter << " iterations... with c=" << c);
  list<VXDTFHit*> allHits;
  vector<VXDTFHit*> currentHits;
  int survivorCtr = 0;
  for (int i = 0; i < numOfTCs; i++) {
    B2DEBUG(10, "tc " << i << " - got final neuron value: " << xMatrix(0, i) << " while having " << int((tcVector[i]->getHits()).size()) << " hits and quality indicator " << tcVector[i]->getTrackQuality())
//    xMatrix(0,i) = floor(xMatrix(0,i) + 0.5);
    if (xMatrix(0, i) > 0.7) { /// do we want to let this threshold hardcoded?
      tcVector[i]->setCondition(true);
      survivorCtr++;
    } else {
      tcVector[i]->setCondition(false);
    }
    tcVector[i]->setNeuronValue(xMatrix(0, i));

    bool condi = tcVector[i]->getCondition();
    if (condi == true) {
      currentHits = tcVector[i]->getHits();
      for (int j = 0; j < int(currentHits.size()); ++j) { allHits.push_back(currentHits[j]); }
    }
//    if (m_eventCounter > 215 ) { B2WARNING("NN event "<<m_eventCounter<<": "<<i<< " -  new value: "<<xMatrix(0,i)<<", condition: "<<condi) }
  }
  if (survivorCtr == 0) {
    m_TESTERbadHopfieldCtr++;
    B2DEBUG(1, "VXDTF event " << m_eventCounter << ": hopfield had no survivors! ")
    for (int i = 0; i < numOfTCs; i++) {
      B2DEBUG(1, "tc " << i << " - got final neuron value: " << xMatrix(0, i) << " while having " << int((tcVector[i]->getHits()).size()) << " hits and quality indicator " << tcVector[i]->getTrackQuality())
    }
  }
  int sizeOld = allHits.size();
  allHits.sort(); allHits.unique();
  int sizeNew = allHits.size();
  if (sizeOld != sizeNew) { B2ERROR("NN event " << m_eventCounter << ": illegal result! Overlapping TCs accepted!")}
}



/** ***** searchSector4Hit ***** **/
/// searches for sectors fitting current hit coordinates, returns blank string if nothing could be found
pair<string, VXDTFModule::MapOfSectors::iterator> VXDTFModule::searchSector4Hit(VxdID aVxdID,
    TVector3 localHit,
    TVector3 sensorSize,
    VXDTFModule::MapOfSectors& m_sectorMap,
    vector<double>& uConfig,
    vector<double>& vConfig)
{
  VXDTFModule::MapOfSectors::iterator secMapIter = m_sectorMap.begin();
  int aUniID = aVxdID.getID();
  int aLayerID = aVxdID.getLayerNumber();
  int aSecID;
  string aSectorName = "";

  for (int j = 0; j != int(uConfig.size() - 1); ++j) {
    B2DEBUG(1000, "uCuts(j)*uSize: " << uConfig[j]*sensorSize[0] << " uCuts(j+1)*uSize: " << uConfig[j + 1]*sensorSize[0]);

    if (localHit[0] >= (uConfig[j]*sensorSize[0] * 2) && localHit[0] <= (uConfig[j + 1]*sensorSize[0] * 2)) {
      for (int k = 0; k != int(vConfig.size() - 1); ++k) {
        B2DEBUG(1000, " vCuts(k)*vSize: " << vConfig[k]*sensorSize[1] << " vCuts(k+1)*vSize: " << vConfig[k + 1]*sensorSize[1]);

        if (localHit[1] >= (vConfig[k]*sensorSize[1] * 2) && localHit[1] <= (vConfig[k + 1]*sensorSize[1] * 2)) {
          aSecID = k + 1 + j * (vConfig.size() - 1);

          aSectorName = (boost::format("%1%0_%2%_%3%") % aLayerID % aUniID % aSecID).str();

          secMapIter = m_sectorMap.find(aSectorName);

          if (secMapIter == m_sectorMap.end()) {
            aSectorName = (boost::format("%1%1_%2%_%3%") % aLayerID % aUniID % aSecID).str();
            secMapIter = m_sectorMap.find(aSectorName);
          }

          if (secMapIter == m_sectorMap.end()) {
            aSectorName = (boost::format("-%1%1_%2%_%3%") % aLayerID % aVxdID.getLadderNumber() % aVxdID.getSensorNumber()).str();
            /*aSectorName = "";*/ // if there is still no sector found, then the hit doesn't lie in defined area.
          }
        }
      }
    }
  } //sector-searching loop
  pair<string, VXDTFModule::MapOfSectors::iterator> result;
  result = make_pair(aSectorName, secMapIter);
  return result;
}



bool VXDTFModule::compareSecSequence(pair<string, MapOfSectors::iterator>& lhs, pair<string, MapOfSectors::iterator>& rhs)
{
  int compareValue = lhs.first.compare(rhs.first); // if lhs.first < rhs.first -> compareValue = -1;
  if (compareValue < 0) {
    return true;
  } else { return false; }
}



/** ***** segFinder ***** **/
/// searches for segments in given pass and returns number of discarded segments
int VXDTFModule::segFinder(CurrentPassData* currentPass)
{
  string currentLayerID, friendLayerID, currentScope, currentSecID, currentFriendID, oldFriendID, ownSecName, friendSecName;
  TVector3 currentCoords, friendCoords, currentVector, tempVector;
  bool accepted = false; // recycled return value of the filters
  int simpleSegmentQI; // better than segmentApproved, but still digital (only min and max cutoff values), but could be weighed by order of relevance
//   int scoreQQQ;  //best would be quasi analogue quality indicator QAQI
  int discardedSegmentsCounter = 0;
  OperationSequenceOfActivatedSectors::const_iterator secSequenceIter;
  MapOfSectors::iterator mainSecIter;
  MapOfSectors::iterator currentFriendSecIter;
  for (secSequenceIter = currentPass->sectorSequence.begin(); secSequenceIter != currentPass->sectorSequence.end(); ++secSequenceIter) {
    B2DEBUG(1000, "SectorSequence is called " << secSequenceIter->first);
    mainSecIter = secSequenceIter->second;
    currentLayerID = mainSecIter->second->getSecID()[0];
    currentSecID = mainSecIter->second->getSecID();
    B2DEBUG(1000, " checking " << currentSecID)
    vector<string> hisFriends = mainSecIter->second->getFriends(); // loading friends of sector
    int nFriends = hisFriends.size();

    vector<VXDTFHit*> allFriendHits;
    for (int i = 0; i < nFriends; ++i) {
      string thisFriendSector = hisFriends[i];
      B2DEBUG(1000, " > friendSector is called: " << thisFriendSector);
      currentFriendSecIter = currentPass->sectorMap.find(thisFriendSector);
      if (currentFriendSecIter == currentPass->sectorMap.end()) {
        B2DEBUG(1, "event " << m_eventCounter << ": friendSector " << thisFriendSector << " not found. No friendHits imported...");
        m_badFriendCounter++;
        continue;
      } else {
        vector<VXDTFHit*> friendHits = currentFriendSecIter->second->getHits();
        allFriendHits.insert(allFriendHits.end(), friendHits.begin(), friendHits.end());
      }
    } // iterating through friendsectors and importing their containing hits

    currentFriendSecIter = currentPass->sectorMap.begin(); // reset after first usage
    vector<VXDTFHit*> ownHits = mainSecIter->second->getHits(); // loading own hits of sector

    int numOfCurrentHits = ownHits.size();
    for (int currentHit = 0; currentHit < numOfCurrentHits; currentHit++) {

      currentCoords = ownHits[currentHit]->getHitCoordinates();

      int numOfFriendHits = allFriendHits.size();
      if (numOfFriendHits == 0 && ownHits[currentHit]->getNumberOfSegments() == 0) {
        B2DEBUG(10, "event " << m_eventCounter << ": current Hit has no friendHits although layer is " << mainSecIter->second->getSecID()[0] << " and secID: " << ownHits[currentHit]->getSectorName())
      }
      B2DEBUG(50, "Sector " << mainSecIter->first << " has got " << numOfFriendHits << " hits lying among his friends! ");
      for (int friendHit = 0; friendHit < numOfFriendHits; ++friendHit) {
        simpleSegmentQI = 0;
//         scoreQQQ = 0;

        currentFriendID = allFriendHits[friendHit]->getSectorName();
        if (currentFriendID != oldFriendID) {
          currentFriendSecIter = currentPass->sectorMap.find(currentFriendID);
          friendLayerID = currentFriendID[0];
        }
        //         currentScope = str(format("L%1%L%2%") % friendLayerID % currentLayerID);

        friendCoords = allFriendHits[friendHit]->getHitCoordinates();
        m_twoHitFilterBox.resetValues(currentCoords, friendCoords, mainSecIter->second, currentFriendID);

        if (currentPass->distance3D.first == true) { // min & max!
          accepted = m_twoHitFilterBox.checkDist3D(m_filterNameBox.nameDistance3D);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " dist3d: segment approved!")
          } else {
            B2DEBUG(150, " dist3d: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_twoHitFilterBox.getCutoffs(m_filterNameBox.nameDistance3D);
              B2WARNING("dist3D - minCutoff: " << cutoffs.first << ", calcValue: " << m_twoHitFilterBox.calcDist3D() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " dist3d is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->distanceXY.first == true) { // min & max!
          accepted = m_twoHitFilterBox.checkDistXY(m_filterNameBox.nameDistanceXY);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " distxy: segment approved!")
          } else {
            B2DEBUG(150, " distxy: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_twoHitFilterBox.getCutoffs(m_filterNameBox.nameDistanceXY);
              B2WARNING("distxy - minCutoff: " << cutoffs.first << ", calcValue: " << m_twoHitFilterBox.calcDistXY() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " distxy is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->distanceZ.first == true) { // min & max!
          accepted = m_twoHitFilterBox.checkDistZ(m_filterNameBox.nameDistanceZ);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " distz: segment approved!")
          } else {
            B2DEBUG(150, " distz: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_twoHitFilterBox.getCutoffs(m_filterNameBox.nameDistanceZ);
              B2WARNING("distz - minCutoff: " << cutoffs.first << ", calcValue: " << m_twoHitFilterBox.calcDistZ() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " distz is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->normedDistance3D.first == true) { // max only
          accepted = m_twoHitFilterBox.checkNormedDist3D(m_filterNameBox.nameNormedDistance3D);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " normeddist3d: segment approved!")
          } else {
            B2DEBUG(150, " normeddist3d: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_twoHitFilterBox.getCutoffs(m_filterNameBox.nameNormedDistance3D);
              B2WARNING("normeddist3d - minCutoff: " << cutoffs.first << ", calcValue: " << m_twoHitFilterBox.calcNormedDist3D() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " normeddist3d is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->slopeRZ.first == true) { // min & max!
          accepted = m_twoHitFilterBox.checkSlopeRZ(m_filterNameBox.nameSlopeRZ);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " slopeRZ: segment approved!")
          } else {
            B2DEBUG(150, " slopeRZ: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_twoHitFilterBox.getCutoffs(m_filterNameBox.nameSlopeRZ);
              B2WARNING("slopeRZ - minCutoff: " << cutoffs.first << ", calcValue: " << m_twoHitFilterBox.calcSlopeRZ() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " slopeRZ is not activated for pass: " << currentPass->sectorSetup << "!") }


        if (simpleSegmentQI < currentPass->activatedSegFinderTests) {
          B2DEBUG(150, "SegFINDER: segment discarded! simpleSegmentQI = " << simpleSegmentQI << ", threshold: " << currentPass->activatedSegFinderTests)
          discardedSegmentsCounter++;
          oldFriendID = currentFriendID;
          continue;
        }

        if (m_highOccupancyCase == true && currentHit < (numOfCurrentHits - 1)) {
          TVector3 origin(0., 0., 0.);
          m_threeHitFilterBox.resetValues(currentCoords, friendCoords, origin, mainSecIter->second, currentFriendID);
          bool testPassed = SegFinderHighOccupancy(currentPass, m_threeHitFilterBox);
          if (testPassed == false) {
            B2DEBUG(150, "SegFINDERHighOccupancy: segment discarded! ")
            discardedSegmentsCounter++;
            oldFriendID = currentFriendID;
            continue;
          }
        }
        B2DEBUG(100, " segment approved")
        VXDSegmentCell* pCell = new VXDSegmentCell(ownHits[currentHit],
                                                   allFriendHits[friendHit],
                                                   mainSecIter,
                                                   currentFriendSecIter);

        currentPass->activeCellList.push_back(pCell);
        currentPass->totalCellVector.push_back(pCell);
        int segPos = currentPass->totalCellVector.size() - 1;
        allFriendHits[friendHit]->addOuterCell(segPos);
        ownHits[currentHit]->addInnerCell(segPos);

        mainSecIter->second->addInnerSegmentCell(pCell);
        currentFriendSecIter->second->addOuterSegmentCell(pCell);
        oldFriendID = currentFriendID;
      } //iterating through all my friendHits
    }
  } // iterating through all active sectors - segFinder

  return discardedSegmentsCounter;
}




bool VXDTFModule::SegFinderHighOccupancy(CurrentPassData* currentPass, NbFinderFilters& threeHitFilterBox)
{
  int simpleSegmentQI = 0;
  bool accepted;

  if (currentPass->anglesHighOccupancy3D.first == true) { // min & max!
    accepted = threeHitFilterBox.checkAngle3D(m_filterNameBox.nameAnglesHighOccupancy3D);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " anglesHighOccupancy3D: segment approved!")
    } else {
      B2DEBUG(150, " anglesHighOccupancy3D: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.nameAnglesHighOccupancy3D);
        B2WARNING("anglesHighOccupancy3D - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcAngle3D() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " anglesHighOccupancy3D is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->anglesHighOccupancyXY.first == true) { // min & max!
    accepted = threeHitFilterBox.checkAngleXY(m_filterNameBox.nameAnglesHighOccupancyXY);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " anglesHighOccupancyxy: segment approved!")
    } else {
      B2DEBUG(150, " anglesHighOccupancyxy: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.nameAnglesHighOccupancyXY);
        B2WARNING("anglesHighOccupancyxy - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcAngleXY() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " anglesHighOccupancyxy is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->anglesHighOccupancyRZ.first == true) { // min & max!
    accepted = threeHitFilterBox.checkAngleRZ(m_filterNameBox.nameAnglesHighOccupancyRZ);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " anglesHighOccupancyRZ: segment approved!")
    } else {
      B2DEBUG(150, " anglesHighOccupancyRZ: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.nameAnglesHighOccupancyRZ);
        B2WARNING("anglesHighOccupancyRZ - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcAngleRZ() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " anglesHighOccupancyRZ is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->distanceHighOccupancy2IP.first == true) { // max only
    accepted = threeHitFilterBox.checkCircleDist2IP(m_filterNameBox.nameDistanceHighOccupancy2IP);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " distanceHighOccupancy2IP: segment approved!")
    } else {
      B2DEBUG(150, " distanceHighOccupancy2IP: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.nameDistanceHighOccupancy2IP);
        B2WARNING("distanceHighOccupancy2IP - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcCircleDist2IP() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " distanceHighOccupancy2IP is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->deltaSlopeHighOccupancyRZ.first == true) { // min & max!
    accepted = threeHitFilterBox.checkDeltaSlopeRZ(m_filterNameBox.nameDeltaSlopeHighOccupancyRZ);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " deltaSlopeHighOccupancyRZ: segment approved!")
    } else {
      B2DEBUG(150, " deltaSlopeHighOccupancyRZ: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.nameDeltaSlopeHighOccupancyRZ);
        B2WARNING("deltaSlopeHighOccupancyRZ - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcDeltaSlopeRZ() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " deltaSlopeHighOccupancyRZ is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->pTHighOccupancy.first == true) { // min & max!
    accepted = threeHitFilterBox.checkPt(m_filterNameBox.namePTHighOccupancy);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " pTHighOccupancy: segment approved!")
    } else {
      B2DEBUG(150, " pTHighOccupancy: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.namePTHighOccupancy);
        B2WARNING("pTHighOccupancy - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcPt() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " pTHighOccupancy is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->helixHighOccupancyFit.first == true) { // min & max!
    accepted = threeHitFilterBox.checkHelixFit(m_filterNameBox.nameHelixHighOccupancyFit);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " helixHighOccupancyFit: segment approved!")
    } else {
      B2DEBUG(150, " helixHighOccupancyFit: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = threeHitFilterBox.getCutoffs(m_filterNameBox.nameHelixHighOccupancyFit);
        B2WARNING("helixHighOccupancyFit - minCutoff: " << cutoffs.first << ", calcValue: " << threeHitFilterBox.calcHelixFit() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " slopeRZ is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (simpleSegmentQI < currentPass->activatedHighOccupancySegFinderTests) {
    return false;
  } else { return true; }
}




/** ***** neighbourFinder ***** **/
/// filters neighbouring segments in given pass and returns number of discarded segments
int VXDTFModule::neighbourFinder(CurrentPassData* currentPass)
{
  bool accepted = false; // recycled return value of the filters
  int NFdiscardedSegmentsCounter = 0;
  string  centerLayerID, currentFriendID; // not needed: outerLayerID, innerLayerID
  TVector3 outerCoords, outerCenterCoords, centerCoords, innerCenterCoords, innerCoords, outerVector, centerVector, innerVector, outerTempVector;
  TVector3 innerTempVector, cpA/*central point of innerSegment*/, cpB/*central point of mediumSegment*/, nA/*normal vector of segment a*/, nB/*normal vector of segment b*/, intersectionAB;
  int simpleSegmentQI; // better than segmentApproved, but still digital (only min and max cutoff values), but could be weighed by order of relevance
  int centerLayerIDNumber = 0;
  OperationSequenceOfActivatedSectors::const_iterator secSequenceIter;
  MapOfSectors::iterator mainSecIter;
  MapOfSectors::iterator friendSecIter;
  for (secSequenceIter = currentPass->sectorSequence.begin(); secSequenceIter != currentPass->sectorSequence.end(); ++secSequenceIter) {
    B2DEBUG(150, "SectorSequence is named " << (*secSequenceIter).first);
    mainSecIter = (*secSequenceIter).second;

    vector<VXDSegmentCell*> outerSegments = mainSecIter->second->getInnerSegmentCells(); // loading segments of sector
    int nOuterSegments = outerSegments.size();
    for (int thisOuterSegment = 0; thisOuterSegment < nOuterSegments; thisOuterSegment++) {
      outerCoords = outerSegments[thisOuterSegment]->getOuterHit()->getHitCoordinates();
      centerCoords = outerSegments[thisOuterSegment]->getInnerHit()->getHitCoordinates();
      currentFriendID = outerSegments[thisOuterSegment]->getInnerHit()->getSectorName();
      centerLayerID = currentFriendID[0];
      centerLayerIDNumber = boost::lexical_cast<int>(centerLayerID);

      const vector<int>& innerSegments = outerSegments[thisOuterSegment]->getInnerHit()->getAttachedInnerCell();
      int nInnerSegments = innerSegments.size();

      for (int thisInnerSegment = 0; thisInnerSegment < nInnerSegments; thisInnerSegment++) {
        VXDSegmentCell* currentInnerSeg = currentPass->totalCellVector[innerSegments[thisInnerSegment]];
        innerCoords = currentInnerSeg->getInnerHit()->getHitCoordinates();

        simpleSegmentQI = 0;
        m_threeHitFilterBox.resetValues(outerCoords, centerCoords, innerCoords, mainSecIter->second, currentFriendID);

        if (currentPass->angles3D.first == true) { // min & max!
          accepted = m_threeHitFilterBox.checkAngle3D(m_filterNameBox.nameAngles3D);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " angles3D: segment approved!")
          } else {
            B2DEBUG(150, " angles3D: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.nameAngles3D);
              B2WARNING("angles3D - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcAngle3D() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " angles3D is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->anglesXY.first == true) { // min & max!
          accepted = m_threeHitFilterBox.checkAngleXY(m_filterNameBox.nameAnglesXY);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " anglesXY: segment approved!")
          } else {
            B2DEBUG(150, " anglesXY: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.nameAnglesXY);
              B2WARNING("anglesXY - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcAngleXY() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " anglesXY is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->anglesRZ.first == true) { // min & max!
          accepted = m_threeHitFilterBox.checkAngleRZ(m_filterNameBox.nameAnglesRZ);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " anglesRZ: segment approved!")
          } else {
            B2DEBUG(150, " anglesRZ: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.nameAnglesRZ);
              B2WARNING("anglesRZ - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcAngleRZ() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " anglesRZ is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->distance2IP.first == true) { // max only
          accepted = m_threeHitFilterBox.checkCircleDist2IP(m_filterNameBox.nameDistance2IP);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " distance2IP: segment approved!")
          } else {
            B2DEBUG(150, " distance2IP: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.nameDistance2IP);
              B2WARNING("distance2IP - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcCircleDist2IP() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " distance2IP is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->deltaSlopeRZ.first == true) { // min & max!
          accepted = m_threeHitFilterBox.checkDeltaSlopeRZ(m_filterNameBox.nameDeltaSlopeRZ);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " deltaSlopeRZ: segment approved!")
          } else {
            B2DEBUG(150, " deltaSlopeRZ: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.nameDeltaSlopeRZ);
              B2WARNING("deltaSlopeRZ - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcDeltaSlopeRZ() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " deltaSlopeRZ is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->pT.first == true) { // min & max!
          accepted = m_threeHitFilterBox.checkPt(m_filterNameBox.namePT);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " pT: segment approved!")
          } else {
            B2DEBUG(150, " pT: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.namePT);
              B2WARNING("pT - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcPt() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " pT is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (currentPass->helixFit.first == true) { // min & max!
          accepted = m_threeHitFilterBox.checkHelixFit(m_filterNameBox.nameHelixFit);
          if (accepted == true) {
            simpleSegmentQI++;
            B2DEBUG(150, " helixFit: segment approved!")
          } else {
            B2DEBUG(150, " helixFit: segment discarded!")
            if (m_PARAMDebugMode == true) {
              pair <double, double> cutoffs = m_threeHitFilterBox.getCutoffs(m_filterNameBox.nameHelixFit);
              B2WARNING("helixFit - minCutoff: " << cutoffs.first << ", calcValue: " << m_threeHitFilterBox.calcHelixFit() << ", maxCutoff: " << cutoffs.second)
            }
          } // else segment not approved
        } else { B2DEBUG(175, " helixFit is not activated for pass: " << currentPass->sectorSetup << "!") }

        if (simpleSegmentQI < currentPass->activatedNbFinderTests) {
          B2DEBUG(150, "neighbourFINDER: segment discarded! simpleSegmentQI = " << simpleSegmentQI);
          continue;
        }
        if (m_highOccupancyCase == true) {
          TVector3 origin(0., 0., 0.);
          if (origin != innerCoords) {
            m_fourHitFilterBox.resetValues(outerCoords, centerCoords, innerCoords, origin, mainSecIter->second, currentFriendID);
            bool testPassed = NbFinderHighOccupancy(currentPass, m_fourHitFilterBox);
            if (testPassed == false) {
              B2DEBUG(150, "NbFINDERHighOccupancy: segment discarded! ")
              continue;
            }
          }
        }

        outerSegments[thisOuterSegment]->addInnerNeighbour(currentInnerSeg);
        currentInnerSeg->addOuterNeighbour(outerSegments[thisOuterSegment]);

        if (centerLayerIDNumber < currentPass->highestAllowedLayer) {
          currentInnerSeg->setSeed(false);
        }
      } // iterating through inner segments
    } // iterating through outer segments
  } // iterating through all aktive sectors - friendFinder

  //filtering lost segments (those without neighbours left):
  int countedSegments = 0;
  ActiveSegmentsOfEvent newActiveList;
  BOOST_FOREACH(VXDSegmentCell * currentSeg, currentPass->activeCellList) {
    if (currentSeg->getInnerNeighbours().size() == 0 && currentSeg->getOuterNeighbours().size() == 0) {
      currentSeg->setActivationState(false);
      NFdiscardedSegmentsCounter++;
    } else {
      currentSeg->copyNeighbourList(); /// IMPORTANT, without this step, no TCs can be found since all neighbours of each cell are reased from current list
      newActiveList.push_back(currentSeg);
      countedSegments++;
      if (currentSeg->getInnerNeighbours().size() != 0) {
        currentSeg->increaseState();
      }
    }
  }
  currentPass->activeCellList = newActiveList;

  return NFdiscardedSegmentsCounter;
}


bool VXDTFModule::NbFinderHighOccupancy(CurrentPassData* currentPass, TcFourHitFilters& fourHitFilterBox)
{
  int simpleSegmentQI = 0;
  bool accepted;

  if (currentPass->deltaDistanceHighOccupancy2IP.first == true) { // max only
    accepted = fourHitFilterBox.checkDeltaDistCircleCenter(m_filterNameBox.nameDeltaDistanceHighOccupancy2IP);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " deltaDistanceHighOccupancy2IP: segment approved!")
    } else {
      B2DEBUG(150, " deltaDistanceHighOccupancy2IP: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = fourHitFilterBox.getCutoffs(m_filterNameBox.nameDeltaDistanceHighOccupancy2IP);
        B2WARNING("deltaDistanceHighOccupancy2IP - minCutoff: " << cutoffs.first << ", calcValue: " << fourHitFilterBox.deltaDistCircleCenter() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " deltaDistanceHighOccupancy2IP is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (currentPass->pTHighOccupancy.first == true) { // min & max!
    accepted = fourHitFilterBox.checkDeltapT(m_filterNameBox.nameDeltapTHighOccupancy);
    if (accepted == true) {
      simpleSegmentQI++;
      B2DEBUG(150, " deltaPtHighOccupancy: segment approved!")
    } else {
      B2DEBUG(150, " deltaPtHighOccupancy: segment discarded!")
      if (m_PARAMDebugMode == true) {
        pair <double, double> cutoffs = fourHitFilterBox.getCutoffs(m_filterNameBox.nameDeltapTHighOccupancy);
        B2WARNING("deltaPtHighOccupancy - minCutoff: " << cutoffs.first << ", calcValue: " << fourHitFilterBox.deltapT() << ", maxCutoff: " << cutoffs.second)
      }
    } // else segment not approved
  } else { B2DEBUG(175, " pTHighOccupancy is not activated for pass: " << currentPass->sectorSetup << "!") }

  if (simpleSegmentQI < currentPass->activatedHighOccupancyNbFinderTests) {
    return false;
  } else { return true; }
}


/** ***** cellular automaton ***** **/
/// uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection
int VXDTFModule::cellularAutomaton(CurrentPassData* currentPass)
{
  int activeCells = 1;
  int caRound = 0;
  int goodNeighbours, countedSegments, segState;

  while (activeCells != 0) {
    activeCells = 0;
    caRound++;

    /// CAstep:
    BOOST_FOREACH(VXDSegmentCell * currentSeg, currentPass->activeCellList) {
      if (currentSeg->isActivated() == false) { continue; }

      goodNeighbours = 0;
      list<VXDSegmentCell*>& currentNeighbourList = currentSeg->getInnerNeighbours();

      list<VXDSegmentCell*>::iterator currentNeighbour = currentNeighbourList.begin();
      while (currentNeighbour != currentNeighbourList.end()) {
        //TODO probably outdated: can't deal with cells of special shape (e.g. two-layer cells) in some uncommon scenario yet -> search for test scenario
        if (currentSeg->getState() == (*currentNeighbour)->getState()) {
          goodNeighbours++;
          ++currentNeighbour;
        } else {
          currentNeighbour = currentSeg->eraseInnerNeighbour(currentNeighbour); // includes currentNeighbour++;
        }
      }
      if (goodNeighbours != 0) {
        currentSeg->allowStateUpgrade(true); activeCells++;
        B2DEBUG(1000, "good cell found!");
      } else {currentSeg->setActivationState(false); }
    }//CAStep

    /// Updatestep:
    BOOST_FOREACH(VXDSegmentCell * currentSeg, currentPass->activeCellList) {
      if (currentSeg->isUpgradeAllowed() == false) { continue; }

      currentSeg->allowStateUpgrade(false);
      currentSeg->increaseState();
      B2DEBUG(500, "good cell upgraded!")
    }

    B2DEBUG(10, "CA: " << caRound << ". round - " << activeCells << " living cells remaining.");

    if (caRound > 30) { /// WARNING: hardcoded value
      B2ERROR("event " << m_eventCounter << ": VXDTF-CA: more than 100 ca rounds! " << activeCells << " living cells remaining");
      string currentSectors, space = " "; // stringstream currentSectors;
      BOOST_FOREACH(SectorNameAndPointerPair key, currentPass->sectorSequence) {
        currentSectors += key.first; // currentSectors + key.first
        currentSectors += space;
      }
      B2ERROR("event " << m_eventCounter << ": VXDTF-CA: activated sectors: " << currentSectors)
      m_TESTERbrokenCaRound++;
      caRound = -1;
      break;
    }
  }

  //segmentOutPut:
  //  int segState;
  countedSegments = 0;
  BOOST_FOREACH(VXDSegmentCell * currentSeg, currentPass->activeCellList) {
    segState = currentSeg->getState();
    B2DEBUG(150, "Post CA - Current state of cell: " << segState);
    if (currentSeg->getInnerNeighbours().size() == 0 && currentSeg->getOuterNeighbours().size() == 0) { continue; }
    countedSegments++;
  }
  B2DEBUG(10, "CA - " << countedSegments << " segments have at least one friend");
  return caRound;
}



/** ***** Track Candidate Collector (TCC) ***** **/
/// uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection
void VXDTFModule::tcCollector(CurrentPassData* currentPass)
{
  int mayorLayerID, nSegmentsInSector;
  int findTCsCounter = 0;
  int tccMinState = currentPass->minState;
  OperationSequenceOfActivatedSectors::iterator secSequenceIter;
  for (secSequenceIter = currentPass->sectorSequence.begin(); secSequenceIter != currentPass->sectorSequence.end(); ++secSequenceIter) {
    B2DEBUG(100, "TCC - entering sector " << (*secSequenceIter).first);
    MapOfSectors::const_iterator sectorOfSequIter = (*secSequenceIter).second;

    mayorLayerID = lexical_cast<int>(sectorOfSequIter->second->getSecID()[0]);
    if (mayorLayerID < currentPass->minLayer) { continue; }

    vector<VXDSegmentCell*> segmentsOfSector = sectorOfSequIter->second->getInnerSegmentCells(); // loading segments of sector
    nSegmentsInSector = segmentsOfSector.size();

    B2DEBUG(100, "sector " << sectorOfSequIter->second->getSecID() << " has got " << nSegmentsInSector << " segments in its area");

    for (int thisSegmentInSector = 0; thisSegmentInSector < nSegmentsInSector; thisSegmentInSector++) {
      if (segmentsOfSector[thisSegmentInSector]->isSeed() == false || segmentsOfSector[thisSegmentInSector]->getState() < tccMinState) {
        if (segmentsOfSector[thisSegmentInSector]->isSeed() == false) {
          B2DEBUG(100, "current segment is no seed!");
        } else { B2DEBUG(100, "current segment has no sufficent cellstate..."); }
        continue;
      }

      VXDTFTrackCandidate* pTC = new VXDTFTrackCandidate();
      pTC->addSegments(segmentsOfSector[thisSegmentInSector]);
      pTC->addHits(segmentsOfSector[thisSegmentInSector]->getOuterHit());
      pTC->addHits(segmentsOfSector[thisSegmentInSector]->getInnerHit());

      findTCs(currentPass->tcVector, pTC, boost::lexical_cast<string>(currentPass->highestAllowedLayer));
      findTCsCounter++;
    }
  }
  int numTCsafterTCC = currentPass->tcVector.size(); // total number of tc's

  B2DEBUG(10, "findTCs activated " << findTCsCounter << " times, resulting in " << numTCsafterTCC << " track candidates")
  m_TESTERcountTotalTCsAfterTCC += numTCsafterTCC;
}



/** ***** Track Candidate Filter (tcFilter) ***** **/
int VXDTFModule::tcFilter(CurrentPassData* currentPass, int passNumber, vector<ClusterInfo>& clustersOfEvent)
{
  TCsOfEvent::iterator currentTC;
  TVector3 hitA, hitB, hitC, hitD;
  TCsOfEvent tempTCList = currentPass->tcVector;
  int numTCsafterTCC = tempTCList.size();
  vector<TCsOfEvent::iterator> goodTCIndices;
  goodTCIndices.clear();
  int tcCtr = 0;
  B2DEBUG(10, "TC-filter: pass " << passNumber << " has got " << currentPass->tcVector.size() << " tcs")
  for (currentTC = currentPass->tcVector.begin(); currentTC != currentPass->tcVector.end(); ++currentTC) { // need iterators for later use
    vector<VXDTFHit*> currentHits = (*currentTC)->getHits(); /// IMPORTANT: currentHits[0] is outermost hit!
    int numOfCurrentHits = currentHits.size();

    if (numOfCurrentHits == 3) {  /// in this case, dPt and zigzag filtering does not make sense
      (*currentTC)->removeVirtualHit();
      numOfCurrentHits = (*currentTC)->size();
      if (numOfCurrentHits < 3) {
        (*currentTC)->setCondition(false);
        B2DEBUG(20, " tc " << tcCtr << " got " << numOfCurrentHits << " hits and therefore will be deleted");
      } else {
        goodTCIndices.push_back(currentTC); // we can not filter it, therefore we have to accept it
        B2DEBUG(20, " tc " << tcCtr << " got " << numOfCurrentHits << " hits and therefore won't be checked by TCC");
      }
      tcCtr++;
      continue; // no further testing possible
    } else if (numOfCurrentHits < 3) {
      B2DEBUG(20, " tc " << tcCtr << " got " << numOfCurrentHits << " hits and therefore will be deleted");
      tcCtr++;
      (*currentTC)->setCondition(false);
      continue;
    }

    vector<TVector3> currentHitPositions;
    stringstream secNameOutput;
    secNameOutput << endl << " tc " << tcCtr << " got " << numOfCurrentHits << " hits and the following secIDs: ";
    BOOST_FOREACH(VXDTFHit * currentHit, currentHits) {
      currentHitPositions.push_back(currentHit->getHitCoordinates());
      string aSecName = currentHit->getSectorName();
      secNameOutput << aSecName << " ";
    } // used for output
    B2DEBUG(20, " " << secNameOutput.str() << " and " <<  numOfCurrentHits << " hits");

    // feeding trackletFilterbox with hits:
    m_trackletFilterBox.resetValues(currentHitPositions);
    bool isZiggZagging;

    if (currentPass->zigzagXY.first == true) {
      isZiggZagging = m_trackletFilterBox.ziggZaggXY();
      if (isZiggZagging == true) {
        B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by ziggZaggXY! ");
        m_TESTERtriggeredZigZagXY++; tcCtr++;
        (*currentTC)->setCondition(false);
        continue;
      }
      B2DEBUG(20, " TCC filter ziggZaggXY approved TC " << tcCtr);
    }

    if (currentPass->circleFit.first == true) {
      double closestApproachPhi, closestApproachR;
      double chi2 = m_trackletFilterBox.circleFit(closestApproachPhi, closestApproachR);
      if (chi2 > currentPass->circleFit.second) {  // means chi2 is bad
        B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by circleFit! ");
        m_TESTERtriggeredCircleFit++; tcCtr++;
        (*currentTC)->setCondition(false);
        continue;
      }
      B2DEBUG(20, " TCC filter circleFit approved TC " << tcCtr);
    }


    int a = 0, b = 1, c = 2, d = 3;
    bool abortTC = false, accepted;
    for (; d <  numOfCurrentHits;) {
      hitA = currentHits[a]->getHitCoordinates();
      VXDSector* thisSector = currentPass->sectorMap.find(currentHits[a]->getSectorName())->second;
      hitB = currentHits[b]->getHitCoordinates();
      string friendID = currentHits[b]->getSectorName();
      hitC = currentHits[c]->getHitCoordinates();
      hitD = currentHits[d]->getHitCoordinates();

      m_fourHitFilterBox.resetValues(hitA, hitB, hitC, hitD, thisSector, friendID);

      if (currentPass->deltaPt.first == true) {
        accepted = m_fourHitFilterBox.checkDeltapT(m_filterNameBox.nameDeltapT);
        if (accepted == false) {
          B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by deltaPt! ");
          m_TESTERtriggeredDpT++;
          (*currentTC)->setCondition(false);
          if (m_PARAMDebugMode == true) {
            pair <double, double> cutoffs = m_fourHitFilterBox.getCutoffs(m_filterNameBox.nameDeltapT);
            B2WARNING("deltaPt - minCutoff: " << cutoffs.first << ", calcValue: " << m_fourHitFilterBox.deltapT() << ", maxCutoff: " << cutoffs.second)
          }
          abortTC = true;
          break;
        }
        B2DEBUG(20, " TCC filter deltaPt approved TC " << tcCtr);
      }
      if (currentPass->deltaDistance2IP.first == true) {
        accepted = m_fourHitFilterBox.checkDeltaDistCircleCenter(m_filterNameBox.nameDeltaDistance2IP);
        if (accepted == false) {
          B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by deltaDistance2IP! ");
          m_TESTERtriggeredDD2IP++;
          (*currentTC)->setCondition(false);
          if (m_PARAMDebugMode == true) {
            pair <double, double> cutoffs = m_fourHitFilterBox.getCutoffs(m_filterNameBox.nameDeltaDistance2IP);
            B2WARNING("deltaDistance2IP - minCutoff: " << cutoffs.first << ", calcValue: " << m_fourHitFilterBox.deltaDistCircleCenter() << ", maxCutoff: " << cutoffs.second)
          }
          abortTC = true;
          break;
        }
        B2DEBUG(20, " TCC filter deltaDistance2IP approved TC " << tcCtr);
      }
      a++; b++; c++; d++;
    }
    if (abortTC == true) { continue; }

    // now we can delete the virtual hit for tests done without it (e.g. tests dealing with the r-z-plane, where the IP is not well defined beforehand)
    (*currentTC)->removeVirtualHit();

    if (currentPass->zigzagRZ.first == true and numOfCurrentHits > 4) {
      // in this case we have still got enough hits for this test after removing virtual hit
      vector<VXDTFHit*> currentHits = (*currentTC)->getHits();

      vector<TVector3> currentHitPositions;
      BOOST_FOREACH(VXDTFHit * currentHit, currentHits) {
        currentHitPositions.push_back(currentHit->getHitCoordinates());
      }
      m_trackletFilterBox.resetValues(currentHitPositions);

      isZiggZagging = m_trackletFilterBox.ziggZaggRZ();
      if (isZiggZagging == true) {
        B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected by ziggZaggRZ! ");
        m_TESTERtriggeredZigZagRZ++; tcCtr++;
        (*currentTC)->setCondition(false);
        continue;
      }
      B2DEBUG(20, " TCC filter ziggZaggRZ approved TC " << tcCtr);
    }
    (*currentTC)->setPassIndex(passNumber);
    (*currentTC)->setCondition(true);
    goodTCIndices.push_back(currentTC);
    B2DEBUG(20, " TCC filter approved TC " << tcCtr);
    m_TESTERapprovedByTCC++;

    tcCtr++;
  }

  ///deleting TCs which did not survive the tcFilter-step
  int goodOnes = goodTCIndices.size(); // number of tc's after TCC-Filter
  if (goodOnes not_eq numTCsafterTCC) {
    currentPass->tcVector.clear();
    BOOST_FOREACH(TCsOfEvent::iterator goodTCIndex, goodTCIndices) { currentPass->tcVector.push_back((*goodTCIndex)); }
  }

  int numTC = 0;
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentPass->tcVector) {
    vector<VXDTFHit*> currentHits = currentTC->getHits();
    int numOfHits = currentHits.size();
    stringstream secNameOutput;
    secNameOutput << endl << "after filtering virtual entries: tc " << numTC << " got " << numOfHits << " hits and the following secIDs: ";
    BOOST_FOREACH(VXDTFHit * currentHit, currentHits) { // now we are informing each cluster which TC is using it
      string aSecName = currentHit->getSectorName();
      secNameOutput << aSecName << " ";
      if (currentHit->getDetectorType() == 0) {   // PXD
        clustersOfEvent[currentHit->getClusterIndexUV()].addTrackCandidate(currentTC);
      } else {
        clustersOfEvent[currentHit->getClusterIndexU()].addTrackCandidate(currentTC); // should mean aVXDTFTrackCandidate*
        clustersOfEvent[currentHit->getClusterIndexV()].addTrackCandidate(currentTC);
      }
    } // used for output and informing each cluster which TC is using it
    B2DEBUG(20, " " << secNameOutput.str() << " and " << numOfHits << " hits");
    numTC++;
  }

  m_TESTERcountTotalTCsAfterTCCFilter += goodOnes;
  B2DEBUG(10, "TCC-filter: tcList had " << numTCsafterTCC << " TCs. Of these, " << goodOnes << " TC's were accepted as good ones by the TCC-Filter")

  return goodOnes;
}



// double VXDTFModule::circleFit(vector<VXDTFHit*> hits, double& chi2)
// {
//   TVector3 hitPos;
//   double meanX = 0, meanY = 0, meanX2 = 0, meanY2 = 0, meanR2 = 0, meanR4 = 0, meanXR2 = 0, meanYR2 = 0, meanXY = 0; //mean values
//   double numHits = hits.size();
//   double r2 = 0, x = 0, y = 0, x2 = 0, y2 = 0, divisor = 1. / numHits; // coords and divisor which is the same for all of them
//   double covXX = 0, covXY = 0, covYY = 0, covXR2 = 0, covYR2 = 0, covR2R2 = 0; // covariances
//   double q1 = 0, q2 = 0; // helping variables, to make the code more readable
//   // looping over all hits and do the division afterwards
//   BOOST_FOREACH(VXDTFHit * hit, hits) {
//     hitPos = hit->getHitCoordinates();
//     x = hitPos.X();
//     y = hitPos.Y();
//     x2 = x * x;
//     y2 = y * y;
//     r2 = x2 + y2;
//     meanX += x;
//     meanY += y;
//     meanXY += x * y;
//     meanX2 += x2;
//     meanY2 += y2;
//     meanXR2 += x * r2;
//     meanYR2 += y * r2;
//     meanR2 += r2;
//     meanR4 += r2 * r2;
//   }
//   meanX *= divisor;
//   meanY *= divisor;
//   meanXY *= divisor;
//   meanY2 *= divisor;
//   meanX2 *= divisor;
//   meanXR2 *= divisor;
//   meanYR2 *= divisor;
//   meanR2 *= divisor;
//   meanR4 *= divisor;
//
//   covXX = meanX2 - meanX * meanX;
//   covXY = meanXY - meanX * meanY;
//   covYY = meanY2 - meanY * meanY;
//   covXR2 = meanXR2 - meanX * meanR2;
//   covYR2 = meanYR2 - meanY * meanR2;
//   covR2R2 = meanR4 - meanR2 * meanR2;
//
//   q1 = covR2R2 * covXY - covXR2 * covYR2;
//   q2 = covR2R2 * (covXX - covYY) - covXR2 * covXR2 + covYR2 * covYR2;
//   double phi = 0.5 * atan(2. * q1 / q2); // physical meaning: direction of flight at point of closest approach to origin (not relevant here, since we need the direction of flight at a completely different location)
//   B2DEBUG(50, " calculated direction of propagation at point of closest approach to origin (phi): " << phi)
//   double sinPhi = sin(phi);
//   double cosPhi = cos(phi);
//   double kappa = (sinPhi * covXR2 - cosPhi * covYR2) / covR2R2;
//   double delta = -kappa * meanR2 + sinPhi * meanX - cosPhi * meanY;
//   double rootTerm = sqrt(1. - 4.*delta * kappa);
//   double rho = 2.*kappa / (rootTerm); // rho = 1/curvature in X-Y-plane = radius of fitting circle, used for pT-calculation
//   if (chi2 > 0) {
//     double dist = 2.*delta / (1 + rootTerm);
//     chi2 = numHits * (1 + rho * dist) * (1 + rho * dist) * (sinPhi * sinPhi * covXX - 2.*sinPhi * cosPhi * covXY + cosPhi * cosPhi * covYY - kappa * kappa * covR2R2);
//   } else {
//    if (m_PARAMqiSmear == true) {
//
//         chi2 = m_littleHelperBox. smearPositiveGauss(0.);
//       }
//  }
//   return rho;
// }



void VXDTFModule::calcInitialValues4TCs(TCsOfEvent& tcVector) /// TODO: use vxdCaTracking-classes to reduce code errors
{
  vector<VXDTFHit*> currentHits;
  TVector3 hitA, hitB, hitC, hitA_T, hitB_T, hitC_T; // those with _T are the hits of the transverlal plane
  TVector3 intersection, radialVector, pTVector, pVector; //coords of center of projected circle of trajectory & vector pointing from center to innermost hit
  TVector3 segAB, segBC, segAC, cpAB, cpBC, nAB, nBC;
  int numOfCurrentHits, signCurvature, pdGCode;
  double muVal, radiusInCm, pT, theta, pZ, preFactor; // needed for dPt calculation
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, tcVector) {

    if (aTC->getCondition() == false) { continue; }
    currentHits = aTC->getHits();
    numOfCurrentHits = currentHits.size();

/// method A: 3 neighbouring inner hits:
    if (m_KFBackwardFilter == true) {
      hitA = currentHits[2]->getHitCoordinates();
      hitB = currentHits[1]->getHitCoordinates();
      hitC = currentHits[0]->getHitCoordinates(); // outermost hit and initial value for GFTrackCandidate
    } else {
      hitA = currentHits[numOfCurrentHits - 1]->getHitCoordinates(); // innermost hit and initial value for GFTrackCandidate
      hitB = currentHits[numOfCurrentHits - 2]->getHitCoordinates();
      hitC = currentHits[numOfCurrentHits - 3]->getHitCoordinates();
    }

    hitA_T = hitA; hitA_T.SetZ(0.);
    hitB_T = hitB; hitB_T.SetZ(0.);
    hitC_T = hitC; hitC_T.SetZ(0.);
    segAB = hitB - hitA;
    segAC = hitC - hitA;
    theta = segAC.Theta();

/// method B: using (0,0,0) as innermost hit:
//    hitA.SetX(0.);
//    hitA.SetY(0.);
//    hitA.SetZ(0.);
// //     hitA = currentHits[numOfCurrentHits - 1]->getHitCoordinates(); // innermost hit and initial value for GFTrackCandidate
//    hitA_T = hitA; hitA_T.SetZ(0.);
//    hitB = currentHits[numOfCurrentHits - 1]->getHitCoordinates();
//    hitB_T = hitB; hitB_T.SetZ(0.);
//    hitC = currentHits[0]->getHitCoordinates();
//    hitC_T = hitC; hitC_T.SetZ(0.);

/// method C: innermost, outermost and a center hit:
//    hitA = currentHits[numOfCurrentHits - 1]->getHitCoordinates(); // innermost hit and initial value for GFTrackCandidate
//    hitA_T = hitA; hitA_T.SetZ(0.);
//    int centerHit = float(numOfCurrentHits)*0.5 +1;
//    if (centerHit == numOfCurrentHits) { centerHit--; } else if (centerHit == 1 ) { centerHit++; }
//    hitB = currentHits[centerHit - 1]->getHitCoordinates();
//    hitB_T = hitB; hitB_T.SetZ(0.);
//    hitC = currentHits[0]->getHitCoordinates(); // outermost hit
//    hitC_T = hitC; hitC_T.SetZ(0.);
//    segAB = hitB - hitA;
//    theta = segAB.Theta();

    segAB.SetZ(0.);
    segBC = hitC_T - hitB_T;
    cpAB = 0.5 * segAB + hitA_T; // central point of innerSegment
    cpBC = 0.5 * segBC + hitB_T;// central point of mediumSegment
    nAB = segAB.Orthogonal();
    nBC = segBC.Orthogonal();

    signCurvature = sign(nBC * segAB);

    muVal = (((cpAB(1) - cpBC(1)) / nBC(1)) + (((cpBC(0) - cpAB(0)) / nAB(0)) * nAB(1) / nBC(1))) / (1. - ((nBC(0) / nAB(0)) * (nAB(1) / nBC(1))));
    intersection.SetX(cpBC(0) + muVal * nBC(0)); // x-coord of intersection point
    intersection.SetY(cpBC(1) + muVal * nBC(1)); // y-coord of intersection point
    intersection.SetZ(0.);
    if (m_KFBackwardFilter == true) {
      radialVector = (intersection - hitC);
    } else {
      radialVector = (intersection - hitA);
    }
    /// following line needed for method A-C:
    radiusInCm = radialVector.Mag(); // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1

    /// method D: using circleFit
//       double chi2 = -1; // means, no chi2 will be calculated
//     radiusInCm = circleFit(currentHits,chi2);

    pT = 0.45 * radiusInCm * 0.01; // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100 = 0.45*r*0.01 length of pT
//     B2INFO("event: "<<m_eventCounter<<": calculated pT: " << pT);
    pZ = pT / tan(theta);
    preFactor = pT / radiusInCm;
    pTVector = preFactor * radialVector.Orthogonal() ;

    if (m_KFBackwardFilter == true) {
      if ((hitC + pTVector).Mag() < hitC.Mag()) { pTVector = pTVector * -1; }
    } else {
      if ((hitA + pTVector).Mag() < hitA.Mag()) { pTVector = pTVector * -1; }
    }


    pVector = pTVector;
    pVector.SetZ(pZ);
    double pValue = pVector.Mag();

    // the sign of curvature determines the charge of the particle, negative sign for curvature means positively charged particle. The signFactor is needed since the sign of PDG-codes are not defined by their charge but by being a particle or an antiparticle

///     if ( m_KFBackwardFilter == true ) {
///       pdGCode = signCurvature * m_PARAMpdGCode * m_chargeSignFactor * (-1);
///     } else {
    pdGCode = signCurvature * m_PARAMpdGCode * m_chargeSignFactor;
///     }

    if (m_KFBackwardFilter == true) {
      aTC->setInitialValue(hitC, pVector, pdGCode);
    } else {
      aTC->setInitialValue(hitA, pVector, pdGCode);
    }

    B2DEBUG(10, " TC has got momentum of " << pValue << "GeV and estimated pdgCode " << pdGCode);

//    if ( pValue < 0.02 ) {
//      aTC->setCondition(false);
//      m_TESTERfilteredBadSeedTCs++;
//      B2DEBUG(10, "Total momentum of current TC is " << pValue << "GeV and and therefore neglected by calcInitialValues4TCs");
//    }
  }
}



void VXDTFModule::calcQIbyLength(TCsOfEvent& tcVector, PassSetupVector& passSetups)
{
  /// setting quality indices and smear result if chosen
  double firstValue = 0.0, rngValue = 0.0, setupWeigh = 0.0, maxLength = 0.0, numTotalLayers;

  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, tcVector) {
    if (currentTC->getCondition() == false) { continue; }
    rngValue = gRandom->Gaus(m_PARAMsmearMean, m_PARAMsmearSigma);
//     setupWeigh = passSetups[currentTC->getPassIndex()]->setupWeigh;
    int currentPassIndex = currentTC->getPassIndex();
    B2DEBUG(100, " passIndex of currentTC: " << currentTC->getPassIndex())
    if (currentPassIndex < 0 or currentPassIndex > m_numOfSectorSetups - 1) { currentPassIndex = 0; }
    if (passSetups.size() == 0) {
      numTotalLayers = 6;
    } else {
      numTotalLayers = passSetups.at(currentPassIndex)->numTotalLayers;
    }
    B2DEBUG(100, " numTotalLayers of currentTC: " << numTotalLayers)
    maxLength =  0.5 / numTotalLayers;

    if (rngValue < -0.4) { rngValue = -0.4; } else if (rngValue > 0.4) { rngValue = 0.4; }

    if (m_PARAMqiSmear == true) {
      firstValue = rngValue + 1.0 - setupWeigh;

    } else {
      firstValue = 1.0 - setupWeigh;
    }
    B2DEBUG(50, "setQQQ gets the following values: setupWeigh: " << setupWeigh << ", first value: " << firstValue << ", QQQScore: " << sqrt(firstValue * 0.5))
    currentTC->setQQQ(firstValue, 2.0);  // resulting QI = 0.3-0.7
    currentTC->setTrackQuality((currentTC->getQQQ() * double(currentTC->getHits().size()*maxLength)));
    currentTC->setFitSucceeded(true); // no real fit means, allways succeeded
  }
}



// void VXDTFModule::calcQIbyKalman(TCsOfEvent& tcVector, StoreArray<PXDCluster>& pxdClusters, StoreArray<SVDCluster>& svdClusters, vector<ClusterInfo>& clusters) //old version, keeping as backup (13-03-29)
void VXDTFModule::calcQIbyKalman(TCsOfEvent& tcVector, StoreArray<PXDCluster>& pxdClusters)
{
  /// produce GFTrackCands for each currently living TC and calculate real kalman-QI's
  GFKalman kalmanFilter;
  kalmanFilter.setBlowUpFactor(500.0); // TODO: hardcoded values should be set as steering file-parameter

  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, tcVector) {
    if (currentTC->getCondition() == false) { continue; }

//    B2DEBUG(50, "VXDTFModule::calcQIbyKalman, printing GFTrackCand, RKTrackRep, GFTrack")

    TVector3 errors(0.01, 0.01, 0.04); // rather generic values for a rough start
    RKTrackRep* trackRep = new RKTrackRep(currentTC->getInitialCoordinates(), currentTC->getInitialMomentum(), errors, errors, currentTC->getPDGCode());
//     trackRep->setPropDir(-1); // 1 = start moving outwards (seed has to be innermost hit), -1 start moving inwards (seed has to be outermost hit)

//    trackRep->Print();
    GFTrack track(trackRep);
//    track.Print();
    track.setSmoothing(false);

    int numOfPxdIndices = currentTC->getPXDHitIndices().size();
    int numOfSvdIndices = currentTC->getSVDHitIndices().size();

    BOOST_FOREACH(VXDTFHit * tfHit, currentTC->getHits()) {
      if (tfHit->getDetectorType() == Const::PXD) {
        PXDRecoHit* newRecoHit = new PXDRecoHit(pxdClusters[tfHit->getClusterIndexUV()]);
        track.addHit(newRecoHit);
      } else if (tfHit->getDetectorType() == Const::SVD) {
        TVector3 pos = tfHit->getHitCoordinates();
        SVDRecoHit2D* newRecoHit = new SVDRecoHit2D(tfHit->getVxdID(), pos[0], pos[1]);
        track.addHit(newRecoHit);
      } else {
        B2ERROR("VXDTFModule::calcQIbyKalman: event " << m_eventCounter << " a hit has unknown detector type ( " << tfHit->getDetectorType() << ") discarding hit")
      }
    }

    const int nHitsInTrack = track.getNumHits();
    B2DEBUG(50, "VXDTFModule::calcQIbyKalman, numOfPxdIndices : " << numOfPxdIndices << ", numOfSvdIndices : " << numOfSvdIndices << ", nHitsInTrack: " << nHitsInTrack)

    if (m_KFBackwardFilter == true) {
      kalmanFilter.setNumIterations(0);
      try {
        kalmanFilter.processTrack(&track);
      } catch (exception& e) {
        std::cerr << e.what();
        B2WARNING("VXDTFModule::calcQIbyKalman event " << m_eventCounter << ":, processTrack failed! skipping current TC")
        currentTC->setCondition(false); // do not store TCs with failed fits if param-flag is set to false
        continue;
      }

      track.setNextHitToFit(track.getNumHits() - 1);
      kalmanFilter.fittingPass(&track, -1) ;   // -1 means backward filtering!
    } else {
      kalmanFilter.setNumIterations(1); // TODO: hardcoded values should be set as steering file-parameter
      try {
        kalmanFilter.processTrack(&track);
      } catch (exception& e) {
        std::cerr << e.what();
        B2WARNING("VXDTFModule::calcQIbyKalman event " << m_eventCounter << ":, processTrack failed 111, skipping current TC")
        currentTC->setCondition(false); // do not store TCs with failed fits if param-flag is set to false
        continue;
      }
    }

    if (trackRep->getStatusFlag() == 0) { // 0 means, it was successfull
      double pVal = track.getCardinalRep()->getPVal();
      B2DEBUG(10, "calcQI4TC succeeded: calculated kalmanQI: " << track.getChiSqu() << ", forward-QI: " << track.getForwardChiSqu() << " with NDF: " << track.getNDF() << ", p-value: " << pVal << ", numOfHits: " <<  numOfPxdIndices + numOfSvdIndices)
      if (pVal < 0.000001 and m_PARAMqiSmear == true) {
        currentTC->setTrackQuality(m_littleHelperBox.smearNormalizedGauss(pVal));
      } else {
        currentTC->setTrackQuality(pVal);
      }

      currentTC->setFitSucceeded(true);
      m_TESTERgoodFitsCtr++;
    } else {
      B2DEBUG(10, "calcQI4TC failed...")
      m_TESTERbadFitsCtr++;
      currentTC->setFitSucceeded(false);
      if (m_PARAMqiSmear == true) {
        currentTC->setTrackQuality(m_littleHelperBox.smearNormalizedGauss(0.));
      } else {
        currentTC->setTrackQuality(0);
      }

      if (m_PARAMstoreBrokenQI == false) {
        currentTC->setCondition(false); // do not store TCs with failed fits if param-flag is set to false
      }
    }
  }
}



void VXDTFModule::calcQIbyCircleFit(TCsOfEvent& tcVector)
{
  /// setting quality indices and smear result if chosen
  double chi2 = 0.;
  int tcCtr = 0;
  vector<TVector3> currentHits;
  boostNsec duration;
  B2DEBUG(10, "entered calcQIbyCircleFit, executing " << tcVector.size() << " TCs")
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, tcVector) {
    boostClock::time_point timer = boostClock::now();
    if (currentTC->getCondition() == false) { tcCtr++; continue; }
    currentHits = currentTC->getHitCoordinates();
    int numHits = currentHits.size();
    B2DEBUG(100, "calcQIbyCircleFit, TC " << tcCtr << " got " << numHits << " hits")
    if (numHits < 4) {
      if (m_PARAMqiSmear == true) {
        currentTC->setTrackQuality(m_littleHelperBox.smearNormalizedGauss(0.));
      } else {
        currentTC->setTrackQuality(0);
//        currentTC->setFitSucceeded(false);
      }
      continue;
    }

    m_trackletFilterBox.resetValues(currentHits);
    chi2 = m_trackletFilterBox.circleFit();  /// circleFit
    B2DEBUG(100, "calcQIbyCircleFit-external, TC " << tcCtr << " got fitted with chi2 of " << chi2 << " and probability of " << TMath::Prob(chi2, numHits - 3) << " with ndf: " << numHits - 3)

    if (chi2 < 0) { chi2 = 0; }
    double probability = TMath::Prob(chi2, numHits - 3);
    if (m_PARAMqiSmear == true) { probability = m_littleHelperBox.smearNormalizedGauss(probability); }

    currentTC->setTrackQuality(probability);

    boostClock::time_point timer2 = boostClock::now();
    duration = boost::chrono::duration_cast<boostNsec>(timer2 - timer);

    B2DEBUG(10, "calcQIbyCircleFit succeeded: calculated circleQI external: " << chi2 << " with NDF: " << numHits - 3 << ", p-value: " << probability << ", numOfHits: " <<  numHits << ", time consumption: " << duration.count() << " ns")
    B2DEBUG(10, "calcQIbyCircleFit succeeded: calculated circleQI external with ndf*2: " << chi2 << " with NDF: " << 2 * numHits - 3 << ", p-value: " << m_littleHelperBox.smearNormalizedGauss(TMath::Prob(chi2, 2 * numHits - 3)) << ", numOfHits: " <<  numHits << ", time consumption: " << duration.count() << " ns")

    currentTC->setFitSucceeded(true);
    tcCtr++;
  }
}


GFTrackCand VXDTFModule::generateGFTrackCand(VXDTFTrackCandidate* currentTC, vector<ClusterInfo>& clusters)
{
  GFTrackCand newGFTrackCand;

  B2DEBUG(50, "VXDTFModule::generateGFTrackCand, after newGFTrackCand")
  TVector3 posIn = currentTC->getInitialCoordinates();
  TVector3 momIn = currentTC->getInitialMomentum();
  TVectorD stateSeed(6); //(x,y,z,px,py,pz)
  TMatrixDSym covSeed(6);
  int pdgCode = currentTC->getPDGCode();
  vector<int> pxdHits = currentTC->getPXDHitIndices();
  vector<int> pxdClusters;
  stringstream printIndices;
  printIndices << "PXD: ";
  BOOST_FOREACH(int index, pxdHits) {
    int clusterIndex = clusters[index].getIndex();
    printIndices << clusterIndex << " ";
    pxdClusters.push_back(clusterIndex);
  }
  vector<int> svdHits = currentTC->getSVDHitIndices();
  vector<int> svdClusters;
  printIndices << ", SVD: ";
  BOOST_FOREACH(int index, svdHits) {
    int clusterIndex = clusters[index].getIndex();
    printIndices << clusterIndex << " ";
    svdClusters.push_back(clusterIndex);
  }
  B2DEBUG(10, "generated GFTC with following hits: " << printIndices.str());

  stateSeed(0) = posIn[0]; stateSeed(1) = posIn[1]; stateSeed(2) = posIn[2];
  stateSeed(3) = momIn[0]; stateSeed(4) = momIn[1]; stateSeed(5) = momIn[2];
  covSeed(0, 0) = 0.1 * 0.1; covSeed(1, 1) = 0.1 * 0.1; covSeed(2, 2) = 0.2 * 0.2;
  covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.2 * 0.2;
  B2DEBUG(10, "generating GFTrackCandidate: posIn.Mag(): " << posIn.Mag() << ", momIn.Mag(): " << momIn.Mag() << ", pdgCode: " << pdgCode);

//  newGFTrackCand.setComplTrackSeed(posIn, momIn, pdgCode, TVector3(0.1, 0.1, 0.1), TVector3(0.1, 0.1, 0.2)); /// old method

  newGFTrackCand.set6DSeedAndPdgCode(stateSeed, pdgCode, covSeed);

  BOOST_REVERSE_FOREACH(int hitIndex, pxdClusters) {  // order of hits within VXDTFTrackCandidate: outer->inner hits. GFTrackCand: inner->outer hits
    newGFTrackCand.addHit(Const::PXD, hitIndex);
  }
  BOOST_REVERSE_FOREACH(int hitIndex, svdClusters) {  // order of hits within VXDTFTrackCandidate: outer->tcFilterinner hits. GFTrackCand: inner->outer hits
    newGFTrackCand.addHit(Const::SVD, hitIndex);
  }

  return newGFTrackCand;
}



int VXDTFModule::cleanOverlappingSet(TCsOfEvent& tcVector)
{
  int numOfIHits = 0, numOfJHits = 0, numOfMergedHits = 0, killedTCs = 0;
  list<int> ihitIDs, jhitIDs, mergedHitIDs;

  vector<VXDTFTrackCandidate*> rivals;
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, tcVector) {
    if (aTC->getCondition() == false) { continue; }
    ihitIDs = aTC->getHopfieldHitIndices();
    numOfIHits = ihitIDs.size();

    rivals = aTC->getBookingRivals();
    BOOST_FOREACH(VXDTFTrackCandidate * bTC, rivals) {
      if (bTC->getCondition() == false) { continue; }
      jhitIDs = bTC->getHopfieldHitIndices();
      numOfJHits = jhitIDs.size();

      mergedHitIDs = ihitIDs;
      mergedHitIDs.insert(mergedHitIDs.end(), jhitIDs.begin(), jhitIDs.end());
      mergedHitIDs.sort();
      mergedHitIDs.unique();
      numOfMergedHits = mergedHitIDs.size();

      if (numOfMergedHits <= numOfIHits || numOfMergedHits <= numOfJHits) { // in this case one TC is part of the other one
        killedTCs++;
        if (aTC->getTrackQuality() > bTC->getTrackQuality()) {
          bTC->setCondition(false);
        } else {
          aTC->setCondition(false); // although this one already died, we have to continue since there could be other TCs sharing all their hits with him TODO WHY? explanation not sufficient!
          break; // since explanation is not good enough, we stop the loop of the rivals since there is no check whether aTC is still alive and therefore many bTCs could die although aTC is already dead
        }
      }
    }
  }
  m_TESTERfilteredOverlapsQI += killedTCs;
  B2DEBUG(10, "VXDTFModule event " << m_eventCounter << ": so far, " << m_TESTERfilteredOverlapsQI << " TCs have been killed by funcCleanOverlappingSet...")
  /// now all TCs which were completely redundant should be dead. Therefore some TCs could be clean by now -> collecting all overlapping TCs which are still alive:
  if (killedTCs not_eq 0) {
    TCsOfEvent newOverlappingTCs;
    BOOST_FOREACH(VXDTFTrackCandidate * aTC, tcVector) {
      if (aTC->getCondition() == false) { continue; }
      if (aTC->checkOverlappingState() == true) {
        newOverlappingTCs.push_back(aTC);
      }
    }
    int cleanedTCs = tcVector.size() - newOverlappingTCs.size() - killedTCs;
    B2DEBUG(10, "within funcCleanOverlappingSet: tcVector.size(): " << tcVector.size() << ", newOverlappingTCs.size(): " << newOverlappingTCs.size() << " and " << cleanedTCs << " former overlapping TCs are now clean")
    tcVector = newOverlappingTCs;
    m_TESTERfilteredOverlapsQICtr++;
  } else { m_TESTERNotFilteredOverlapsQI++; B2DEBUG(10, "VXDTFModule event " << m_eventCounter << ": no TC is subset of other TC")}
  return killedTCs;
}


/** cleaning part **/
void VXDTFModule::cleanEvent(CurrentPassData* currentPass, string centerSector)
{

  // sectors stay alive, therefore only reset!
  typedef pair<string, MapOfSectors::iterator> secEntry;
  BOOST_FOREACH(secEntry aSector, currentPass->sectorSequence) {
    aSector.second->second->resetSector();
  }
  currentPass->sectorMap.find(centerSector)->second->resetSector(); // doing the same for the virtual sector which is not in the operation sequence
  currentPass->sectorSequence.clear();


  //segments, Hits and TrackCandidates can be deleted
  BOOST_FOREACH(VXDSegmentCell * aCell, currentPass->totalCellVector) {
    delete  aCell;
  }
  currentPass->totalCellVector.clear();
  currentPass->activeCellList.clear();

  BOOST_FOREACH(VXDTFHit * aHit, currentPass->hitVector) {
    delete  aHit;
  }
  currentPass->hitVector.clear();

  BOOST_FOREACH(VXDTFTrackCandidate * aTC, currentPass->tcVector) {
    delete  aTC;
  }
  currentPass->tcVector.clear();
  m_tcVectorOverlapped.clear();
  m_tcVector.clear();
}



string VXDTFModule::EventInfoPackage::Print()
{
  stringstream output;
  output << " timeConsumption of event " << evtNumber << " in ns: " << endl;

  output << "total: " << totalTime.count();
  output << ", hitsorting: " << sectionConsumption.hitSorting.count();
  output << ", sf: "  << sectionConsumption.segFinder.count();
  output << ", nf: " << sectionConsumption.nbFinder.count();
  output << ", ca: " << sectionConsumption.cellularAutomaton.count();
  output << ", tcc: " << sectionConsumption.tcc.count();
  output << ", tcfilter: " << sectionConsumption.postCAFilter.count() << endl;
  output << ", overlap: " << sectionConsumption.checkOverlap.count();
  output << ", kf: " << sectionConsumption.kalmanStuff.count();
  output << ", cleanOverlap: " << sectionConsumption.cleanOverlap.count();
  output << ", nn: " << sectionConsumption.neuronalStuff.count();
  output << ", other: " << sectionConsumption.intermediateStuff.count() << endl;

//  output.precision(6);

  output << " results: " << endl;
  output << "nPXDCluster: " << numPXDCluster << ", nSVDCluster: " << numSVDCluster << ", nSVDHits(x-Passes): " << numSVDHits << endl;
  output << "sfActivated: " << segFinderActivated << ", discarded: " << segFinderDiscarded << ", nfActivated: " << nbFinderActivated << ", discarded: " << nbFinderDiscarded << endl;
  output << "tccApproved: " << tccApprovedTCs << ", nTCsAfterTCC: " << numTCsAfterTCC << ", nTCsPostTCfilter: " << numTCsAfterTCCfilter << ", nTCsKilledOverlap: " << numTCsKilledByCleanOverlap << ", nTCsFinal: " << numTCsfinal << endl;

  return output.str();
}


double VXDTFModule::addExtraGain(double prefix, double cutOff, double generalTune, double specialTune)
{
  if (cutOff > 0.) {
    return cutOff + prefix * cutOff * generalTune + prefix * cutOff * specialTune;
  } else {
    return cutOff - prefix * cutOff * generalTune - prefix * cutOff * specialTune;
  }
}


double VXDTFModule::getXMLValue(GearDir& quantiles, string& valueType, string& filterType)
{
  double aValue;
  try {
    aValue = quantiles.getDouble(valueType);
  } catch (...) {
    B2WARNING("import of " << filterType << "-" << valueType << "-value failed! Setting to 0!!")
    aValue = 0;
  }
  return aValue;
}
