/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/VXDDedxPID/VXDDedxPIDModule.h>
#include <reconstruction/modules/VXDDedxPID/HelixHelper.h>

#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <vxd/geometry/GeoCache.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <genfit/MaterialEffects.h>

#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>

#include <memory>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(VXDDedxPID)

VXDDedxPIDModule::VXDDedxPIDModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  //Set module properties
  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the SVD and PXD.");

  //Parameter definitions
  addParam("useIndividualHits", m_useIndividualHits,
           "Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used.", true);
  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.25));

  addParam("onlyPrimaryParticles", m_onlyPrimaryParticles, "Only save data for primary particles (as determined by MC truth)", false);
  addParam("usePXD", m_usePXD, "Use PXDClusters for dE/dx calculation", false);
  addParam("useSVD", m_useSVD, "Use SVDClusters for dE/dx calculation", true);

  addParam("trackDistanceThreshold", m_trackDistanceThreshhold,
           "Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off.", double(4.0));
  addParam("enableDebugOutput", m_enableDebugOutput, "Option to write out debugging information to DedxTracks (DataStore objects).",
           false);

  addParam("ignoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);

  m_eventID = -1;
  m_trackID = 0;
}

VXDDedxPIDModule::~VXDDedxPIDModule() { }

void VXDDedxPIDModule::checkPDFs()
{
  //load dedx:momentum PDFs
  if (!m_DBDedxPDFs) B2FATAL("No VXD Dedx PDFS available");
  int nBinsXPXD, nBinsYPXD;
  double xMinPXD, xMaxPXD, yMinPXD, yMaxPXD;
  nBinsXPXD = nBinsYPXD = -1;
  xMinPXD = xMaxPXD = yMinPXD = yMaxPXD = 0.0;

  int nBinsXSVD, nBinsYSVD;
  double xMinSVD, xMaxSVD, yMinSVD, yMaxSVD;
  nBinsXSVD = nBinsYSVD = -1;
  xMinSVD = xMaxSVD = yMinSVD = yMaxSVD = 0.0;

  for (unsigned int iPart = 0; iPart < 6; iPart++) {
    const int pdgCode = Const::chargedStableSet.at(iPart).getPDGCode();
    const TH2F* svd_pdf = m_DBDedxPDFs->getSVDPDF(iPart, !m_useIndividualHits);
    const TH2F* pxd_pdf = m_DBDedxPDFs->getPXDPDF(iPart, !m_useIndividualHits);

    if (pxd_pdf->GetEntries() == 0 || svd_pdf->GetEntries() == 0) {
      if (m_ignoreMissingParticles)
        continue;
      B2FATAL("Couldn't find PDF for PDG " << pdgCode);
    }

    //check that PXD PDFs have the same dimensions and same binning
    const double epsFactor = 1e-5;
    if (nBinsXPXD == -1 and nBinsYPXD == -1) {
      nBinsXPXD = pxd_pdf->GetNbinsX();
      nBinsYPXD = pxd_pdf->GetNbinsY();
      xMinPXD = pxd_pdf->GetXaxis()->GetXmin();
      xMaxPXD = pxd_pdf->GetXaxis()->GetXmax();
      yMinPXD = pxd_pdf->GetYaxis()->GetXmin();
      yMaxPXD = pxd_pdf->GetYaxis()->GetXmax();
    } else if (nBinsXPXD != pxd_pdf->GetNbinsX()
               or nBinsYPXD != pxd_pdf->GetNbinsY()
               or fabs(xMinPXD - pxd_pdf->GetXaxis()->GetXmin()) > epsFactor * xMaxPXD
               or fabs(xMaxPXD - pxd_pdf->GetXaxis()->GetXmax()) > epsFactor * xMaxPXD
               or fabs(yMinPXD - pxd_pdf->GetYaxis()->GetXmin()) > epsFactor * yMaxPXD
               or fabs(yMaxPXD - pxd_pdf->GetYaxis()->GetXmax()) > epsFactor * yMaxPXD) {
      B2FATAL("PDF for PDG " << pdgCode << ", PXD has binning/dimensions differing from previous PDF.");
    }

    //check that SVD PDFs have the same dimensions and same binning
    if (nBinsXSVD == -1 and nBinsYSVD == -1) {
      nBinsXSVD = svd_pdf->GetNbinsX();
      nBinsYSVD = svd_pdf->GetNbinsY();
      xMinSVD = svd_pdf->GetXaxis()->GetXmin();
      xMaxSVD = svd_pdf->GetXaxis()->GetXmax();
      yMinSVD = svd_pdf->GetYaxis()->GetXmin();
      yMaxSVD = svd_pdf->GetYaxis()->GetXmax();
    } else if (nBinsXSVD != svd_pdf->GetNbinsX()
               or nBinsYSVD != svd_pdf->GetNbinsY()
               or fabs(xMinSVD - svd_pdf->GetXaxis()->GetXmin()) > epsFactor * xMaxSVD
               or fabs(xMaxSVD - svd_pdf->GetXaxis()->GetXmax()) > epsFactor * xMaxSVD
               or fabs(yMinSVD - svd_pdf->GetYaxis()->GetXmin()) > epsFactor * yMaxSVD
               or fabs(yMaxSVD - svd_pdf->GetYaxis()->GetXmax()) > epsFactor * yMaxSVD) {
      B2FATAL("PDF for PDG " << pdgCode << ", PXD has binning/dimensions differing from previous PDF.");
    }
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

  // register optional outputs
  if (m_enableDebugOutput) {
    m_dedxTracks.registerInDataStore();
    m_tracks.registerRelationTo(m_dedxTracks);
  }

  // register outputs
  m_dedxLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_dedxLikelihoods);

  m_DBDedxPDFs.addCallback([this]() {checkPDFs();});
  checkPDFs();


  // create instances here to not confuse profiling
  VXD::GeoCache::getInstance();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Need to have SetupGenfitExtrapolationModule in path before this one.");
  }
}

void VXDDedxPIDModule::event()
{
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

    if ((m_enableDebugOutput or m_onlyPrimaryParticles) and numMCParticles != 0) {
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

        const TVector3 trueMomentum = mcpart->getMomentum();
        dedxTrack->m_pTrue = trueMomentum.Mag();
      }
    }

    // get momentum (at origin) from fit result
    const TVector3& trackPos = fitResult->getPosition();
    const TVector3& trackMom = fitResult->getMomentum();
    dedxTrack->m_p = trackMom.Mag();
    dedxTrack->m_cosTheta = trackMom.CosTheta();
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

    // calculate likelihoods for truncated mean
    if (!m_useIndividualHits) {
      for (int detector = 0; detector <= c_SVD; detector++) {
        if (!detectorEnabled(static_cast<Detector>(detector)))
          continue; //unwanted detector

        if (detector == 0) savePXDLogLikelihood(dedxTrack->m_vxdLogl, dedxTrack->m_p, dedxTrack->m_dedxAvgTruncated[detector]);
        else if (detector == 1) saveSVDLogLikelihood(dedxTrack->m_vxdLogl, dedxTrack->m_p, dedxTrack->m_dedxAvgTruncated[detector]);
      }
    }

    if (m_enableDebugOutput) {
      // add a few last things to the VXDDedxTrack
      const int numDedx = dedxTrack->dedx.size();
      dedxTrack->m_nHits = numDedx;
      // add a factor of 0.5 here to make sure we are rounding appropriately...
      const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.5);
      const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.5);
      dedxTrack->m_nHitsUsed = highEdgeTrunc - lowEdgeTrunc;

      // now book the information for this track
      VXDDedxTrack* newVXDDedxTrack = m_dedxTracks.appendNew(*dedxTrack);
      track.addRelationTo(newVXDDedxTrack);
    }

    // save VXDDedxLikelihood
    VXDDedxLikelihood* likelihoodObj = m_dedxLikelihoods.appendNew(dedxTrack->m_vxdLogl);
    track.addRelationTo(likelihoodObj);

  } // end of loop over tracks
}

void VXDDedxPIDModule::terminate()
{

  B2DEBUG(50, "VXDDedxPIDModule exiting after processing " << m_trackID <<
          " tracks in " << m_eventID + 1 << " events.");
}

void VXDDedxPIDModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr,
                                      const std::vector<double>& dedx) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());

  double truncatedMeanTmp = 0.0;
  double meanTmp = 0.0;
  double sumOfSquares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();

  // add a factor of 0.5 here to make sure we are rounding appropriately...
  const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.5);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.5);
  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
    if (i >= lowEdgeTrunc and i < highEdgeTrunc) {
      truncatedMeanTmp += sortedDedx[i];
      sumOfSquares += sortedDedx[i] * sortedDedx[i];
      numValuesTrunc++;
    }
  }

  if (numDedx != 0) {
    meanTmp /= numDedx;
  }
  if (numValuesTrunc != 0) {
    truncatedMeanTmp /= numValuesTrunc;
  } else {
    truncatedMeanTmp = meanTmp;
  }

  *mean = meanTmp;
  *truncatedMean = truncatedMeanTmp;

  if (numValuesTrunc > 1) {
    *truncatedMeanErr = sqrt(sumOfSquares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(
                          numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}


double VXDDedxPIDModule::getTraversedLength(const PXDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  const TVector3 localPos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
  const TVector3& globalPos = sensor.pointToGlobal(localPos);
  const TVector3& localMomentum = helix->momentum(helix->pathLengthToPoint(globalPos));

  const TVector3& sensorNormal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
  const double angle = sensorNormal.Angle(localMomentum); //includes theta and phi components

  //I'm assuming there's only one hit per sensor, there are _very_ rare exceptions to that (most likely curlers)
  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}


double VXDDedxPIDModule::getTraversedLength(const SVDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  TVector3 a, b;
  if (hit->isUCluster()) {
    const float u = hit->getPosition();
    a = sensor.pointToGlobal(TVector3(sensor.getBackwardWidth() / sensor.getWidth(0) * u, -0.5 * sensor.getLength(), 0.0));
    b = sensor.pointToGlobal(TVector3(sensor.getForwardWidth() / sensor.getWidth(0) * u, +0.5 * sensor.getLength(), 0.0));
  } else {
    const float v = hit->getPosition();
    a = sensor.pointToGlobal(TVector3(-0.5 * sensor.getWidth(v), v, 0.0));
    b = sensor.pointToGlobal(TVector3(+0.5 * sensor.getWidth(v), v, 0.0));
  }
  const double pathLength = helix->pathLengthToLine(a, b);
  const TVector3& localMomentum = helix->momentum(pathLength);

  const TVector3& sensorNormal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
  const double angle = sensorNormal.Angle(localMomentum); //includes theta and phi components

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
  assert(currentDetector <= 1); //used as array index

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
    int layer = -1;
    if (m_enableDebugOutput) {
      layer = -currentSensor.getLayerNumber();
      assert(layer >= -6 && layer < 0);
    }

    //active medium traversed, in cm (can traverse one sensor at most)
    //assumption: Si detectors are close enough to the origin that helix is still accurate
    const double totalDistance = getTraversedLength(hit, &helix);
    const float charge = hit->getCharge();
    const float dedx = charge / totalDistance;
    if (dedx <= 0) {
      B2WARNING("dE/dx is " << dedx << " in layer " << layer);
    } else if (i == 0 or prevSensor != currentSensor) { //only save once per sensor (u and v hits share charge!)
      prevSensor = currentSensor;
      //store data
      siliconDedx.push_back(dedx);
      track->m_dedxAvg[currentDetector] += dedx;
      track->addDedx(layer, totalDistance, dedx);
      if (m_useIndividualHits) {
        if (currentDetector == 0) savePXDLogLikelihood(track->m_vxdLogl, track->m_p, dedx);
        else if (currentDetector == 1) saveSVDLogLikelihood(track->m_vxdLogl, track->m_p, dedx);
      }
    }

    if (m_enableDebugOutput) {
      track->addHit(currentSensor, layer, charge, totalDistance, dedx);
    }
  }

  //save averages averages
  if (!m_useIndividualHits or m_enableDebugOutput) {
    calculateMeans(&(track->m_dedxAvg[currentDetector]),
                   &(track->m_dedxAvgTruncated[currentDetector]),
                   &(track->m_dedxAvgTruncatedErr[currentDetector]),
                   siliconDedx);
  }
}

void VXDDedxPIDModule::savePXDLogLikelihood(double(&logl)[Const::ChargedStable::c_SetSize], double p, float dedx) const
{
  //all pdfs have the same dimensions
  const TH2F* pdf = m_DBDedxPDFs->getPXDPDF(0, !m_useIndividualHits);
  const Int_t binX = pdf->GetXaxis()->FindFixBin(p);
  const Int_t binY = pdf->GetYaxis()->FindFixBin(dedx);

  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    pdf = m_DBDedxPDFs->getPXDPDF(iPart, !m_useIndividualHits);
    if (pdf->GetEntries() == 0) //might be NULL if m_ignoreMissingParticles is set
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (binX < 1 or binX > pdf->GetNbinsX()
        or binY < 1 or binY > pdf->GetNbinsY()) {
      probability = pdf->GetBinContent(binX, binY);
    } else {
      //in normal histogram range. Of course ROOT has a bug that Interpolate()
      //is not declared as const but it does not modify the internal state so
      //fine, const_cast it is.
      probability = const_cast<TH2F*>(pdf)->Interpolate(p, dedx);
    }

    if (probability != probability)
      B2ERROR("probability NAN for a track with p=" << p << " and dedx=" << dedx);

    //my pdfs aren't perfect...
    if (probability == 0.0)
      probability = m_useIndividualHits ? (1e-5) : (1e-3); //likelihoods for truncated mean are much higher

    logl[iPart] += log(probability);
  }
}

void VXDDedxPIDModule::saveSVDLogLikelihood(double(&logl)[Const::ChargedStable::c_SetSize], double p, float dedx) const
{
  //all pdfs have the same dimensions
  const TH2F* pdf = m_DBDedxPDFs->getSVDPDF(0, !m_useIndividualHits);
  const Int_t binX = pdf->GetXaxis()->FindFixBin(p);
  const Int_t binY = pdf->GetYaxis()->FindFixBin(dedx);

  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    pdf = m_DBDedxPDFs->getSVDPDF(0, !m_useIndividualHits);
    if (pdf->GetEntries() == 0) //might be NULL if m_ignoreMissingParticles is set
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (binX < 1 or binX > pdf->GetNbinsX()
        or binY < 1 or binY > pdf->GetNbinsY()) {
      probability = pdf->GetBinContent(binX, binY);
    } else {
      //in normal histogram range. Of course ROOT has a bug that Interpolate()
      //is not declared as const but it does not modify the internal state so
      //fine, const_cast it is.
      probability = const_cast<TH2F*>(pdf)->Interpolate(p, dedx);
    }

    if (probability != probability)
      B2ERROR("probability NAN for a track with p=" << p << " and dedx=" << dedx);

    //my pdfs aren't perfect...
    if (probability == 0.0)
      probability = m_useIndividualHits ? (1e-5) : (1e-3); //likelihoods for truncated mean are much higher

    logl[iPart] += log(probability);
  }
}
