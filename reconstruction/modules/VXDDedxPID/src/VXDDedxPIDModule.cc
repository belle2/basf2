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

VXDDedxPIDModule::VXDDedxPIDModule() : Module(), m_pdfs()
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

  addParam("pdfFile", m_pdfFile, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.",
           std::string("/data/reconstruction/dedxPID_PDFs_7b7a9f_500k_events.root"));
  addParam("ignoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);

  m_eventID = -1;
  m_trackID = 0;
}

VXDDedxPIDModule::~VXDDedxPIDModule() { }

void VXDDedxPIDModule::initialize()
{

  // check for a pdf file - necessary for likelihood calculations
  if (!m_pdfFile.empty()) {

    std::string fullPath = FileSystem::findFile(m_pdfFile);
    if (fullPath.empty()) {
      B2ERROR("PDF file " << m_pdfFile << " not found!");
    }
    m_pdfFile = fullPath;
  }
  if (!m_enableDebugOutput and m_pdfFile.empty()) {
    B2ERROR("No PDFFile given and debug output disabled. This module will produce no output!");
  }

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

  // register outputs
  if (m_enableDebugOutput) {
    m_dedxTracks.registerInDataStore();
    m_tracks.registerRelationTo(m_dedxTracks);
  }

  if (!m_pdfFile.empty()) {
    m_dedxLikelihoods.registerInDataStore();
    m_tracks.registerRelationTo(m_dedxLikelihoods);

    //load pdfs
    TFile* pdfFile = new TFile(m_pdfFile.c_str(), "READ");
    if (!pdfFile->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFile);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    for (int detector = 0; detector <= c_SVD; detector++) {
      int nBinsX, nBinsY;
      double xMin, xMax, yMin, yMax;
      nBinsX = nBinsY = -1;
      xMin = xMax = yMin = yMax = 0.0;
      for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
        const int pdgCode = Const::chargedStableSet.at(iPart).getPDGCode();
        m_pdfs[detector][iPart] =
          dynamic_cast<TH2F*>(pdfFile->Get(TString::Format("hist_d%i_%i%s", detector, pdgCode, suffix)));

        if (!m_pdfs[detector][iPart]) {
          if (m_ignoreMissingParticles)
            continue;
          B2FATAL("Couldn't find PDF for PDG " << pdgCode << ", detector " << detector << suffix);
        }

        //check that PDFs have the same dimensions and same binning
        const double epsFactor = 1e-5;
        if (nBinsX == -1 and nBinsY == -1) {
          nBinsX = m_pdfs[detector][iPart]->GetNbinsX();
          nBinsY = m_pdfs[detector][iPart]->GetNbinsY();
          xMin = m_pdfs[detector][iPart]->GetXaxis()->GetXmin();
          xMax = m_pdfs[detector][iPart]->GetXaxis()->GetXmax();
          yMin = m_pdfs[detector][iPart]->GetYaxis()->GetXmin();
          yMax = m_pdfs[detector][iPart]->GetYaxis()->GetXmax();
        } else if (nBinsX != m_pdfs[detector][iPart]->GetNbinsX()
                   or nBinsY != m_pdfs[detector][iPart]->GetNbinsY()
                   or fabs(xMin - m_pdfs[detector][iPart]->GetXaxis()->GetXmin()) > epsFactor * xMax
                   or fabs(xMax - m_pdfs[detector][iPart]->GetXaxis()->GetXmax()) > epsFactor * xMax
                   or fabs(yMin - m_pdfs[detector][iPart]->GetYaxis()->GetXmin()) > epsFactor * yMax
                   or fabs(yMax - m_pdfs[detector][iPart]->GetYaxis()->GetXmax()) > epsFactor * yMax) {
          B2FATAL("PDF for PDG " << pdgCode << ", detector " << detector << suffix << " has binning/dimensions differing from previous PDF.");
        }
      }
    }

    //leaking pdfFile so I can access the histograms
  }

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
        dedxTrack->m_mother_pdg = mother ? mother->getPDG() : 0;

        const TVector3 trueMomentum = mcpart->getMomentum();
        dedxTrack->m_p_true = trueMomentum.Mag();
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

        saveLogLikelihood(dedxTrack->m_vxdLogl, dedxTrack->m_p, dedxTrack->m_dedx_avg_truncated[detector], m_pdfs[detector]);
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
    if (!m_pdfFile.empty()) {
      VXDDedxLikelihood* likelihoodObj = m_dedxLikelihoods.appendNew(dedxTrack->m_vxdLogl);
      track.addRelationTo(likelihoodObj);
    }

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
      track->m_dedx_avg[currentDetector] += dedx;
      track->addDedx(layer, totalDistance, dedx);
      if (!m_pdfFile.empty() and m_useIndividualHits) {
        saveLogLikelihood(track->m_vxdLogl, track->m_p, dedx, m_pdfs[currentDetector]);
      }
    }

    if (m_enableDebugOutput) {
      track->addHit(currentSensor, layer, charge, totalDistance, dedx);
    }
  }

  //save averages averages
  if (!m_useIndividualHits or m_enableDebugOutput) {
    calculateMeans(&(track->m_dedx_avg[currentDetector]),
                   &(track->m_dedx_avg_truncated[currentDetector]),
                   &(track->m_dedx_avg_truncated_err[currentDetector]),
                   siliconDedx);
  }
}


void VXDDedxPIDModule::saveLogLikelihood(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx,
                                         TH2F* const* pdf) const
{
  //all pdfs have the same dimensions
  const Int_t binX = pdf[0]->GetXaxis()->FindFixBin(p);
  const Int_t binY = pdf[0]->GetYaxis()->FindFixBin(dedx);


  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    if (!pdf[iPart]) //might be NULL if m_ignoreMissingParticles is set
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (binX < 1 or binX > pdf[iPart]->GetNbinsX()
        or binY < 1 or binY > pdf[iPart]->GetNbinsY()) {
      probability = pdf[iPart]->GetBinContent(binX, binY);
    } else {
      //in normal histogram range
      probability = pdf[iPart]->Interpolate(p, dedx);
    }

    if (probability != probability)
      B2ERROR("probability NAN for a track with p=" << p << " and dedx=" << dedx);

    //my pdfs aren't perfect...
    if (probability == 0.0)
      probability = m_useIndividualHits ? (1e-5) : (1e-3); //likelihoods for truncated mean are much higher

    logl[iPart] += log(probability);
  }
}
