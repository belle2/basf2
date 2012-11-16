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
#include <pxd/geometry/SensorInfo.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

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

  std::vector<double> defaultConfigU;
  std::vector<double> defaultConfigV;
  defaultConfigU.push_back(0.0);
  defaultConfigU.push_back(0.5);
  defaultConfigU.push_back(1.0);
  defaultConfigV.push_back(0.0);
  defaultConfigV.push_back(0.5);
  defaultConfigV.push_back(1.0);

  addParam("exportSectorCoords", m_PARAMexportSectorCoords, "set true if you want to export coordinates of the sectors too", bool(true));
  addParam("sectorSetupFileName", m_PARAMsectorSetupFileName, "enables personal sector setups (can be loaded by the vxd track finder)", string("genericSectorMap"));
  addParam("tracksPerEvent", m_PARAMtracksPerEvent, "defines the number of exported tracks per event (should not be higher than real number of tracks per event)", int(1));
  addParam("detectorType", m_PARAMdetectorType, "defines which detector type has to be exported. VXD: -1, PXD: 1, SVD: 2", int(2));
//  addParam("percentageOfFMSectorFriends", m_percentageOfFMSectorFriends, "allows filtering extreme rare sector combinations by setting the value between 0.0 and 1.0", float(0.999));
  addParam("sectorConfigU", m_PARAMsectorConfigU, "allows defining the the config of the sectors in U direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigU);
  addParam("sectorConfigV", m_PARAMsectorConfigV, "allows defining the the config of the sectors in V direction value is valid for each sensor of chosen detector setup, minimum 2 values between 0.0 and 1.0", defaultConfigV);
  addParam("trackErrorTracks", m_PARAMtrackErrorTracks, "track tracks which cause strange results", bool(false));
  addParam("highestAllowedLayer", m_PARAMhighestAllowedLayer, "set value below 6 if you want to exclude outer layers (standard is 6)", int(6));

  addParam("uniSigma", m_PARAMuniSigma, "standard value is 1/sqrt(12). Change this value for sharper or more diffuse hits (coupled with 'smearHits')", double(1 / sqrt(12)));
  addParam("smearHits", m_PARAMsmearHits, "if True, hits get smeared by pitch/uniSigma", bool(false));
  addParam("noCurler", m_PARAMnoCurler, "if True, curling tracks get reduced to first part of trajectory before starting to curl back", bool(false));
  addParam("minTrackletLength", m_PARAMminTrackletLength, "defines the number of hits needed to be stored as a tracklet", int(2));

  //2 hit filters:
  addParam("logDistanceXY", m_PARAMlogDistanceXY, "set 'true' if you want to log distances (XY) between trackHits", bool(true));
  addParam("logDistanceZ", m_PARAMlogDistanceZ, "set 'true' if you want to log distances (Z) between trackHits", bool(true));
  addParam("logDistance3D", m_PARAMlogDistance3D, "set 'true' if you want to log distances (3D) between trackHits", bool(true));
  addParam("logNormedDistance3D", m_PARAMlogNormedDistance3D, " set 'true' to log improved 3D distance between trackHits", bool(true));
  //3 hit filters;
  addParam("logAngles3D", m_PARAMlogAngles3D, "set 'true' if you want to log angles3D between segments", bool(true));
  addParam("logAnglesXY", m_PARAMlogAnglesXY, "set 'true' if you want to log anglesXY between segments", bool(true));
  addParam("logAnglesRZ", m_PARAMlogAnglesRZ, "set 'true' if you want to log anglesRZ between segments", bool(true));
  addParam("logDeltaZ", m_PARAMlogDeltaZ, "set 'true' if you want to log delta Z-values between segments", bool(true));
  addParam("logPt", m_PARAMlogPt, "set 'true' if you want to log Pt between segments", bool(true));
  addParam("logTRadius2IPDistance", m_PARAMlogTRadiustoIPDistance, " set 'true' to log the difference between the radius of the track circle in x-y-plan and the distance of the center of the circle to the IP", bool(true));
  //4 hit filters;
  addParam("logDeltaPt", m_PARAMlogDeltaPt, "set 'true' if you want to log delta Pt between segments", bool(true));
  addParam("logDeltaTRadius2IPDistance", m_PARAMlogDeltaTRadiustoIPDistance, "set 'true' to compare different TRadius2IPDistances", bool(true));

}


FilterCalculatorModule::~FilterCalculatorModule()
{
  cerr << " final brainfuck... " << endl;
}


void FilterCalculatorModule::initialize()
{
  m_sectorMap.clear();
  B2INFO("~~~~~~~~~~~FilterCalculator - initialize ~~~~~~~~~~")
  B2INFO("chosen detectorType: " << m_PARAMdetectorType << ", highestAllowedLayer: " << m_PARAMhighestAllowedLayer << ", smearHits: " << m_PARAMsmearHits << ", noCurler: " << m_PARAMnoCurler << ", uniSigma: " << m_PARAMuniSigma)
}


void FilterCalculatorModule::beginRun()
{
  B2INFO("~~~~~~~~~~~FilterCalculator - beginRun ~~~~~~~~~~")
  m_eventCounter = 0;
  m_totalLocalCoordValue = 0;
  m_totalGlobalCoordValue = 0;
  m_totalHitCounter = 0;
  m_longTrackCounter = 0;
  m_numOfLayers = 0;
  m_badFilterValueCtr = 0;
  if (m_PARAMdetectorType == -1) {
    m_numOfLayers = 6;
  } else if (m_PARAMdetectorType == Const::PXD) {
    m_numOfLayers = 2;
  } else { m_numOfLayers = 4; }
  for (int i = 0; i < m_numOfLayers * 2; i++) {
    m_trackletLengthCounter.push_back(0);
  }

//  vector<int> bled;
//  bled.push_back(4); bled.push_back(13); bled.push_back(25); bled.push_back(99);
//  cout << " Testrunde1: " << bled[0] << " " << bled[1] << " " << bled[2] << " " << bled[3] << endl;
//  bled[0]++;
//  bled[1]+= 5;
//  bled[2]+= bled[3];
//  bled[3] = bled[0];
//  cout << " Testrunde2: " << bled[0] << " " << bled[1] << " " << bled[2] << " " << bled[3] << endl;
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

  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  int numOfPxdTrueHits = aPxdTrueHitArray.getEntries();
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  int numOfSvdTrueHits = aSvdTrueHitArray.getEntries();

  if (numOfMcParticles == 0) {
    B2FATAL("event" << m_eventCounter << ": there is no MCParticle!")
  } else if (numOfPxdTrueHits == 0 && m_PARAMdetectorType not_eq Const::SVD) {
    B2ERROR("event" << m_eventCounter << ": there are no PXDTrueHits")
    return;
  } else if (numOfSvdTrueHits == 0 && m_PARAMdetectorType not_eq Const::PXD) {
    B2ERROR("event" << m_eventCounter << ": there are no SVDTrueHits")
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

  for (int iPart = 0; iPart not_eq m_PARAMtracksPerEvent; ++iPart) {
    const MCParticle* const aMcParticlePtr = aMcParticleArray[iPart];
    aMcParticlePtr->fixParticleList();
    int pdg = aMcParticlePtr->getPDG();


    /** getting full track hit chains for each track */
    if (aMcParticlePtr->hasStatus(MCParticle::c_PrimaryParticle) == true) {
      VXDTrack newTrack(iPart);
      if (m_PARAMdetectorType not_eq Const::SVD) { // want PXDhits
        B2DEBUG(11, "I'm in PXD and chosen detectorType is: " << m_PARAMdetectorType)
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
          B2DEBUG(11, "local pxd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ") @layer: " << aLayerID);
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
                  B2DEBUG(11, "I have found a SecID: " << aSecID);

                  double v1 = 0, v2 = 0, u1v1 = 0, u1v2 = 0, u2v1 = 0, u2v2 = 0;
                  sectorEdgeV1 = m_PARAMsectorConfigV[k] * vSize1 * 2 - vSize1;
                  sectorEdgeV2 = m_PARAMsectorConfigV[k + 1] * vSize1 * 2 - vSize1;
                  uSizeAtv1 = 0.5 * aSensorInfo.getUSize(sectorEdgeV1);
                  uSizeAtv2 = 0.5 * aSensorInfo.getUSize(sectorEdgeV2);
                  sectorEdgeU1OfV1 = m_PARAMsectorConfigU[j] * uSizeAtv1 * 2 - uSizeAtv1;
                  sectorEdgeU1OfV2 = m_PARAMsectorConfigU[j] * uSizeAtv2 * 2 - uSizeAtv2;
                  sectorEdgeU2OfV1 = m_PARAMsectorConfigU[j + 1] * uSizeAtv1 * 2 - uSizeAtv1;
                  sectorEdgeU2OfV2 = m_PARAMsectorConfigU[j + 1] * uSizeAtv2 * 2 - uSizeAtv2;

                  v1 = sectorEdgeV1;
                  v2 = sectorEdgeV2;
                  u1v1 = sectorEdgeU1OfV1;
                  u1v2 = sectorEdgeU1OfV2;
                  u2v1 = sectorEdgeU2OfV1;
                  u2v2 = sectorEdgeU2OfV2;

//                  B2INFO("Sector edges: O("<<u1v1<<","<<v1<<"), U("<<u2v1<<","<<v1<<"), V("<<u1v2<<","<<v2<<"), UV("<<u2v2<<","<<v2<<")")

                  aSectorName = (boost::format("%1%_%2%_%3%") % aLayerID % aUniID % aSecID).str();
                  if (m_sectorMap.find(aSectorName) == m_sectorMap.end()) {
                    Sector newSector(v1, v2, u1v1, u1v2, u2v1, u2v2, aSectorName);
                    m_sectorMap.insert(make_pair(aSectorName, newSector));
                  } else {
                    m_sectorMap.find(aSectorName)->second.increaseCounter();
                  }
                }
              }
            }
          } //sector-searching loop

          if (aLayerID <= m_PARAMhighestAllowedLayer) {
            B2DEBUG(11, "adding new PXD hit of track " << iPart)
            VXDHit newHit(Const::PXD, aSectorName, hitGlobal, pGlobal, pdg); // (0 = PXD hit)
            newHit.setPXDHit(aSiTrueHitPtr);
            newTrack.addHit(newHit);
          }
          ++iterPairMcPxd.first;
//          ctr++;
        }
      }
      if (m_PARAMdetectorType not_eq Const::PXD) { // want SVDhits
        B2DEBUG(11, "I'm in SVD and chosen detectorType is: " << m_PARAMdetectorType)
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

          B2DEBUG(11, "local svd hit coordinates (u,v): (" << hitLocal[0] << "," << hitLocal[1] << ") @layer: " << aLayerID);
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
                  B2DEBUG(11, "I have found a SecID: " << aSecID);

                  double v1 = 0, v2 = 0, u1v1 = 0, u1v2 = 0, u2v1 = 0, u2v2 = 0;
                  sectorEdgeV1 = m_PARAMsectorConfigV[k] * vSize1 * 2 - vSize1;
                  sectorEdgeV2 = m_PARAMsectorConfigV[k + 1] * vSize1 * 2 - vSize1;
                  uSizeAtv1 = 0.5 * aSensorInfo.getUSize(sectorEdgeV1);
                  uSizeAtv2 = 0.5 * aSensorInfo.getUSize(sectorEdgeV2);
                  sectorEdgeU1OfV1 = m_PARAMsectorConfigU[j] * uSizeAtv1 * 2 - uSizeAtv1;
                  sectorEdgeU1OfV2 = m_PARAMsectorConfigU[j] * uSizeAtv2 * 2 - uSizeAtv2;
                  sectorEdgeU2OfV1 = m_PARAMsectorConfigU[j + 1] * uSizeAtv1 * 2 - uSizeAtv1;
                  sectorEdgeU2OfV2 = m_PARAMsectorConfigU[j + 1] * uSizeAtv2 * 2 - uSizeAtv2;

                  v1 = sectorEdgeV1;
                  v2 = sectorEdgeV2;
                  u1v1 = sectorEdgeU1OfV1;
                  u1v2 = sectorEdgeU1OfV2;
                  u2v1 = sectorEdgeU2OfV1;
                  u2v2 = sectorEdgeU2OfV2;

                  B2DEBUG(1000, "Sector edges: O(" << u1v1 << "," << v1 << "), U(" << u2v1 << "," << v1 << "), V(" << u1v2 << "," << v2 << "), UV(" << u2v2 << "," << v2 << ")")

                  aSectorName = (boost::format("%1%_%2%_%3%") % aLayerID % aUniID % aSecID).str();
                  if (m_sectorMap.find(aSectorName) == m_sectorMap.end()) {
                    Sector newSector(v1, v2, u1v1, u1v2, u2v1, u2v2, aSectorName);
                    m_sectorMap.insert(make_pair(aSectorName, newSector));
                  } else {
                    m_sectorMap.find(aSectorName)->second.increaseCounter();
                  }
                }
              }
            }
          } //sector-searching loop

          if (aLayerID <= m_PARAMhighestAllowedLayer) {
            B2DEBUG(11, "adding new SVD hit of track " << iPart)
            VXDHit newHit(Const::SVD, aSectorName, hitGlobal, pGlobal, pdg); // (1 = SVD hit)
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
      VXDHit newHit(Const::IR, aSectorName, mcvertex, mcmom, mcPdg);
      newHit.setVertex();
      newTrack.addHit(newHit); /// OPTIONAL: instead of attaching virtual hit only to track, it could be attached to tracklets, but in this case, a tracklet (curler) recognition using 2D-Filters has to be implemented

      newTrack.sort(); // now all the hits are ordered by their global time, which allows cutting the tracks into tracklets where the momenta of all hits point into the same direction.

      tracksOfEvent.push_back(newTrack);

      list<VXDHit> thisTrack = newTrack.getTrack();
      if (thisTrack.size() > 30) { B2WARNING("event: " << m_eventCounter << " beware, tracklength is " << thisTrack.size()) }
      if (int (thisTrack.size()) > m_numOfLayers * 2) { m_longTrackCounter++; }
      for (list<VXDHit>::iterator it = thisTrack.begin() ; it != thisTrack.end(); it++) {
        string currentSector = it->getSectorID();
        B2DEBUG(11, "track has a hit in the following sector: " << currentSector)
      }
      list<VXDHit>::reverse_iterator riter;
      for (riter = thisTrack.rbegin(); riter != thisTrack.rend();) {

        VXDTrack newTracklet(newTrack.getParticleID());

        bool direction = riter->getParticleMovement();
        B2DEBUG(11, "adding Hit to tracklet: ")
        newTracklet.addHit(*riter);

        B2DEBUG(11, "copying track-segment into tracklet")
        riter++;
        if (riter != thisTrack.rend()) {   //
          if (direction == riter->getParticleMovement()) {
            while (direction == riter->getParticleMovement()) {
              if (riter != thisTrack.rend()) {
                newTracklet.addHit(*riter);
                riter++;
              } else { break; }
            }
          }
        }


        m_trackletLengthCounter[newTracklet.size() - 1] ++;

        if (newTracklet.size() >= m_PARAMminTrackletLength) {
          if (direction == true) { // in that case the momentum vector of the tracklet points away from the IP -> outward movement
            newTracklet.reverse(); // ->inward "movement" of the hits, needed for the filtering, no presorting needed, the hits were in the right order.
            B2DEBUG(11, " change direction of tracklet...")
          }

//          newTracklet.addHit(newHit); /// adding virtual hit to tracklet!

          B2DEBUG(11, "adding tracklet to list")
          trackletsOfEvent.push_back(newTracklet);

          if (newTracklet.size() > m_numOfLayers * 2 + 1) {
            m_longTrackletCounter++;
          }
        } else { B2DEBUG(11, "tracklet too small -> discarded") }

        if (m_PARAMnoCurler == true) {  // do not store following tracklets, when no curlers shall be recorded...
          riter = thisTrack.rend();
        }
      } // slicing current track into moouth sized tracklets
    } // check whether current particle (and all its hits) belong to a primary particle
  } // looping through particles

  B2DEBUG(1, "finished tracklet-generation. " << trackletsOfEvent.size() << " tracklets and " << tracksOfEvent.size() << " tracks found")

  string currentSector, friendSector;
//4hit-variables:
  TVector3 hitG, moHitG, graMoHitG, greGraMoHitG, segA, segB, segC, centralPointA, centralPointB, centralPointC, nA, nB, nC, cpA, cpB, cpC, intersectionAB, intersectionBC;
  double muVal, nuVal, normedAB1, normedAB2, normedAB3, normedBC1, normedBC2, normedBC3, inverseKappaAB, inverseKappaBC, pTAB, pTBC, dist2IPAB, dist2IPBC, deltaDist2IP, deltapT;
// 3hit-variables:
  TVector3 motherHitGlobal, grandMotherHitGlobal, segmentVector1, segmentVector2, segVec1, segVec2;
  double dist2IP, tempR1, seg1Length, tempR2, seg2Length, distanceDeltaZ, angles3D, anglesXY, anglesRZ;
// 2hit-variables:
  TVector3 segmentVector;
  double distanceXY, distanceZ, distance3D, distanceR, normedDistance3D;

  for (int i = 0; i < int(trackletsOfEvent.size()); i++) {

    bool firstrun = true, secondrun = true, thirdrun = true;
    list<VXDHit> aTracklet = trackletsOfEvent[i].getTrack();
    list<VXDHit>::iterator iter = aTracklet.begin(); // main iterator looping through the whole tracklet, "innermost Hit"
    list<VXDHit>::iterator it2HitsFilter = aTracklet.begin(); //important for 2hit-Filters: points to current hit of 2-hit-processes " next to innermost hit"
    list<VXDHit>::iterator it3HitsFilter = aTracklet.begin(); // -"- 3hit-Filters...
    list<VXDHit>::iterator it4HitsFilter = aTracklet.begin(); // -"- 4hit-Filters... "outermost Hit"

    B2DEBUG(11, "executing " << i + 1 << "th tracklet with size " << aTracklet.size())
    for (list<VXDHit>::iterator it = aTracklet.begin() ; it != aTracklet.end(); ++it) {
      currentSector = it->getSectorID();
      B2DEBUG(11, "tracklet has a hit in the following sector: " << currentSector)
    }

    MapOfSectors::iterator thisSectorPos;
    for (; iter != aTracklet.end(); ++iter) {

      if (firstrun == false) {
        B2DEBUG(11, "calculating 2-hit-filters")
        if (secondrun == false) {
          B2DEBUG(11, "calculating 3-hit-filters")
          if (thirdrun == false) {
            B2DEBUG(11, "calculating 4-hit-filters")
            currentSector = it4HitsFilter->getSectorID();
            friendSector = it3HitsFilter->getSectorID();

            hitG = it4HitsFilter->getHitPosition();
            hitG.SetZ(0.);
            moHitG = it3HitsFilter->getHitPosition();
            moHitG.SetZ(0.);
            graMoHitG = it2HitsFilter->getHitPosition();
            graMoHitG.SetZ(0.);
            greGraMoHitG = iter->getHitPosition();
            greGraMoHitG.SetZ(0.);

            segA = moHitG - hitG;
            segB = graMoHitG - moHitG;
            segC = greGraMoHitG - graMoHitG;
            centralPointA = 0.5 * segA + hitG; //([kx ky]  -[jx jy])/2 + [jx jy] = central point of innerSegment (k-j)/2+j
            centralPointB = 0.5 * segB + moHitG; // = central point of mediumSegment (l-k)/2+k
            centralPointC = 0.5 * segC + graMoHitG; // = central point of outerSegment (m-l)/2+l
            nA.SetXYZ(-segA(1), segA(0), 0.); //normal vector of segment a
            nB.SetXYZ(-segB(1), segB(0), 0.); //normal vector of segment b
            nC.SetXYZ(-segC(1), segC(0), 0.); //normal vector of segment c -> alternative: v2 = v1.Orthogonal(); get vector orthogonal to v1

            int signCurvature = boost::math::sign(nB * segA);

            B2DEBUG(11, "sign of particle (calculated): " << signCurvature << ", pdgCode: " << it4HitsFilter->getPDG())

            cpA = centralPointA;
            cpB = centralPointB;
            cpC = centralPointC;
            muVal = (((cpA(1) - cpB(1)) / nB(1)) + (((cpB(0) - cpA(0)) / nA(0)) * nA(1) / nB(1))) / (1. - ((nB(0) / nA(0)) * (nA(1) / nB(1))));
            nuVal = ((cpB(1) - cpC(1)) / nC(1) + ((cpC(0) - cpB(0)) / nB(0)) * nB(1) / nC(1)) / (1. - nC(0) / nB(0) * nB(1) / nC(1));
            intersectionAB.SetX(cpB(0) + muVal * nB(0)); // x-coord of intersection point
            intersectionAB.SetY(cpB(1) + muVal * nB(1)); // y-coord of intersection point
            intersectionAB.SetZ(0.);
            intersectionBC.SetX(cpC(0) + nuVal * nC(0));
            intersectionBC.SetY(cpC(1) + nuVal * nC(1));
            intersectionBC.SetZ(0.);

            normedAB1 = (intersectionAB - hitG).Mag();
            normedAB2 = (intersectionAB - moHitG).Mag();
            normedAB3 = (intersectionAB - graMoHitG).Mag();
            normedBC1 = (intersectionBC - moHitG).Mag();
            normedBC2 = (intersectionBC - graMoHitG).Mag();
            normedBC3 = (intersectionBC - greGraMoHitG).Mag();

            inverseKappaAB = (normedAB1 + normedAB2 + normedAB3) / 3.0; // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1
            inverseKappaBC = (normedBC1 + normedBC2 + normedBC3) / 3.0; // = radius in [cm], sign here not needed. normally: signKappaBC/normBC1

            thisSectorPos = m_sectorMap.find(currentSector);

            if (m_PARAMlogDeltaPt == true) {
              pTAB = 0.45 * inverseKappaAB * 0.01; // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100
              pTBC = 0.45 * inverseKappaBC * 0.01;

              deltapT = abs(abs(pTAB) - abs(pTBC));

              if (isnan(deltapT) == false) {
                thisSectorPos->second.addValue(friendSector, "deltaPt", deltapT);
              } else { m_badFilterValueCtr++; }

              if (m_PARAMtrackErrorTracks == true) {
                if (pTAB > 5.0 * pMaxInMeV * 0.001 || pTAB < 0.2 * pMinInMeV * 0.001) { /** pTAB - inner pair of segments **/
                  B2WARNING(" at event " << m_eventCounter << " there is a problem with filter deltaPt (AB): " << pTAB << "GeV/c");
                } else if (pTBC > 5.0 * pMaxInMeV * 0.001 || pTBC < 0.2 * pMinInMeV * 0.001) { /** pTBC - outer pair of segments **/
                  B2WARNING(" at event " << m_eventCounter << " there is a problem with filter deltaPt (BC): " << pTBC << "GeV/c");
                }
              }
            }

            if (m_PARAMlogDeltaTRadiustoIPDistance == true) {
              dist2IPAB = abs(intersectionAB.Mag() - inverseKappaAB); // distance between interaction point and intersection point.
              dist2IPBC = abs(intersectionBC.Mag() - inverseKappaBC); // distance between interaction point and intersection point.

              deltaDist2IP = abs(dist2IPAB - dist2IPBC);

              if (isnan(deltaDist2IP) == false) {
                thisSectorPos->second.addValue(friendSector, "deltaDist2IP", deltaDist2IP);
              } else { m_badFilterValueCtr++; }
            }

            ++it4HitsFilter; // -"- 4hit-Filters...
          } else { thirdrun = false; }

          currentSector = it3HitsFilter->getSectorID();
          friendSector = it2HitsFilter->getSectorID();
          thisSectorPos = m_sectorMap.find(currentSector);

          hitGlobal = it3HitsFilter->getHitPosition();
          motherHitGlobal = it2HitsFilter->getHitPosition();
          grandMotherHitGlobal = iter->getHitPosition();
          segmentVector1 = motherHitGlobal - hitGlobal;
          segmentVector2 = grandMotherHitGlobal - motherHitGlobal;

          if (m_PARAMlogTRadiustoIPDistance == true or m_PARAMlogPt == true) {
            hitG = hitGlobal;
            hitG.SetZ(0.);
            moHitG = motherHitGlobal;
            moHitG.SetZ(0.);
            graMoHitG = grandMotherHitGlobal;
            graMoHitG.SetZ(0.);

            segA = moHitG - hitG;
            segB = graMoHitG - moHitG;
            centralPointA = 0.5 * segA + hitG; //([kx ky]  -[jx jy])/2 + [jx jy] = central point of innerSegment (k-j)/2+j
            centralPointB = 0.5 * segB + moHitG; // = central point of outerSegment (l-k)/2+k
            nA.SetXYZ(-segA(1), segA(0), 0.); //normal vector of segment a
            nB.SetXYZ(-segB(1), segB(0), 0.); //normal vector of segment b

            cpA = centralPointA;
            cpB = centralPointB;
            muVal = (((cpA(1) - cpB(1)) / nB(1)) + (((cpB(0) - cpA(0)) / nA(0)) * nA(1) / nB(1))) / (1. - ((nB(0) / nA(0)) * (nA(1) / nB(1))));
            intersectionAB.SetX(cpB(0) + muVal * nB(0)); // x-coord of intersection point
            intersectionAB.SetY(cpB(1) + muVal * nB(1)); // y-coord of intersection point
            intersectionAB.SetZ(0.);

            normedAB1 = (intersectionAB - hitG).Mag();
            normedAB2 = (intersectionAB - moHitG).Mag();
            normedAB3 = (intersectionAB - graMoHitG).Mag();

            inverseKappaAB = (normedAB1 + normedAB2 + normedAB3) * 0.3333333; // = radius in [cm], sign here not needed. normally: signKappaAB/normAB1

            if (m_PARAMlogTRadiustoIPDistance == true) {
              dist2IP = abs(intersectionAB.Mag() - inverseKappaAB); // distance between interaction point and intersection point.

              if (isnan(dist2IP) == false) {
                thisSectorPos->second.addValue(friendSector, "dist2IP", dist2IP);
              } else { m_badFilterValueCtr++; }
            }
            if (m_PARAMlogPt == true) {
              pTAB = 0.45 * inverseKappaAB * 0.01; // pT[GeV/c] = 0.3*B[T]*r[m] = 0.45*r[cm]/100

              if (isnan(pTAB) == false) {
                thisSectorPos->second.addValue(friendSector, "pT", pTAB);
              } else { m_badFilterValueCtr++; }
            }
          }

          if (m_PARAMlogDeltaZ == true) {
            tempR1 = sqrt(segmentVector1[0] * segmentVector1[0] + segmentVector1[1] * segmentVector1[1]);
            segVec1.SetXYZ(tempR1, segmentVector1[2], 0.);
            seg1Length = segVec1.Mag();
            tempR2 = sqrt(segmentVector2[0] * segmentVector2[0] + segmentVector2[1] * segmentVector2[1]);
            segVec2.SetXYZ(tempR2, segmentVector2[2], 0.);
            seg2Length = segVec2.Mag();

            distanceDeltaZ = segVec2(1) / seg2Length - segVec1(1) / seg1Length;

            if (isnan(distanceDeltaZ) == false) {
              thisSectorPos->second.addValue(friendSector, "distanceDeltaZ", distanceDeltaZ);
            } else { m_badFilterValueCtr++; }
          }

          if (m_PARAMlogAngles3D == true) {
            seg1Length = segmentVector1.Mag();
            seg2Length = segmentVector2.Mag();

            angles3D = acos((segmentVector1.Dot(segmentVector2)) / (seg1Length * seg2Length));

            if (isnan(angles3D) == false) {
              thisSectorPos->second.addValue(friendSector, "angles3D", angles3D);
            } else { m_badFilterValueCtr++; }
          }

          if (m_PARAMlogAnglesXY == true) {
            segVec1.SetXYZ(segmentVector1[0], segmentVector1[1], 0.);
            seg1Length = segVec1.Mag();
            segVec2.SetXYZ(segmentVector2[0], segmentVector2[1], 0.);
            seg2Length = segVec2.Mag();

            anglesXY = acos((segVec1.Dot(segVec2)) / (seg1Length * seg2Length));

            if (isnan(anglesXY) == false) {
              thisSectorPos->second.addValue(friendSector, "anglesXY", anglesXY);
            }  else { m_badFilterValueCtr++; }
          }

          if (m_PARAMlogAnglesRZ == true) {
            tempR1 = sqrt(segmentVector1[0] * segmentVector1[0] + segmentVector1[1] * segmentVector1[1]);
            segVec1.SetXYZ(tempR1, segmentVector1[2], 0.);
            seg1Length = segVec1.Mag();
            tempR2 = sqrt(segmentVector2[0] * segmentVector2[0] + segmentVector2[1] * segmentVector2[1]);
            segVec2.SetXYZ(tempR2, segmentVector2[2], 0.);
            seg2Length = segVec2.Mag();

            anglesRZ = acos((segVec1.Dot(segVec2)) / (seg1Length * seg2Length));

            if (isnan(anglesRZ) == false) {
              thisSectorPos->second.addValue(friendSector, "anglesRZ", anglesRZ);
            } else { m_badFilterValueCtr++; }
          }

          ++it3HitsFilter; // -"- 3hit-Filters...

        } else { secondrun = false; }

        currentSector = it2HitsFilter->getSectorID();
        friendSector = iter->getSectorID();
        thisSectorPos = m_sectorMap.find(currentSector);
        string testString = "_";
        if (typeid(string).name() != typeid(friendSector).name()) {
          B2WARNING("FilterCalculator event " << m_eventCounter << ": type of friendSector is no string, aborting tracklet...")
          continue;
        }
//        if ( friendSector.at(2) != testString ) { continue; } // filters bad Sector names

        hitGlobal = it2HitsFilter->getHitPosition();
        motherHitGlobal = iter->getHitPosition();
        segmentVector = motherHitGlobal - hitGlobal;

        if (m_PARAMlogDistanceXY == true) {
          segVec1.SetXYZ(segmentVector[0], segmentVector[1], 0.);
          distanceXY = segVec1.Mag();

          if (isnan(distanceXY) == false) {
            thisSectorPos->second.addValue(friendSector, "distanceXY", distanceXY);
          } else { m_badFilterValueCtr++; }
        }

        if (m_PARAMlogDistanceZ == true) {
          segVec1.SetXYZ(0., 0., segmentVector[2]);
          distanceZ = segVec1.Mag();

          if (isnan(distanceZ) == false) {
            thisSectorPos->second.addValue(friendSector, "distanceZ", distanceZ);
          } else { m_badFilterValueCtr++; }
        }

        if (m_PARAMlogDistance3D == true) {
          distance3D = segmentVector.Mag();

          if (isnan(distance3D) == false) {
            thisSectorPos->second.addValue(friendSector, "distance3D", distance3D);
          } else { m_badFilterValueCtr++; }
        }

        if (m_PARAMlogNormedDistance3D == true) {
          distance3D = segmentVector.Mag();
          segVec1.SetXYZ(segmentVector[0], segmentVector[1], 0.);
          distanceR = segVec1.Mag();

          normedDistance3D = distance3D / distanceR;
          if (isnan(normedDistance3D) == false) {
            thisSectorPos->second.addValue(friendSector, "normedDistance3D", normedDistance3D);
          } else { m_badFilterValueCtr++; }
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
  for (int i = 0; i < int(m_trackletLengthCounter.size()); i++) {
    B2INFO(" within " << m_eventCounter << " events we got " << m_trackletLengthCounter[i] << " tracklets containing " << i + 1 << " hits")
    totalTrackletCounter += m_trackletLengthCounter[i];
    totalHitCounter += m_trackletLengthCounter[i] * (i + 1);
  }

  B2INFO(" there were " << float(totalTrackletCounter) / float(m_eventCounter) << " tracklets per event...")
  B2INFO(" there were " << m_longTrackCounter << " Tracks having more than " << m_numOfLayers * 2 << " hits...")
  B2INFO(" there were " << m_longTrackletCounter << " Tracklets having more than " << m_numOfLayers * 2 << " hits!!!")
  B2INFO(" totalGlobalCoordValue: " << m_totalGlobalCoordValue << ", totalLocalCoordValue: " << m_totalLocalCoordValue << ", of " << totalHitCounter << " hits total (" << m_totalHitCounter << " counted manually) ")
  B2INFO(m_badFilterValueCtr << " times, a filter produced invalid results ('nan')")
  B2INFO("~~~~~~~~~~~FilterCalculator - endRun ~~~~~~~~~~")
}


void FilterCalculatorModule::terminate()
{

  for (MapOfSectors::iterator secIter = m_sectorMap.begin(); secIter != m_sectorMap.end(); ++secIter) {
    secIter->second.exportFriends(m_PARAMsectorSetupFileName);
  }
  m_sectorMap.clear();
  B2INFO(" FilterCalculatorModule, everything is done. Terminating.")
}
