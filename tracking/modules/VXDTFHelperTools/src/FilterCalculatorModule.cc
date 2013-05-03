/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2012 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler                                      *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include "tracking/modules/VXDTFHelperTools/FilterCalculatorModule.h"

#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Const.h>
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include "tracking/vxdCaTracking/FilterID.h"

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <math.h>
#include <TRandom.h>
using namespace std;
using namespace Belle2;



//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FilterCalculator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FilterCalculatorModule::FilterCalculatorModule() : Module()
{

  //Set module properties
  setDescription("This module calculates the relations of sectors and exports sector dependent filtervalues. Size of sectors and type of filters can be set by steering file. Only one track per event, needs many events for usefull outcome.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  std::vector<double> defaultConfigU;
  std::vector<double> defaultConfigV;
  defaultConfigU.push_back(0.0);
  defaultConfigU.push_back(0.5);
  defaultConfigU.push_back(1.0);
  defaultConfigV.push_back(0.0);
  defaultConfigV.push_back(0.33);
  defaultConfigV.push_back(0.67);
  defaultConfigV.push_back(1.0);
  std::vector<double> defaultpTcuts;
  defaultpTcuts.push_back(0.035);
  std::vector<std::string> defaultSecNames;
  defaultSecNames.push_back("fullRange");

  addParam("exportSectorCoords", m_PARAMexportSectorCoords, "set true if you want to export coordinates of the sectors too", bool(true));
  addParam("sectorSetupFileName", m_PARAMsectorSetupFileName, "enables personal sector setups (can be loaded by the vxd track finder)", string("genericSectorMap"));
  addParam("tracksPerEvent", m_PARAMtracksPerEvent, "defines the number of exported tracks per event (should not be higher than real number of tracks per event)", int(1));
  addParam("useEvtgen", m_PARAMuseEvtgen, "warning, overrides tracksPerEvent if True! set true if evtGen is used for filtergeneration, set false for pGun", bool(false));
  addParam("pTcuts", m_PARAMpTcuts, "minimal number of entries is 1. first entry defines lower threshold for pT in GeV/c. Each further value defines a momentum range for a new sectorMap", defaultpTcuts);
  addParam("detectorType", m_PARAMdetectorType, "defines which detector type has to be exported. VXD: -1, PXD: 1, SVD: 2", int(2));
//  addParam("percentageOfFMSectorFriends", m_percentageOfFMSectorFriends, "allows filtering extreme rare sector combinations by setting the value between 0.0 and 1.0", float(0.999));
  addParam("maxXYvertexDistance", m_PARAMmaxXYvertexDistance, "allows to abort particles having their production vertex too far away from the origin (xy-plane)", double(0.5));
  addParam("maxZvertexDistance", m_PARAMmaxZvertexDistance, "allows to abort particles having their production vertex too far away from the origin (z-dist)", double(2.0));

  addParam("sectorConfigU", m_PARAMsectorConfigU, "allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigU);
  addParam("sectorConfigV", m_PARAMsectorConfigV, "allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigV);
  addParam("trackErrorTracks", m_PARAMtrackErrorTracks, "track tracks which cause strange results", bool(false));
  addParam("highestAllowedLayer", m_PARAMhighestAllowedLayer, "set value below 6 if you want to exclude outer layers (standard is 6)", int(6));

  addParam("uniSigma", m_PARAMuniSigma, "standard value is 1/sqrt(12). Change this value for sharper or more diffuse hits (coupled with 'smearHits')", double(1 / sqrt(12)));
  addParam("smearHits", m_PARAMsmearHits, "if True, hits get smeared by pitch/uniSigma", bool(false));
  addParam("noCurler", m_PARAMnoCurler, "if True, curling tracks get reduced to first part of trajectory before starting to curl back", bool(false));
  addParam("minTrackletLength", m_PARAMminTrackletLength, "defines the number of hits needed to be stored as a tracklet", int(3));

  //2 hit filters:
  addParam("logDistanceXY", m_PARAMlogDistanceXY, "set 'true' if you want to log distances (XY) between trackHits", bool(true));
  addParam("logDistanceZ", m_PARAMlogDistanceZ, "set 'true' if you want to log distances (Z) between trackHits", bool(true));
  addParam("logDistance3D", m_PARAMlogDistance3D, "set 'true' if you want to log distances (3D) between trackHits", bool(true));
  addParam("logNormedDistance3D", m_PARAMlogNormedDistance3D, " set 'true' to log improved 3D distance between trackHits", bool(true));
  addParam("logSlopeRZ", m_PARAMlogSlopeRZ, " set 'true' to log slope in r-z-plane for line of 2 trackHits", bool(true));

  //3 hit filters;
  addParam("logAngles3D", m_PARAMlogAngles3D, "set 'true' if you want to log angles3D between segments", bool(true));
  addParam("logAnglesXY", m_PARAMlogAnglesXY, "set 'true' if you want to log anglesXY between segments", bool(true));
  addParam("logAnglesRZ", m_PARAMlogAnglesRZ, "set 'true' if you want to log anglesRZ between segments", bool(true));
  addParam("logDeltaSlopeRZ", m_PARAMlogDeltaSlopeRZ, "set 'true' if you want to log delta slopes in r-z-plane between segments", bool(true));
  addParam("logPt", m_PARAMlogPt, "set 'true' if you want to log Pt between segments", bool(true));
  addParam("logTRadius2IPDistance", m_PARAMlogTRadiustoIPDistance, " set 'true' to log the difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP", bool(true));
  addParam("logHelixFit", m_PARAMlogHelixFit, "set 'true' if you want to log delta ((helix-circle-segment-angle) / deltaZ)", bool(true));

  // 2+1 hit high occupancy filters:
  addParam("logAnglesHighOccupancy3D", m_PARAMlogAnglesHighOccupancy3D, "set 'true' if you want to log anglesHighOccupancy3D between segments", bool(true));
  addParam("logAnglesHighOccupancyXY", m_PARAMlogAnglesHighOccupancyXY, "set 'true' if you want to log anglesHighOccupancyXY between segments", bool(true));
  addParam("logAnglesHighOccupancyRZ", m_PARAMlogAnglesHighOccupancyRZ, "set 'true' if you want to log anglesHighOccupancyRZ between segments", bool(true));
  addParam("logDeltaSlopeHighOccupancyRZ", m_PARAMlogDeltaSlopeHighOccupancyRZ, "set 'true' if you want to log HighOccupancy delta slopes in r-z-plane between segments", bool(true));
  addParam("logPtHighOccupancy", m_PARAMlogPtHighOccupancy, "set 'true' if you want to log PtHighOccupancy between segments", bool(true));
  addParam("logTRadiusHighOccupancy2IPDistance", m_PARAMlogTRadiusHighOccupancytoIPDistance, " set 'true' to log the HighOccupancy difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP", bool(true));
  addParam("logHelixHighOccupancyFit", m_PARAMlogHelixHighOccupancyFit, "set 'true' if you want to log HighOccupancy delta ((helix-circle-segment-angle) / deltaZ)", bool(true));

  //4 hit filters;
  addParam("logDeltaPt", m_PARAMlogDeltaPt, "set 'true' if you want to log delta Pt between segments", bool(true));
  addParam("logDeltaDistCircleCenter", m_PARAMlogDeltaDistCircleCenter, "set 'true' to compare the distance of the calculated centers of track circles", bool(true));

  //3+1 hit high occupancy filters:
  addParam("logDeltaPtHighOccupancy", m_PARAMlogDeltaPtHighOccupancy, "set 'true' if you want to log delta Pt HighOccupancy between segments", bool(true));
  addParam("logDeltaDistCircleCenterHighOccupancy", m_PARAMlogDeltaDistCircleCenterHighOccupancy, "set 'true' to compare the HighOccupancy distance of the calculated centers of track circles", bool(true));

}


FilterCalculatorModule::~FilterCalculatorModule()
{
}


void FilterCalculatorModule::initialize()
{

  int numCuts = m_PARAMpTcuts.size();

  for (int i = 0; i < numCuts - 1; ++i) {
    string secMapName = (boost::format("%1%_%2%to%3%Gev") % m_PARAMsectorSetupFileName % m_PARAMpTcuts[i] % m_PARAMpTcuts[i + 1]).str();
    for (int i = 0; i < int(secMapName.length()); ++i) {
      switch (secMapName[i]) {
        case '_':
          secMapName[i] = '-';
        case '.':
          secMapName[i] = '-';
      }
    }
    m_PARAMsecMapNames.push_back(secMapName);
    B2WARNING("FilterCalculatorModule-start: will use " << secMapName << " for storing cutoffs")
  }
  string secMapName = (boost::format("%1%-%2%toXGev") % m_PARAMsectorSetupFileName % m_PARAMpTcuts[numCuts - 1]).str();
  for (int i = 0; i < int(secMapName.length()); ++i) {
    switch (secMapName[i]) {
      case '_':
        secMapName[i] = '-';
      case '.':
        secMapName[i] = '-';
    }
  }
  B2WARNING("FilterCalculatorModule-start: will use " << secMapName << " for storing cutoffs")
  m_PARAMsecMapNames.push_back(secMapName);


  m_trackletMomentumCounter.resize(numCuts, 0);

  for (int i = 0; i < int(m_PARAMpTcuts.size());) {
    MapOfSectors* secMap = new MapOfSectors;
    secMap->clear();
    m_sectorMaps.push_back(secMap);
    ++i;
  }

  m_sectorMap.clear();
  B2INFO("~~~~~~~~~~~FilterCalculator - initialize ~~~~~~~~~~")
  B2INFO("chosen detectorType: " << m_PARAMdetectorType << ", highestAllowedLayer: " << m_PARAMhighestAllowedLayer << ", smearHits: " << m_PARAMsmearHits << ", noCurler: " << m_PARAMnoCurler << ", uniSigma: " << m_PARAMuniSigma)

  StoreArray<MCParticle>::required();
  StoreArray<PXDTrueHit>::required();
  StoreArray<SVDTrueHit>::required();
}


void FilterCalculatorModule::beginRun()
{
  B2INFO("~~~~~~~~~~~FilterCalculator - beginRun ~~~~~~~~~~")
  m_eventCounter = 0;
  m_badHitsCounter = 0;
  m_totalLocalCoordValue = 0;
  m_totalGlobalCoordValue = 0;
  m_totalHitCounter = 0;
  m_longTrackCounter = 0;
  m_numOfLayers = 0;
  m_badFilterValueCtr = 0;
  if (m_PARAMdetectorType == -1) {
    m_numOfLayers = 6;
  } else if (m_PARAMdetectorType == 0) {
    m_PARAMdetectorType = Const::PXD;
    m_numOfLayers = 2;
  } else { m_numOfLayers = 4; m_PARAMdetectorType = Const::SVD; }
  for (int i = 0; i < m_numOfLayers * 2; i++) {
    m_trackletLengthCounter.push_back(0);
  }
  bool bleh = false;
  if (m_PARAMdetectorType == Const::SVD) { bleh = true; }
  B2INFO("chosen detectorType: " << m_PARAMdetectorType << ", Const::PXD: " << Const::PXD << ", Const::SVD: " << Const::SVD << ", detectorType = SVD: " << bleh << ", uniSigma: " << m_PARAMuniSigma)
}


void FilterCalculatorModule::event()
{
  double pMaxInMeV = 10000., pMinInMeV = 10.;


  VXD::GeoCache& geometry = VXD::GeoCache::getInstance();

  //get the data
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  m_eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(1, "~~~~~~~~~~~FilterCalculator - event " << m_eventCounter << " ~~~~~~~~~~")

  //simulated particles and hits
  StoreArray<MCParticle> aMcParticleArray("");
  int numOfMcParticles = aMcParticleArray.getEntries();
  B2DEBUG(100, "FilterCalculator: total Number of MCParticles: " << numOfMcParticles);

  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  int numOfPxdTrueHits = aPxdTrueHitArray.getEntries();
  B2DEBUG(100, "FilterCalculator: total Number of PXDTrueHits: " << numOfPxdTrueHits);
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  int numOfSvdTrueHits = aSvdTrueHitArray.getEntries();
  B2DEBUG(100, "FilterCalculator: total Number of SVDTrueHits: " << numOfSvdTrueHits);

  if (numOfMcParticles == 0) {
    B2FATAL("event" << m_eventCounter << ": there is no MCParticle!")
  } else if (numOfPxdTrueHits == 0 && m_PARAMdetectorType not_eq Const::SVD) {
    B2WARNING("event" << m_eventCounter << ": there are no PXDTrueHits")
    return;
  } else if (numOfSvdTrueHits == 0 && m_PARAMdetectorType not_eq Const::PXD) {
    B2WARNING("event" << m_eventCounter << ": there are no SVDTrueHits")
    return;
  }

  RelationIndex<MCParticle, PXDTrueHit> relationMcPxdTrueHit;
//   int sizeOfRelMcPxdTrueHit = relationMcPxdTrueHit.size();
  RelationIndex<MCParticle, SVDTrueHit> relationMcSvdTrueHit;
//  int sizeOfRelMcSvdTrueHit = relationMcSvdTrueHit.size();

  if (m_PARAMtracksPerEvent > numOfMcParticles) {
    B2ERROR("FilterCalculatorModule: input parameter wrong (tracks per event) - reset to maximum value")
    m_PARAMtracksPerEvent = numOfMcParticles;
  }

  B2DEBUG(1, "FilterCalculatorModule, event " << m_eventCounter << ": size of arrays, SvdTrueHit: " << numOfSvdTrueHits << ", mcPart: " << numOfMcParticles << ", PxDTrueHits: " << numOfPxdTrueHits << endl);


//  bool firstrun = true;
  TVector3 oldpGlobal;
  TVector3 oldhitGlobal;

  /** collecting all hits of primary particles in a track for each particle and sorts them
  *(first entry is first hit in detector, last hit is last one before leaving the detector forever)
  **/
  TVector3 hitGlobal, hitLocal, pGlobal, pLocal;

  vector<VXDTrack> tracksOfEvent;
  vector<VXDTrack> trackletsOfEvent; // tracks cut into bite-sized pieces for the filtering part

  int trackThreshold;
  if (m_PARAMuseEvtgen == true) {
    trackThreshold = numOfMcParticles;
  } else { trackThreshold = m_PARAMtracksPerEvent; }

  for (int iPart = 0; iPart not_eq trackThreshold; ++iPart) {
    const MCParticle* const aMcParticlePtr = aMcParticleArray[iPart];
    aMcParticlePtr->fixParticleList();
    int pdg = aMcParticlePtr->getPDG();


    /** getting full chain of track hits for each track */
    if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle) == false) { continue; } // check whether current particle (and all its hits) belong to a primary particle

    TVector3 mcVertexPos = aMcParticlePtr->getVertex();
    TVector3 mcMomentum = aMcParticlePtr->getMomentum(); /// filter hier nach MomentumStufe
    if (mcVertexPos.Perp() > m_PARAMmaxXYvertexDistance) {
      B2DEBUG(10, "FilterCalculatorModule - event " << m_eventCounter << ": mcParticle with index " << iPart << " discarded because of bad rho-value (" << mcVertexPos.Perp() << " is bigger than threshold: " << m_PARAMmaxXYvertexDistance << ") of the particle-vertex")
      continue;
    }
    if (abs(mcVertexPos[2]) > m_PARAMmaxZvertexDistance) {
      B2DEBUG(10, "FilterCalculatorModule - event " << m_eventCounter << ": mcParticle with index " << iPart << " discarded because of bad z-value (" << abs(mcVertexPos[2]) << " is bigger than threshold: " << m_PARAMmaxZvertexDistance << ") of the particle-vertex")
      continue;
    }
    double mcMomValue = mcMomentum.Perp();
    if (mcMomValue < m_PARAMpTcuts[0]) {
      B2DEBUG(10, "FilterCalculatorModule - event " << m_eventCounter << ": mcParticle with index " << iPart << " discarded because of bad pT-Value " << mcMomValue << " (below threshold of " << m_PARAMpTcuts[0] << ")")
      continue;
    } // filtering all particles having less than minimal threshold value of pT

    int chosenSecMap = -1;
    BOOST_FOREACH(double ptCutoff, m_PARAMpTcuts) {
      if (mcMomValue > ptCutoff) {
        chosenSecMap++;
      } else { break; }
    }
    if (chosenSecMap == -1) {
      B2WARNING("FilterCalculatorModule - event " << m_eventCounter << ": invalid choice of sectorMap, please check parameter pTcuts in steering file")
      continue;
    }

//    ++m_trackletMomentumCounter[chosenSecMap];

    B2DEBUG(10, "FilterCalculatorModule - event " << m_eventCounter << ": chosenSecMap is " << chosenSecMap << " with lower pt threshold of " << m_PARAMpTcuts[chosenSecMap])
    MapOfSectors* thisSecMap = m_sectorMaps[chosenSecMap];

    VXDTrack newTrack(iPart);
    if (m_PARAMdetectorType not_eq Const::SVD) { // want PXDhits
      B2DEBUG(10, "I'm in PXD and chosen detectorType is: " << m_PARAMdetectorType)
//        int ctr = 0;
      RelationIndex<MCParticle, PXDTrueHit>::range_from iterPairMcPxd = relationMcPxdTrueHit.getElementsFrom(aMcParticlePtr);

      while (iterPairMcPxd.first not_eq iterPairMcPxd.second) {
        const PXDTrueHit* const aSiTrueHitPtr = iterPairMcPxd.first->to;
        double uTrue = aSiTrueHitPtr->getU();
        double vTrue = aSiTrueHitPtr->getV();
        double u = uTrue;
        double v = vTrue;
        pLocal = aSiTrueHitPtr->getMomentum();

        VxdID aVxdID = aSiTrueHitPtr->getSensorID();
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);

        // local(0,0,0) is the center of the sensorplane
        double vSize1 = 0.5 * aSensorInfo.getVSize();
        double uSizeAtHit = 0.5 * aSensorInfo.getUSize(v);

        if (m_PARAMsmearHits == true) {
          const PXD::SensorInfo& geometryPXD = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(aVxdID));
          double sigmaU = geometryPXD.getUPitch(uTrue) * m_PARAMuniSigma;
          double sigmaV = geometryPXD.getVPitch(vTrue) * m_PARAMuniSigma;

          B2DEBUG(1000, "sigU sigV: " << sigmaU << " " << sigmaV);
          u = gRandom->Gaus(uTrue, sigmaU);
          v = gRandom->Gaus(vTrue, sigmaV);

          if (v + vSize1 < 0 or v > vSize1) { v = vTrue; }
          uSizeAtHit = 0.5 * aSensorInfo.getUSize(v);

          if (u + uSizeAtHit < 0 or u > uSizeAtHit) { u = uTrue; }
        }

        hitLocal.SetXYZ(u, v, 0);
        hitGlobal = aSensorInfo.pointToGlobal(hitLocal);
        pGlobal = aSensorInfo.vectorToGlobal(pLocal);

        double uCoord = hitLocal[0] + uSizeAtHit; // *0,5 putting (0,0) from the center to the edge of the plane (considers the trapeziodal shape)
        double vCoord = hitLocal[1] + vSize1;

        m_totalGlobalCoordValue += hitGlobal[0] + hitGlobal[1] + hitGlobal[2];
        m_totalLocalCoordValue += uCoord + vCoord;
        m_totalHitCounter++;

        double sectorEdgeV1 = 0, sectorEdgeV2 = 0, uSizeAtv1 = 0, uSizeAtv2 = 0, sectorEdgeU1OfV1 = 0, sectorEdgeU1OfV2 = 0, sectorEdgeU2OfV1 = 0, sectorEdgeU2OfV2 = 0;

//          B2INFO("Sensor edges: O("<<-uSize1<<","<<-vSize1<<"), U("<<uSize1<<","<<-vSize1<<"), V("<<-uSize2<<","<<vSize1<<
//          "), UV("<<uSize2<<","<<vSize1<<")");

        int aUniID = aVxdID.getID();
        int aLayerID = aVxdID.getLayerNumber();
        B2DEBUG(10, "local pxd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ") @layer: " << aLayerID);
        unsigned int aSecID = 0;
        string aSectorName;

        // searching for sector:
        for (int j = 0; j != int(m_PARAMsectorConfigU.size() - 1); ++j) {
//            B2INFO("uCuts(j)*uSize: " << m_PARAMsectorConfigU[j]*uSizeAtHit << " uCuts(j+1)*uSize: " << m_PARAMsectorConfigU[j+1]*uSizeAtHit );

          if (uCoord >= (m_PARAMsectorConfigU[j]*uSizeAtHit * 2) && uCoord <= (m_PARAMsectorConfigU[j + 1]*uSizeAtHit * 2)) {

            for (int k = 0; k != int(m_PARAMsectorConfigV.size() - 1); ++k) {
//                B2INFO(" vCuts(k)*vSize: " << m_PARAMsectorConfigV[k]*vSize1 << " vCuts(k+1)*vSize: " << m_PARAMsectorConfigV[k+1]*vSize1 );

              if (vCoord >= (m_PARAMsectorConfigV[k]*vSize1 * 2) && vCoord <= (m_PARAMsectorConfigV[k + 1]*vSize1 * 2)) {
                aSecID = k + 1 + j * (m_PARAMsectorConfigV.size() - 1);
                B2DEBUG(10, "I have found a SecID: " << aSecID);

                sectorEdgeV1 = m_PARAMsectorConfigV[k] * vSize1 * 2 - vSize1;
                sectorEdgeV2 = m_PARAMsectorConfigV[k + 1] * vSize1 * 2 - vSize1;
                uSizeAtv1 = 0.5 * aSensorInfo.getUSize(sectorEdgeV1);
                uSizeAtv2 = 0.5 * aSensorInfo.getUSize(sectorEdgeV2);
                sectorEdgeU1OfV1 = m_PARAMsectorConfigU[j] * uSizeAtv1 * 2 - uSizeAtv1;
                sectorEdgeU1OfV2 = m_PARAMsectorConfigU[j] * uSizeAtv2 * 2 - uSizeAtv2;
                sectorEdgeU2OfV1 = m_PARAMsectorConfigU[j + 1] * uSizeAtv1 * 2 - uSizeAtv1;
                sectorEdgeU2OfV2 = m_PARAMsectorConfigU[j + 1] * uSizeAtv2 * 2 - uSizeAtv2;

                B2DEBUG(1000, "Sector edges: O(" << sectorEdgeU1OfV1 << "," << sectorEdgeV1 << "), U(" << sectorEdgeU2OfV1 << "," << sectorEdgeV1 << "), V(" << sectorEdgeU1OfV2 << "," << sectorEdgeV2 << "), UV(" << sectorEdgeU2OfV2 << "," << sectorEdgeV2 << ")")
                aSectorName = (boost::format("%1%_%2%_%3%") % aLayerID % aUniID % aSecID).str();
                if (thisSecMap->find(aSectorName) == thisSecMap->end()) { // fallback solution using one secMap for whole range of pT
                  Sector newSector(sectorEdgeV1, sectorEdgeV2, sectorEdgeU1OfV1, sectorEdgeU1OfV2, sectorEdgeU2OfV1, sectorEdgeU2OfV2, aSectorName);
                  thisSecMap->insert(make_pair(aSectorName, newSector));
                } else {
                  thisSecMap->find(aSectorName)->second.increaseCounter();
                }
              }
            }
          }
        } //sector-searching loop

        if (aLayerID <= m_PARAMhighestAllowedLayer) {
          B2DEBUG(10, "adding new PXD hit of track " << iPart)
          VXDHit newHit(Const::PXD, aSectorName, aUniID, hitGlobal, pGlobal, pdg); // (0 = PXD hit)
          newHit.setPXDHit(aSiTrueHitPtr);
          newTrack.addHit(newHit);
        }
        ++iterPairMcPxd.first;
//          ctr++;
      }
    }
    if (m_PARAMdetectorType not_eq Const::PXD) { // want SVDhits
      B2DEBUG(10, "I'm in SVD and chosen detectorType is: " << m_PARAMdetectorType)
//        int ctr = 0;
      RelationIndex<MCParticle, SVDTrueHit>::range_from iterPairMcSvd = relationMcSvdTrueHit.getElementsFrom(aMcParticlePtr);
      while (iterPairMcSvd.first not_eq iterPairMcSvd.second) {

        const SVDTrueHit* const aSiTrueHitPtr = iterPairMcSvd.first->to;
        double uTrue = aSiTrueHitPtr->getU();
        double vTrue = aSiTrueHitPtr->getV();
        double u = uTrue;
        double v = vTrue;
        pLocal = aSiTrueHitPtr->getMomentum();

        VxdID aVxdID = aSiTrueHitPtr->getSensorID();
        VXD::SensorInfoBase aSensorInfo = geometry.getSensorInfo(aVxdID);

        // local(0,0,0) is the center of the sensorplane
        double vSize1 = 0.5 * aSensorInfo.getVSize();
        double uSizeAtHit = 0.5 * aSensorInfo.getUSize(v);

        if (m_PARAMsmearHits == true) {
          const SVD::SensorInfo& geometrySVD = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(aVxdID));
          double sigmaU = geometrySVD.getUPitch(uTrue) * m_PARAMuniSigma;
          double sigmaV = geometrySVD.getVPitch(vTrue) * m_PARAMuniSigma;

          B2DEBUG(1000, "sigU sigV: " << sigmaU << " " << sigmaV);
          u = gRandom->Gaus(uTrue, sigmaU);
          v = gRandom->Gaus(vTrue, sigmaV);

          if (v + vSize1 < 0 or v > vSize1) { v = vTrue; }
          uSizeAtHit = 0.5 * aSensorInfo.getUSize(v);

          if (u + uSizeAtHit < 0 or u > uSizeAtHit) { u = uTrue; }
        }

        hitLocal.SetXYZ(u, v, 0);
        hitGlobal = aSensorInfo.pointToGlobal(hitLocal);

        double uCoord = hitLocal[0] + uSizeAtHit; // *0,5 putting (0,0) from the center to the edge of the plane (considers the trapeziodal shape)
        double vCoord = hitLocal[1] + vSize1;

        m_totalGlobalCoordValue += hitGlobal[0] + hitGlobal[1] + hitGlobal[2];
        m_totalLocalCoordValue += uCoord + vCoord;
        m_totalHitCounter++;

        double sectorEdgeV1 = 0, sectorEdgeV2 = 0, uSizeAtv1 = 0, uSizeAtv2 = 0, sectorEdgeU1OfV1 = 0, sectorEdgeU1OfV2 = 0, sectorEdgeU2OfV1 = 0, sectorEdgeU2OfV2 = 0;


        int aUniID = aVxdID.getID();
        int aLayerID = aVxdID.getLayerNumber();

        B2DEBUG(10, "local svd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ") @layer: " << aLayerID);
        unsigned int aSecID = 0;
        string aSectorName;
        // searching for sector:
        for (int j = 0; j != int(m_PARAMsectorConfigU.size() - 1); ++j) {
          B2DEBUG(1000, "uCuts(j)*uSize: " << m_PARAMsectorConfigU[j]*uSizeAtHit << " uCuts(j+1)*uSize: " << m_PARAMsectorConfigU[j + 1]*uSizeAtHit);

          if (uCoord >= (m_PARAMsectorConfigU[j]*uSizeAtHit * 2) && uCoord <= (m_PARAMsectorConfigU[j + 1]*uSizeAtHit * 2)) {

            for (int k = 0; k != int(m_PARAMsectorConfigV.size() - 1); ++k) {
              B2DEBUG(1000, " vCuts(k)*vSize: " << m_PARAMsectorConfigV[k]*vSize1 << " vCuts(k+1)*vSize: " << m_PARAMsectorConfigV[k + 1]*vSize1);

              if (vCoord >= (m_PARAMsectorConfigV[k]*vSize1 * 2) && vCoord <= (m_PARAMsectorConfigV[k + 1]*vSize1 * 2)) {
                aSecID = k + 1 + j * (m_PARAMsectorConfigV.size() - 1);
                B2DEBUG(10, "I have found a SecID: " << aSecID);

                sectorEdgeV1 = m_PARAMsectorConfigV[k] * vSize1 * 2 - vSize1;
                sectorEdgeV2 = m_PARAMsectorConfigV[k + 1] * vSize1 * 2 - vSize1;
                uSizeAtv1 = 0.5 * aSensorInfo.getUSize(sectorEdgeV1);
                uSizeAtv2 = 0.5 * aSensorInfo.getUSize(sectorEdgeV2);
                sectorEdgeU1OfV1 = m_PARAMsectorConfigU[j] * uSizeAtv1 * 2 - uSizeAtv1;
                sectorEdgeU1OfV2 = m_PARAMsectorConfigU[j] * uSizeAtv2 * 2 - uSizeAtv2;
                sectorEdgeU2OfV1 = m_PARAMsectorConfigU[j + 1] * uSizeAtv1 * 2 - uSizeAtv1;
                sectorEdgeU2OfV2 = m_PARAMsectorConfigU[j + 1] * uSizeAtv2 * 2 - uSizeAtv2;

                B2DEBUG(1000, "Sector edges: O(" << sectorEdgeU1OfV1 << "," << sectorEdgeV1 << "), U(" << sectorEdgeU2OfV1 << "," << sectorEdgeV1 << "), V(" << sectorEdgeU1OfV2 << "," << sectorEdgeV2 << "), UV(" << sectorEdgeU2OfV2 << "," << sectorEdgeV2 << ")")

                aSectorName = (boost::format("%1%_%2%_%3%") % aLayerID % aUniID % aSecID).str();
                if (thisSecMap->find(aSectorName) == thisSecMap->end()) { // fallback solution using one secMap for whole range of pT
                  Sector newSector(sectorEdgeV1, sectorEdgeV2, sectorEdgeU1OfV1, sectorEdgeU1OfV2, sectorEdgeU2OfV1, sectorEdgeU2OfV2, aSectorName);
                  thisSecMap->insert(make_pair(aSectorName, newSector));
                } else {
                  thisSecMap->find(aSectorName)->second.increaseCounter();
                }
              }
            }
          }
        } //sector-searching loop

        if (aLayerID <= m_PARAMhighestAllowedLayer) {
          B2DEBUG(10, "adding new SVD hit of track " << iPart)
          VXDHit newHit(Const::SVD, aSectorName, aUniID, hitGlobal, pGlobal, pdg); // (1 = SVD hit)
          newHit.setSVDHit(aSiTrueHitPtr);
          newTrack.addHit(newHit);
        }
//          ctr++;
        ++iterPairMcSvd.first;
      } // now each hit knows in which direction the particle goes, in which sector it lies and where it is
    }

    string aSectorName = "0_00_0"; // virtual sector for decay vertices.
    MCParticle* mcp = aMcParticleArray[newTrack.getParticleID()];
//      TVector3 mcvertex = mcp->getVertex();
    TVector3 mcvertex(0. , 0. , 0.); /// we do not know the real vertex, therefore (0,0,0) has to be asumed!
    TVector3 mcmom = mcp->getMomentum();
    int mcPdg = mcp->getPDG();
    VXDHit newHit(Const::IR, aSectorName, 0, mcvertex, mcmom, mcPdg);
    newHit.setVertex();
    newTrack.setPt(mcmom.Perp()); // needed for dynamic classifying of sectormap
    newTrack.addHit(newHit); /// OPTIONAL: instead of attaching virtual hit only to track, it could be attached to tracklets, but in this case, a tracklet (curler) recognition using 2D-Filters has to be implemented

    newTrack.sort(); // now all the hits are ordered by their global time, which allows cutting the tracks into tracklets where the momenta of all hits point into the same direction.

    tracksOfEvent.push_back(newTrack);

    list<VXDHit> thisTrack = newTrack.getTrack();
    if (thisTrack.size() > 30) { B2WARNING("event: " << m_eventCounter << " beware, tracklength is " << thisTrack.size()) }
    if (int (thisTrack.size()) > m_numOfLayers * 2) { m_longTrackCounter++; }
    for (list<VXDHit>::iterator it = thisTrack.begin() ; it != thisTrack.end(); ++it) {
      B2DEBUG(10, "track has a hit in the following sector: " << it->getSectorID())
    }
    int thisUniID, friendUniID;
    list<VXDHit>::reverse_iterator riter, oldRiter;
    for (riter = thisTrack.rbegin(); riter != thisTrack.rend();) {

      VXDTrack newTracklet(newTrack.getParticleID(), newTrack.getPt(), thisSecMap);

      bool direction = riter->getParticleMovement();
      thisUniID = riter->getUniID();
      oldRiter = riter;
      B2DEBUG(10, "adding Hit to tracklet: ")
      newTracklet.addHit(*riter);

      B2DEBUG(10, "copying track-segment into tracklet")
      ++riter;
      if (riter != thisTrack.rend()) {   //
        if (direction == riter->getParticleMovement()) {
          while (direction == riter->getParticleMovement()) {
            if (riter != thisTrack.rend()) {
              friendUniID = riter->getUniID();
              if (thisUniID != friendUniID) {
                newTracklet.addHit(*riter);
              } else {
                string thisSecName = riter->getSectorID();
                B2WARNING("at event " << m_eventCounter << ": track " << newTrack.getParticleID() << " with momentum of " << newTrack.getPt() << "GeV/c has got two trueHits with same direction of flight, distance of " << (oldRiter->getHitPosition() - riter->getHitPosition()).Mag() << " of each other and deltatimestamp " << (oldRiter->getTimeStamp() - riter->getTimeStamp()) << " in the same sensor :" << thisSecName << ". Hit discarded!")
                thisSecMap->find(thisSecName)->second.decreaseCounter();
                m_badHitsCounter++;
              }
              oldRiter = riter; ++riter; thisUniID = friendUniID;
            } else { break; }
          }
        }
      }

      m_trackletLengthCounter[newTracklet.size() - 1] ++;

      if (newTracklet.size() >= m_PARAMminTrackletLength) {

        int chosenSecMap = -1;
        BOOST_FOREACH(double ptCutoff, m_PARAMpTcuts) {
          if (mcMomValue > ptCutoff) {
            chosenSecMap++;
          } else { break; }
        }
        m_trackletMomentumCounter[chosenSecMap]++;
        if (direction == true) { // in that case the momentum vector of the tracklet points away from the IP -> outward movement
          newTracklet.reverse(); // ->inward "movement" of the hits, needed for the filtering, no presorting needed, the hits were in the right order.
          B2DEBUG(10, " change direction of tracklet...")
        }

//          newTracklet.addHit(newHit); /// adding virtual hit to tracklet!

        B2DEBUG(10, "adding tracklet to list")
        trackletsOfEvent.push_back(newTracklet);

        if (newTracklet.size() > m_numOfLayers * 2 + 1) {
          m_longTrackletCounter++;
        }
      } else { B2DEBUG(10, "tracklet too small -> discarded") }

      if (m_PARAMnoCurler == true) {  // do not store following tracklets, when no curlers shall be recorded...
        riter = thisTrack.rend();
      }
    } // slicing current track into moouth sized tracklets
  } // looping through particles

  B2DEBUG(1, "finished tracklet-generation. " << trackletsOfEvent.size() << " tracklets and " << tracksOfEvent.size() << " tracks found")
  string currentSector, friendSector;
//4hit-variables:
  TVector3 hitG, moHitG, graMoHitG, greGraMoHitG, origin(0., 0., 0.);
  double deltaDistCircleCenter, deltapT;
// 3hit-variables:
  TVector3 motherHitGlobal, grandMotherHitGlobal;
  double dist2IP, angles3D, anglesXY, anglesRZ, helixFit, deltaSlopeRZ, pT;
// 2hit-variables:
  TVector3 segmentVector;
  double distanceXY, distanceZ, distance3D, normedDistance3D, slopeRZ;

  for (int i = 0; i < int(trackletsOfEvent.size()); i++) {

    bool firstrun = true, secondrun = true, thirdrun = true, lastRun = false;
    list<VXDHit> aTracklet = trackletsOfEvent[i].getTrack();
    MapOfSectors* thisSecMap = trackletsOfEvent[i].getSecMap();
    list<VXDHit>::iterator iter = aTracklet.begin(); // main iterator looping through the whole tracklet, "innermost Hit"
    list<VXDHit>::iterator it2HitsFilter = aTracklet.begin(); //important for 2hit-Filters: points to current hit of 2-hit-processes " next to innermost hit"
    list<VXDHit>::iterator it3HitsFilter = aTracklet.begin(); // -"- 3hit-Filters...m_PARAMlogTRadiusHighOccupancytoIPDistance
    list<VXDHit>::iterator it4HitsFilter = aTracklet.begin(); // -"- 4hit-Filters... "outermost Hit"

    B2DEBUG(10, "executing " << i + 1 << "th tracklet with size " << aTracklet.size())
    for (list<VXDHit>::iterator it = aTracklet.begin() ; it != aTracklet.end(); ++it) {
      currentSector = it->getSectorID();
      B2DEBUG(10, "tracklet has a hit in the following sector: " << currentSector)
    }

    MapOfSectors::iterator thisSectorPos;
    for (; iter != aTracklet.end(); ++iter) {
      // moving from outermost hit inwards. iter points at the innermost hit of the current line of hits (means in the first iteration it points at the outermost hit, in the 2nd iteration it points at the inner hit next to the outermost hit and it2HitsFilter points to the outermost hit) with every iteration, iter leads the row of iterators inwards.
      list<VXDHit>::iterator tempIter = iter; ++tempIter;
      if (tempIter == aTracklet.end()) {
        lastRun = true;
      }

      if (firstrun == false) {
        B2DEBUG(10, "calculating 2-hit-filters")
        if (secondrun == false) {
          B2DEBUG(10, "calculating 3-hit-filters")
          if (thirdrun == false) {
            B2DEBUG(10, "calculating 4-hit-filters")
            currentSector = it4HitsFilter->getSectorID(); /// ???
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
            m_fourHitFilterBox.resetValues(hitG, moHitG, graMoHitG, greGraMoHitG); // outerhit, centerhit, innerhit

            if (m_PARAMlogDeltaPt == true) {
              deltapT = m_fourHitFilterBox.deltapT();
              if (isnan(deltapT) == false) {
                B2DEBUG(100, "4-hit-filter in event " << m_eventCounter << ": calculated deltapT-Value: " << deltapT << " gets stored ")
                thisSectorPos->second.addValue(friendSector, FilterID::nameDeltapT, deltapT);
              } else {
                m_badFilterValueCtr++; B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltapT-Value: " << deltapT << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
                hitG.Print(); moHitG.Print(); graMoHitG.Print(); greGraMoHitG.Print();
              }
            }

            if (m_PARAMlogDeltaDistCircleCenter == true) {
              deltaDistCircleCenter =  m_fourHitFilterBox.deltaDistCircleCenter();
              if (isnan(deltaDistCircleCenter) == false) {
                B2DEBUG(100, "4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenter-Value: " << deltaDistCircleCenter << " gets stored ")
                thisSectorPos->second.addValue(friendSector, FilterID::nameDeltaDistance2IP, deltaDistCircleCenter);
              } else {
                m_badFilterValueCtr++; B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenter-Value: " << deltaDistCircleCenter << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
                hitG.Print(); moHitG.Print(); graMoHitG.Print(); greGraMoHitG.Print();
              }
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
          m_fourHitFilterBox.resetValues(hitGlobal, motherHitGlobal, grandMotherHitGlobal, origin); // outerhit, centerhit, innerhit
          if (m_PARAMlogDeltaPtHighOccupancy == true && lastRun == false) {
            deltapT = m_fourHitFilterBox.deltapT();
            if (isnan(deltapT) == false) {
              B2DEBUG(100, "4-hit-filter in event " << m_eventCounter << ": calculated deltapTHighOccupancy-Value: " << deltapT << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameDeltapTHighOccupancy, deltapT);
            } else {
              m_badFilterValueCtr++; B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltapTHighOccupancy-Value: " << deltapT << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print(); origin.Print();
            }
          }
          if (m_PARAMlogDeltaDistCircleCenterHighOccupancy == true && lastRun == false) {
            deltaDistCircleCenter =  m_fourHitFilterBox.deltaDistCircleCenter();
            if (isnan(deltaDistCircleCenter) == false) {
              B2DEBUG(100, "4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenterHighOccupancy-Value: " << deltaDistCircleCenter << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameDeltaDistanceHighOccupancy2IP, deltaDistCircleCenter);
            } else {
              m_badFilterValueCtr++; B2WARNING("4-hit-filter in event " << m_eventCounter << ": calculated deltaDistCircleCenterHighOccupancy-Value: " << deltaDistCircleCenter << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print(); origin.Print();
            }
          }
          /// high occupancy mode - end

          m_threeHitFilterBox.resetValues(hitGlobal, motherHitGlobal, grandMotherHitGlobal); // outerhit, centerhit, innerhit

          if (m_PARAMlogTRadiustoIPDistance == true) {
            dist2IP = m_threeHitFilterBox.calcCircleDist2IP();
            if (isnan(dist2IP) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated dist2IP-Value: " << dist2IP << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameDistance2IP, dist2IP);
            } else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated dist2IP-Value: " << dist2IP << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print(); origin.Print();
            }
          }


          if (m_PARAMlogPt == true) {
            pT = m_threeHitFilterBox.calcPt();
            if (isnan(pT) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated pT-Value: " << pT << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::namePT, pT);
            } else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated pT-Value: " << pT << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
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
            if (isnan(deltaSlopeRZ) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeRZ-Value: " << deltaSlopeRZ << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameDeltaSlopeRZ, deltaSlopeRZ);
            } else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeRZ-Value: " << deltaSlopeRZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }


          if (m_PARAMlogAngles3D == true) {
            angles3D = m_threeHitFilterBox.calcAngle3D();
            if (isnan(angles3D) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated angles3D-Value: " << angles3D << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameAngles3D, angles3D);
            } else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated angles3D-Value: " << angles3D << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }


          if (m_PARAMlogAnglesXY == true) {
            anglesXY = m_threeHitFilterBox.calcAngleXY();
            if (isnan(anglesXY) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated anglesXY-Value: " << anglesXY << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameAnglesXY, anglesXY);
            }  else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesXY-Value: " << anglesXY << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }


          if (m_PARAMlogAnglesRZ == true) {
            anglesRZ = m_threeHitFilterBox.calcAngleRZ();
            if (isnan(anglesRZ) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated anglesRZ-Value: " << anglesRZ << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameAnglesRZ, anglesRZ);
            } else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesRZ-Value: " << anglesRZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }


          if (m_PARAMlogHelixFit == true) {
            helixFit = m_threeHitFilterBox.calcHelixFit();
            if (isnan(helixFit) == false) {
              B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated helixFit-Value: " << helixFit << " gets stored ")
              thisSectorPos->second.addValue(friendSector, FilterID::nameHelixFit, helixFit);
            } else {
              m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated helixFit-Value: " << helixFit << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
              hitGlobal.Print(); motherHitGlobal.Print(); grandMotherHitGlobal.Print();
            }
          }

          ++it3HitsFilter; /// -"- 3hit-Filters...

        } else { secondrun = false; }

        currentSector = it2HitsFilter->getSectorID();
        friendSector = iter->getSectorID();
        thisSectorPos = thisSecMap->find(currentSector);

        if (typeid(string).name() != typeid(friendSector).name()) {
          B2WARNING("FilterCalculator event " << m_eventCounter << ": type of friendSector is no string, aborting tracklet...")
          continue;
        }

        hitGlobal = it2HitsFilter->getHitPosition();
        motherHitGlobal = iter->getHitPosition();

        /// high occupancy mode for 2+1 hits
        m_threeHitFilterBox.resetValues(hitGlobal, motherHitGlobal, origin); // outerhit, centerhit, innerhit
        if (m_PARAMlogTRadiusHighOccupancytoIPDistance == true && lastRun == false) {
          dist2IP = m_threeHitFilterBox.calcCircleDist2IP();
          if (isnan(dist2IP) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated distHighOccupancy2IP-Value: " << dist2IP << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameDistanceHighOccupancy2IP, dist2IP);
          } else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated distHighOccupancy2IP-Value: " << dist2IP << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        if (m_PARAMlogPtHighOccupancy == true && lastRun == false) {
          pT = m_threeHitFilterBox.calcPt();
          if (isnan(pT) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated pTHighOccupancy-Value: " << pT << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::namePTHighOccupancy, pT);
          } else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated pTHighOccupancy-Value: " << pT << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        if (m_PARAMlogDeltaSlopeHighOccupancyRZ == true && lastRun == false) {
          deltaSlopeRZ = m_threeHitFilterBox.calcDeltaSlopeRZ();
          if (isnan(deltaSlopeRZ) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeHighOccupancyRZ-Value: " << deltaSlopeRZ << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameDeltaSlopeHighOccupancyRZ, deltaSlopeRZ);
          } else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated deltaSlopeHighOccupancyRZ-Value: " << deltaSlopeRZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        if (m_PARAMlogAnglesHighOccupancy3D == true && lastRun == false) {
          angles3D = m_threeHitFilterBox.calcAngle3D();
          if (isnan(angles3D) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancy3D-Value: " << angles3D << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameAnglesHighOccupancy3D, angles3D);
          } else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancy3D-Value: " << angles3D << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        if (m_PARAMlogAnglesHighOccupancyXY == true && lastRun == false) {
          anglesXY = m_threeHitFilterBox.calcAngleXY();
          if (isnan(anglesXY) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyXY-Value: " << anglesXY << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameAnglesHighOccupancyXY, anglesXY);
          }  else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyXY-Value: " << anglesXY << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        if (m_PARAMlogAnglesHighOccupancyRZ == true && lastRun == false) {
          anglesRZ = m_threeHitFilterBox.calcAngleRZ();
          if (isnan(anglesRZ) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyRZ-Value: " << anglesRZ << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameAnglesHighOccupancyRZ, anglesRZ);
          } else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated anglesHighOccupancyRZ-Value: " << anglesRZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        if (m_PARAMlogHelixHighOccupancyFit == true && lastRun == false) {
          helixFit = m_threeHitFilterBox.calcHelixFit();
          if (isnan(helixFit) == false) {
            B2DEBUG(100, "3-hit-filter in event " << m_eventCounter << ": calculated helixHighOccupancyFit-Value: " << helixFit << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameHelixHighOccupancyFit, helixFit);
          } else {
            m_badFilterValueCtr++; B2WARNING("3-hit-filter in event " << m_eventCounter << ": calculated helixHighOccupancyFit-Value: " << helixFit << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print(); origin.Print();
          }
        }
        /// high occupancy mode - end

        m_twoHitFilterBox.resetValues(hitGlobal, motherHitGlobal); // outerhit, innerhit

        if (m_PARAMlogDistanceXY == true) {
          distanceXY = m_twoHitFilterBox.calcDistXY();
          if (isnan(distanceXY) == false) {
            B2DEBUG(100, "2-hit-filter in event " << m_eventCounter << ": calculated distanceXY-Value: " << distanceXY << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameDistanceXY, distanceXY);
          } else {
            m_badFilterValueCtr++; B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated distanceXY-Value: " << distanceXY << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogDistanceZ == true) {
          distanceZ = m_twoHitFilterBox.calcDistZ();
          if (isnan(distanceZ) == false) {
            B2DEBUG(100, "2-hit-filter in event " << m_eventCounter << ": calculated distanceZ-Value: " << distanceZ << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameDistanceZ, distanceZ);
          } else {
            m_badFilterValueCtr++; B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated distanceZ-Value: " << distanceZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogDistance3D == true) {
          distance3D = m_twoHitFilterBox.calcDist3D();
          if (isnan(distance3D) == false) {
            B2DEBUG(100, "2-hit-filter in event " << m_eventCounter << ": calculated distance3D-Value: " << distance3D << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameDistance3D, distance3D);
          } else {
            m_badFilterValueCtr++; B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated distance3D-Value: " << distance3D << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogNormedDistance3D == true) {
          normedDistance3D = m_twoHitFilterBox.calcNormedDist3D();
          if (isnan(normedDistance3D) == false) {
            B2DEBUG(100, "2-hit-filter in event " << m_eventCounter << ": calculated normedDistance3D-Value: " << normedDistance3D << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameNormedDistance3D, normedDistance3D);
          } else {
            m_badFilterValueCtr++; B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated normedDistance3D-Value: " << normedDistance3D << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        if (m_PARAMlogSlopeRZ == true) {
          slopeRZ = m_twoHitFilterBox.calcSlopeRZ();
          if (isnan(slopeRZ) == false) {
            B2DEBUG(100, "2-hit-filter in event " << m_eventCounter << ": calculated slopeRZ-Value: " << slopeRZ << " gets stored ")
            thisSectorPos->second.addValue(friendSector, FilterID::nameSlopeRZ, slopeRZ);
          } else {
            m_badFilterValueCtr++; B2WARNING("2-hit-filter in event " << m_eventCounter << ": calculated slopeRZ-Value: " << slopeRZ << " is 'nan'! currentSec: " << currentSector << ", friendSec: " << friendSector << ". Printing Vectors(outer2inner): ")
            hitGlobal.Print(); motherHitGlobal.Print();
          }
        }

        ++it2HitsFilter; //important for 2hit-Filters: points to current hit of 2-hit-processes

      } else { firstrun = false; }
    } // looping through current tracklet
  } // looping through all tracklets
  B2DEBUG(1, "FilterCalculator - event calculations done!")
}


void FilterCalculatorModule::endRun()
{
  int totalTrackletCounter = 0;
  int totalHitCounter = 0;
  m_eventCounter++;
  B2INFO("~~~~~~~~~~~FilterCalculator - endRun ~~~~~~~~~~")
  for (int i = 0; i < int(m_PARAMsecMapNames.size()); ++i) {
    B2INFO(" within " << m_eventCounter << " events we got " << m_trackletMomentumCounter.at(i) << " tracklets in the " << m_PARAMsecMapNames[i] << " setup")
  }
  for (int i = 0; i < int(m_trackletLengthCounter.size()); i++) {
//    if ( m_trackletLengthCounter[i] == 0 ) { continue; }
    B2INFO(" within " << m_eventCounter << " events we got " << m_trackletLengthCounter[i] << " tracklets containing " << i + 1 << " hits")
    totalTrackletCounter += m_trackletLengthCounter[i];
    totalHitCounter += m_trackletLengthCounter[i] * (i + 1);
  }
  B2INFO(m_badHitsCounter << " hits had to be discarded because of double impact in same sensor having same direction of flight")
  B2INFO(" there were " << float(totalTrackletCounter) / float(m_eventCounter) << " tracklets per event...")
  B2INFO(" there were " << m_longTrackCounter << " Tracks having more than " << m_numOfLayers * 2 << " hits...")
  B2INFO(" there were " << m_longTrackletCounter << " Tracklets having more than " << m_numOfLayers * 2 << " hits!!!")
  B2INFO(" totalGlobalCoordValue: " << m_totalGlobalCoordValue << ", totalLocalCoordValue: " << m_totalLocalCoordValue << ", of " << totalHitCounter << " hits total (" << m_totalHitCounter << " counted manually) ")
  B2INFO(m_badFilterValueCtr << " times, a filter produced invalid results ('nan')")
  B2INFO("~~~~~~~~~~~FilterCalculator - endRun ~~~~~~~~~~")
}


void FilterCalculatorModule::terminate()
{

  int ctr = 0;
  BOOST_FOREACH(MapOfSectors * thisMap, m_sectorMaps) {
    BOOST_FOREACH(SecMapEntry thisEntry, *thisMap) {
//      string secMapName = m_PARAMsecMapNames.at(ctr);
      thisEntry.second.exportFriends(m_PARAMsecMapNames.at(ctr));
    }
    thisMap->clear();
    ctr++;
  }
  m_sectorMaps.clear();
//  for (MapOfSectors::iterator secIter = m_sectorMap.begin(); secIter != m_sectorMap.end(); ++secIter) {
//    secIter->second.exportFriends(m_PARAMsectorSetupFileName);
//  }
//  m_sectorMap.clear();
  B2INFO(" FilterCalculatorModule, everything is done. Terminating.")
}
