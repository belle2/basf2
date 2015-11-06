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

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
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
           "Option to write out debugging information to CDCDedxTracks (DataStore objects).", false);
  addParam("pdfFile", m_pdfFile, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.",
           std::string("/data/reconstruction/dedxPID_PDFs_r22564_400k_events.root"));
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
  StoreArray<genfit::Track> gfTracks;
  StoreArray<TrackFitResult> trackfitResults;
  StoreArray<genfit::TrackCand> trackCandidates;

  tracks.isRequired();
  gfTracks.isRequired();
  trackfitResults.isRequired();
  trackCandidates.isRequired();

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
    TFile* pdf_file = new TFile(m_pdfFile.c_str(), "READ");
    if (!pdf_file->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFile);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    int nBinsX, nBinsY;
    double xMin, xMax, yMin, yMax;
    nBinsX = nBinsY = -1;
    xMin = xMax = yMin = yMax = 0.0;
    for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
      const int pdg_code = Const::chargedStableSet.at(iPart).getPDGCode();
      m_pdfs[2][iPart] =
        dynamic_cast<TH2F*>(pdf_file->Get(TString::Format("hist_d%i_%i%s", 2, pdg_code, suffix)));
      //dynamic_cast<TH2F*>(pdf_file->Get(TString::Format("hist_%i%s", pdg_code, suffix)));

      if (!m_pdfs[2][iPart]) {
        if (m_ignoreMissingParticles)
          continue;
        B2FATAL("Couldn't find PDF for PDG " << pdg_code << suffix);
      }

      //check that PDFs have the same dimensions and same binning
      const double eps_factor = 1e-5;
      if (nBinsX == -1 and nBinsY == -1) {
        nBinsX = m_pdfs[2][iPart]->GetNbinsX();
        nBinsY = m_pdfs[2][iPart]->GetNbinsY();
        xMin = m_pdfs[2][iPart]->GetXaxis()->GetXmin();
        xMax = m_pdfs[2][iPart]->GetXaxis()->GetXmax();
        yMin = m_pdfs[2][iPart]->GetYaxis()->GetXmin();
        yMax = m_pdfs[2][iPart]->GetYaxis()->GetXmax();
      } else if (nBinsX != m_pdfs[2][iPart]->GetNbinsX()
                 or nBinsY != m_pdfs[2][iPart]->GetNbinsY()
                 or fabs(xMin - m_pdfs[2][iPart]->GetXaxis()->GetXmin()) > eps_factor * xMax
                 or fabs(xMax - m_pdfs[2][iPart]->GetXaxis()->GetXmax()) > eps_factor * xMax
                 or fabs(yMin - m_pdfs[2][iPart]->GetYaxis()->GetXmin()) > eps_factor * yMax
                 or fabs(yMax - m_pdfs[2][iPart]->GetYaxis()->GetXmax()) > eps_factor * yMax) {
        B2FATAL("PDF for PDG " << pdg_code << suffix << " has binning/dimensions differing from previous PDF.")
      }
    }

    //leaking pdf_file so I can access the histograms
  }

  // create instances here to not confuse profiling
  CDCGeometryPar::Instance();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Need to have SetupGenfitExtrapolationModule in path before this one.");
  }
}

void CDCDedxPIDModule::event()
{
  // go through Tracks
  // get fitresult and gftrack and do extrapolations, save corresponding dE/dx and likelihood values
  //   get genfit::TrackCand through genfit::Track::getCand()
  //   get hit indices through genfit::TrackCand::getHit(...)
  //   create one CDCDedxTrack per fitresult/gftrack
  //create one DedkLikelihood per Track (plus rel)
  m_eventID++;

  // inputs
  StoreArray<Track> tracks;
  StoreArray<MCParticle> mcparticles;
  const int num_mcparticles = mcparticles.getEntries();

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

    if ((m_enableDebugOutput or m_onlyPrimaryParticles) and num_mcparticles != 0) {
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

        const TVector3 true_momentum = mcpart->getMomentum();
        dedxTrack->m_p_true = true_momentum.Mag();
      }
    }

    // get momentum (at origin) from fit result
    const TVector3& trackMom = fitResult->getMomentum();
    dedxTrack->m_p = trackMom.Mag();
    dedxTrack->m_cosTheta = trackMom.CosTheta();
    dedxTrack->m_charge = fitResult->getChargeSign();

    // dE/dx values will be calculated using associated genfit::Track
    const genfit::Track* gftrack = fitResult->getRelatedFrom<genfit::Track>();
    if (!gftrack) {
      B2WARNING("No related track for this fit...");
      continue;
    }

    // Check to see if the track is pruned
    genfit::AbsTrackRep* trackrep = gftrack->getCardinalRep();
    if (gftrack->getFitStatus(trackrep)->isTrackPruned()) {
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
    const std::vector< genfit::TrackPoint* > gftrackPoints = gftrack->getPointsWithMeasurement();
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
      const genfit::AbsFitterInfo* fi = (*tp)->getFitterInfo(trackrep);
      if (!fi) {
        B2DEBUG("No fitter info, skipping...");
        continue;
      }

      // get the global wire ID (between 0 and 14336) and the layer info
      WireID wireID = cdcRecoHit->getWireID();
      const int wire = wireID.getIWire();
      int layer = cdcHit->getILayer();
      int superlayer = cdcHit->getISuperLayer();
      int currentLayer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer);

      // if multiple hits in a layer, we may combine the hits
      const bool lastHit = (tp + 1 == gftrackPoints.end());
      bool lastHitInCurrentLayer = lastHit;
      if (!lastHit) {
        //peek at next hit
        genfit::AbsMeasurement* aAbsMeasurementPtr = (*(tp + 1))->getRawMeasurement(0);
        const CDCRecoHit* nextcdcRecoHit = dynamic_cast<const CDCRecoHit* >(aAbsMeasurementPtr);
        if (!nextcdcRecoHit) {
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
      const TVector3& wirePosF = cdcgeo.wireForwardPosition(wireID);
      const TVector3& wirePosB = cdcgeo.wireBackwardPosition(wireID);
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
        if (!c.isValid()) continue;

        layerdE += adcCount;
        layerdx += celldx;

        // save individual hits
        double cellDedx = (adcCount / celldx) * sin(trackMom.Theta());
        if (m_enableDebugOutput)
          dedxTrack->addHit(wire, currentLayer, doca, entAng, adcCount, hitCharge, celldx, cellDedx, cellHeight, cellHalfWidth, driftT,
                            driftDRealistic, driftDRealisticRes);
      } catch (genfit::Exception) {
        B2WARNING("Event " << m_eventID << ", Track: " << m_trackID << ": genfit::MeasuredStateOnPlane exception...");
        continue;
      }

      // check if there are any more hits in this layer
      if (lastHitInCurrentLayer) {
        double totalDistance = layerdx / sin(trackMom.Theta());
        double layerDedx = layerdE / totalDistance;

        // save the information for this layer
        if (layerDedx > 0) {
          dedxTrack->addDedx(currentLayer, totalDistance, layerDedx);
          // save the PID information if using individual hits
          if (!m_pdfFile.empty() and m_useIndividualHits) {
            // use the momentum valid in the cdc
            saveLogLikelihood(dedxTrack->m_cdcLogl, dedxTrack->m_p_cdc, layerDedx, m_pdfs[2]);
          }
        }

        layerdE = 0;
        layerdx = 0;
      }
    } // end of loop over CDC hits for this track


    if (dedxTrack->dedx.empty()) {
      B2DEBUG("Found track with no hits, ignoring.");
      continue;
    }

    // calculate likelihoods for truncated mean
    if (!m_useIndividualHits or m_enableDebugOutput) {
      calculateMeans(&(dedxTrack->m_dedx_avg),
                     &(dedxTrack->m_dedx_avg_truncated),
                     &(dedxTrack->m_dedx_avg_truncated_err),
                     dedxTrack->dedx);
      const int numDedx = dedxTrack->dedx.size();
      dedxTrack->m_nHits = numDedx;
      // add a factor of 0.5 here to make sure we are rounding appropriately...
      const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.5);
      const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.5);
      dedxTrack->m_nHitsUsed = highEdgeTrunc - lowEdgeTrunc;
    }

    // save the PID information if not using individual hits
    if (!m_useIndividualHits) {
      saveLogLikelihood(dedxTrack->m_cdcLogl, dedxTrack->m_p_cdc, dedxTrack->m_dedx_avg_truncated, m_pdfs[2]);
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
  double sum_of_squares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();

  // add a factor of 0.5 here to make sure we are rounding appropriately...
  const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.5);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.5);
  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
    if (i >= lowEdgeTrunc and i < highEdgeTrunc) {
      truncatedMeanTmp += sortedDedx[i];
      sum_of_squares += sortedDedx[i] * sortedDedx[i];
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
    *truncatedMeanErr = sqrt(sum_of_squares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(
                          numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}

void CDCDedxPIDModule::saveLogLikelihood(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx,
                                         TH2F* const* pdf) const
{
  //all pdfs have the same dimensions
  const Int_t bin_x = pdf[0]->GetXaxis()->FindFixBin(p);
  const Int_t bin_y = pdf[0]->GetYaxis()->FindFixBin(dedx);


  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    if (!pdf[iPart]) //might be NULL if m_ignoreMissingParticles is set
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (bin_x < 1 or bin_x > pdf[iPart]->GetNbinsX()
        or bin_y < 1 or bin_y > pdf[iPart]->GetNbinsY()) {
      probability = pdf[iPart]->GetBinContent(bin_x, bin_y);
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
