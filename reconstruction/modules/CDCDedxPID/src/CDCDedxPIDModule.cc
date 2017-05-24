/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxPID/CDCDedxPIDModule.h>
#include <reconstruction/modules/CDCDedxPID/LineHelper.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/MaterialEffects.h>
#include <genfit/StateOnPlane.h>

#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>

#include <memory>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>

using namespace Belle2;
using namespace CDC;
using namespace Dedx;

REG_MODULE(CDCDedxPID)

CDCDedxPIDModule::CDCDedxPIDModule() : Module(), m_pdfs()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  //Set module properties
  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the CDC.");

  //Parameter definitions
  addParam("useIndividualHits", m_useIndividualHits,
           "Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values will be used.", true);
  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.25));

  addParam("onlyPrimaryParticles", m_onlyPrimaryParticles, "Only save data for primary particles (as determined by MC truth)", false);
  addParam("enableDebugOutput", m_enableDebugOutput,
           "Option to write out debugging information to CDCDedxTracks (DataStore objects).", true);
  addParam("pdfFile", m_pdfFile, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.",
           std::string("/data/reconstruction/dedxPID_PDFs_fbf2a31_500k_events.root"));
  addParam("ignoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);

  m_eventID = -1;
  m_trackID = 0;
}

CDCDedxPIDModule::~CDCDedxPIDModule() { }

void CDCDedxPIDModule::initialize()
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
  StoreArray<Track> tracks;
  StoreArray<RecoTrack> recoTracks;
  StoreArray<TrackFitResult> trackfitResults;

  tracks.isRequired();
  recoTracks.isRequired();
  trackfitResults.isRequired();

  //optional inputs
  StoreArray<MCParticle> mcparticles;
  mcparticles.isOptional();
  tracks.optionalRelationTo(mcparticles);
  StoreArray<CDCHit>::required();

  // register outputs
  if (m_enableDebugOutput) {
    StoreArray<CDCDedxTrack> dedxTracks;
    dedxTracks.registerInDataStore();
    tracks.registerRelationTo(dedxTracks);
  }

  if (!m_pdfFile.empty()) {
    StoreArray<CDCDedxLikelihood> dedxLikelihoods;
    dedxLikelihoods.registerInDataStore();
    tracks.registerRelationTo(dedxLikelihoods);

    //load pdfs
    TFile* pdfFile = new TFile(m_pdfFile.c_str(), "READ");
    if (!pdfFile->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFile);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    int nBinsX, nBinsY;
    double xMin, xMax, yMin, yMax;
    nBinsX = nBinsY = -1;
    xMin = xMax = yMin = yMax = 0.0;
    for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
      const int pdgCode = Const::chargedStableSet.at(iPart).getPDGCode();
      m_pdfs[2][iPart] =
        dynamic_cast<TH2F*>(pdfFile->Get(TString::Format("hist_d%i_%i%s", 2, pdgCode, suffix)));
      //dynamic_cast<TH2F*>(pdfFile->Get(TString::Format("hist_%i%s", pdgCode, suffix)));

      if (!m_pdfs[2][iPart]) {
        if (m_ignoreMissingParticles)
          continue;
        B2FATAL("Couldn't find PDF for PDG " << pdgCode << suffix);
      }

      //check that PDFs have the same dimensions and same binning
      const double epsFactor = 1e-5;
      if (nBinsX == -1 and nBinsY == -1) {
        nBinsX = m_pdfs[2][iPart]->GetNbinsX();
        nBinsY = m_pdfs[2][iPart]->GetNbinsY();
        xMin = m_pdfs[2][iPart]->GetXaxis()->GetXmin();
        xMax = m_pdfs[2][iPart]->GetXaxis()->GetXmax();
        yMin = m_pdfs[2][iPart]->GetYaxis()->GetXmin();
        yMax = m_pdfs[2][iPart]->GetYaxis()->GetXmax();
      } else if (nBinsX != m_pdfs[2][iPart]->GetNbinsX()
                 or nBinsY != m_pdfs[2][iPart]->GetNbinsY()
                 or fabs(xMin - m_pdfs[2][iPart]->GetXaxis()->GetXmin()) > epsFactor * xMax
                 or fabs(xMax - m_pdfs[2][iPart]->GetXaxis()->GetXmax()) > epsFactor * xMax
                 or fabs(yMin - m_pdfs[2][iPart]->GetYaxis()->GetXmin()) > epsFactor * yMax
                 or fabs(yMax - m_pdfs[2][iPart]->GetYaxis()->GetXmax()) > epsFactor * yMax) {
        B2FATAL("PDF for PDG " << pdgCode << suffix << " has binning/dimensions differing from previous PDF.");
      }
    }

    //leaking pdfFile so I can access the histograms
  }

  // load constants
  // setting these manually for now, but they should be read in from the DB
  m_curvepars[0] = 0.00392444;
  m_curvepars[1] = 26.8709;
  m_curvepars[2] = 3.78296;
  m_curvepars[3] = -0.534117;
  m_curvepars[4] = 6.63276;
  m_curvepars[5] = 0.494768;
  m_curvepars[6] = -1.9872;
  m_curvepars[7] = -0.000117405;
  m_curvepars[8] = 0.00283862;
  m_curvepars[9] = -0.0131518;
  m_curvepars[10] = 0.706402;
  m_curvepars[11] = 0.0803046;
  m_curvepars[12] = 1.08118;
  m_curvepars[13] = 0.543991;
  m_curvepars[14] = 0.0031143;

  m_sigmapars[0] = 0.0115507;
  m_sigmapars[1] = 0.0773491;
  m_sigmapars[2] = 3.9199e-05;
  m_sigmapars[3] = -0.00345398;
  m_sigmapars[4] = 0.111907;
  m_sigmapars[5] = -1.59311;
  m_sigmapars[6] = 9.16672;
  m_sigmapars[7] = 43.9322;
  m_sigmapars[8] = -124.213;
  m_sigmapars[9] = 127.56;
  m_sigmapars[10] = -56.7759;
  m_sigmapars[11] = 10.3113;

  // create instances here to not confuse profiling
  CDCGeometryPar::Instance();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Need to have SetupGenfitExtrapolationModule in path before this one.");
  }
}

void CDCDedxPIDModule::event()
{
  // go through Tracks
  // get fitresult and RecoTrack and do extrapolations, save corresponding dE/dx and likelihood values
  //   get hit indices through RecoTrack::getHitPointsWithMeasurement(...)
  //   create one CDCDedxTrack per fitresult/recoTrack
  //create one DedkLikelihood per Track (plus rel)
  m_eventID++;

  // inputs
  StoreArray<Track> tracks;
  StoreArray<MCParticle> mcparticles;
  const int numMCParticles = mcparticles.getEntries();

  // outputs
  StoreArray<CDCDedxTrack> dedxArray;
  StoreArray<CDCDedxLikelihood> likelihoodArray;

  // get the geometry of the cdc
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  // **************************************************
  //
  //  LOOP OVER TRACKS
  //
  // **************************************************

  for (const auto& track : tracks) {
    m_trackID++;

    std::shared_ptr<CDCDedxTrack> dedxTrack = std::make_shared<CDCDedxTrack>();
    dedxTrack->m_eventID = m_eventID;
    dedxTrack->m_trackID = m_trackID;

    // get pion fit hypothesis for now
    //  Should be ok in most cases, for MC fitting this will return the fit with the
    //  true PDG value. At some point, it might be worthwhile to look into using a
    //  different fit if the differences are large
    const TrackFitResult* fitResult = track.getTrackFitResult(Const::pion);
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

        //add some MC truths to CDCDedxTrack object
        dedxTrack->m_pdg = mcpart->getPDG();
        const MCParticle* mother = mcpart->getMother();
        dedxTrack->m_mother_pdg = mother ? mother->getPDG() : 0;

        const TVector3 trueMomentum = mcpart->getMomentum();
        dedxTrack->m_p_true = trueMomentum.Mag();
      }
    }

    // get momentum (at origin) from fit result
    const TVector3& trackMom = fitResult->getMomentum();
    dedxTrack->m_p = trackMom.Mag();
    bool nomom = (dedxTrack->m_p != dedxTrack->m_p);
    if (nomom) {
      dedxTrack->m_cosTheta = std::cos(std::atan(1 / fitResult->getCotTheta()));
      dedxTrack->m_charge = 1.0;
    } else {
      dedxTrack->m_cosTheta = trackMom.CosTheta();
      dedxTrack->m_charge = fitResult->getChargeSign();
    }
    // dE/dx values will be calculated using associated RecoTrack
    const RecoTrack* recoTrack = track.getRelatedTo<RecoTrack>();
    if (!recoTrack) {
      B2WARNING("No related track for this fit...");
      continue;
    }

    // Check to see if the track is pruned
    if (recoTrack->getTrackFitStatus()->isTrackPruned()) {
      B2ERROR("GFTrack is pruned, please run CDCDedxPID only on unpruned tracks! Skipping this track.");
      continue;
    }

    double layerdE = 0.0; // total charge in current layer
    double layerdx = 0.0; // total path length in current layer
    double cdcMom = 0.0; // momentum valid in the CDC

    // loop over all CDC hits from this track
    // Get the TrackPoints, which contain the hit information we need.
    // Then iterate over each point.
    int tpcounter = 0;
    const std::vector< genfit::TrackPoint* >& gftrackPoints = recoTrack->getHitPointsWithMeasurement();
    for (std::vector< genfit::TrackPoint* >::const_iterator tp = gftrackPoints.begin();
         tp != gftrackPoints.end(); tp++) {
      tpcounter++;

      // should also be possible to use this for svd and pxd hits...
      genfit::AbsMeasurement* aAbsMeasurementPtr = (*tp)->getRawMeasurement(0);
      const CDCRecoHit* cdcRecoHit = dynamic_cast<const CDCRecoHit* >(aAbsMeasurementPtr);
      if (!cdcRecoHit) continue;
      const CDCHit* cdcHit = cdcRecoHit->getCDCHit();
      if (!cdcHit) continue;

      // get the poca on the wire and track momentum for this hit
      // make sure the fitter info exists
      const genfit::AbsFitterInfo* fi = (*tp)->getFitterInfo();
      if (!fi) {
        B2DEBUG(50, "No fitter info, skipping...");
        continue;
      }

      // get the global wire ID (between 0 and 14336) and the layer info
      WireID wireID = cdcRecoHit->getWireID();
      const int wire = wireID.getEWire();
      int layer = cdcHit->getILayer();
      int superlayer = cdcHit->getISuperLayer();
      int currentLayer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer);

      // if multiple hits in a layer, we may combine the hits
      const bool lastHit = (tp + 1 == gftrackPoints.end());
      bool lastHitInCurrentLayer = lastHit;
      if (!lastHit) {
        // peek at next hit
        genfit::AbsMeasurement* aAbsMeasurementPtr = (*(tp + 1))->getRawMeasurement(0);
        const CDCRecoHit* nextcdcRecoHit = dynamic_cast<const CDCRecoHit* >(aAbsMeasurementPtr);
        // if next hit fails, assume this is the last hit in the layer
        if (!nextcdcRecoHit || !(cdcRecoHit->getCDCHit()) || !((*(tp + 1))->getFitterInfo())) {
          lastHitInCurrentLayer = true;
          break;
        }
        const CDCHit* nextcdcHit = nextcdcRecoHit->getCDCHit();
        const int nextILayer = nextcdcHit->getILayer();
        const int nextSuperlayer = nextcdcHit->getISuperLayer();
        const int nextLayer = (nextSuperlayer == 0) ? nextILayer : (8 + (nextSuperlayer - 1) * 6 + nextILayer);
        lastHitInCurrentLayer = (nextLayer != currentLayer);
      }

      // find the position of the endpoints of the sense wire
      const TVector3& wirePosF = cdcgeo.wireForwardPosition(wireID, CDCGeometryPar::c_Aligned);
      const TVector3& wirePosB = cdcgeo.wireBackwardPosition(wireID, CDCGeometryPar::c_Aligned);
      const TVector3 wireDir = (wirePosB - wirePosF).Unit();

      int nWires = cdcgeo.nWiresInLayer(currentLayer);

      // radii of field wires for this layer
      double inner = cdcgeo.innerRadiusWireLayer()[currentLayer];
      double outer = cdcgeo.outerRadiusWireLayer()[currentLayer];

      double topHeight = outer - wirePosF.Perp();
      double bottomHeight = wirePosF.Perp() - inner;
      double cellHeight = topHeight + bottomHeight;
      double topHalfWidth = PI * outer / nWires;
      double bottomHalfWidth = PI * inner / nWires;
      double cellHalfWidth = PI * wirePosF.Perp() / nWires;

      // first construct the boundary lines, then create the cell
      const DedxPoint tl = DedxPoint(-topHalfWidth, topHeight);
      const DedxPoint tr = DedxPoint(topHalfWidth, topHeight);
      const DedxPoint br = DedxPoint(bottomHalfWidth, -bottomHeight);
      const DedxPoint bl = DedxPoint(-bottomHalfWidth, -bottomHeight);
      DedxDriftCell c = DedxDriftCell(tl, tr, br, bl);

      // make sure the MOP is reasonable (an exception is thrown for bad numerics)
      try {
        const genfit::MeasuredStateOnPlane& mop = fi->getFittedState();

        // use the MOP to determine the DOCA and entrance angle
        B2Vector3D fittedPoca = mop.getPos();
        const TVector3& pocaMom = mop.getMom();
        if (tp == gftrackPoints.begin() || cdcMom == 0) {
          cdcMom = pocaMom.Mag();
          dedxTrack->m_p_cdc = cdcMom;
        }
        if (nomom)
          dedxTrack->m_p = cdcMom;

        // get the doca and entrance angle information.
        // constructPlane places the coordinate center in the POCA to the
        // wire.  Using this is the default behavior.  If this should be too
        // slow, as it has to re-evaluate the POCA
        //  B2Vector3D pocaOnWire = cdcRecoHit->constructPlane(mop)->getO();

        // uses the plane determined by the track fit.
        B2Vector3D pocaOnWire = mop.getPlane()->getO();

        // The vector from the wire to the track.
        B2Vector3D B2WireDoca = fittedPoca - pocaOnWire;

        // the sign of the doca is defined here to be positive in the +x dir
        double doca = B2WireDoca.Perp();
        if (B2WireDoca.X() < 0) doca = -1.0 * doca;

        // The opening angle of the track momentum direction
        const double px = pocaMom.x();
        const double py = pocaMom.y();
        const double wx = pocaOnWire.x();
        const double wy = pocaOnWire.y();
        const double cross = wx * py - wy * px;
        const double dot   = wx * px + wy * py;
        double entAng = atan2(cross, dot);

        LinearGlobalADCCountTranslator translator;
        double adcCount = cdcHit->getADCCount(); // pedestal subtracted?
        double hitCharge = translator.getCharge(adcCount, wireID, false, pocaOnWire.Z(), pocaMom.Phi());
        int driftT = cdcHit->getTDCCount();

        RealisticTDCCountTranslator realistictdc;
        double driftDRealistic = realistictdc.getDriftLength(driftT, wireID, 0, true, pocaOnWire.Z(), pocaMom.Phi(), pocaMom.Theta());
        double driftDRealisticRes = realistictdc.getDriftLengthResolution(driftDRealistic, wireID, true, pocaOnWire.Z(), pocaMom.Phi(),
                                    pocaMom.Theta());

        // now calculate the path length for this hit
        double celldx = c.dx(doca, entAng);
        if (c.isValid()) {

          layerdE += adcCount;
          layerdx += celldx;

          // save individual hits
          double cellDedx = (adcCount / celldx);
          if (nomom) cellDedx *= sin(std::atan(1 / fitResult->getCotTheta()));
          else  cellDedx *= sin(trackMom.Theta());

          if (m_enableDebugOutput)
            dedxTrack->addHit(wire, currentLayer, doca, entAng, adcCount, hitCharge, celldx, cellDedx, cellHeight, cellHalfWidth, driftT,
                              driftDRealistic, driftDRealisticRes);
        }
      } catch (genfit::Exception) {
        B2WARNING("Event " << m_eventID << ", Track: " << m_trackID << ": genfit::MeasuredStateOnPlane exception...");
        continue;
      }

      // check if there are any more hits in this layer
      if (lastHitInCurrentLayer) {
        double totalDistance;
        if (nomom) totalDistance = layerdx / sin(std::atan(1 / fitResult->getCotTheta()));
        else  totalDistance = layerdx / sin(trackMom.Theta());
        double layerDedx = layerdE / totalDistance;

        // save the information for this layer
        if (layerDedx > 0) {
          dedxTrack->addDedx(currentLayer, totalDistance, layerDedx);
          // save the PID information if using individual hits
          if (!m_pdfFile.empty() and m_useIndividualHits) {
            // use the momentum valid in the cdc
            saveLookupLogl(dedxTrack->m_cdcLogl, dedxTrack->m_p_cdc, layerDedx, m_pdfs[2]);
          }
        }

        layerdE = 0;
        layerdx = 0;
      }
    } // end of loop over CDC hits for this track

    if (dedxTrack->dedx.empty()) {
      B2DEBUG(50, "Found track with no hits, ignoring.");
      continue;
    }

    // calculate likelihoods for truncated mean
    if (!m_useIndividualHits or m_enableDebugOutput) {
      calculateMeans(&(dedxTrack->m_dedx_avg),
                     &(dedxTrack->m_dedx_avg_truncated),
                     &(dedxTrack->m_dedx_avg_truncated_err),
                     dedxTrack->dedx);
      const int numDedx = dedxTrack->dedx.size();
      dedxTrack->m_nLayerHits = numDedx;
      // add a factor of 0.5 here to make sure we are rounding appropriately...
      const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.5);
      const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.5);
      dedxTrack->m_nLayerHitsUsed = highEdgeTrunc - lowEdgeTrunc;
      saveChiValue(dedxTrack->m_cdcChi, dedxTrack->m_predmean, dedxTrack->m_predres, dedxTrack->m_p_cdc, dedxTrack->m_dedx_avg_truncated,
                   std::sqrt(1 - dedxTrack->m_cosTheta * dedxTrack->m_cosTheta), dedxTrack->m_nLayerHitsUsed);
    }

    // save the PID information if not using individual hits
    if (!m_useIndividualHits) {
      saveLookupLogl(dedxTrack->m_cdcLogl, dedxTrack->m_p_cdc, dedxTrack->m_dedx_avg_truncated, m_pdfs[2]);
    }

    if (m_enableDebugOutput) {
      // book the information for this track
      CDCDedxTrack* newCDCDedxTrack = dedxArray.appendNew(*dedxTrack);
      track.addRelationTo(newCDCDedxTrack);
    }

    // save CDCDedxLikelihood
    if (!m_pdfFile.empty()) {
      CDCDedxLikelihood* likelihoodObj = likelihoodArray.appendNew(dedxTrack->m_cdcLogl);
      track.addRelationTo(likelihoodObj);
    }

  } // end of loop over tracks
}

void CDCDedxPIDModule::terminate()
{

  B2INFO("CDCDedxPIDModule exiting after processing " << m_trackID <<
         " tracks in " << m_eventID + 1 << " events.");
}

void CDCDedxPIDModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr,
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

void CDCDedxPIDModule::saveLookupLogl(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx,
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

double CDCDedxPIDModule::bgCurve(double* x, double* par) const
{
  // calculate the predicted mean value as a function of beta-gamma (bg)
  // this is done with a different function depending on the value of bg
  double f = 0;
  if (par[0] == 1)
    f = par[1] * std::pow(std::sqrt(x[0] * x[0] + 1), par[3]) / std::pow(x[0], par[3]) *
        (par[2] - par[5] * std::log(1 / x[0])) - par[4] + std::exp(par[6] + par[7] * x[0]);
  else if (par[0] == 2)
    f = par[1] * std::pow(x[0], 3) + par[2] * x[0] * x[0] + par[3] * x[0] + par[4];
  else if (par[0] == 3)
    f = -1.0 * par[1] * std::log(par[4] + std::pow(1 / x[0], par[2])) + par[3];

  return f;
}

double CDCDedxPIDModule::getMean(double bg) const
{
  // define the section of the curve to use
  double A = 0, B = 0, C = 0;
  if (bg < 4.5)
    A = 1;
  else if (bg < 10)
    B = 1;
  else
    C = 1;

  double x[1]; x[0] = bg;
  double parsA[9];
  double parsB[5];
  double parsC[5];

  parsA[0] = 1; parsB[0] = 2; parsC[0] = 3;
  for (int i = 0; i < 15; ++i) {
    if (i < 7) parsA[i + 1] = m_curvepars[i];
    else if (i < 11) parsB[i % 7 + 1] = m_curvepars[i];
    else parsC[i % 11 + 1] = m_curvepars[i];
  }

  // calculate dE/dx from the Bethe-Bloch curve
  double partA = bgCurve(x, parsA);
  double partB = bgCurve(x, parsB);
  double partC = bgCurve(x, parsC);

  return (A * partA + B * partB + C * partC);
}

double CDCDedxPIDModule::sigmaCurve(double* x, double* par) const
{
  // calculate the predicted mean value as a function of beta-gamma (bg)
  // this is done with a different function depending dE/dx, nhit, and sin(theta)
  double f = 0;
  if (par[0] == 1) {
    // return dedx parameterization
    f = par[1] + par[2] * x[0];
  } else if (par[0] == 2) {
    // return nhit or sin(theta) parameterization
    f = par[1] * std::pow(x[0], 4) + par[2] * std::pow(x[0], 3) +
        par[3] * x[0] * x[0] + par[4] * x[0] + par[5];
  }

  return f;
}


double CDCDedxPIDModule::getSigma(double dedx, double nhit, double sin) const
{
  if (nhit < 5) nhit = 5;
  if (sin > 0.99) sin = 0.99;

  double x[1];
  double dedxpar[3];
  double nhitpar[6];
  double sinpar[6];

  dedxpar[0] = 1; nhitpar[0] = 2; sinpar[0] = 2;
  for (int i = 0; i < 5; ++i) {
    if (i < 2) dedxpar[i + 1] = m_sigmapars[i];
    nhitpar[i + 1] = m_sigmapars[i + 2];
    sinpar[i + 1] = m_sigmapars[i + 7];
  }

  // determine sigma from the parameterization
  x[0] = dedx;
  double corDedx = sigmaCurve(x, dedxpar);
  x[0] = nhit;
  double corNHit = sigmaCurve(x, nhitpar);
  if (nhit > 35) corNHit = 1.0;
  x[0] = sin;
  double corSin = sigmaCurve(x, sinpar);

  return (corDedx * corSin * corNHit);
}


void CDCDedxPIDModule::saveChiValue(double(&chi)[Const::ChargedStable::c_SetSize],
                                    double(&predmean)[Const::ChargedStable::c_SetSize], double(&predsigma)[Const::ChargedStable::c_SetSize], double p, double dedx,
                                    double sin, int nhit) const
{
  // account for the fact that electron truncated mean peaks near 60 units -> scale to 1
  // this should be done via calibration
  dedx = dedx / 59.267;

  // determine a chi value for each particle type
  Const::ParticleSet set = Const::chargedStableSet;
  for (const Const::ChargedStable& pdgIter : set) {
    double bg = p / pdgIter.getMass();

    // determine the predicted mean and resolution
    double mean = getMean(bg);
    double sigma = getSigma(mean, nhit, sin);

    predmean[pdgIter.getIndex()] = mean;
    predsigma[pdgIter.getIndex()] = sigma;

    // fill the chi value for this particle type
    if (sigma != 0) chi[pdgIter.getIndex()] = ((dedx - mean) / (sigma));
  }
}
