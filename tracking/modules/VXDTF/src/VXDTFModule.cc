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
#include <framework/gearbox/GearDir.h> // needed for reading xml-files
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

//root packages:
#include <TMatrixT.h>
#include <TMatrixD.h>
#include <TGeoManager.h>

//STL-packages;
#include <algorithm>
#include <functional>
#include <utility>
#include <math.h>

//Boost-packages:
#include <boost/foreach.hpp>
#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp> // needed for is_any_of
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/sign.hpp>



using namespace std;
using namespace Belle2;
using boost::format;
using boost::lexical_cast;
using boost::math::sign;
using boost::bind;


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
  std::vector<float> defaultConfigU; // sector sizes
  std::vector<float> defaultConfigV;
  defaultConfigU.push_back(0.0);
  defaultConfigU.push_back(0.5);
  defaultConfigU.push_back(1.0);
  defaultConfigV.push_back(0.0);
  defaultConfigV.push_back(0.5);
  defaultConfigV.push_back(1.0);

  std::vector<std::string> sectorSetup;
  sectorSetup.push_back("std");

  vector<int> detectorType, highestAllowedLayer, minLayer, minState;
  detectorType.push_back(-1);
  highestAllowedLayer.push_back(6);
  minLayer.push_back(4);
  minState.push_back(2);

  vector<double> setupWeigh;
  setupWeigh.push_back(0.0);

  vector<bool> activateDistance3D, activateDistanceXY, activateDistanceZ, activateNormedDistance3D, activateAngles3D, activateAnglesXY, activateAnglesRZ, activateDistanceDeltaZ, activateDistance2IP, activatePT, activateZigZag, activateDeltaPt, activateDeltaDistance2IP;
  activateDistance3D.push_back(true);
  activateDistanceXY.push_back(true);
  activateDistanceZ.push_back(true);
  activateNormedDistance3D.push_back(false);
  activateAngles3D.push_back(true);
  activateAnglesXY.push_back(true);
  activateAnglesRZ.push_back(true);
  activateDistanceDeltaZ.push_back(false);
  activatePT.push_back(false);
  activateDistance2IP.push_back(false);
  activateZigZag.push_back(true);
  activateDeltaPt.push_back(true);
  activateDeltaDistance2IP.push_back(false);
  vector<double> tuneDistance3D, tuneDistanceXY, tuneDistanceZ, tuneNormedDistance3D, tuneAngles3D, tuneAnglesXY, tuneAnglesRZ, tuneDistanceDeltaZ, tuneDistance2IP, tunePT, tuneZigZag, tuneDeltaPt, tuneDeltaDistance2IP;
  tuneDistance3D.push_back(0);
  tuneDistanceXY.push_back(0);
  tuneDistanceZ.push_back(0);
  tuneNormedDistance3D.push_back(0);
  tuneAngles3D.push_back(0);
  tuneAnglesXY.push_back(0);
  tuneAnglesRZ.push_back(0);
  tuneDistanceDeltaZ.push_back(0);
  tuneDistance2IP.push_back(0);
  tunePT.push_back(0);
  tuneZigZag.push_back(0);
  tuneDeltaPt.push_back(0);
  tuneDeltaDistance2IP.push_back(0);

  //Set module properties
  setDescription(" trackfinder for the SVD using cellular automaton techniques, kalman filter (genfit) and a hopfield network as well.");

  ///Steering parameter import

  addParam("sectorConfigU", m_PARAMsectorConfigU, "allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigU);
  addParam("sectorConfigV", m_PARAMsectorConfigV, "allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigV);

  addParam("detectorType", m_PARAMdetectorType, "defines which detector type has to be exported. VXD: -1, PXD: 0, SVD: 1", detectorType);
  addParam("sectorSetup", m_PARAMsectorSetup, "lets you chose the sectorSetup (compatibility of sensors, individual cutoffs,...) accepts 'std', 'low', 'high' and 'personal', please note that the chosen setup has to exist as a xml-file in ../tracking/data/friendList_XXX.xml. If you can not create your own xml files using e.g. the filterCalculatorModule, use params for  'tuneCutoffXXX' or 'setupWeigh' instead. multipass supported by setting setups in a row", sectorSetup);

  addParam("tuneCutoffs", m_PARAMtuneCutoffs, "for rapid changes of cutoffs (no personal xml files needed), reduces/enlarges the range of the cutoffs in percent (lower and upper values are changed by this value). Only valid in range -50% < x < +1000%", double(0.0));

  addParam("setupWeigh", m_PARAMsetupWeigh, " allows to individually weigh (level of surpression) each setup/pass. Please choose value between 0 and 100 (%), higher momentum setups are more reliable, therefore should be weighed best", setupWeigh);

  addParam("activateDistance3D", m_PARAMactivateDistance3D, " set True/False for each setup individually", activateDistance3D);
  addParam("activateDistanceXY", m_PARAMactivateDistanceXY, " set True/False for each setup individually", activateDistanceXY);
  addParam("activateDistanceZ", m_PARAMactivateDistanceZ, " set True/False for each setup individually", activateDistanceZ);
  addParam("activateNormedDistance3D", m_PARAMactivateNormedDistance3D, " set True/False for each setup individually", activateNormedDistance3D);

  addParam("activateAngles3D", m_PARAMactivateAngles3D, " set True/False for each setup individually", activateAngles3D);
  addParam("activateAnglesXY", m_PARAMactivateAnglesXY, " set True/False for each setup individually", activateAnglesXY);
  addParam("activateAnglesRZ", m_PARAMactivateAnglesRZ, " set True/False for each setup individually", activateAnglesRZ);
  addParam("activateDistanceDeltaZ", m_PARAMactivateDistanceDeltaZ, " set True/False for each setup individually", activateDistanceDeltaZ);
  addParam("activateDistance2IP", m_PARAMactivateDistance2IP, " set True/False for each setup individually", activateDistance2IP);
  addParam("activatePT", m_PARAMactivatePT, " set True/False for each setup individually", activatePT);

  addParam("activateZigZag", m_PARAMactivateZigZag, " set True/False for each setup individually", activateZigZag);
  addParam("activateDeltaPt", m_PARAMactivateDeltaPt, " set True/False for each setup individually", activateDeltaPt);
  addParam("activateDeltaDistance2IP", m_PARAMactivateDeltaDistance2IP, " set True/False for each setup individually", activateDeltaDistance2IP);


  addParam("tuneDistance3D", m_PARAMtuneDistance3D, " tune for each setup individually", tuneDistance3D);
  addParam("tuneDistanceXY", m_PARAMtuneDistanceXY, " tune for each setup individually", tuneDistanceXY);
  addParam("tuneDistanceZ", m_PARAMtuneDistanceZ, " tune for each setup individually", tuneDistanceZ);
  addParam("tuneNnormedDistance3D", m_PARAMtuneNormedDistance3D, " tune for each setup individually", tuneNormedDistance3D);

  addParam("tuneAngles3D", m_PARAMtuneAngles3D, " tune for each setup individually", tuneAngles3D);
  addParam("tuneAnglesXY", m_PARAMtuneAnglesXY, " tune for each setup individually", tuneAnglesXY);
  addParam("tuneAnglesRZ", m_PARAMtuneAnglesRZ, " tune for each setup individually", tuneAnglesRZ);
  addParam("tuneDistanceDeltaZ", m_PARAMtuneDistanceDeltaZ, " tune for each setup individually", tuneDistanceDeltaZ);
  addParam("tuneDistance2IP", m_PARAMtuneDistance2IP, " tune for each setup individually", tuneDistance2IP);
  addParam("tunePT", m_PARAMtunePT, " tune for each setup individually", tunePT);

  addParam("tuneZigZag", m_PARAMtuneZigZag, " tune for each setup individually", tuneZigZag);
  addParam("tuneDeltaPt", m_PARAMtuneDeltaPt, " tune for each setup individually", tuneDeltaPt);
  addParam("tuneDeltaDistance2IP", m_PARAMtuneDeltaDistance2IP, " tune for each setup individually", tuneDeltaDistance2IP);


  addParam("tccMinLayer", m_PARAMminLayer, "determines lowest layer considered by track candidate collector", minLayer);
  addParam("tccMinState", m_PARAMminState, "determines lowest state of cells considered by track candidate collector", minState);
  addParam("omega", m_PARAMomega, "tuning parameter for the hopfield network", double(0.5));

  //for testing purposes:
  addParam("highestAllowedLayer", m_PARAMhighestAllowedLayer, "set value below 6 if you want to exclude outer layers (standard is 6)", highestAllowedLayer);
  addParam("standardPdgCode", m_PARAMpdGCode, "standard value is 211 (pi+), ATTENTION, instead of using inconsistent sign of PdGList, in this module positively charged particles are always positive and negatively charged ones are negative (relevant for leptons)", int(211));

  addParam("cleanOverlappingSet", m_PARAMcleanOverlappingSet, "when true, TCs which are found more than once (possible because of multipass) will get filtered", bool(true));
  addParam("useHopfield", m_PARAMuseHopfield, "allows to deactivate hopfield, so overlapping TCs are exported", bool(true));

  addParam("qiSmear", m_PARAMqiSmear, " set True if you want to smear QI's of TCs (needed when no Kalman filter activated) ", bool(false));
  addParam("smearMean", m_PARAMsmearMean, " when qiSmear = True, bias of perturbation can be set here", double(0.0));
  addParam("smearSigma", m_PARAMsmearSigma, " when qiSmear = True, degree of perturbation can be set here", double(0.1));

  addParam("calcQIType", m_PARAMcalcQIType, "allows you to chose the way, the QI's of the TC's shall be calculated. currently supported: 'kalman','trackLength'", string("trackLength"));
  addParam("storeBrokenQI", m_PARAMstoreBrokenQI, "if true, TC survives QI-calculation-process even if fit was not possible", bool(true));

  /// temporarily disabled (maybe used later)
  //  addParam("activateQQQMode", m_activateQQQMode, " set True to calc QQQ-values for TCs", bool(false));
}


VXDTFModule::~VXDTFModule()
{

}

void VXDTFModule::initialize()
{

  if (m_PARAMpdGCode > 10 and m_PARAMpdGCode < 18) { // in this case, its a lepton. since leptons with positive sign have got negative codes, this must taken into notice
    m_chargeSignFactor = 1;
  } else { m_chargeSignFactor = -1; }

  // check misusage of parameters:
  if (m_PARAMtuneCutoffs <= -50.0 || m_PARAMtuneCutoffs > 1000.0) {
    B2WARNING("VXDTF: chosen value for parameter 'tuneCutoffs' is invalid, reseting value to standard (=0.0)...")
    m_PARAMtuneCutoffs = 0;
  }

  if (gGeoManager == NULL) {
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    GFFieldManager::getInstance()->init(new GFGeant4Field());
  }

/// TODO: further checks for validity needed!

  m_nameAngles3D = "angles3D";
  m_nameAnglesRZ = "anglesRZ";
  m_nameAnglesXY = "anglesXY";
  m_nameDistance3D = "distance3D";
  m_nameDistanceXY = "distanceXY";
  m_nameDistanceZ = "distanceZ";
  m_nameDistanceDeltaZ = "distanceDeltaZ";
  m_namePT = "pT";
  m_nameDeltapT = "deltaPt";
  m_nameNormedDistance3D = "normedDistance3D";
  m_nameTRadius2IPDistance = "dist2IP";
  m_nameDeltaTRadius2IPDistance = "deltaDist2IP";


  /// GFTrackCandidate
  StoreArray<GFTrackCand>::registerPersistent();
  StoreArray<GFTrack>::registerPersistent();

  /// temporary members for testing purposes (minimal testing routines)
  m_TESTERtriggeredZigZag = 0;
  m_TESTERtriggeredDpT = 0;
  m_TESTERapprovedByTCC = 0;
  m_TESTERcountTotalTCsAfterTCC = 0;
  m_TESTERcountTotalTCsAfterTCCFilter = 0;
  m_TESTERcountTotalTCsFinal = 0;
  m_TESTERbadHopfieldCtr = 0;
  m_TESTERfilteredOverlapsQI = 0;
  m_TESTERNotFilteredOverlapsQI = 0;
  m_TESTERfilteredOverlapsQICtr = 0;
  m_TESTERcleanOverlappingSetStartedCtr = 0;
  m_TESTERgoodFitsCtr = 0;
  m_TESTERbadFitsCtr = 0;
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
  for (int i = 0; i < int(m_PARAMsectorSetup.size()); ++i) {
    CurrentPassData* newPass = new CurrentPassData;

    newPass->sectorSetup = m_PARAMsectorSetup[i];

    if (int (m_PARAMdetectorType.size()) < i) {
      newPass->detectorType = m_PARAMdetectorType[m_PARAMdetectorType.size() - 1];
    } else {
      newPass->detectorType = m_PARAMdetectorType[i];
    }
    if (int (m_PARAMsetupWeigh.size()) < i) {
      newPass->distance3D.first = m_PARAMsetupWeigh[m_PARAMsetupWeigh.size() - 1] * 0.01;
    } else {
      newPass->setupWeigh = m_PARAMsetupWeigh[i] * 0.01;
      if (newPass->setupWeigh < 0) { newPass->setupWeigh = 0; } else if (newPass->setupWeigh > 1.) { newPass->setupWeigh = 1.; }
    }
    if (int (m_PARAMhighestAllowedLayer.size()) < i) {
      newPass->highestAllowedLayer = m_PARAMhighestAllowedLayer[m_PARAMhighestAllowedLayer.size() - 1];
    } else {
      newPass->highestAllowedLayer = m_PARAMhighestAllowedLayer[i];
    }
    if (int (m_PARAMminLayer.size()) < i) {
      newPass->minLayer = m_PARAMminLayer[m_PARAMminLayer.size() - 1];
    } else {
      newPass->minLayer = m_PARAMminLayer[i];
    }
    if (int (m_PARAMminState.size()) < i) {
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
//    /*if (newPass->numtotalLayers > m_numtotalLayers ) { m_numtotalLayers = newPass->numtotalLayers; } // this value only stores the highest number of total layers*/
    B2INFO("Pass " << i << ": chosen detectorType: " << newPass->chosenDetectorType << " and chosen sectorSetup: " << newPass->sectorSetup)

    newPass->secConfigU = m_PARAMsectorConfigU; // currently same sectorConfig for each pass, future support of different setups prepared
    newPass->secConfigV = m_PARAMsectorConfigV; // currently same sectorConfig for each pass, future support of different setups prepared

    int sfCtr = 0, nfCtr = 0, tccfCtr = 0; // counting number of activated tests for each filter step
    ///sFinder:
    if (int (m_PARAMactivateDistance3D.size()) < i) {
      newPass->distance3D.first = m_PARAMactivateDistance3D[m_PARAMactivateDistance3D.size() - 1];
    } else {
      newPass->distance3D.first = m_PARAMactivateDistance3D[i];
    }
    if (int (m_PARAMactivateDistanceXY.size()) < i) {
      newPass->distanceXY.first = m_PARAMactivateDistanceXY[m_PARAMactivateDistanceXY.size() - 1];
    } else {
      newPass->distanceXY.first = m_PARAMactivateDistanceXY[i];;
    }
    if (int (m_PARAMactivateDistanceZ.size()) < i) {
      newPass->distanceZ.first = m_PARAMactivateDistanceZ[m_PARAMactivateDistanceZ.size() - 1];
    } else {
      newPass->distanceZ.first = m_PARAMactivateDistanceZ[i];
    }
    if (int (m_PARAMactivateNormedDistance3D.size()) < i) {
      newPass->normedDistance3D.first = m_PARAMactivateNormedDistance3D[m_PARAMactivateNormedDistance3D.size() - 1];
    } else {
      newPass->normedDistance3D.first = m_PARAMactivateNormedDistance3D[i];
    }
    if (newPass->distance3D.first == true) { sfCtr++; }
    if (newPass->distanceXY.first == true) { sfCtr++; }
    if (newPass->distanceZ.first == true) { sfCtr++; }
    if (newPass->normedDistance3D.first == true) { sfCtr++; }

    ///nbFinder:
    if (int (m_PARAMactivateAngles3D.size()) < i) {
      newPass->angles3D.first = m_PARAMactivateAngles3D[m_PARAMactivateAngles3D.size() - 1];
    } else {
      newPass->angles3D.first =  m_PARAMactivateAngles3D[i];
    }
    if (int (m_PARAMactivateAnglesXY.size()) < i) {
      newPass->anglesXY.first = m_PARAMactivateAnglesXY[m_PARAMactivateAnglesXY.size() - 1];
    } else {
      newPass->anglesXY.first = m_PARAMactivateAnglesXY[i];
    }
    if (int (m_PARAMactivateAnglesRZ.size()) < i) {
      newPass->anglesRZ.first = m_PARAMactivateAnglesRZ[m_PARAMactivateAnglesRZ.size() - 1];
    } else {
      newPass->anglesRZ.first = m_PARAMactivateAnglesRZ[i];
    }
    if (int (m_PARAMactivateDistanceDeltaZ.size()) < i) {
      newPass->distanceDeltaZ.first = m_PARAMactivateDistanceDeltaZ[m_PARAMactivateDistanceDeltaZ.size() - 1];
    } else {
      newPass->distanceDeltaZ.first = m_PARAMactivateDistanceDeltaZ[i];
    }
    if (int (m_PARAMactivateDistance2IP.size()) < i) {
      newPass->distance2IP.first = m_PARAMactivateDistance2IP[m_PARAMactivateDistance2IP.size() - 1];
    } else {
      newPass->distance2IP.first =  m_PARAMactivateDistance2IP[i];
    }
    if (int (m_PARAMactivatePT.size()) < i) {
      newPass->pT.first = m_PARAMactivatePT[m_PARAMactivatePT.size() - 1];
    } else {
      newPass->pT.first =  m_PARAMactivatePT[i];
    }
    if (newPass->angles3D.first == true) { nfCtr++; }
    if (newPass->anglesXY.first == true) { nfCtr++; }
    if (newPass->anglesRZ.first == true) { nfCtr++; }
    if (newPass->distanceDeltaZ.first == true) { nfCtr++; }
    if (newPass->distance2IP.first == true) { nfCtr++; }
    if (newPass->pT.first == true) { nfCtr++; }

    /// post-TCC-filter:
    if (int (m_PARAMactivateZigZag.size()) < i) {
      newPass->zigzag.first = m_PARAMactivateZigZag[m_PARAMactivateZigZag.size() - 1];
    } else {
      newPass->zigzag.first = m_PARAMactivateZigZag[i];
    }
    if (int (m_PARAMactivateDeltaPt.size()) < i) {
      newPass->deltaPt.first = m_PARAMactivateDeltaPt[m_PARAMactivateDeltaPt.size() - 1];
    } else {
      newPass->deltaPt.first = m_PARAMactivateDeltaPt[i];
    }
    if (int (m_PARAMactivateDeltaDistance2IP.size()) < i) {
      newPass->deltaDistance2IP.first = m_PARAMactivateDeltaDistance2IP[m_PARAMactivateDeltaDistance2IP.size() - 1];
    } else {
      newPass->deltaDistance2IP.first = m_PARAMactivateDeltaDistance2IP[i];
    }
    if (newPass->zigzag.first == true) { tccfCtr++; }
    if (newPass->deltaPt.first == true) { tccfCtr++; }
    if (newPass->deltaDistance2IP.first == true) { tccfCtr++; }

    newPass->activatedSegFinderTests = sfCtr;
    newPass->activatedNbFinderTests = nfCtr;
    newPass->activatedTccFilterTests = tccfCtr;
    B2INFO("Pass " << i << " VXD Track finder: " << sfCtr << " segFinder tests, " << nfCtr << " friendFinder tests and " << tccfCtr << " TCC filter tests are enabled.");

    if (int (m_PARAMtuneDistance3D.size()) < i) {
      newPass->distance3D.second = m_PARAMtuneDistance3D[m_PARAMtuneDistance3D.size() - 1];
    } else {
      newPass->distance3D.second = m_PARAMtuneDistance3D[i];
    }
    if (int (m_PARAMtuneDistanceXY.size()) < i) {
      newPass->distanceXY.second = m_PARAMtuneDistanceXY[m_PARAMtuneDistanceXY.size() - 1];
    } else {
      newPass->distanceXY.second = m_PARAMtuneDistanceXY[i];;
    }
    if (int (m_PARAMtuneDistanceZ.size()) < i) {
      newPass->distanceZ.second = m_PARAMtuneDistanceZ[m_PARAMtuneDistanceZ.size() - 1];
    } else {
      newPass->distanceZ.second = m_PARAMtuneDistanceZ[i];
    }
    if (int (m_PARAMtuneNormedDistance3D.size()) < i) {
      newPass->normedDistance3D.second = m_PARAMtuneNormedDistance3D[m_PARAMtuneNormedDistance3D.size() - 1];
    } else {
      newPass->normedDistance3D.second = m_PARAMtuneNormedDistance3D[i];
    }
    if (int (m_PARAMtuneAngles3D.size()) < i) {
      newPass->angles3D.second = m_PARAMtuneAngles3D[m_PARAMtuneAngles3D.size() - 1];
    } else {
      newPass->angles3D.second =  m_PARAMtuneAngles3D[i];
    }
    if (int (m_PARAMtuneAnglesRZ.size()) < i) {
      newPass->anglesXY.second = m_PARAMtuneAnglesRZ[m_PARAMtuneAnglesRZ.size() - 1];
    } else {
      newPass->anglesXY.second = m_PARAMtuneAnglesRZ[i];
    }
    if (int (m_PARAMtuneAnglesRZ.size()) < i) {
      newPass->anglesRZ.second = m_PARAMtuneAnglesRZ[m_PARAMtuneAnglesRZ.size() - 1];
    } else {
      newPass->anglesRZ.second = m_PARAMtuneAnglesRZ[i];
    }
    if (int (m_PARAMtuneDistanceDeltaZ.size()) < i) {
      newPass->distanceDeltaZ.second = m_PARAMtuneDistanceDeltaZ[m_PARAMtuneDistanceDeltaZ.size() - 1];
    } else {
      newPass->distanceDeltaZ.second = m_PARAMtuneDistanceDeltaZ[i];
    }
    if (int (m_PARAMtuneDistance2IP.size()) < i) {
      newPass->distance2IP.second = m_PARAMtuneDistance2IP[m_PARAMtuneDistance2IP.size() - 1];
    } else {
      newPass->distance2IP.second =  m_PARAMtuneDistance2IP[i];
    }
    if (int (m_PARAMtunePT.size()) < i) {
      newPass->pT.second = m_PARAMtunePT[m_PARAMtunePT.size() - 1];
    } else {
      newPass->pT.second =  m_PARAMtunePT[i];
    }
    if (int (m_PARAMtuneZigZag.size()) < i) {
      newPass->zigzag.second = m_PARAMtuneZigZag[m_PARAMtuneZigZag.size() - 1];
    } else {
      newPass->zigzag.second = m_PARAMtuneZigZag[i];
    }
    if (int (m_PARAMtuneDeltaPt.size()) < i) {
      newPass->deltaPt.second = m_PARAMtuneDeltaPt[m_PARAMtuneDeltaPt.size() - 1];
    } else {
      newPass->deltaPt.second = m_PARAMtuneDeltaPt[i];
    }
    if (int (m_PARAMtuneDeltaDistance2IP.size()) < i) {
      newPass->deltaDistance2IP.second = m_PARAMtuneDeltaDistance2IP[m_PARAMtuneDeltaDistance2IP.size() - 1];
    } else {
      newPass->deltaDistance2IP.second = m_PARAMtuneDeltaDistance2IP[i];
    }



    /// importing sectorMap including friend Information and friend specific cutoffs
    newPass->sectorMap.clear();
    totalFriendCounter = 0;

    GearDir belleDir("/Detector/Tracking/CATFParameters/");

    string chosenSetup = (boost::format("sectorList_%1%_%2%") % newPass->sectorSetup  % newPass->chosenDetectorType).str();

    GearDir sectorList(belleDir, chosenSetup);

    B2INFO("importing sectors, using " << chosenSetup << " > " << sectorList.getNumberNodes("aSector") << " sectors found");

    if (sectorList.getNumberNodes("aSector") == 0) {
      B2FATAL("Failed to import sector map " << chosenSetup << "! No track finding possible...")
    }
    BOOST_FOREACH(const GearDir & aSector, sectorList.getNodes("aSector")) {

      string aSectorName = aSector.getString("sectorOfInterest");

      GearDir friendList(aSector, "friendList/");

      B2DEBUG(1000, " > > importing sector: " << aSectorName << " including " << friendList.getNumberNodes("aFriend") << " friends. ");
      totalFriendCounter += friendList.getNumberNodes("aFriend");
      VXDSector* pSector = new VXDSector(aSectorName);

      BOOST_FOREACH(const GearDir & aFriend, friendList.getNodes("aFriend")) {

        string aFriendName = aFriend.getString("FriendOfInterest");

        GearDir filterList(aFriend, "filterList/");

        BOOST_FOREACH(const GearDir & aFilter, filterList.getNodes("aFilter")) {

          string aFilterName = aFilter.getString("FilterOfInterest");

          GearDir quantiles(aFilter, "quantiles/");

          double cutoffMinValue = quantiles.getDouble("Min");
          double cutoffMaxValue = quantiles.getDouble("Max");
          /*      tempQuantile = make_pair("q0.01", cutoffMinValue);
           p Cutoff->addQuantile(tempQuantile);*/

          pair<double, double> cutoff = make_pair(cutoffMinValue, cutoffMaxValue);

          pSector->addCutoff(aFilterName, aFriendName, cutoff);
        }
      }
      newPass->sectorMap.insert(make_pair(aSectorName, pSector));
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
  m_badFriendCounter = 0;
}

/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** *********************************+ event +********************************* **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/

void VXDTFModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
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


  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();

  // importing hits
  StoreArray<PXDCluster> aPxdClusterArray;
  int numOfPxdClusters = aPxdClusterArray.getEntries();
  StoreArray<SVDCluster> aSvdClusterArray;
  int numOfSvdClusters = aSvdClusterArray.getEntries();

  // preparing storearray for trackCandidates and fitted tracks
  StoreArray<GFTrackCand> finalTrackCandidates;
  finalTrackCandidates.create();
  StoreArray<GFTrack> fittedTracks; //needed for Kalman filter
  fittedTracks.create();

  B2DEBUG(10, "VXDTF event " << m_eventCounter << ": size of arrays, PXDCluster: " << numOfPxdClusters << ", SVDCLuster: " << numOfSvdClusters);

  TVector3 hitGlobal, hitLocal, transformedHitLocal, localSensorSize;
  double vSize, uSizeAtHit, uCoord, vCoord, aLayerID;
  string aSectorName;
  VxdID aVxdID;

  if (m_usePXDorSVDorVXDhits not_eq 1) {   // means: is true when at least one pass wants PXD hits

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

        if (aSectorName == "") {
          B2DEBUG(10, "VXDTF - event " << m_eventCounter << ": pxdhit out of sector range(setup " << currentPass->sectorSetup << ", type  " << currentPass->chosenDetectorType << "). (" << aSectorName << " does not exist) Discarding hit...");
          m_badSectorRangeCounter++;
          continue;
        }

        B2DEBUG(50, " PXDCluster: with posOfHit in StoreArray: " << iPart << " is found again within secID " << aSectorName << " using sectorSetup " << currentPass->sectorSetup);
        VXDTFHit* pTFHit = new VXDTFHit(hitGlobal, passNumber, 0, 0, iPart, 0, aSectorName, aVxdID, 0.0); // no timeInfo for PXDHits

        currentPass->hitVector.push_back(pTFHit);
        secMapIter->second->addHit(pTFHit);
        currentPass->sectorSequence.push_back(activatedSector);

        B2DEBUG(200, "size of sectorSequence: " << currentPass->sectorSequence.size() << "size of hitVector: " << currentPass->hitVector.size());
        passNumber++;
      }
    }
  }

  if (m_usePXDorSVDorVXDhits not_eq 0) {   // means: is true when at least one pass wants SVD hits

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
    BOOST_FOREACH(mapEntry aSensor, activatedSensors) {
      B2DEBUG(100, " sensor " << aSensor.first << " has got " << aSensor.second.uClusters.size() << " uClusters and " << aSensor.second.vClusters.size() << " vClusters")

      for (int uClNum = 0; uClNum < int(aSensor.second.uClusters.size()); ++uClNum) {
        for (int vClNum = 0; vClNum < int(aSensor.second.vClusters.size()); ++vClNum) {
          ClusterHit aHit;
          aHit.uCluster = aSensor.second.uClusters[uClNum].second;
          aHit.uClusterIndex = aSensor.second.uClusters[uClNum].first;
          aHit.vCluster = aSensor.second.vClusters[vClNum].second;
          aHit.vClusterIndex = aSensor.second.vClusters[vClNum].first;
          clusterHitList.push_back(aHit);
        }
      }
    }

    B2DEBUG(20, clusterHitList.size() << " SVDCluster combinations found, while there are " << numOfSvdClusters << " SVDClusters")

    for (int iPart = 0; iPart < int(clusterHitList.size()); ++iPart) {

      const SVDCluster* const aClusterPtr = clusterHitList[iPart].uCluster; // uCluster
      const SVDCluster* const aSecondClusterPtr = clusterHitList[iPart].vCluster; // vCluster
      int clusterIndexU = clusterHitList[iPart].uClusterIndex;
      int clusterIndexV = clusterHitList[iPart].vClusterIndex;

      float timeStampU = aClusterPtr->getClsTime();
      float timeStampV = aSecondClusterPtr->getClsTime();

      hitLocal.SetXYZ(aClusterPtr->getPosition(), aSecondClusterPtr->getPosition(), 0); // WARNING: this version is wrong for slanted parts, therefore only a temporary solution

      VxdID aVxdID = aClusterPtr->getSensorID();
      VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);
      hitGlobal = aSensorInfo.pointToGlobal(hitLocal);

      // local(0,0,0) is the center of the sensorplane
      vSize = 0.5 * aSensorInfo.getVSize();
      uSizeAtHit = 0.5 * aSensorInfo.getUSize(hitLocal[1]);

      uCoord = hitLocal[0] + uSizeAtHit; // *0,5 putting (0,0) from the center to the edge of the plane (considers the trapeziodal shape)
      vCoord = hitLocal[1] + vSize;

      transformedHitLocal.SetXYZ(uCoord, vCoord, 0);
      localSensorSize.SetXYZ(uSizeAtHit, vSize, 0);

//      B2DEBUG(1000,"Sensor edges: O("<<-uSize1<<","<<-vSize1<<"), U("<<uSize1<<","<<-vSize1<<"), V("<<-uSize2<<","<<vSize1<<"), UV("<<uSize2<<","<<vSize1<<")");
      B2DEBUG(1000, "local svd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ")");

      passNumber = 0;
      BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
        if (aLayerID > currentPass->highestAllowedLayer) { continue; }   // skip particle if True
        if (currentPass->detectorType not_eq 1 && currentPass->detectorType not_eq - 1) { continue; }  // SVD is included in 1 & -1

        pair<string, MapOfSectors::iterator> activatedSector = searchSector4Hit(aVxdID,
                                                               transformedHitLocal,
                                                               localSensorSize,
                                                               currentPass->sectorMap,
                                                               currentPass->secConfigU,
                                                               currentPass->secConfigV);
        aSectorName = activatedSector.first;
        MapOfSectors::iterator secMapIter =  activatedSector.second;

        if (aSectorName == "") {
          B2DEBUG(10, "VXDTF - event " << m_eventCounter << ": svdhit out of sector range(setup " << currentPass->sectorSetup << ", type  " << currentPass->chosenDetectorType << "). (" << aSectorName << " does not exist) Discarding hit...");
          m_badSectorRangeCounter++;
          continue;
        }

        B2DEBUG(50, " SVDCluster: with posOfHit in StoreArray: " << iPart << " is found again within secID " << aSectorName << " using sectorSetup " << currentPass->sectorSetup);
        VXDTFHit* pTFHit = new VXDTFHit(hitGlobal, passNumber, clusterIndexU, clusterIndexV, 0, 1, aSectorName, aVxdID,  0.5 * (timeStampU + timeStampV));

        currentPass->hitVector.push_back(pTFHit);
        secMapIter->second->addHit(pTFHit);
        currentPass->sectorSequence.push_back(activatedSector);

        B2DEBUG(200, "size of sectorSequence: " << currentPass->sectorSequence.size() << "size of hitVector: " << currentPass->hitVector.size());
        passNumber++;
      }
    }
  }
  B2DEBUG(10, " of " << numOfSvdClusters << " svdClusters and " << numOfPxdClusters  << " pxdClusters, " << m_badSectorRangeCounter << " hits had to be discarded because out of sector range")
  /** Section 3 - end **/



  /** Section 4 - SEGFINDER **/
  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
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

    B2DEBUG(10, "Pass " << passNumber << ": " << currentPass->sectorSequence.size() << " sectors activated, ");
    passNumber++;
  }

  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    B2DEBUG(1, "pass " << passNumber << ": starting segFinder...");
    int discardedSegments = segFinder(currentPass); /// calling funtion "segFinder"
    B2DEBUG(10, "VXDTF-event" << m_eventCounter << ", pass" << passNumber << " @ segfinder - " << currentPass->activeCellList.size() << " segments activated, " << discardedSegments << " discarded");
    passNumber++;
  }
  /** Section 4 - end **/



  /** Section 5 - NEIGHBOURFINDER **/
  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    B2DEBUG(1, "pass " << passNumber << ": starting neighbourFinder...");
    int discardedSegments = neighbourFinder(currentPass); /// calling funtion "neighbourFinder"
    B2DEBUG(10, "VXDTF-event" << m_eventCounter << ", pass" << passNumber << " @ nbfinder - " << currentPass->activeCellList.size() << " segments activated, " << discardedSegments << " discarded");
    passNumber++;
  }
  /** Section 5 - end **/



  /** Section 6 - Cellular Automaton**/
  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    B2DEBUG(1, "pass " << passNumber << ": starting cellular automaton...");
    cellularAutomaton(currentPass);
    passNumber++;
  }
  /** Section 6 - end **/



  /** Section 7 - Track Candidate Collector (TCC) **/
  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    delFalseFriends(currentPass, centerPosition);
    B2DEBUG(1, "pass " << passNumber << ": track candidate collector...");
    tcCollector(currentPass);
    passNumber++;
  }
  /** Section 7 - end **/

  /** Section 8 - tcFilter **/
  passNumber = 0;
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    B2DEBUG(1, "pass " << passNumber << ": track candidate filter...");
    tcFilter(currentPass, passNumber);
    passNumber++;
  }
  /** Section 8 - end **/


  /// getting a list of clusterIndices to be able to search for overlapping TCs no matter which pass they come from
  ClusterUsage totalClusterUsage;
// // //  B2ERROR("before reserving space for clusters: size of SVDClusters:" << totalClusterUsage.SVDClusters.size() << ", numOfSvdClusters: " << numOfSvdClusters << "PXDDClusters:" << totalClusterUsage.PXDClusters.size() << ", numOfPxdClusters: " << numOfPxdClusters)
  totalClusterUsage.PXDClusters.resize(numOfPxdClusters);
  totalClusterUsage.SVDClusters.resize(numOfSvdClusters); // u is true, v is false
// // //  B2WARNING("after reserving space for clusters: size of SVDClusters:" << totalClusterUsage.SVDClusters.size() << ", numOfSvdClusters: " << numOfSvdClusters << "PXDDClusters:" << totalClusterUsage.PXDClusters.size() << ", numOfPxdClusters: " << numOfPxdClusters)

  /// for each hit of each tc of each pass get clusterIndices and store a pointer to currentTC
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentPass->tcVector) {
      if (currentTC->getCondition() == false) { continue; }   // in this case, the TC is already filtered

      BOOST_FOREACH(VXDTFHit * currentHit, currentTC->getHits()) {
        int detectorType = currentHit->getDetectorType();

        if (detectorType == 1) {   // SVD
          int clusterIndexU = currentHit->getClusterIndexU();
          B2DEBUG(100, "clusterIndexU=" << clusterIndexU << ", size of SVDClusters:" << totalClusterUsage.SVDClusters.size() << ", numOfSvdClusters: " << numOfSvdClusters << ", currentTC: " << currentTC)
          totalClusterUsage.SVDClusters[clusterIndexU].push_back(currentTC);
          currentTC->addSVDClusterIndex(clusterIndexU);
          currentTC->addHopfieldClusterIndex(clusterIndexU);

          int clusterIndexV = currentHit->getClusterIndexV();
          B2DEBUG(100, "clusterIndexV=" << clusterIndexV << ", size of SVDClusters:" << totalClusterUsage.SVDClusters.size() << ", numOfSvdClusters: " << numOfSvdClusters << ", currentTC: " << currentTC)
          totalClusterUsage.SVDClusters[clusterIndexV].push_back(currentTC);
          currentTC->addSVDClusterIndex(clusterIndexV);
          currentTC->addHopfieldClusterIndex(clusterIndexV);

        } else if (detectorType == 0) {   // PXD
          int clusterIndexUV = currentHit->getClusterIndexUV();
          totalClusterUsage.PXDClusters[clusterIndexUV].push_back(currentTC);
          currentTC->addPXDClusterIndex(clusterIndexUV);
          currentTC->addHopfieldClusterIndex(clusterIndexUV + numOfSvdClusters); // although they dont work as indices anymore, they still are unique and therefore interesting for the Hopfield network

        } else {
          B2FATAL("event " << m_eventCounter << ": tcc-Filter - illegal detector assignement to hit! Assigned detector type: " << detectorType)
        }
      }
    }
  }

  /// check each hit vor overlapping TCs and inform TCs about their overlapping state, collect overlapping TCs for Hopfield network:
  BOOST_FOREACH(TCsOfEvent currentTcVector, totalClusterUsage.SVDClusters) {  /// SVD
    if (int(currentTcVector.size()) < 2) { continue; }  // if true, hit is used only once

    BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentTcVector) {
      currentTC->setOverlappingState(true);
    }
  }
  BOOST_FOREACH(TCsOfEvent currentTcVector, totalClusterUsage.PXDClusters) {  /// PXD
    if (int(currentTcVector.size()) < 2) { continue; }  // if true, hit is used only once

    BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentTcVector) {
      currentTC->setOverlappingState(true);
    }
  }

  ///collect TCs and separately store overlapping TCs for hopfield used
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {
    BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentPass->tcVector) {
      bool overbooked = currentTC->getOverlappingState();
      if (currentTC->getCondition() == false) { continue; }
      m_tcVector.push_back(currentTC);
      if (overbooked == true) {
        m_tcVectorOverlapped.push_back(currentTC);
      }
    }
  }
  int totalOverlaps = m_tcVectorOverlapped.size();
  B2DEBUG(10, "event " << m_eventCounter << ": " << totalOverlaps << " overlapping track candidates found")



  /// determine initial values for all track Candidates:
  calcInitialValues4TCs(m_tcVector);


  ///calc QI for each TC:
  if (m_PARAMcalcQIType == "kalman") {
    vector<GFTrackCand> temporalTrackCandidates = calcQIbyKalman(m_tcVector, aPxdClusterArray, aSvdClusterArray); /// calcQIbyKalman
  } else if (m_PARAMcalcQIType == "trackLength") {
    calcQIbyLength(m_tcVector, m_passSetupVector);  /// calcQIbyLength
  }

  if (totalOverlaps > 2 && m_PARAMcleanOverlappingSet == true) {
    int olSize = m_tcVectorOverlapped.size();
    cleanOverlappingSet(m_tcVectorOverlapped); /// removes TCs which are found more than once completely
    m_TESTERcleanOverlappingSetStartedCtr++;
    totalOverlaps = m_tcVectorOverlapped.size();
    B2DEBUG(10, "out of funcCleanOverlappingSet: tcVectorBefore.size(): " << olSize << ", tcVectorAfter.size(): " << totalOverlaps)
  }

  if (m_PARAMuseHopfield == true) {
    /// checking overlapping TCs for best subset, if there are more than 2 different TC's
    if (totalOverlaps > 2) {
      hopfield(m_tcVectorOverlapped, m_PARAMomega); /// hopfield
    } else if (totalOverlaps == 2) {
      // for that easy situation we dont need the neuronal network, especially when the nn does sometimes chose the wrong one...
      if (m_tcVectorOverlapped[0]->getTrackQuality() > m_tcVectorOverlapped[1]->getTrackQuality()) {
        m_tcVectorOverlapped[1]->setCondition(false);
      } else { m_tcVectorOverlapped[0]->setCondition(false); }
    } else { B2DEBUG(10, " less than 2 overlapping Track Candidates found, no need for neuronal network") }
  }

  B2DEBUG(10, "before exporting TCs, length of m_tcVector: " << m_tcVector.size() << ", m_tcVectorOverlapped: " << m_tcVectorOverlapped.size());
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, m_tcVector) {
    if (currentTC->getCondition() == false) { continue; }

    GFTrackCand gfTC = generateGFTrackCand(currentTC); /// generateGFTrackCand
    finalTrackCandidates.appendNew(gfTC);
  }

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
  B2DEBUG(1, "vxdtf - event " << m_eventCounter << ": " << finalTrackCandidates.getEntries() << " final track candidates determined!")
  m_TESTERcountTotalTCsFinal += finalTrackCandidates.getEntries();



  /** cleaning part **/
  BOOST_FOREACH(CurrentPassData * currentPass, m_passSetupVector) {

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
}


/** *************************************+************************************* **/
/** ***********************************+ + +*********************************** **/
/** ********************************+  endRun +******************************** **/
/** ***********************************+ + +*********************************** **/
/** *************************************+************************************* **/


void VXDTFModule::endRun()
{
  B2INFO("VXDTF endRun, after " << m_eventCounter + 1 << " events, ZigZag triggered " << m_TESTERtriggeredZigZag << " times, and dpT triggered " << m_TESTERtriggeredDpT << " times, TCC approved " << m_TESTERapprovedByTCC << " TCs, " << m_badFriendCounter << " hits lost because of hit having no neighbouring hits in friend sector, " << m_TESTERbadHopfieldCtr << " times, the Hopfield network had no survivors!")
  B2INFO("VXDTF endRun, total number of TCs after TCC: " << m_TESTERcountTotalTCsAfterTCC << ", after TCC-filter: " << m_TESTERcountTotalTCsAfterTCCFilter << ", final: " << m_TESTERcountTotalTCsFinal)
  B2INFO("VXDTF endRun, numOfTimes cleanOverlappingSet got activated:" << m_TESTERcleanOverlappingSetStartedCtr << ", cleanOverlappingSet killed numTCs: " << m_TESTERfilteredOverlapsQI << ", numOfTimes cleanOverlappingSet did/didn't filter TCs: " << m_TESTERfilteredOverlapsQICtr << "/" << m_TESTERNotFilteredOverlapsQI)
  B2INFO("VXDTF endRun, number of times, where a kalman fit was possible: " << m_TESTERgoodFitsCtr << ", where it failed: " << m_TESTERbadFitsCtr)
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

    const std::vector<VXDTFHit*>& hitsOfTC = currentTC->getHits();
    for (int i = 0; i < int(hitsOfTC.size()); i++) {
      hitsOfTC[i]->addTrackCandidate(); // needed for overlapping TC detection
    }

  } else if (nbSize == 1) {
    VXDSegmentCell* pNextSeg =  *nbIter;
    currentTC->addSegments(pNextSeg);
    currentTC->addHits(pNextSeg->getInnerHit());

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
        W(itrk, jtrk) = (1.0 - omega) / double(numOfTCs - 1);
        W(jtrk, itrk) = (1.0 - omega) / double(numOfTCs - 1);
      }
    }
  } // checking compatibility of TCs (compatible if they dont share hits, not compatible if they share ones)

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
    nNcounter = nNcounter + 1;

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
  if (survivorCtr == 0) { m_TESTERbadHopfieldCtr++; }
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
    vector<float>& uConfig,
    vector<float>& vConfig
                                                                               )
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
            aSectorName = ""; // if there is still no sector found, then the hit doesn't lie in defined area.
          }
        }
      }
    }
  } //sector-searching loop
  pair<string, VXDTFModule::MapOfSectors::iterator> result;
  result = make_pair(aSectorName, secMapIter);
  return result;
}



/** ***** searchTCs4overlaps ***** **/
/// checks for each TC of any pass whether they share hits or not
// void VXDTFModule::searchTCs4overlaps (TCsOfEvent& tcList,  VXDTFTrackCandidate* currentTC, std::string maxLayer) {
/// TODO TODO TODO
/**
 * develop noncombinatorial method to find overlapping TCs -.- -.- -.-
 */
// }



bool VXDTFModule::compareSecSequence(pair<string, MapOfSectors::iterator>& lhs, pair<string, MapOfSectors::iterator>& rhs)
{

  int compareValue = lhs.first.compare(rhs.first); // if lhs.first < rhs.first -> compareValue = -1;

  if (compareValue < 0) {
    return true;
  } else {
    return false;
  }
}


/** ***** segFinder ***** **/
/// searches for segments in given pass and returns number of discarded segments
int VXDTFModule::segFinder(CurrentPassData* currentPass)
{
  string currentLayerID, friendLayerID, currentScope, currentSecID, currentFriendID, ownSecName, friendSecName;
  TVector3 currentCoords, friendCoords, currentVector, tempVector;
  double currentDistance, currentDeltaValue;
  int simpleSegmentQI; // better than segmentApproved, but still digital (only min and max cutoff values), but could be weighed by order of relevance
//   int scoreQQQ;  //best would be quasi analogue quality indicator QAQI
  int discardedSegmentsCounter = 0;
  OperationSequenceOfActivatedSectors::const_iterator secSequenceIter;
  MapOfSectors::iterator mainSecIter;
  MapOfSectors::iterator friendSecIter;
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
      friendSecIter = currentPass->sectorMap.find(thisFriendSector);
      if (friendSecIter == currentPass->sectorMap.end()) {
        B2DEBUG(0, "event " << m_eventCounter << ": friendSector " << thisFriendSector << " not found. No friendHits imported...");
        m_badFriendCounter++;
        continue;
      } else {
        vector<VXDTFHit*> friendHits = friendSecIter->second->getHits();
        allFriendHits.insert(allFriendHits.end(), friendHits.begin(), friendHits.end());
      }
    } // iterating through friendsectors and importing their containing hits

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
        MapOfSectors::iterator currentFriendIter = currentPass->sectorMap.find(currentFriendID);
        friendLayerID = currentFriendID[0];
        //         currentScope = str(format("L%1%L%2%") % friendLayerID % currentLayerID);

        friendCoords = allFriendHits[friendHit]->getHitCoordinates();

        if (currentPass->distance3D.first == true) { // min & max!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameDistance3D, currentFriendID);
          //           const Cutoff* pCutoff = mainSectorIter->second->getCutoff(m_nameDistance3D, currentScope);
          if (pCutoff != NULL) {
            currentVector = currentCoords - friendCoords;
            currentDistance = currentVector.Mag();
            if (currentDistance > pCutoff->getMinValue() && currentDistance < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              //              vector< pair < string, double> > quantilesSet = pCutoff->getQuantiles();
              //              scoreQQQ = scoreQQQ +  calcQQQscore(quantilesSet, currentDistance, m_numOfQuantiles);
              B2DEBUG(1000, " dist3d: segment approved!")
            } else {}/// if the current cutoff doesn't exist, then we simply asume that this segment is invalid -> not approved
          } else {}/// else segment not approved
        } else { /*simpleSegmentQI++;*/ } // no test means that the segment always passes the exam ;) we dont have to count since number of activated Tests are considered

        if (currentPass->distanceXY.first == true) { // min & max!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameDistanceXY, currentFriendID);
          if (pCutoff != NULL) {
            currentVector = currentCoords - friendCoords;
            currentVector.SetZ(0.);
            currentDistance = currentVector.Mag();
            if (currentDistance > pCutoff->getMinValue() && currentDistance < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              B2DEBUG(1000, " distxy: segment approved!")
              //              scoreQQQ = scoreQQQ +  calcQQQscore(pCutoff->getQuantiles(), currentDistance, m_numOfQuantiles);
            } else {}
          } else {}
        } else { /*simpleSegmentQI++;*/ } // no test means that the segment always passes the exam ;)

        if (currentPass->distanceZ.first == true) { // max only!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameDistanceZ, currentFriendID);
          if (pCutoff != NULL) {
            currentVector = currentCoords - friendCoords;
            currentVector.SetX(0.);
            currentVector.SetY(0.);
            currentDistance = currentVector.Mag();
            if (currentDistance < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              B2DEBUG(1000, " distz: segment approved!")
              //              scoreQQQ = scoreQQQ +  calcQQQscoreDeltas(pCutoff->getQuantiles(), currentDistance, m_numOfQuantiles);
            } else {}
          } else {}
        } else { /*simpleSegmentQI++;*/ } // no test means that the segment always passes the exam ;)

        if (currentPass->normedDistance3D.first == true) { // max only!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameNormedDistance3D, currentFriendID);
          if (pCutoff != NULL) {
            currentVector = currentCoords - friendCoords;
            currentDistance = currentVector.Mag();
            currentVector.SetZ(0.);
            currentDeltaValue = currentDistance / currentVector.Mag();
            if (currentDeltaValue < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              B2DEBUG(1000, " normeddist3d: segment approved!")
              //              scoreQQQ = scoreQQQ +  calcQQQscoreDeltas(pCutoff->getQuantiles(), currentDeltaValue, m_numOfQuantiles);
            } else {}
          } else {}
        } else { /*simpleSegmentQI++;*/ } // no test means that the segment always passes the exam ;)

        if (simpleSegmentQI < currentPass->activatedSegFinderTests) {
          B2DEBUG(200, "SegFINDER: segment discarded! simpleSegmentQI = " << simpleSegmentQI)
          discardedSegmentsCounter++;
          continue;
        }
        B2DEBUG(1000, " segment approved")
        VXDSegmentCell* pCell = new VXDSegmentCell(ownHits[currentHit],
                                                   allFriendHits[friendHit],
                                                   mainSecIter,
                                                   currentFriendIter);

        currentPass->activeCellList.push_back(pCell);
        currentPass->totalCellVector.push_back(pCell);
        int segPos = currentPass->totalCellVector.size() - 1;
        allFriendHits[friendHit]->addOuterCell(segPos);
        ownHits[currentHit]->addInnerCell(segPos);

        mainSecIter->second->addInnerSegmentCell(pCell);
        currentFriendIter->second->addOuterSegmentCell(pCell);
      } //iterating through all my friendHits
    }
  } // iterating through all aktive sectors - segFinder

  return discardedSegmentsCounter;
}




/** ***** neighbourFinder ***** **/
/// filters neighbouring segments in given pass and returns number of discarded segments
int VXDTFModule::neighbourFinder(CurrentPassData* currentPass)
{
  int NFdiscardedSegmentsCounter = 0;
  string outerLayerID, centerLayerID, innerLayerID, currentFriendID;
  TVector3 outerCoords, outerCenterCoords, centerCoords, innerCenterCoords, innerCoords, outerVector, centerVector, innerVector, outerTempVector;
  TVector3 innerTempVector, cpA/*central point of innerSegment*/, cpB/*central point of mediumSegment*/, nA/*normal vector of segment a*/, nB/*normal vector of segment b*/, intersectionAB;
  int simpleSegmentQI; // better than segmentApproved, but still digital (only min and max cutoff values), but could be weighed by order of relevance
  OperationSequenceOfActivatedSectors::const_iterator secSequenceIter;
  MapOfSectors::iterator mainSecIter;
  MapOfSectors::iterator friendSecIter;
  for (secSequenceIter = currentPass->sectorSequence.begin(); secSequenceIter != currentPass->sectorSequence.end(); ++secSequenceIter) {
    B2DEBUG(1000, "SectorSequence is named " << (*secSequenceIter).first);
    map<string, VXDSector*>::const_iterator mainSecIter = (*secSequenceIter).second;

    vector<VXDSegmentCell*> outerSegments = mainSecIter->second->getInnerSegmentCells(); // loading segments of sector
    int nOuterSegments = outerSegments.size();
    for (int thisOuterSegment = 0; thisOuterSegment < nOuterSegments; thisOuterSegment++) {
      outerCoords = outerSegments[thisOuterSegment]->getOuterHit()->getHitCoordinates();
      centerCoords = outerSegments[thisOuterSegment]->getInnerHit()->getHitCoordinates();
      outerVector = outerCoords - centerCoords;
      currentFriendID = outerSegments[thisOuterSegment]->getInnerHit()->getSectorName();
      centerLayerID = currentFriendID[0];

      //       vector<VXDSegmentCell*> innerSegments = outerSegments[thisOuterSegment]->getInnerHit()->getAttachedInnerCell();
      const vector<int>& innerSegments = outerSegments[thisOuterSegment]->getInnerHit()->getAttachedInnerCell();
      int nInnerSegments = innerSegments.size();

      for (int thisInnerSegment = 0; thisInnerSegment < nInnerSegments; thisInnerSegment++) {
        VXDSegmentCell* currentInnerSeg = currentPass->totalCellVector[innerSegments[thisInnerSegment]];
        innerCoords = currentInnerSeg->getInnerHit()->getHitCoordinates();
        innerVector = centerCoords - innerCoords;
        innerLayerID = currentInnerSeg->getInnerHit()->getSectorName()[0];

        simpleSegmentQI = 0;

        if (currentPass->angles3D.first == true) {
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameAngles3D, currentFriendID);
          if (pCutoff != NULL) {

            double innerSegLength = innerVector.Mag();
            double outerSegLength = outerVector.Mag();

            double aAngle3D = acos((innerVector.Dot(outerVector)) / (innerSegLength * outerSegLength));

            if (aAngle3D > pCutoff->getMinValue() && aAngle3D < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              B2DEBUG(1000, "angles3D: segment approved! ")
            } else {}/// if the current cutoff doesn't exist, then we simply asume that this segment is invalid -> not approved
          } else {}/// else segment not approved
        } else { /*simpleSegmentQI++;*/ } // no test means that the segment always passes the exam ;)

        if (currentPass->anglesRZ.first == true) { // max only!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameAnglesRZ, currentFriendID);
          if (pCutoff != NULL) {
            innerTempVector = innerVector;
            outerTempVector = outerVector;
            double tempRI = sqrt(innerTempVector[0] * innerTempVector[0] + innerTempVector[1] * innerTempVector[1]);
            TVector3 tempInnerSegVec(tempRI, innerTempVector[2], 0.);
            double innerSegLength = tempInnerSegVec.Mag();
            double tempRO = sqrt(outerTempVector[0] * outerTempVector[0] + outerTempVector[1] * outerTempVector[1]);
            TVector3 tempOuterSegVec(tempRO, outerTempVector[2], 0.);
            double outerSegLength = tempOuterSegVec.Mag();

            double aAngleRZ = acos((tempInnerSegVec.Dot(tempOuterSegVec)) / (innerSegLength * outerSegLength));

            if (aAngleRZ < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              B2DEBUG(1000, "anglesRZ: segment approved! ")
            } else {}
          } else {}
        } else { /*simpleSegmentQI++;*/ }

        if (currentPass->anglesXY.first == true) { // min & max!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameAnglesXY, currentFriendID);
          if (pCutoff != NULL) {
            innerTempVector = innerVector;
            outerTempVector = outerVector;
            innerTempVector.SetZ(0.);
            double innerSegLength = innerTempVector.Mag();
            outerTempVector.SetZ(0.);
            double outerSegLength = outerTempVector.Mag();

            double aAngleXY = acos((innerTempVector.Dot(outerTempVector)) / (innerSegLength * outerSegLength));

            if (aAngleXY > pCutoff->getMinValue() && aAngleXY < pCutoff->getMaxValue()) {
              simpleSegmentQI++;

              B2DEBUG(1000, "anglesXY: segment approved! ")
            } else {}
          } else {}
        } else { /*simpleSegmentQI++;*/ }

        if (currentPass->distance2IP.first == true or currentPass->pT.first == true) {
          Cutoff* pCutoffD2IP = mainSecIter->second->getCutoff(m_nameTRadius2IPDistance, currentFriendID);
          Cutoff* pCutoffPT = mainSecIter->second->getCutoff(m_namePT, currentFriendID);

          cpA = 0.5 * innerVector + innerCoords; // central point of innerSegment
          //            cpA.SetZ(0.);
          cpB = 0.5 * outerVector + centerCoords;// central point of outerSegment
          //            cpB.SetZ(0.);
          nA.SetXYZ(-cpA(1), cpA(0), 0.); //normal vector of segment a
          nB.SetXYZ(-cpB(1), cpB(0), 0.); //normal vector of segment b

          double muVal = (((cpA(1) - cpB(1)) / nB(1)) + (((cpB(0) - cpA(0)) / nA(0)) * nA(1) / nB(1))) / (1. - ((nB(0) / nA(0)) * (nA(1) / nB(1))));
          intersectionAB.SetX(cpB(0) + muVal * nB(0)); // x-coord of intersection point
          intersectionAB.SetY(cpB(1) + muVal * nB(1)); // y-coord of intersection point
          intersectionAB.SetZ(0.);

          double normedAB1 = (intersectionAB - innerCoords).Mag();
          double normedAB2 = (intersectionAB - centerCoords).Mag();
          double normedAB3 = (intersectionAB - outerCoords).Mag();

          double radiusInCmAB = (normedAB1 + normedAB2 + normedAB3) / 3.0; // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1

          if (currentPass->distance2IP.first == true and pCutoffD2IP != NULL) {
            double dist2IP = abs(intersectionAB.Mag() - radiusInCmAB); // distance between interaction point and intersection point.
            if (dist2IP < pCutoffD2IP->getMaxValue()) { // min only!
              simpleSegmentQI++;

              B2DEBUG(1000, "dist2IP: segment approved! ")
            } else {}
          }
          if (currentPass->pT.first == true and pCutoffPT != NULL) {
            double pTAB = 0.45 * radiusInCmAB * 0.01; // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100

            if (pTAB > pCutoffPT->getMinValue() and pTAB < pCutoffPT->getMaxValue()) { // min & max!!!
              simpleSegmentQI++;

              B2DEBUG(1000, "pT: segment approved! ")
            } else {}
          }

        } else { /*simpleSegmentQI++;*/ } // no test means that the segment always passes the exam ;)

        if (currentPass->distanceDeltaZ.first == true) { // min & max!!!
          Cutoff* pCutoff = mainSecIter->second->getCutoff(m_nameDistanceDeltaZ, currentFriendID);
          if (pCutoff != NULL) {
            innerTempVector = innerVector;
            outerTempVector = outerVector;

            double tempRI = sqrt(innerTempVector[0] * innerTempVector[0] + innerTempVector[1] * innerTempVector[1]);
            innerTempVector.SetXYZ(tempRI, innerTempVector[2], 0.);
            double innerSegLength = innerTempVector.Mag();
            double tempRO = sqrt(outerTempVector[0] * outerTempVector[0] + outerTempVector[1] * outerTempVector[1]);
            outerTempVector.SetXYZ(tempRO, outerTempVector[2], 0.);
            double outerSegLength = outerTempVector.Mag();

            double aDistanceDeltaZ = outerTempVector(1) / outerSegLength - innerTempVector(1) / innerSegLength;

            if (aDistanceDeltaZ > pCutoff->getMinValue() && aDistanceDeltaZ < pCutoff->getMaxValue()) { // it makes no sense to use a MinValue here... old: aDistanceDeltaZ > pCutoff->getMinValue() &&
              simpleSegmentQI++;

              B2DEBUG(1000, "distDeltaZ: segment approved! ")
            } else {}
          } else {}
        }

        if (simpleSegmentQI < currentPass->activatedNbFinderTests) { // cheaper alternative: segmentApproved
          B2DEBUG(200, "neighbourFINDER: segment discarded! simpleSegmentQI = " << simpleSegmentQI);
          continue;
        }

        outerSegments[thisOuterSegment]->addInnerNeighbour(currentInnerSeg);
        currentInnerSeg->addOuterNeighbour(outerSegments[thisOuterSegment]);

        if (centerLayerID < currentPass->highestAllowedLayer) {
          currentInnerSeg->setSeed(false);
        }
      } // iterating through inner segments
    } // iterating through outer segments
  } // iterating through all aktive sectors - friendFinder

  B2DEBUG(10, "dist2IP-test is defect, repair that!")

  //filtering lost segments (those without neighbours left):
  int countedSegments = 0;

  BOOST_FOREACH(VXDSegmentCell * currentSeg, currentPass->activeCellList) {
    if (currentSeg->getInnerNeighbours().size() == 0 && currentSeg->getOuterNeighbours().size() == 0) {
      currentSeg->setActivationState(false);
      NFdiscardedSegmentsCounter++;
    } else {
      currentSeg->copyNeighbourList(); /// IMPORTANT, without this step, no TCs can be found since all neighbours of each cell are reased from current list
      countedSegments++;
    }
  }
//  list<VXDSegmentCell*>::const_iterator currentSeg;
//  for (currentSeg = currentPass->activeCellList.begin(); currentSeg != currentPass->activeCellList.end(); ++currentSeg) {
//
//    if ((*currentSeg)->getInnerNeighbours().size() == 0 && (*currentSeg)->getOuterNeighbours().size() == 0) {
//      (*currentSeg)->setActivationState(false);
//      NFdiscardedSegmentsCounter++;
//    } else {
//      (*currentSeg)->copyNeighbourList(); /// IMPORTANT, without this step, no TCs can be found since all neighbours of each cell are reased from current list
//      countedSegments++;
//    }
//  }
  B2DEBUG(10, "neighbourFinder - exporting " << countedSegments << " segments into File");

  return NFdiscardedSegmentsCounter;
}



/** ***** cellular automaton ***** **/
/// uses attribute "state" to find rows of compatible neighbours. State indicates length of row. This allows fast Track candidate collection
void VXDTFModule::cellularAutomaton(CurrentPassData* currentPass)
{
  int activeCells = 1;
  int caRound = 0;
  int goodNeighbours, countedSegments, segState;
  ActiveSegmentsOfEvent::iterator currentSeg;

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
      B2DEBUG(1000, "good cell upgraded!")
    }

    B2DEBUG(10, "CA: " << caRound << ". round - " << activeCells << " living cells remaining.");
  }

  //segmentOutPut:
  //  int segState;
  countedSegments = 0;
  BOOST_FOREACH(VXDSegmentCell * currentSeg, currentPass->activeCellList) {
    segState = currentSeg->getState();
    B2DEBUG(200, "Post CA - Current state of cell: " << segState);
    if (currentSeg->getInnerNeighbours().size() == 0 && currentSeg->getOuterNeighbours().size() == 0) { continue; }
    countedSegments++;
  }
  B2DEBUG(10, "CA - " << countedSegments << " segments have at least one friend");
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
    B2DEBUG(1000, "entering sector " << (*secSequenceIter).first);
    MapOfSectors::const_iterator sectorOfSequIter = (*secSequenceIter).second;

    mayorLayerID = lexical_cast<int>(sectorOfSequIter->second->getSecID()[0]);
    if (mayorLayerID < currentPass->minLayer) { continue; }

    vector<VXDSegmentCell*> segmentsOfSector = sectorOfSequIter->second->getInnerSegmentCells(); // loading segments of sector
    nSegmentsInSector = segmentsOfSector.size();

    B2DEBUG(1000, "sector " << sectorOfSequIter->second->getSecID() << " has got " << nSegmentsInSector << " segments in its area");

    for (int thisSegmentInSector = 0; thisSegmentInSector < nSegmentsInSector; thisSegmentInSector++) {
      if (segmentsOfSector[thisSegmentInSector]->isSeed() == false || segmentsOfSector[thisSegmentInSector]->getState() < tccMinState) {
        if (segmentsOfSector[thisSegmentInSector]->isSeed() == false) {
          B2DEBUG(1000, "current segment is no seed!");
        } else { B2DEBUG(1000, "current segment has no sufficent cellstate..."); }
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
void VXDTFModule::tcFilter(CurrentPassData* currentPass, int passNumber)
{
  TCsOfEvent::iterator currentTC;
  TVector3 hitA, hitB, hitC, segAB, segBC, cpAB, cpBC, nAB, nBC, intersection;
  double muVal, normedI1, normedI2, normedI3, radiusInCm, aCutOff, pT, aDeltapT;
  TCsOfEvent tempTCList = currentPass->tcVector;
  int numTCsafterTCC = tempTCList.size();
  vector<TCsOfEvent::iterator> goodTCIndices;
  goodTCIndices.clear();
  int tcCtr = 0;
  for (currentTC = currentPass->tcVector.begin(); currentTC != currentPass->tcVector.end(); ++currentTC) { // need iterators for later use
    vector<VXDTFHit*> currentHits = (*currentTC)->getHits(); /// IMPORTANT: currentHits[0] is outermost hit!
    int numOfCurrentHits = currentHits.size();

    if (numOfCurrentHits == 3) {  /// in this case, dPt and zigzag filtering does not make sense
      B2DEBUG(20, " tc " << tcCtr << " got " << numOfCurrentHits << " hits and therefore won't be checked by TCC");
      tcCtr++;
      continue;
    } else if (numOfCurrentHits < 3) {
      B2DEBUG(20, " tc " << tcCtr << " got " << numOfCurrentHits << " hits and therefore will be deleted");
      tcCtr++;
      (*currentTC)->setCondition(false);
      continue;
    }

    stringstream secNameOutput;
    secNameOutput << endl << " tc " << tcCtr << " got " << numOfCurrentHits << " hits and the following secIDs: ";
    BOOST_FOREACH(VXDTFHit * currentHit, currentHits) {
      string aSecName = currentHit->getSectorName();
      secNameOutput << aSecName << " ";
    }
    B2DEBUG(20, " " << secNameOutput.str() << " and " <<  numOfCurrentHits << " hits");
    vector<double> pTValues;
    vector<string> secNames;
    list<int> charge;
    int a = 0, b = 1, c = 2, baddpTCtr = 0;
    for (; c <  numOfCurrentHits;) {
      hitA = currentHits[a]->getHitCoordinates(); hitA.SetZ(0.);
      hitB = currentHits[b]->getHitCoordinates(); hitB.SetZ(0.);
      hitC = currentHits[c]->getHitCoordinates(); hitC.SetZ(0.);
      segAB = hitB - hitA;
//      B2DEBUG(100, "segAB: ");segAB.Print();
      segBC = hitC - hitB;
//      B2DEBUG(100, "segBC: ");segBC.Print();
      cpAB = 0.5 * segAB + hitA; // central point of innerSegment
//      B2DEBUG(100, "cpAB: ");cpAB.Print();
      cpBC = 0.5 * segBC + hitB;// central point of mediumSegment
//      B2DEBUG(100, "cpBC: ");cpBC.Print();
      nAB = segAB.Orthogonal();
//      B2DEBUG(100, "nAB: ");nAB.Print();
      nBC = segBC.Orthogonal();
//      B2DEBUG(100, "nBC: ");nBC.Print();
      int signKappaABC = sign(nBC * segAB);
      B2DEBUG(100, " signKappaABC: " << signKappaABC << " scalarABC: " << (nBC * segAB) << ", nAB: & segAB: ");
      charge.push_back(signKappaABC);

      if (currentPass->deltaPt.first == false) { a++; b++; c++; continue; }   // track candidate allways passes the test, when current filter is deactivated

      muVal = (((cpAB(1) - cpBC(1)) / nBC(1)) + (((cpBC(0) - cpAB(0)) / nAB(0)) * nAB(1) / nBC(1))) / (1. - ((nBC(0) / nAB(0)) * (nAB(1) / nBC(1))));
      intersection.SetX(cpBC(0) + muVal * nBC(0)); // x-coord of intersection point
      intersection.SetY(cpBC(1) + muVal * nBC(1)); // y-coord of intersection point
      intersection.SetZ(0.);
      normedI1 = (intersection - hitA).Mag();
      normedI2 = (intersection - hitB).Mag();
      normedI3 = (intersection - hitC).Mag();
      radiusInCm = (normedI1 + normedI2 + normedI3) / 3.0; // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
      pT = 0.45 * radiusInCm * 0.01; // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100 = 0.45*r*0.01
      pTValues.push_back(pT);
      string aSecName = currentHits[a]->getSectorName();
      B2DEBUG(100, "currentHit got secName: " << aSecName);
      secNames.push_back(aSecName);

      if (c <  numOfCurrentHits - 1 && a > 0) { //
        Cutoff* pCutoff = currentPass->sectorMap.find(secNames[a - 1])->second->getCutoff(m_nameDeltapT, secNames[a]);
        if (pCutoff == NULL) {
          B2WARNING("event " << m_eventCounter << ": TCC_dpT-filter: no cutOff found! a: " << a << ", b: " << b << ", c: " << c << ", sizeTC: " << int(currentHits.size()))
          a++; b++; c++;
          continue;
        }
        aDeltapT = abs(abs(pTValues[a - 1]) - abs(pTValues[a]));
        aCutOff = pCutoff->getMaxValue();
        B2DEBUG(100, " aDeltapT: " << aDeltapT << ", aCutOff: " << aCutOff)
        if (aDeltapT > aCutOff) {
          baddpTCtr++;
          B2DEBUG(100, " tccdPT-filter did well!, event " << m_eventCounter << ", dpT Value: " << aDeltapT << ", cutoff: " << aCutOff)
        }
      }
      a++; b++; c++;
    }
    (*currentTC)->setPassIndex(passNumber);
    charge.sort(); charge.unique();
    int numOfCharges = charge.size();
    if (numOfCharges == 1 && baddpTCtr == 0) {
      goodTCIndices.push_back(currentTC);
      B2DEBUG(20, " TCC filter approved TC " << tcCtr);
      m_TESTERapprovedByTCC++;
    } else {
      B2DEBUG(20, "TCC filter: tc " << tcCtr << " rejected! charge.size() " << charge.size() << ", baddpTCtr: " << baddpTCtr);
      (*currentTC)->setCondition(false);
      if (numOfCharges != 1) {
        m_TESTERtriggeredZigZag++;
      } else {
        m_TESTERtriggeredDpT++;
      }
    }
    tcCtr++;
  }

  ///deleting TCs which did not survive the tcFilter-step and remove virtual hits
  int goodOnes = goodTCIndices.size(); // number of tc's after TCC-Filter
  if (goodOnes not_eq numTCsafterTCC) {
    currentPass->tcVector.clear();
    BOOST_FOREACH(TCsOfEvent::iterator goodTCIndex, goodTCIndices) {
      (*goodTCIndex)->removeVirtualHit();
      currentPass->tcVector.push_back((*goodTCIndex));
    }
  } else {
    BOOST_FOREACH(TCsOfEvent::iterator goodTCIndex, goodTCIndices) {
      (*goodTCIndex)->removeVirtualHit();
    }
  }

  ///Testing purposes:
  int numTC = 0;
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, currentPass->tcVector) {
    vector<VXDTFHit*> currentHits = currentTC->getHits();
    int numOfHits = currentHits.size();
    stringstream secNameOutput;
    secNameOutput << endl << "after filtering virtual entries: tc " << numTC << " got " << numOfHits << " hits and the following secIDs: ";
    BOOST_FOREACH(VXDTFHit * currentHit, currentHits) {
      string aSecName = currentHit->getSectorName();
      secNameOutput << aSecName << " ";
    }
    B2DEBUG(20, " " << secNameOutput.str() << " and " << numOfHits << " hits");
    numTC++;
  }
  /// testing purposes end

  m_TESTERcountTotalTCsAfterTCCFilter += goodOnes;
  B2DEBUG(10, "TCC-filter: tcList had " << numTCsafterTCC << " TCs. Of these, " << goodOnes << " TC's were accepted as good ones by the TCC-Filter")
}



void VXDTFModule::calcInitialValues4TCs(TCsOfEvent& tcVector)
{
  vector<VXDTFHit*> currentHits;
  TVector3 hitA, hitB, hitC, hitA_T, hitB_T, hitC_T; // those with _T are the hits of the transverlal plane
  TVector3 intersection, radialVector, pTVector, pVector; //coords of center of projected circle of trajectory & vector pointing from center to innermost hit
  TVector3 segAB, segBC, cpAB, cpBC, nAB, nBC, zVector;
  zVector.SetZ(1.);
  int numOfCurrentHits, signCurvature, pdGCode;
  double muVal, radiusInCm, pT, theta, pZ, preFactor; // needed for dPt calculation
  BOOST_FOREACH(VXDTFTrackCandidate * aTC, tcVector) {

    if (aTC->getCondition() == false) { continue; }
    currentHits = aTC->getHits();
    numOfCurrentHits = currentHits.size();
    hitA = currentHits[numOfCurrentHits - 1]->getHitCoordinates(); // innermost hit and initial value for GFTrackCandidate
    hitA_T = hitA; hitA_T.SetZ(0.);
    hitB = currentHits[numOfCurrentHits - 2]->getHitCoordinates();
    hitB_T = hitB; hitB_T.SetZ(0.);
    hitC = currentHits[numOfCurrentHits - 3]->getHitCoordinates();
    hitC_T = hitC; hitC_T.SetZ(0.);

    segAB = hitB - hitA;
    theta = segAB.Angle(zVector);
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
    radialVector = (intersection - hitA);
    radiusInCm = radialVector.Mag(); // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
    pT = 0.45 * radiusInCm * 0.01; // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100 = 0.45*r*0.01 length of pT
    pZ = pT * tan(theta);
    preFactor = pT / (radialVector.Mag());
    pTVector = preFactor * radialVector.Orthogonal() ;

    if ((hitA + pTVector).Mag() < hitA.Mag()) { pTVector = pTVector * -1; }

    pVector = pTVector;
    pVector.SetZ(pZ);
    double pValue = pVector.Mag();
//     if (pValue < 0.035 || pValue > 5.0) { aTC->setCondition(false); continue; }

    // the sign of curvature determines the charge of the particle, negative sign for curvature means positively charged particle. The signFactor is needed since the sign of PDG-codes are not defined by their charge but by being a particle or an antiparticle
    pdGCode = signCurvature * m_PARAMpdGCode * m_chargeSignFactor;

    aTC->setInitialValue(hitA, pVector, pdGCode);
    B2DEBUG(10, " TC has got momentum of " << pValue << "GeV and estimated pdgCode " << pdGCode);
  }
}



void VXDTFModule::calcQIbyLength(TCsOfEvent& tcVector, PassSetupVector& passSetups)
{
  /// setting quality indices and smear result if chosen
  double firstValue = 0.0, rngValue = 0.0, setupWeigh = 0.0, maxLength = 0.0;
  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, tcVector) {
    rngValue = gRandom->Gaus(m_PARAMsmearMean, m_PARAMsmearSigma);
    setupWeigh = passSetups[currentTC->getPassIndex()]->setupWeigh;
    maxLength =  1.0 / double(passSetups[currentTC->getPassIndex()]->numTotalLayers * 2.0);
    if (rngValue < -0.4) { rngValue = -0.4; } else if (rngValue > 0.4) { rngValue = 0.4; }

    if (m_PARAMqiSmear == true) {
      firstValue = rngValue + 1.0 - setupWeigh;
    } else {
      firstValue = 1.0 - setupWeigh;
    }
    B2DEBUG(50, "setQQQ gets the following values: setupWeigh: " << setupWeigh << ", first value: " << firstValue << ", QQQScore: " << sqrt(firstValue * 0.5))
    currentTC->setQQQ(firstValue, 2.0);  // resulting QI = 0.3-0.7
    currentTC->setTrackQuality((currentTC->getQQQ() * double(currentTC->getHits().size()*maxLength)));
  }

}

vector <GFTrackCand> VXDTFModule::calcQIbyKalman(TCsOfEvent& tcVector, StoreArray<PXDCluster>& pxdClusters, StoreArray<SVDCluster>& svdClusters)
{
  /// produce GFTrackCands for each currently living TC and calculate real kalman-QI's
  GFKalman kalmanFilter;
  kalmanFilter.setNumIterations(1); // TODO: hardcoded values should be set as steering file-parameter
  kalmanFilter.setBlowUpFactor(500.0); // TODO: hardcoded values should be set as steering file-parameter
  vector <GFTrackCand> temporalTCs;

  BOOST_FOREACH(VXDTFTrackCandidate * currentTC, tcVector) {
    if (currentTC->getCondition() == false) { continue; }
    GFTrackCand aGFTC = generateGFTrackCand(currentTC);
    GFTrackCand* aGFTCPtr = &aGFTC;

    RKTrackRep* trackRep = new RKTrackRep(aGFTCPtr);
    trackRep->setPropDir(1); // 1 = start moving outwards (seed has to be innermost hit), -1 start moving inwards (seed has to be outermost hit)

    GFTrack track(trackRep);
    track.setSmoothing(false, false);

    GFRecoHitFactory factory;

    if (int(currentTC->getPXDHitIndices().size()) not_eq 0) {
      GFRecoHitProducer <PXDCluster, PXDRecoHit> * pxdClusterProducer;
      pxdClusterProducer = new GFRecoHitProducer <PXDCluster, PXDRecoHit> (&*pxdClusters);
      factory.addProducer(0, pxdClusterProducer);
    }
    if (int(currentTC->getSVDHitIndices().size()) not_eq 0) {
      GFRecoHitProducer <SVDCluster, SVDRecoHit> * svdClusterProducer;
      svdClusterProducer = new GFRecoHitProducer <SVDCluster, SVDRecoHit> (&*svdClusters);
      factory.addProducer(1, svdClusterProducer);
    }

    vector<GFAbsRecoHit*> factoryHits = factory.createMany(*aGFTCPtr); // use the factory to create RecoHits for all Hits stored in the track candidate
    track.addHitVector(factoryHits);
    track.setCandidate(*aGFTCPtr);

    kalmanFilter.processTrack(&track);

    if (trackRep->getStatusFlag() == 0) {
      B2DEBUG(10, "calcQI4TC suceeded: calculated kalmanQI: " << track.getChiSqu() << " with NDF: " << track.getNDF() << ", p-value: " << track.getCardinalRep()->getPVal())
      currentTC->setTrackQuality(track.getCardinalRep()->getPVal());
      m_TESTERgoodFitsCtr++;
    } else {
      B2DEBUG(10, "calcQI4TC failed...")
      m_TESTERbadFitsCtr++;
      currentTC->setTrackQuality(0.);

      if (m_PARAMstoreBrokenQI == false) {
        currentTC->setCondition(false); // do not store TCs with failed fits if param-flag is set to false
      }
    }


  }

  return temporalTCs;
}

GFTrackCand VXDTFModule::generateGFTrackCand(VXDTFTrackCandidate* currentTC)
{
  GFTrackCand newGFTrackCand;

  TVector3 posIn = currentTC->getInitialCoordinates();
  TVector3 momIn = currentTC->getInitialMomentum();
  TMatrixD stateSeed(6, 1); //(x,y,z,px,py,pz)
  TMatrixD covSeed(6, 6);
  int pdgCode = currentTC->getPDGCode();
  vector<int> pxdHits = currentTC->getPXDHitIndices();
  vector<int> svdHits = currentTC->getSVDHitIndices();

  stateSeed(0, 0) = posIn[0]; stateSeed(1, 0) = posIn[1]; stateSeed(2, 0) = posIn[2];
  stateSeed(3, 0) = momIn[0]; stateSeed(4, 0) = momIn[1]; stateSeed(5, 0) = momIn[2];
  covSeed.Zero();
  covSeed(0, 0) = 0.1 * 0.1; covSeed(1, 1) = 0.1 * 0.1; covSeed(2, 2) = 0.2 * 0.2;
  covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.2 * 0.2;
  B2DEBUG(10, "generating GFTrackCandidate: posIn.Mag(): " << posIn.Mag() << ", momIn.Mag(): " << momIn.Mag() << ", pdgCode: " << pdgCode);

//  newGFTrackCand.setComplTrackSeed(posIn, momIn, pdgCode, TVector3(0.1, 0.1, 0.1), TVector3(0.1, 0.1, 0.2)); /// old method

  newGFTrackCand.set6DSeedAndPdgCode(stateSeed, pdgCode, covSeed);


  BOOST_REVERSE_FOREACH(int hitIndex, pxdHits) {  // order of hits within VXDTFTrackCandidate: outer->inner hits. GFTrackCand: inner->outer hits
    newGFTrackCand.addHit(Const::PXD, hitIndex); // 0 means PXD
  }
  BOOST_REVERSE_FOREACH(int hitIndex, svdHits) {  // order of hits within VXDTFTrackCandidate: outer->inner hits. GFTrackCand: inner->outer hits
    newGFTrackCand.addHit(Const::SVD, hitIndex); // 1 means SVD
  }

  return newGFTrackCand;
}

void VXDTFModule::cleanOverlappingSet(TCsOfEvent& tcVector)
{
  int numOfTCs = tcVector.size(), numOfIHits = 0, numOfJHits = 0, numOfMergedHits = 0, killedTCs = 0;
  list<int> ihitIDs, jhitIDs, mergedHitIDs;

  for (int i = 0; i < numOfTCs; ++i) {
    if (tcVector[i]->getCondition() == false) { continue; }
    ihitIDs = tcVector[i]->getHopfieldHitIndices();
    numOfIHits = ihitIDs.size();
    for (int j = i + 1; j < numOfTCs; ++j) {
      if (tcVector[j]->getCondition() == false) { continue; }
      jhitIDs = tcVector[j]->getHopfieldHitIndices();
      numOfJHits = jhitIDs.size();
      mergedHitIDs = ihitIDs;
      mergedHitIDs.insert(mergedHitIDs.end(), jhitIDs.begin(), jhitIDs.end());
      mergedHitIDs.sort();
      mergedHitIDs.unique();
      numOfMergedHits = mergedHitIDs.size();

      if (numOfMergedHits <= numOfIHits || numOfMergedHits <= numOfJHits) {
        killedTCs++;
        if (tcVector[i]->getTrackQuality() >= tcVector[j]->getTrackQuality()) {
          tcVector[j]->setCondition(false);
        } else {
          tcVector[i]->setCondition(false); // although this one already died, we have to continue since there could be other TCs sharing all their hits with him
        }
      }
    }
  }
  m_TESTERfilteredOverlapsQI += killedTCs;
  B2DEBUG(10, "VXDTFModule event " << m_eventCounter << ": so far, " << m_TESTERfilteredOverlapsQI << " TCs have been killed by funcCleanOverlappingSet...")
  if (killedTCs not_eq 0) {
    TCsOfEvent newOverlappingTCs;
    BOOST_FOREACH(VXDTFTrackCandidate * aTC, tcVector) {
      if (aTC->getCondition() == false) { continue; }
      newOverlappingTCs.push_back(aTC);
    }
    B2DEBUG(10, "within funcCleanOverlappingSet: tcVector.size(): " << tcVector.size() << ", newOverlappingTCs.size(): " << newOverlappingTCs.size())
    tcVector = newOverlappingTCs;
    m_TESTERfilteredOverlapsQICtr++;
  } else { m_TESTERNotFilteredOverlapsQI++; B2DEBUG(10, "VXDTFModule event " << m_eventCounter << ": no TC is subset of other TC")}

}
