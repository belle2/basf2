/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ian J. Watson                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcToVXDExtrapolator/CKFCdcToVxdModule.h>
#include <tracking/modules/cdcToVXDExtrapolator/CKF.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <framework/geometry/B2Vector3.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationArray.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <genfit/FieldManager.h>

#include <genfit/Track.h>
#include <genfit/TrackCand.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsKalmanFitter.h>
#include <genfit/KalmanFitter.h>
#include <genfit/DAF.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/KalmanFitterRefTrack.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/Exception.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>

#include <vxd/geometry/GeoCache.h>

#include "TFile.h"

#include <functional>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CKFCdcToVxd)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CKFCdcToVxdModule::CKFCdcToVxdModule() :
  Module(),
  m_saveInfo(false),
  nCdcRecoOnlyTracks(0),
  nCdcRecoOnlyTracksWithVxdMcHits(0),
  nTracksWithAddedHits(0),
  nTracksWithAddedPxdHits(0),
  nStoredTracks(0),
  HitInfo(0),
  TrackInfo(0),
  m_outputFile(0),
  cdcHitsFromMC(0),
  svdClustersFromMC(0),
  pxdClustersFromMC(0)
{
  // Set module properties
  setDescription("For the given track collection input, finds the set of CDC-only tracks, tries to find VXD hits compatible with the tracks.");

  // Parameter definitions
  addParam("RootOutputFilename", m_rootOutputFilename,
           "Filename for the ROOT file of module information. If \"\" then won't output ROOT file.", std::string(""));
  addParam("MaxHoles", m_maxHoles, "maximum number of holes per added hit in the CKF", 2);
  addParam("MaxChi2Increment", m_maxChi2Increment, "maximum chi2 per added hit in the CKF", 10.0);
  addParam("HolePenalty", m_holePenalty, "Effective chi2/ndof penalty in CKF quality for a hole", 10.0);
  addParam("NMax", m_Nmax, "Maximum number of propagated candidates per step in CKF", 10);
  addParam("HitMultiplier", m_hitMultiplier, "Extra weight multiplier for hits", 1.0);

  // Input
  addParam("GFTrackColName", m_GFTrackColName, "Name of genfit::Track collection (input)", std::string(""));
  addParam("McParticlesColName", m_mcParticlesColName, "Name of MC particles collections", std::string(""));

  // Output
  addParam("OutGFTrackColName", m_OutGFTrackColName, "Name of genfit::Track collection (output)", std::string(""));
  addParam("OutGFTrackCandColName", m_OutGFTrackCandColName, "Name of genfit::TrackCand collection (output)", std::string(""));
}

// account for trapezoidal layers
static inline TVector3 getGlobalPosition(double u, double v, const  VXD::SensorInfoBase& aSensorInfo)
{
  if ((aSensorInfo.getBackwardWidth() > aSensorInfo.getForwardWidth()) == true) { // isWedgeSensor
    double uWedged = (aSensorInfo.getWidth(v) / aSensorInfo.getWidth()) * u;
    return aSensorInfo.pointToGlobal(TVector3(uWedged, v, 0));
  } else {
    return aSensorInfo.pointToGlobal(TVector3(u, v, 0));
  }
}

static bool extrapolateToCylinder(float radius, genfit::Track* track, TVector3& posAtLayer, TVector3& momAtLayer,
                                  TMatrixDSym& covAtLayer)
{
  static const TVector3 origin(0, 0, 0);
  static const TVector3 zaxis(0, 0, 1);

  // assumes the track is ordered correctly
  genfit::MeasuredStateOnPlane mop;
  try {
    mop = track->getFittedState(0);
  } catch (...) {
    B2WARNING("Track without any fitted states!");
    return false;
  }
  // need to check the extrapolation worked?
  mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
  try {
    mop.extrapolateToCylinder(radius, origin, zaxis);
  } catch (...) {
    B2DEBUG(90, "Cylinder extrapolation failed! R:" << radius);
    // extrapolation failed
    return false;
  }
  B2DEBUG(171, "Old posAtLayer " << posAtLayer.X() << " " << posAtLayer.Y() << " " << posAtLayer.Z());
  mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
  B2DEBUG(171, "New posAtLayer " << posAtLayer.X() << " " << posAtLayer.Y() << " " << posAtLayer.Z());
  return true;
}

bool CKFCdcToVxdModule::extrapolateToPXDLayer(genfit::Track* track, std::vector<CKFPartialTrack*>&, unsigned searchLayer,
                                              StoreArray<PXDCluster>& clusters,
                                              std::vector<genfit::AbsMeasurement*>& hits)
{
  B2DEBUG(95, "<----> PXD extrapolateToLayer " << searchLayer << " <---->");

  static float layerRadii[7] = {0, 1.421, 2.179, 3.799, 8.0, 10.4, 13.51};

  // assumes the track is ordered correctly
  TVector3 posAtLayer, momAtLayer;
  TMatrixDSym covAtLayer(6);
  if (!extrapolateToCylinder(layerRadii[searchLayer], track, posAtLayer, momAtLayer, covAtLayer)) {
    return false;
  }

  B2DEBUG(60, "Extrapolating to pixel layer: Pos:"
          << posAtLayer.X() << "," << posAtLayer.Y() << "," << posAtLayer.Z()
          << "Mom:" << momAtLayer.X() << "," << momAtLayer.Y() << "," << momAtLayer.Z());
  std::map<VxdID, TVector3> sensor2pos;
  std::map<VxdID, double> sensor2zSig;
  std::map<VxdID, double> sensor2xySig;
  for (int i = 0; i < clusters.getEntries(); ++i) {
    const auto& cluster = clusters[i];
    if (cluster->getSensorID().getLayerNumber() != searchLayer) continue;
    auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(cluster->getSensorID());
    TVector3 pos = getGlobalPosition(cluster->getU(), cluster->getV(), aSensorInfo);
    // check within a layer width/length
    double sensorWidth = aSensorInfo.getWidth() / 2.;
    TVector3 midpos = getGlobalPosition(0, cluster->getV(), aSensorInfo);
    TVector3 wpos = getGlobalPosition(sensorWidth, cluster->getV(), aSensorInfo);
    sensorWidth = fabs(midpos.DeltaPhi(wpos));

    float dz = pos.Z() - posAtLayer.Z();
    float dxy = fabs(posAtLayer.DeltaPhi(pos));
    B2DEBUG(90, "---- findHits(): Pix cluster position: " << pos.X() << " " << pos.Y() << " " << pos.Z());
    B2DEBUG(101, "Good cluster. dZ:" << dz << " dXY:" << dxy << " swidth:" << sensorWidth << " lng:" << aSensorInfo.getLength());
    if ((fabs(dz) < aSensorInfo.getLength() / 2) && (dxy < sensorWidth)) {
      auto nxtHit = new PXDRecoHit(cluster);
      nxtHit->setHitId(hitCount++);
      hits.push_back(nxtHit);
    }
  }

  B2DEBUG(95, "<----> End PXD extrapolateToLayer " << searchLayer << " <---->");
  return true;
}

bool CKFCdcToVxdModule::extrapolateToSVDLayer(genfit::Track* track, std::vector<CKFPartialTrack*>&, unsigned searchLayer,
                                              StoreArray<SVDCluster>& clusters,
                                              bool isU, std::vector<genfit::AbsMeasurement*>& hits)
{
  B2DEBUG(95, "<----> extrapolateToLayer " << searchLayer << " <---->");
  TVector3 zaxis(0, 0, 1);
  TVector3 mzaxis(0, 0, -1);
  TVector3 origin(0, 0, 0);
  // 1-indexed layer radii, numbers from xml files, should actually load them from these numbers?
  static float layerRadii[7] = {0, 1.421, 2.179, 3.799, 8.0, 10.4, 13.51};
  // similarly for the boundaries
  //static float boundaryZ[7] = {0, 1.421, 2.179, 3.799, 8.0, 10.4, 13.515};
  // for the layers with bent parts, at what z does the bent part start?
  static float bentZ[7] = {0, 0, 0, 0, 8.94, 17.51, 24.61};
  // and what is the opening angle of the "cone" created by the bent layers
  const float degToRad = 3.14159265 / 180.0;
  static float openingAngle[7] = {0, 0, 0, 0, 11.9f * degToRad, 16.0f * degToRad, 21.1f * degToRad};

  // assumes the track is ordered correctly
  TVector3 posAtLayer, momAtLayer;
  TMatrixDSym covAtLayer(6);
  if (!extrapolateToCylinder(layerRadii[searchLayer], track, posAtLayer, momAtLayer, covAtLayer)) {
    return false;
  }

  // check for slant layer
  //  bool isCone = false;
  if ((searchLayer > 3) && (posAtLayer.Z() > bentZ[searchLayer])) {
    // extrapolate to cone
    //isCone = true;

    /// at bentZ[layer], the radius should match the cylinder, and given the opening angle, should know the beginning of the cone
    genfit::MeasuredStateOnPlane mop;
    auto coneVertex = TVector3(0, 0, bentZ[searchLayer] + layerRadii[searchLayer] / tan(openingAngle[searchLayer]));
    try {
      mop = track->getFittedState(0);
    } catch (...) {
      B2WARNING("Track without any fitted states after search!");
      return false;
    }
    try {
      B2DEBUG(190, "extrapolate to cone on layer " << searchLayer << " svd. On cylinder: " << posAtLayer.X() << " " << posAtLayer.Y() <<
              " " << posAtLayer.Z() << " " << " ConeV: " << coneVertex.Z() << " openingA: " << openingAngle[searchLayer]);
      mop.extrapolateToCone(openingAngle[searchLayer], coneVertex, mzaxis);
    } catch (...) {
      B2DEBUG(90, ">>>>> Conical extrapolation failed!");
    }
    mop.getPosMomCov(posAtLayer, momAtLayer, covAtLayer);
    B2DEBUG(190, ">>>>>>>>>>>> extrapolate to cone on layer " << searchLayer << " svd. On cone: " << posAtLayer.X() << " " <<
            posAtLayer.Y() << " " << posAtLayer.Z());
  }

  B2DEBUG(190, "end of extrapolation");
  B2DEBUG(190, "extrapolated to layer " << searchLayer << " svd");
  B2DEBUG(101, "   ------------<> Extrapolation result: Position " << posAtLayer.x() << " " << posAtLayer.y() << " " << posAtLayer.z()
          <<
          " Momentum " << momAtLayer.x() << " " << momAtLayer.y() << " " << momAtLayer.z() << " Cov_ii " << sqrt(covAtLayer[0][0]) << " " <<
          sqrt(covAtLayer[1][1])
          << " " << sqrt(covAtLayer[2][2]) << " " << sqrt(covAtLayer[3][3]) << " " << sqrt(covAtLayer[4][4]) << " " << sqrt(
            covAtLayer[5][5]) << " cov01:" << covAtLayer[0][1] << " cov02:" << covAtLayer[0][2] << " cov12:" << covAtLayer[1][2]);

  double r = sqrt(pow(posAtLayer.X(), 2) + pow(posAtLayer.Y(), 2));
  B2DEBUG(190, "   --->Track Position r: " << r << " " << posAtLayer.Perp() << " z:" << posAtLayer.Z() <<
          " zsigma:" << sqrt(covAtLayer[2][2]) << " VXX:" << covAtLayer[0][0] << " sXX:" << sqrt(covAtLayer[0][0]) << " VYY:" <<
          covAtLayer[1][1] << " sYY:" << sqrt(covAtLayer[1][1]) << " VXY:" << covAtLayer[0][1]);
  // try to find a v cluster

  /// look for a v cluster
  if (!isU) {
    for (int i = 0; i < clusters.getEntries(); ++i) {
      const auto& cluster = clusters[i];

      if (cluster->isUCluster()) continue;
      if (cluster->getSensorID().getLayerNumber() != searchLayer) continue;

      auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(cluster->getSensorID());
      TVector3 pos = getGlobalPosition(0, cluster->getPosition(), aSensorInfo);
      double sensorWidth = aSensorInfo.getWidth() / 2.;
      TVector3 midpos = getGlobalPosition(0, cluster->getPosition(), aSensorInfo);
      TVector3 wpos = getGlobalPosition(sensorWidth, cluster->getPosition(), aSensorInfo);
      sensorWidth = fabs(midpos.DeltaPhi(wpos));

      B2DEBUG(221, "Looking for V cluster dz: " << fabs(pos.Z() - posAtLayer.Z()) << " dphi:" <<
              fabs(posAtLayer.DeltaPhi(pos)) << " dz:" << fabs(pos.Z() - posAtLayer.Z()));
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 221, PACKAGENAME()) == true) {
        pos.Print();
      }

      double absdz = fabs(pos.Z() - posAtLayer.Z());
      double dxy = fabs(posAtLayer.DeltaPhi(pos));
      if ((absdz < (aSensorInfo.getLength() / 2.)) && (dxy < sensorWidth)) {

        bool truHit = false;
        for (auto& clu : *svdClustersFromMC) {
          if ((cluster->getSensorID() == clu.getSensorID()) && (clu.isUCluster() == cluster->isUCluster())
              && (clu.getPosition() == cluster->getPosition())) {
            truHit  = true;
            break;
          }
        }


        B2DEBUG(90, "---- findHits(): V: " << pos.X() << " " << pos.Y() << " " << pos.Z() << " : " << aSensorInfo.getWidth() / 2. << " T "
                << truHit << " id:" << cluster->getSensorID());
        auto nxtHit = new SVDRecoHit(cluster);
        nxtHit->setHitId(hitCount++);
        hits.push_back(nxtHit);
      }
    }
  } else { // look for a u cluster
    for (int i = 0; i < clusters.getEntries(); ++i) {
      const auto& cluster = clusters[i];

      if (cluster->getSensorID().getLayerNumber() != searchLayer) continue;
      if (!cluster->isUCluster()) continue;
      auto aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(cluster->getSensorID());
      double sensorWidth = aSensorInfo.getWidth() / 2.;
      TVector3 midpos = getGlobalPosition(0, 0, aSensorInfo);
      TVector3 wpos = getGlobalPosition(sensorWidth, 0, aSensorInfo);
      sensorWidth = fabs(midpos.DeltaPhi(wpos));

      // only hits on the same sensor
      TVector3 pos = getGlobalPosition(cluster->getPosition(), 0, aSensorInfo);
      double dxy = fabs(posAtLayer.DeltaPhi(pos));
      double absdz = fabs(pos.Z() - posAtLayer.Z());
      if ((dxy < sensorWidth) && (absdz < (aSensorInfo.getLength() / 2.))) {
        bool truHit = false;
        for (auto& clu : *svdClustersFromMC) {
          if ((cluster->getSensorID() == clu.getSensorID()) && (clu.isUCluster() == cluster->isUCluster())
              && (clu.getPosition() == cluster->getPosition())) {
            truHit  = true;
            break;
          }
        }

        B2DEBUG(90, "---- findHits(): U: " << pos.X() << " " << pos.Y() << " " << pos.Z() << " : " << aSensorInfo.getLength() / 2. << " T "
                << truHit << " id:" << cluster->getSensorID());
        auto nxtHit = new SVDRecoHit(cluster);
        nxtHit->setHitId(hitCount++);
        hits.push_back(nxtHit);
      }
    }
  }
  return true;
}

bool CKFCdcToVxdModule::findHits(genfit::Track* track, std::vector<CKFPartialTrack*>& current, unsigned counter,
                                 std::vector<genfit::AbsMeasurement*>& hits, void* data)
{
  CKFCdcToVxdModule* module = (CKFCdcToVxdModule*) data;
  if ((counter == 0) || (counter == 1))
    return module->extrapolateToSVDLayer(track, current, 6, *module->svdClusters, (counter % 2) == true, hits);
  if ((counter == 2) || (counter == 3))
    return module->extrapolateToSVDLayer(track, current, 5, *module->svdClusters, (counter % 2) == true, hits);
  if ((counter == 4) || (counter == 5))
    return module->extrapolateToSVDLayer(track, current, 4, *module->svdClusters, (counter % 2) == true, hits);
  if ((counter == 6) || (counter == 7))
    return module->extrapolateToSVDLayer(track, current, 3, *module->svdClusters, (counter % 2) == true, hits);
  if (counter == 8)
    return module->extrapolateToPXDLayer(track, current, 2, *module->pxdClusters, hits);
  if (counter == 9)
    return module->extrapolateToPXDLayer(track, current, 1, *module->pxdClusters, hits);
  return false;
}

void CKFCdcToVxdModule::initialize()
{
  B2DEBUG(80, "Entered CKFCdcToVxdModule::initialize()");
  // Input
  StoreArray<MCParticle>::required(m_mcParticlesColName);
  StoreArray<genfit::Track>::required(m_GFTrackColName);
  StoreArray<CDCHit>::required();
  StoreArray<SVDCluster>::required();
  StoreArray<PXDTrueHit>::required();
  StoreArray<MCParticle> mcParticles;
  mcParticles.isOptional();
  // output
  StoreArray<genfit::Track>::registerPersistent(m_OutGFTrackColName);
  StoreArray<genfit::TrackCand>::registerPersistent(m_OutGFTrackCandColName);

  // register relations
  StoreArray<genfit::Track> gftracks(m_OutGFTrackColName);
  StoreArray<genfit::TrackCand> gfTrackCands(m_OutGFTrackCandColName);
  gfTrackCands.registerRelationTo(gftracks);
  gftracks.registerRelationTo(gfTrackCands);
  gftracks.registerRelationTo(mcParticles);

  // setup magnetic field
  if (!genfit::FieldManager::getInstance()->isInitialized()) {
    B2WARNING("Magnetic field not set up, doing this myself.");

    // pass the magnetic field to genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::FieldManager::getInstance()->useCache();
  }


  m_saveInfo = false;
  if (m_rootOutputFilename != std::string("")) {
    m_outputFile = new TFile(m_rootOutputFilename.c_str(), "recreate");

    m_saveInfo = true;

    HitInfo = new TTree("HitInfo", "HitInfo");
    HitInfo->Branch("stHitInfo", &stHitInfo,
                    "z/F:rphi/F:pull_z/F:pull_rphi/F:chi2Inc/F:du/F:dv/F:dw/F:layer/I:cone/I:tru/I:isPxd/O");
    TrackInfo = new TTree("TrackInfo", "TrackInfo");
    TrackInfo->Branch("trkInfo", &trkInfo,
                      "StartHitIdx/i:EndHitIdx/i:pt/F:th/F:d0/F:pval/F:truPxd/i:truSvd/i:nRec/i:cdconly/O:refit/O");
  }
}

void CKFCdcToVxdModule::storeTrack(genfit::Track& crnt, StoreArray<genfit::Track>& outGfTracks,
                                   StoreArray<genfit::TrackCand>& outGfTrackCands,
                                   RelationArray& gfTrackCandidatesTogfTracks,
                                   RelationArray& gfTracksToMCPart)
{
  // store gftrack
  genfit::Track* extrapolatedTrack = outGfTracks.appendNew(crnt);

  // store the TrackCand
  genfit::TrackCand* theCand = extrapolatedTrack->constructTrackCand();
  //genfit::TrackCand* extrCand =
  outGfTrackCands.appendNew(*theCand);
  delete theCand;
  gfTrackCandidatesTogfTracks.add(nOutTracks, nOutTracks);

  gfTracksToMCPart.add(nOutTracks, crnt.getMcTrackId());

  nOutTracks++;
  nStoredTracks++;
}

void getHits(genfit::Track& track, uint& nPxd, uint& nSvd2, uint& nSvd, uint& nCdc, uint& nUnk)
{
  nPxd = 0, nSvd2 = 0, nSvd = 0, nCdc = 0, nUnk = 0;
  for (auto pt : track.getPointsWithMeasurement()) {
    for (auto absMeas : pt->getRawMeasurements())
      if (dynamic_cast<PXDRecoHit*>(absMeas))
        ++nPxd;
      else if (dynamic_cast<SVDRecoHit2D*>(absMeas))
        ++nSvd2;
      else if (dynamic_cast<SVDRecoHit*>(absMeas))
        ++nSvd;
      else if (dynamic_cast<CDCRecoHit*>(absMeas))
        ++nCdc;
      else
        ++nUnk;
  }
  B2DEBUG(80, "-- Track Hits: nMeas: " << track.getPointsWithMeasurement().size() << " nAll: " << track.getPoints().size()  <<
          " -- nPxd " << nPxd << " nSvd2 " << nSvd2 << " nSvd " << nSvd << " nCdc " << nCdc << " nUnk " << nUnk);
}

bool isCdcOnly(genfit::Track& track)
{
  uint nPxd = 0, nSvd2 = 0, nSvd = 0, nCdc = 0, nUnk = 0;
  getHits(track, nPxd, nSvd2, nSvd, nCdc, nUnk);
  return ((nPxd == 0) && (nSvd2 == 0) && (nSvd == 0));
}

void CKFCdcToVxdModule::event()
{
  /// --- Retrieve necessary arrays, setup relations
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(80, "<--> CKFCdcToVxdModule::event() : " << eventCounter << " <-->");
  StoreArray<genfit::Track> gfTracks(m_GFTrackColName);
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  int nTracks = gfTracks.getEntries();
  StoreArray<genfit::Track> outGfTracks(m_OutGFTrackColName);
  StoreArray<genfit::TrackCand> outGfTrackCands(m_OutGFTrackCandColName);
  RelationArray gfTrackCandidatesTogfTracks(outGfTrackCands, outGfTracks);
  RelationArray gfTracksToMCPart(outGfTracks, mcParticles);

  StoreArray<PXDCluster> pxdClusters_;
  pxdClusters = &pxdClusters_;
  StoreArray<SVDCluster> svdClusters_;
  svdClusters = &svdClusters_;
  StoreArray<CDCHit> cdcHits;
  StoreArray<SVDRecoHit> svdRecoHits;

  B2DEBUG(80, "Number of tracks in the genfit::Track StoreArray: " <<  nTracks);
  nOutTracks = 0; // number of tracks in our output array
  hitCount = 0; // unique id for each genfit::AbsMeasurement we create in findHits()

  /// --- Run the CKF over the output tracks which are CDC only
  for (genfit::Track& crnt : gfTracks) {
    B2DEBUG(110, "<---> Processing next track " << nOutTracks << " <--->");
    nTotalTracks++;

    RelationVector<MCParticle> MCParticles_fromTrack = DataStore::getRelationsWithObj<MCParticle>(&crnt);
    if (cdcHitsFromMC) delete cdcHitsFromMC;
    cdcHitsFromMC = new RelationVector<CDCHit>(DataStore::getRelationsWithObj<CDCHit>(MCParticles_fromTrack[0]));
    if (svdClustersFromMC) delete svdClustersFromMC;
    svdClustersFromMC =  new RelationVector<SVDCluster>(DataStore::getRelationsWithObj<SVDCluster>(MCParticles_fromTrack[0]));
    if (pxdClustersFromMC) delete pxdClustersFromMC;
    pxdClustersFromMC =  new RelationVector<PXDCluster>(DataStore::getRelationsWithObj<PXDCluster>(MCParticles_fromTrack[0]));
    B2DEBUG(100, "--- Number of related MC particles: " << MCParticles_fromTrack.size() << " npxd " << pxdClustersFromMC->size() <<
            " nsvd " << svdClustersFromMC->size() << " ncdc " << cdcHitsFromMC->size());

    trkInfo.cdconly = trkInfo.refit = false;
    trkInfo.nRec = 0;
    if (isCdcOnly(crnt)) {
      trkInfo.cdconly = true;
      B2DEBUG(100, "<----> CDCOnly Track Found Running CKF " << " <--->");
      CKF ckf(&crnt, findHits, (void*) this, m_maxChi2Increment, m_maxHoles, m_holePenalty, m_Nmax, m_hitMultiplier);
      genfit::Track* newTrk = ckf.processTrack();
      if (newTrk) {
        storeTrack(*newTrk, outGfTracks, outGfTrackCands, gfTrackCandidatesTogfTracks, gfTracksToMCPart);
        if (newTrk != &crnt) {
          trkInfo.refit = true;
          trkInfo.nRec = newTrk->getNumPointsWithMeasurement() - crnt.getNumPointsWithMeasurement();
          trkInfo.truSvd = svdClustersFromMC->size();
          trkInfo.truPxd = pxdClustersFromMC->size();

          // store added hit info
          for (unsigned i = 0; i < newTrk->getNumPointsWithMeasurement(); ++i) {
            genfit::AbsMeasurement* meas = newTrk->getPointWithMeasurement(i)->getRawMeasurement();
            SVDRecoHit* svd = dynamic_cast<SVDRecoHit*>(meas);
            PXDRecoHit* pxd = dynamic_cast<PXDRecoHit*>(meas);
            if (svd) {
              auto clus = svd->getCluster();
              bool truHit = false;
              for (auto& clu : *svdClustersFromMC) {
                if ((clus->getSensorID() == clu.getSensorID()) && (clu.isUCluster() == clus->isUCluster())
                    && (clu.getPosition() == clus->getPosition())) {
                  truHit  = true;
                  break;
                } else if ((clus->getSensorID() == clu.getSensorID()) && (clu.isUCluster() == clus->isUCluster())
                           && (clu.getPosition() == clus->getPosition())) {
                  truHit  = true;
                  break;
                }
              }
              stHitInfo.layer = clus->getSensorID().getLayerNumber();
              stHitInfo.tru = truHit;
              stHitInfo.isPxd = false;
              stHitInfo.chi2Inc = ckf.getChi2Inc(meas->getHitId());
              stHitInfo.pull_z = newTrk->getPointWithMeasurement(i)->getKalmanFitterInfo(newTrk->getCardinalRep())->getUpdate(
                                   -1)->getChiSquareIncrement();
              stHitInfo.pull_rphi = newTrk->getPointWithMeasurement(i)->getKalmanFitterInfo(newTrk->getCardinalRep())->getUpdate(
                                      -1)->getChiSquareIncrement();
              HitInfo->Fill();
            } else if (pxd) {
              auto cluster = pxd->getCluster();
              bool truHit = false;
              for (auto& clu : *pxdClustersFromMC) {
                if ((cluster->getSensorID() == clu.getSensorID()) && (clu.getU() == cluster->getU()) && (clu.getV() == cluster->getV()))
                  truHit  = true;
              }
              stHitInfo.layer = cluster->getSensorID().getLayerNumber();
              stHitInfo.tru = truHit;
              stHitInfo.isPxd = true;

              HitInfo->Fill();
            }
          }

          // new track created by the CKF module, and storeTrack
          // appendNew's onto the output. Need to keep crnt alive
          // because it stored in the StoreArray and referenced by
          // other modules
          delete newTrk;
        }
      } else
        storeTrack(crnt, outGfTracks, outGfTrackCands, gfTrackCandidatesTogfTracks, gfTracksToMCPart);
    } else {
      storeTrack(crnt, outGfTracks, outGfTrackCands, gfTrackCandidatesTogfTracks, gfTracksToMCPart);
    }
    delete cdcHitsFromMC; cdcHitsFromMC = 0;
    delete svdClustersFromMC; svdClustersFromMC = 0;
    delete pxdClustersFromMC; pxdClustersFromMC = 0;

    if (m_saveInfo)
      TrackInfo->Fill();
    B2DEBUG(110, "<---> Finished track " << nOutTracks << " <--->");
  }
  B2DEBUG(110, "<--> Finished Event " << eventCounter << " <-->");
}

void CKFCdcToVxdModule::terminate()
{
  B2INFO("Terminated CKFCdcToVxdModule");
  B2INFO("nTotalTracks: " << nTotalTracks);
  B2INFO("nCdcRecoOnlyTracks: " << nCdcRecoOnlyTracks);
  B2INFO("nCdcRecoOnlyTracksWithVxdMcHits: " << nCdcRecoOnlyTracksWithVxdMcHits);
  B2INFO("nTracksWithAddedHits: " << nTracksWithAddedHits);
  B2INFO("nTracksWithAddedPxdHits: " << nTracksWithAddedPxdHits);
  B2INFO("nStoredTracks: " << nStoredTracks);

  if (m_saveInfo) {
    m_outputFile->cd();

    HitInfo->Write();
    TrackInfo->Write();

    m_outputFile->Close();
  }
}

