/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/VXDDedxPID/VXDDedxPIDModule.h>
#include <reconstruction/modules/VXDDedxPID/HelixHelper.h>

#include <framework/gearbox/Const.h>

#include <vxd/geometry/GeoCache.h>

#include <genfit/MaterialEffects.h>

#include <Math/VectorUtil.h>
#include <TH2F.h>

#include <memory>
#include <cassert>
#include <cmath>
#include <algorithm>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(VXDDedxPID);

VXDDedxPIDModule::VXDDedxPIDModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  //Set module properties
  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the SVD and PXD.");

  //Parameter definitions
  addParam("useIndividualHits", m_useIndividualHits,
           "Use individual hits (true) or truncated mean (false) to determine likelihoods", false);
  addParam("onlyPrimaryParticles", m_onlyPrimaryParticles,
           "For MC only: if true, only save data for primary particles (as determined by MC truth)", false);
  addParam("usePXD", m_usePXD, "Use PXDClusters for dE/dx calculation", false);
  addParam("useSVD", m_useSVD, "Use SVDClusters for dE/dx calculation", true);
  addParam("trackDistanceThreshold", m_trackDistanceThreshhold,
           "Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off.", double(4.0));

  m_eventID = -1;
  m_trackID = 0;
}

VXDDedxPIDModule::~VXDDedxPIDModule() { }

void VXDDedxPIDModule::checkPDFs()
{
  if (m_usePXD) {
    if (not m_PXDDedxPDFs) B2FATAL("No PXD dE/dx PDF's available");
    bool ok = m_PXDDedxPDFs->checkPDFs(not m_useIndividualHits);
    if (not ok) B2FATAL("Binning or ranges of PXD dE/dx PDF's differ");
  }
  if (m_useSVD) {
    if (not m_SVDDedxPDFs) B2FATAL("No SVD Dedx PDF's available");
    bool ok = m_SVDDedxPDFs->checkPDFs(not m_useIndividualHits);
    if (not ok) B2FATAL("Binning or ranges of SVD dE/dx PDF's differ");
  }
}

void VXDDedxPIDModule::initialize()
{

  // required inputs
  m_tracks.isRequired();
  m_recoTracks.isRequired();

  //optional inputs
  m_mcparticles.isOptional();
  m_tracks.optionalRelationTo(m_mcparticles);

  if (m_useSVD)
    m_svdClusters.isRequired();
  else
    m_svdClusters.isOptional();
  if (m_usePXD)
    m_pxdClusters.isRequired();
  else
    m_pxdClusters.isOptional();

  // register dE/dx data points
  m_dedxTracks.registerInDataStore();
  m_tracks.registerRelationTo(m_dedxTracks);

  // register likelihoods
  m_dedxLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_dedxLikelihoods);

  m_SVDDedxPDFs.addCallback([this]() {checkPDFs();});
  m_PXDDedxPDFs.addCallback([this]() {checkPDFs();});
  checkPDFs();


  // create instances here to not confuse profiling
  VXD::GeoCache::getInstance();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Need to have SetupGenfitExtrapolationModule in path before this one.");
  }
}

void VXDDedxPIDModule::event()
{
  m_dedxTracks.clear();
  m_dedxLikelihoods.clear();

  // go through Tracks
  // get fitresult and gftrack and do extrapolations, save corresponding dE/dx and likelihood values
  //   get genfit::TrackCand through genfit::Track::getCand()
  //   get hit indices through genfit::TrackCand::getHit(...)
  //   create one VXDDedxTrack per fitresult/gftrack
  //create one DedkLikelihood per Track (plus rel)
  m_eventID++;

  // inputs
  const int numMCParticles = m_mcparticles.getEntries();

  // **************************************************
  //
  //  LOOP OVER TRACKS
  //
  // **************************************************

  for (const auto& track : m_tracks) {
    m_trackID++;

    std::shared_ptr<VXDDedxTrack> dedxTrack = std::make_shared<VXDDedxTrack>();
    dedxTrack->m_eventID = m_eventID;
    dedxTrack->m_trackID = m_trackID;

    // load the pion fit hypothesis or the hypothesis which is the closest in mass to a pion
    // the tracking will not always successfully fit with a pion hypothesis
    const TrackFitResult* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for track ...");
      continue;
    }

    if (numMCParticles != 0) {
      // find MCParticle corresponding to this track
      const MCParticle* mcpart = track.getRelatedTo<MCParticle>();

      if (mcpart) {
        if (m_onlyPrimaryParticles && !mcpart->hasStatus(MCParticle::c_PrimaryParticle)) {
          continue; //not a primary particle, ignore
        }

        //add some MC truths to VXDDedxTrack object
        dedxTrack->m_pdg = mcpart->getPDG();
        const MCParticle* mother = mcpart->getMother();
        dedxTrack->m_motherPDG = mother ? mother->getPDG() : 0;

        const ROOT::Math::XYZVector trueMomentum = mcpart->getMomentum();
        dedxTrack->m_pTrue = trueMomentum.R();
      }
    }

    // get momentum (at origin) from fit result
    const ROOT::Math::XYZVector& trackPos = fitResult->getPosition();
    const ROOT::Math::XYZVector& trackMom = fitResult->getMomentum();
    dedxTrack->m_p = trackMom.R();
    dedxTrack->m_cosTheta = cos(trackMom.Theta());
    dedxTrack->m_charge = fitResult->getChargeSign();

    // dE/dx values will be calculated using associated RecoTrack
    const RecoTrack* recoTrack = track.getRelatedTo<RecoTrack>();
    if (!recoTrack) {
      B2WARNING("No related track for this fit...");
      continue;
    }

    // Check to see if the track is pruned. We use the cardinal representation for this, as we do not expect that one
    // representation is pruned whereas the other is not.
    if (recoTrack->getTrackFitStatus()->isTrackPruned()) {
      B2ERROR("GFTrack is pruned, please run VXDDedxPID only on unpruned tracks! Skipping this track.");
      continue;
    }

    //used for PXD/SVD hits
    const HelixHelper helixAtOrigin(trackPos, trackMom, dedxTrack->m_charge);

    if (m_usePXD) {
      const std::vector<PXDCluster*>& pxdClusters = recoTrack->getPXDHitList();
      saveSiHits(dedxTrack.get(), helixAtOrigin, pxdClusters);
    }

    if (m_useSVD) {
      const std::vector<SVDCluster*>& svdClusters = recoTrack->getSVDHitList();
      saveSiHits(dedxTrack.get(), helixAtOrigin, svdClusters);
    }

    if (dedxTrack->dedx.empty()) {
      B2DEBUG(50, "Found track with no hits, ignoring.");
      continue;
    }

    // add a few last things to the VXDDedxTrack
    const int numDedx = dedxTrack->dedx.size();
    dedxTrack->m_nHits = numDedx;
    // no need to define lowedgetruncated and highedgetruncated as we always remove the highest 2 dE/dx values from 8 dE/dx value
    dedxTrack->m_nHitsUsed = numDedx - 2;

    // calculate log likelihoods
    dedxTrack->clearLogLikelihoods();
    bool truncated = not m_useIndividualHits;
    if (m_usePXD) dedxTrack->addLogLikelihoods(m_PXDDedxPDFs->getPDFs(truncated), Dedx::c_PXD, truncated);
    if (m_useSVD) dedxTrack->addLogLikelihoods(m_SVDDedxPDFs->getPDFs(truncated), Dedx::c_SVD, truncated);

    // save log likelihoods
    if (dedxTrack->areLogLikelihoodsAvailable()) {
      VXDDedxLikelihood* likelihoodObj = m_dedxLikelihoods.appendNew(dedxTrack->m_vxdLogl);
      track.addRelationTo(likelihoodObj);
    }

    // save the VXDDedxTrack
    VXDDedxTrack* newVXDDedxTrack = m_dedxTracks.appendNew(*dedxTrack);
    track.addRelationTo(newVXDDedxTrack);

  } // end of loop over tracks
}

void VXDDedxPIDModule::terminate()
{

  B2DEBUG(50, "VXDDedxPIDModule exiting after processing " << m_trackID <<
          " tracks in " << m_eventID + 1 << " events.");
}


// calculateMeans need some change as we always remove highest 2 dE/dx values
void VXDDedxPIDModule::calculateMeans(double& mean, double& truncatedMean, double& truncatedMeanErr,
                                      const std::vector<double>& dedx) const
{
  // Calculate the truncated average by skipping only highest two value
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());

  double truncatedMeanTmp = 0.0;
  double meanTmp = 0.0;
  double sumOfSquares = 0.0;
  const int numDedx = sortedDedx.size();


  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
  }
  if (numDedx != 0) {
    meanTmp /= numDedx;
  }

  for (int i = 0; i < numDedx - 2; i++) {
    truncatedMeanTmp += sortedDedx[i];
    sumOfSquares += sortedDedx[i] * sortedDedx[i];
  }
  if (numDedx - 2 != 0) {
    truncatedMeanTmp /= numDedx - 2;
  }

  mean = meanTmp;
  truncatedMean = truncatedMeanTmp;

  if (numDedx - 2 > 1) {
    truncatedMeanErr = sqrt(sumOfSquares / double(numDedx - 2) - truncatedMeanTmp * truncatedMeanTmp) / double((numDedx - 2) - 1);
  } else {
    truncatedMeanErr = 0;
  }
}

double VXDDedxPIDModule::getTraversedLength(const PXDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  const ROOT::Math::XYZVector localPos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
  const ROOT::Math::XYZVector& globalPos = sensor.pointToGlobal(localPos);
  const ROOT::Math::XYZVector& localMomentum = helix->momentum(helix->pathLengthToPoint(globalPos));

  const ROOT::Math::XYZVector& sensorNormal = sensor.vectorToGlobal(ROOT::Math::XYZVector(0.0, 0.0, 1.0));
  const double angle = ROOT::Math::VectorUtil::Angle(sensorNormal, localMomentum); //includes theta and phi components

  //I'm assuming there's only one hit per sensor, there are _very_ rare exceptions to that (most likely curlers)
  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}


double VXDDedxPIDModule::getTraversedLength(const SVDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  ROOT::Math::XYZVector a, b;
  if (hit->isUCluster()) {
    const double u = hit->getPosition();
    a = sensor.pointToGlobal(ROOT::Math::XYZVector(sensor.getBackwardWidth() / sensor.getWidth(0) * u, -0.5 * sensor.getLength(), 0.0));
    b = sensor.pointToGlobal(ROOT::Math::XYZVector(sensor.getForwardWidth() / sensor.getWidth(0) * u, +0.5 * sensor.getLength(), 0.0));
  } else {
    const double v = hit->getPosition();
    a = sensor.pointToGlobal(ROOT::Math::XYZVector(-0.5 * sensor.getWidth(v), v, 0.0));
    b = sensor.pointToGlobal(ROOT::Math::XYZVector(+0.5 * sensor.getWidth(v), v, 0.0));
  }
  const double pathLength = helix->pathLengthToLine(ROOT::Math::XYZVector(a), ROOT::Math::XYZVector(b));
  const ROOT::Math::XYZVector& localMomentum = helix->momentum(pathLength);

  const ROOT::Math::XYZVector& sensorNormal = sensor.vectorToGlobal(ROOT::Math::XYZVector(0.0, 0.0, 1.0));
  const double angle = ROOT::Math::VectorUtil::Angle(sensorNormal, localMomentum); //includes theta and phi components

  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}


template <class HitClass> void VXDDedxPIDModule::saveSiHits(VXDDedxTrack* track, const HelixHelper& helix,
                                                            const std::vector<HitClass*>& hits) const
{
  const int numHits = hits.size();
  if (numHits == 0)
    return;

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //figure out which detector to assign hits to
  const int currentDetector = geo.get(hits.front()->getSensorID()).getType();
  assert(currentDetector == VXD::SensorInfoBase::PXD or currentDetector == VXD::SensorInfoBase::SVD);
  //  assert(currentDetector <= 1); //used as array index
  assert(currentDetector == 0 or currentDetector == 1); //used as array index

  std::vector<double> siliconDedx; //used for averages
  siliconDedx.reserve(numHits);

  VxdID prevSensor;
  for (int i = 0; i < numHits; i++) {
    const HitClass* hit = hits[i];
    if (!hit) {
      B2ERROR("Added hit is a null pointer!");
      continue;
    }
    const VxdID& currentSensor = hit->getSensorID();
    int layer = -currentSensor.getLayerNumber();
    assert(layer >= -6 && layer < 0);

    //active medium traversed, in cm (can traverse one sensor at most)
    //assumption: Si detectors are close enough to the origin that helix is still accurate
    const double totalDistance = getTraversedLength(hit, &helix);
    const double charge = hit->getCharge();
    const double dedx = charge / totalDistance;
    if (dedx <= 0) {
      B2WARNING("dE/dx is " << dedx << " in layer " << layer);
    } else if (i == 0 or prevSensor != currentSensor) { //only save once per sensor (u and v hits share charge!)
      prevSensor = currentSensor;
      //store data
      siliconDedx.push_back(dedx);
      track->m_dedxAvg[currentDetector] += dedx;
      track->addDedx(layer, totalDistance, dedx);
    }

    track->addHit(currentSensor, layer, charge, totalDistance, dedx);
  }

  //save averages
  calculateMeans(track->m_dedxAvg[currentDetector],
                 track->m_dedxAvgTruncated[currentDetector],
                 track->m_dedxAvgTruncatedErr[currentDetector],
                 siliconDedx);
}

