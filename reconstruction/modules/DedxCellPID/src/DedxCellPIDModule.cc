/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/DedxCellPID/DedxCellPIDModule.h>
#include <reconstruction/modules/DedxCellPID/LineHelper.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <reconstruction/dataobjects/DedxCell.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>
#include <geometry/GeometryManager.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/TGeoMaterialInterface.h>
#include <genfit/StateOnPlane.h>

#include <TGeoManager.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <time.h>

using namespace Belle2;
using namespace CDC;
using namespace Dedx;

REG_MODULE(DedxCellPID)

DedxCellPIDModule::DedxCellPIDModule() : Module(), m_pdfs()
{

  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the CDC, SVD and PXD.");

  addParam("useIndividualHits", m_useIndividualHits, "Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used.", true);
  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.0));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.2));

  addParam("enableDebugOutput", m_enableDebugOutput, "Option to write out debugging information to DedxCells (DataStore objects).", false);

  addParam("pdfFile", m_pdfFile, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.", std::string("/data/reconstruction/dedxPID_PDFs_r8682_200k_events_upper_80perc_trunc.root"));
  addParam("ignoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);

  m_eventID = -1;
  m_trackID = 0;
}

DedxCellPIDModule::~DedxCellPIDModule() { }

void DedxCellPIDModule::initialize()
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
  StoreArray<CDCHit> CDCHits;

  tracks.isRequired();
  gfTracks.isRequired();
  trackfitResults.isRequired();
  trackCandidates.isRequired();
  CDCHits.isRequired();

  //optional inputs
  StoreArray<MCParticle> mcparticles;
  mcparticles.isOptional();
  tracks.optionalRelationTo(mcparticles);

  // register outputs
  if (m_enableDebugOutput) {
    StoreArray<DedxCell> dedxCells;
    dedxCells.registerInDataStore();
    tracks.registerRelationTo(dedxCells);
  }

  if (!m_pdfFile.empty()) {
    StoreArray<DedxLikelihood> dedxLikelihoods;
    dedxLikelihoods.registerInDataStore();
    tracks.registerRelationTo(dedxLikelihoods);

    //load pdfs
    TFile* pdf_file = new TFile(m_pdfFile.c_str(), "READ");
    if (!pdf_file->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFile);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    for (int detector = 0; detector < c_num_detectors; detector++) {
      int nBinsX, nBinsY;
      double xMin, xMax, yMin, yMax;
      nBinsX = nBinsY = -1;
      xMin = xMax = yMin = yMax = 0.0;
      for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
        const int pdg_code = Const::chargedStableSet.at(iPart).getPDGCode();
        m_pdfs[detector][iPart] =
          dynamic_cast<TH2F*>(pdf_file->Get(TString::Format("hist_d%i_%i%s", detector, pdg_code, suffix)));

        if (!m_pdfs[detector][iPart]) {
          if (m_ignoreMissingParticles)
            continue;
          B2FATAL("Couldn't find PDF for PDG " << pdg_code << ", detector " << detector << suffix);
        }

        //check that PDFs have the same dimensions and same binning
        const double eps_factor = 1e-5;
        if (nBinsX == -1 and nBinsY == -1) {
          nBinsX = m_pdfs[detector][iPart]->GetNbinsX();
          nBinsY = m_pdfs[detector][iPart]->GetNbinsY();
          xMin = m_pdfs[detector][iPart]->GetXaxis()->GetXmin();
          xMax = m_pdfs[detector][iPart]->GetXaxis()->GetXmax();
          yMin = m_pdfs[detector][iPart]->GetYaxis()->GetXmin();
          yMax = m_pdfs[detector][iPart]->GetYaxis()->GetXmax();
        } else if (nBinsX != m_pdfs[detector][iPart]->GetNbinsX()
                   or nBinsY != m_pdfs[detector][iPart]->GetNbinsY()
                   or fabs(xMin - m_pdfs[detector][iPart]->GetXaxis()->GetXmin()) > eps_factor * xMax
                   or fabs(xMax - m_pdfs[detector][iPart]->GetXaxis()->GetXmax()) > eps_factor * xMax
                   or fabs(yMin - m_pdfs[detector][iPart]->GetYaxis()->GetXmin()) > eps_factor * yMax
                   or fabs(yMax - m_pdfs[detector][iPart]->GetYaxis()->GetXmax()) > eps_factor * yMax) {
          B2FATAL("PDF for PDG " << pdg_code << ", detector " << detector << suffix << " has binning/dimensions differing from previous PDF.")
        }
      }
    }

    //leaking pdf_file so I can access the histograms
  }

  // create instances here to not confuse profiling
  CDCGeometryPar::Instance();
  VXD::GeoCache::getInstance();

  // if the TGeo geometry is not initialized, do it ourselves
  if (!gGeoManager) {
    // convert geant4 geometry to TGeo geometry
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();

    // initialize some things for genfit
    genfit::FieldManager::getInstance()->init(new GFGeant4Field());
    genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());
  }
}

void DedxCellPIDModule::event()
{

  m_eventID++;

  // inputs
  StoreArray<Track> tracks;
  StoreArray<CDCHit> cdcHits;
  StoreArray<MCParticle> mcparticles;

  // outputs
  StoreArray<DedxCell> dedxArray;
  StoreArray<DedxLikelihood> likelihoodArray;

  // get the geometry of the cdc
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  // **************************************************
  //
  //  LOOP OVER TRACKS
  //
  // **************************************************

  m_trackID = 0;
  for (int iTrack = 0; iTrack < tracks.getEntries(); iTrack++) {
    const Track* track = tracks[iTrack];
    m_trackID++;

    boost::shared_ptr<DedxCell> dedxCell = boost::make_shared<DedxCell>();
    dedxCell->m_eventID = m_eventID;
    dedxCell->m_trackID = m_trackID;

    // get pion fit hypothesis for now
    const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }
    TVector3 trackMom = fitResult->getMomentum();
    TVector3 trackPos = fitResult->getPosition();
    dedxCell->m_p = trackMom.Mag();
    dedxCell->m_cosTheta = trackMom.CosTheta();

    // calculate dE/dx values using associated genfit::Track
    const genfit::Track* gftrack = fitResult->getRelatedFrom<genfit::Track>();
    if (!gftrack) {
      B2WARNING("No related track for this fit...");
      continue;
    }

    // use the cardinal track representation (by default, the first element)
    genfit::AbsTrackRep* trackrep = gftrack->getCardinalRep();
    const genfit::AbsFitterInfo* fitterInfo = gftrack->getPointWithMeasurement(0)->getFitterInfo(trackrep);
    if (!fitterInfo) {
      B2WARNING("No fitterInfo found, skipping track...");
      continue;
    }

    if (gftrack->getFitStatus(trackrep)->isTrackPruned()) {
      B2ERROR("GFTrack is pruned, please run DedxPID only on unpruned tracks! Skipping this track.");
      continue;
    }

    // this enables an extrapolation of the track fit
    genfit::StateOnPlane pocaState = fitterInfo->getFittedState(true);

    // loop over all CDC hits from this track
    genfit::TrackCand* gftrackcand = fitResult->getRelatedFrom<genfit::TrackCand>();
    if (!gftrackcand || gftrackcand->getNHits() == 0) {
      B2WARNING("Track has no associated hits, skipping");
      continue;
    }
    gftrackcand->sortHits();

    const std::vector<int>& cdcHitIDs = gftrackcand->getHitIDs(Const::CDC);
    for (unsigned int iCDC = 0; iCDC < cdcHitIDs.size(); ++iCDC) {
      int cdcIDx = cdcHitIDs[iCDC];

      // get the global wire ID (between 0 and 14336) and the layer info
      WireID wireID(cdcHits[cdcIDx]->getID());
      int layer = cdcHits[cdcIDx]->getILayer();
      int superlayer = cdcHits[cdcIDx]->getISuperLayer();
      int currentLayer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer);

      // only keep one hit per layer
      if (iCDC > 0) {
        int prevILayer = cdcHits[cdcHitIDs[iCDC - 1]]->getILayer();
        int prevSuperlayer = cdcHits[cdcHitIDs[iCDC - 1]]->getISuperLayer();
        int prevLayer = (prevSuperlayer == 0) ? prevILayer : (8 + (prevSuperlayer - 1) * 6 + prevILayer);

        if (currentLayer == prevLayer) {
          continue;
        }
      }

      if (iCDC + 1 < cdcHitIDs.size()) {
        int nextILayer = cdcHits[cdcHitIDs[iCDC + 1]]->getILayer();
        int nextSuperlayer = cdcHits[cdcHitIDs[iCDC + 1]]->getISuperLayer();
        int nextLayer = (nextSuperlayer == 0) ? nextILayer : (8 + (nextSuperlayer - 1) * 6 + nextILayer);

        if (currentLayer == nextLayer) {
          continue;
        }
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
      const Point tl = Point(-topHalfWidth, topHeight);
      const Point tr = Point(topHalfWidth, topHeight);
      const Point br = Point(bottomHalfWidth, -bottomHeight);
      const Point bl = Point(-bottomHalfWidth, -bottomHeight);
      Cell c = Cell(tl, tr, br, bl);

      // determine the doca for this hit
      try {

        // extrapolate the track to the sense wire of interest
        trackrep->extrapolateToLine(pocaState, wirePosF, wireDir);

        TVector3 poca = trackrep->getPos(pocaState);
        TVector3 pocaMom = trackrep->getMom(pocaState);
        TVector3 wirePoca = wirePosF + (poca - wirePosF).Dot(wireDir) * wireDir;
        TVector3 wireDoca = poca - wirePoca;

        // the sign of the doca is defined here to be positive in the +x dir
        double doca = wireDoca.x() / abs(wireDoca.x()) * wireDoca.Perp();
        double entAng = cdcgeo.getAlpha(wirePoca, pocaMom);
        double hitCharge = cdcHits[cdcIDx]->getADCCount(); // pedestal subtracted?

        // now calculate the path length for this hit
        double celldx = c.dx(doca, entAng);
        celldx = celldx / sin(trackMom.Theta());
        if (!c.isValid()) continue;
        double cellDedx = hitCharge / celldx;

        // save the information for this hit
        dedxCell->addHit(currentLayer, (short)wireID, doca, entAng, hitCharge, celldx, cellDedx, cellHeight, cellHalfWidth);
        if (!m_pdfFile.empty() and m_useIndividualHits) {
          saveLogLikelihood(dedxCell->m_cdcLogL, dedxCell->m_p, cellDedx, m_pdfs[c_CDC]);
        }
      } catch (genfit::Exception) {
        B2WARNING("Event " << m_eventID << ", Track: " << iTrack << ": genfit::Track extrapolation failed (in CDC)");
        continue;
      }
    } // end of loop over CDC hits for this track

    if (dedxCell->size() == 0) {
      B2WARNING("No good hits on this track...");
      continue;
    }

    if (!m_useIndividualHits or m_enableDebugOutput) {
      calculateMeans(
        &(dedxCell->m_dedx_avg),
        &(dedxCell->m_dedx_avg_truncated),
        &(dedxCell->m_dedx_avg_truncated_err),
        dedxCell->m_dEdx);
    }

    // calculate likelihoods for truncated mean
    if (!m_useIndividualHits) {
      saveLogLikelihood(dedxCell->m_cdcLogL, dedxCell->m_p, dedxCell->m_dedx_avg_truncated, m_pdfs[c_CDC]);
    }

    if (m_enableDebugOutput) {
      // book the information for this track
      DedxCell* newDedxCell = dedxArray.appendNew(*dedxCell);
      track->addRelationTo(newDedxCell);
    }

    // save DedxLikelihood
    if (!m_pdfFile.empty()) {
      // only cdc information is included for now -> svd likelihood is 0
      DedxLikelihood* likelihoodObj = likelihoodArray.appendNew(dedxCell->m_cdcLogL, dedxCell->m_svdLogL);
      track->addRelationTo(likelihoodObj);
    }

  } // end of loop over tracks
}

void DedxCellPIDModule::terminate()
{

  B2INFO("DedxCellPIDModule exiting after processing " << m_trackID <<
         " tracks in " << m_eventID + 1 << " events.");
}

void DedxCellPIDModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const
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
  const int lowEdgeTrunc = int(numDedx * m_removeLowest);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest));
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
    *truncatedMeanErr = sqrt(sum_of_squares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}


void DedxCellPIDModule::saveLogLikelihood(float(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx, TH2F* const* pdf) const
{

  //all pdfs have the same dimensions
  const Int_t bin_x = pdf[0]->GetXaxis()->FindFixBin(p);
  const Int_t bin_y = pdf[0]->GetYaxis()->FindFixBin(dedx);

  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    if (!pdf[iPart])
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
