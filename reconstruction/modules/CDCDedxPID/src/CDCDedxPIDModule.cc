/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxPID/CDCDedxPIDModule.h>

#include <reconstruction/dataobjects/DedxConstants.h>
#include <reconstruction/modules/CDCDedxPID/LineHelper.h>

#include <framework/gearbox/Const.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/MaterialEffects.h>
#include <genfit/KalmanFitterInfo.h>

#include <TH2F.h>
#include <TRandom.h>

#include <memory>
#include <cmath>
#include <algorithm>

using namespace Belle2;
using namespace CDC;
using namespace Dedx;

REG_MODULE(CDCDedxPID);

CDCDedxPIDModule::CDCDedxPIDModule() : Module()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  //Set module properties
  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the CDC.");

  //Parameter definitions
  addParam("usePrediction", m_usePrediction,
           "Use parameterized means and resolutions to determine PID values. If false, lookup table PDFs are used.", true);
  addParam("removeLowest", m_removeLowest,
           "Portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest,
           "Portion of events with high dE/dx that should be discarded", double(0.25));
  addParam("useBackHalfCurlers", m_backHalfCurlers,
           "Whether to use the back half of curlers", false);
  addParam("enableDebugOutput", m_enableDebugOutput,
           "Option to write out debugging information to CDCDedxTracks (DataStore objects).", true);
  addParam("useIndividualHits", m_useIndividualHits,
           "If using lookup table PDFs, include PDF value for each hit in likelihood. If false, the truncated mean of dedx values will be used.",
           true);
  addParam("ignoreMissingParticles", m_ignoreMissingParticles,
           "Ignore particles for which no PDFs are found", false);
  addParam("trackLevel", m_trackLevel,
           "ONLY USEFUL FOR MC: Use track-level MC. If false, use hit-level MC", true);
  addParam("onlyPrimaryParticles", m_onlyPrimaryParticles,
           "ONLY USEFUL FOR MC: Only save data for primary particles", false);
}

CDCDedxPIDModule::~CDCDedxPIDModule() { }

void CDCDedxPIDModule::checkPDFs()
{
  if (!m_DBDedxPDFs) B2FATAL("No Dedx pdfs found in database");
  //load dedx:momentum PDFs
  int nBinsX, nBinsY;
  double xMin, xMax, yMin, yMax;
  nBinsX = nBinsY = -1;
  xMin = xMax = yMin = yMax = 0.0;
  for (unsigned int iPart = 0; iPart < 6; iPart++) {
    const int pdgCode = Const::chargedStableSet.at(iPart).getPDGCode();
    const TH2F* pdf = m_DBDedxPDFs->getCDCPDF(iPart, !m_useIndividualHits);

    if (pdf->GetEntries() == 0) {
      if (m_ignoreMissingParticles)
        continue;
      B2FATAL("Couldn't find PDF for PDG " << pdgCode);
    }

    //check that PDFs have the same dimensions and same binning
    const double epsFactor = 1e-5;
    if (nBinsX == -1 and nBinsY == -1) {
      nBinsX = pdf->GetNbinsX();
      nBinsY = pdf->GetNbinsY();
      xMin = pdf->GetXaxis()->GetXmin();
      xMax = pdf->GetXaxis()->GetXmax();
      yMin = pdf->GetYaxis()->GetXmin();
      yMax = pdf->GetYaxis()->GetXmax();
    } else if (nBinsX != pdf->GetNbinsX()
               or nBinsY != pdf->GetNbinsY()
               or fabs(xMin - pdf->GetXaxis()->GetXmin()) > epsFactor * xMax
               or fabs(xMax - pdf->GetXaxis()->GetXmax()) > epsFactor * xMax
               or fabs(yMin - pdf->GetYaxis()->GetXmin()) > epsFactor * yMax
               or fabs(yMax - pdf->GetYaxis()->GetXmax()) > epsFactor * yMax) {
      B2FATAL("PDF for PDG " << pdgCode << " has binning/dimensions differing from previous PDF.");
    }
  }
}

void CDCDedxPIDModule::initialize()
{

  // required inputs
  m_tracks.isRequired();
  m_recoTracks.isRequired();

  // optional inputs
  m_mcparticles.isOptional();
  m_tracks.optionalRelationTo(m_mcparticles);

  // register optional outputs
  if (m_enableDebugOutput) {
    m_dedxTracks.registerInDataStore();
    m_tracks.registerRelationTo(m_dedxTracks);
  }

  // register outputs
  m_dedxLikelihoods.registerInDataStore();
  m_tracks.registerRelationTo(m_dedxLikelihoods);

  m_DBDedxPDFs.addCallback([this]() { checkPDFs(); });
  checkPDFs();

  // lookup table for number of wires per layer (indexed on superlayer)
  m_nLayerWires[0] = 1280;
  for (int i = 1; i < 9; ++i) {
    m_nLayerWires[i] = m_nLayerWires[i - 1] + 6 * (160 + (i - 1) * 32);
  }

  // make sure the mean and resolution parameters are reasonable
  if (!m_DBMeanPars || m_DBMeanPars->getSize() == 0) {
    B2WARNING("No dE/dx mean parameters!");
    for (int i = 0; i < 15; ++i)
      m_meanpars.push_back(1.0);
  } else m_meanpars = m_DBMeanPars->getMeanPars();

  if (!m_DBSigmaPars || m_DBSigmaPars->getSize() == 0) {
    B2WARNING("No dE/dx sigma parameters!");
    for (int i = 0; i < 12; ++i)
      m_sigmapars.push_back(1.0);
  } else m_sigmapars = m_DBSigmaPars->getSigmaPars();

  // get the hadron correction parameters
  if (!m_DBHadronCor || m_DBHadronCor->getSize() == 0) {
    B2WARNING("No hadron correction parameters!");
    for (int i = 0; i < 4; ++i)
      m_hadronpars.push_back(0.0);
    m_hadronpars.push_back(1.0);
  } else m_hadronpars = m_DBHadronCor->getHadronPars();

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
  // create one DedkLikelihood per Track (plus rel)

  // inputs
  const int numMCParticles = m_mcparticles.getEntries();

  // get the geometry of the cdc
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  // **************************************************
  //
  //  LOOP OVER TRACKS
  //
  // **************************************************

  m_dedxTracks.clear();
  m_dedxLikelihoods.clear();

  int mtrack = 0;
  for (const auto& track : m_tracks) {
    std::shared_ptr<CDCDedxTrack> dedxTrack = std::make_shared<CDCDedxTrack>();
    dedxTrack->m_track = mtrack++;

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

        //add some MC truths to CDCDedxTrack object
        dedxTrack->m_pdg = mcpart->getPDG();
        dedxTrack->m_mcmass = mcpart->getMass();
        const MCParticle* mother = mcpart->getMother();
        dedxTrack->m_motherPDG = mother ? mother->getPDG() : 0;

        const ROOT::Math::XYZVector trueMomentum = mcpart->getMomentum();
        dedxTrack->m_pTrue = trueMomentum.R();
        dedxTrack->m_cosThetaTrue = cos(trueMomentum.Theta());
      }
    } else {
      dedxTrack->m_pdg = -999;
    }

    // get momentum (at origin) from fit result
    const ROOT::Math::XYZVector& trackMom = fitResult->getMomentum();
    dedxTrack->m_p = trackMom.R();
    bool nomom = (dedxTrack->m_p != dedxTrack->m_p);
    double costh = std::cos(std::atan(1 / fitResult->getCotTheta()));
    int charge = 1;
    if (!nomom) {
      costh = cos(trackMom.Theta());
      charge = fitResult->getChargeSign();
    }
    dedxTrack->m_cosTheta = costh;
    dedxTrack->m_charge = charge;

    double injring = -1.0, injtime = -1.0;
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo;
    if (m_TTDInfo.isValid() && m_TTDInfo->hasInjection()) {
      injring = m_TTDInfo->isHER();
      injtime =  m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds();
    }
    dedxTrack->m_injring = injring;
    dedxTrack->m_injtime = injtime;

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

    // scale factor to make electron dE/dx ~ 1
    dedxTrack->m_scale = (m_DBScaleFactor) ? m_DBScaleFactor->getScaleFactor() : 1.0;

    // store run gains only for data!
    bool isData = false;
    if (m_usePrediction && numMCParticles == 0)isData = true;
    dedxTrack->m_runGain = (m_DBRunGain && isData) ? m_DBRunGain->getRunGain() : 1.0;

    // get the cosine correction only for data!
    dedxTrack->m_cosCor = (m_DBCosineCor && isData) ? m_DBCosineCor->getMean(costh) : 1.0;

    // get the cosine edge correction only for data!
    bool isEdge = false;
    if ((abs(costh + 0.860) < 0.010) || (abs(costh - 0.955) <= 0.005))isEdge = true;
    dedxTrack->m_cosEdgeCor = (m_DBCosEdgeCor && isData && isEdge) ? m_DBCosEdgeCor->getMean(costh) : 1.0;

    bool isvalidTime = true;
    if (injtime < 0 || injring < 0)isvalidTime = false;
    dedxTrack->m_timeGain = (m_DBInjectTime && isData && isvalidTime) ? m_DBInjectTime->getCorrection("mean", injring, injtime) : 1.0;
    dedxTrack->m_timeReso = (m_DBInjectTime && isData && isvalidTime) ? m_DBInjectTime->getCorrection("reso", injring, injtime) : 1.0;

    // initialize a few variables to be used in the loop over track points
    double layerdE = 0.0; // total charge in current layer
    double layerdx = 0.0; // total path length in current layer
    double cdcMom = 0.0; // momentum valid in the CDC
    int nhitscombined = 0; // number of hits combined per layer
    int wirelongesthit = 0; // wire number of longest hit
    double longesthit = 0; // path length of longest hit

    // loop over all CDC hits from this track
    // Get the TrackPoints, which contain the hit information we need.
    // Then iterate over each point.
    const std::vector< genfit::AbsTrackRep* >& gftrackRepresentations = recoTrack->getRepresentations();
    const std::vector< genfit::TrackPoint* >& gftrackPoints = recoTrack->getHitPointsWithMeasurement();
    for (std::vector< genfit::TrackPoint* >::const_iterator tp = gftrackPoints.begin();
         tp != gftrackPoints.end(); ++tp) {

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
        B2DEBUG(29, "No fitter info, skipping...");
        continue;
      }

      // get the wire ID (not between 0 and 14336) and the layer info
      WireID wireID = cdcRecoHit->getWireID();
      const int wire = wireID.getIWire(); // use getEWire() for encoded wire number
      int layer = cdcHit->getILayer(); // layer within superlayer
      int superlayer = cdcHit->getISuperLayer();

      // check which algorithm found this hit
      int foundByTrackFinder = 0;
      const RecoHitInformation* hitInfo = recoTrack->getRecoHitInformation(cdcHit);
      foundByTrackFinder = hitInfo->getFoundByTrackFinder();

      // add weights for hypotheses
      double weightPionHypo = 0;
      double weightProtHypo = 0;
      double weightKaonHypo = 0;
      // loop over all the present hypotheses
      for (std::vector<genfit::AbsTrackRep* >::const_iterator trep = gftrackRepresentations.begin();
           trep != gftrackRepresentations.end(); ++trep) {
        const int pdgCode = TMath::Abs((*trep)->getPDG());
        // configured to only save weights for one of these 3
        if (!(pdgCode == Const::pion.getPDGCode() ||
              pdgCode == Const::kaon.getPDGCode() ||
              pdgCode == Const::proton.getPDGCode())) continue;
        const genfit::KalmanFitterInfo* kalmanFitterInfo = (*tp)->getKalmanFitterInfo(*trep);
        if (kalmanFitterInfo == NULL) {
          //B2WARNING("No KalmanFitterInfo for hit in "<<pdgCode<<" track representationt. Skipping.");
          continue;
        }

        // there are always 2 hits, one of which is ~1, the other ~0; or both ~0. Store only the largest.
        std::vector<double> weights = kalmanFitterInfo->getWeights();
        const double maxWeight = weights[0] > weights[1] ? weights[0] : weights[1];
        if (pdgCode == Const::pion.getPDGCode()) weightPionHypo = maxWeight;
        else if (pdgCode == Const::kaon.getPDGCode()) weightKaonHypo = maxWeight;
        else if (pdgCode == Const::proton.getPDGCode()) weightProtHypo = maxWeight;

      }

      // continuous layer number
      int currentLayer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer);
      int nextLayer = currentLayer;

      // dense packed wire number (between 0 and 14336)
      const int iwire = (superlayer == 0) ? 160 * layer + wire : m_nLayerWires[superlayer - 1] + (160 + 32 *
                        (superlayer - 1)) * layer + wire;

      // if multiple hits in a layer, we may combine the hits
      const bool lastHit = (tp + 1 == gftrackPoints.end());
      bool lastHitInCurrentLayer = lastHit;
      if (!lastHit) {
        // peek at next hit
        genfit::AbsMeasurement* aAbsMeasurementPtrNext = (*(tp + 1))->getRawMeasurement(0);
        const CDCRecoHit* nextcdcRecoHit = dynamic_cast<const CDCRecoHit* >(aAbsMeasurementPtrNext);
        // if next hit fails, assume this is the last hit in the layer
        if (!nextcdcRecoHit || !(cdcRecoHit->getCDCHit()) || !((*(tp + 1))->getFitterInfo())) {
          break;
        }
        const CDCHit* nextcdcHit = nextcdcRecoHit->getCDCHit();
        const int nextILayer = nextcdcHit->getILayer();
        const int nextSuperlayer = nextcdcHit->getISuperLayer();
        nextLayer = (nextSuperlayer == 0) ? nextILayer : (8 + (nextSuperlayer - 1) * 6 + nextILayer);
        lastHitInCurrentLayer = (nextLayer != currentLayer);
      }

      // find the position of the endpoints of the sense wire
      const ROOT::Math::XYZVector& wirePosF = cdcgeo.wireForwardPosition(wireID, CDCGeometryPar::c_Aligned);

      int nWires = cdcgeo.nWiresInLayer(currentLayer);

      // radii of field wires for this layer
      double inner = cdcgeo.innerRadiusWireLayer()[currentLayer];
      double outer = cdcgeo.outerRadiusWireLayer()[currentLayer];

      double topHeight = outer - wirePosF.Rho();
      double bottomHeight = wirePosF.Rho() - inner;
      double cellHeight = topHeight + bottomHeight;
      double topHalfWidth = M_PI * outer / nWires;
      double bottomHalfWidth = M_PI * inner / nWires;
      double cellHalfWidth = M_PI * wirePosF.Rho() / nWires;

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
          dedxTrack->m_pCDC = cdcMom;
        }
        if (nomom)
          dedxTrack->m_p = cdcMom;

        // get the doca and entrance angle information.
        // constructPlane places the coordinate center in the POCA to the
        // wire.  Using this is the default behavior.  If this should be too
        // slow, as it has to re-evaluate the POCA
        //  B2Vector3D pocaOnWire = cdcRecoHit->constructPlane(mop)->getO();

        // uses the plane determined by the track fit.
        B2Vector3D pocaOnWire = mop.getPlane()->getO(); // DOUBLE CHECK THIS --\/

        // The vector from the wire to the track.
        B2Vector3D B2WireDoca = fittedPoca - pocaOnWire;

        // the sign of the doca is defined here to be positive in the +x dir in the cell
        double doca = B2WireDoca.Perp();
        double phidiff = fittedPoca.Phi() - pocaOnWire.Phi();
        // be careful about "wrap around" cases when the poca and wire are close, but
        // the difference in phi is largy
        if (phidiff > -3.1416 && (phidiff < 0 || phidiff > 3.1416)) doca *= -1;

        // The opening angle of the track momentum direction
        const double px = pocaMom.X();
        const double py = pocaMom.Y();
        const double wx = pocaOnWire.X();
        const double wy = pocaOnWire.Y();
        const double cross = wx * py - wy * px;
        const double dot   = wx * px + wy * py;
        double entAng = atan2(cross, dot);

        // re-scaled (RS) doca and entAng variable: map to square cell
        double cellR = 2 * cellHalfWidth / cellHeight;
        double tana = 100.0;
        if (std::abs(2 * atan(1) - std::abs(entAng)) < 0.01)tana = 100 * (entAng / std::abs(entAng)); //avoid infinity at pi/2
        else tana =  std::tan(entAng);
        double docaRS = doca * std::sqrt((1 + cellR * cellR * tana * tana) / (1 + tana * tana));
        double entAngRS = std::atan(tana / cellR);

        LinearGlobalADCCountTranslator translator;
        int adcbaseCount = cdcHit->getADCCount(); // pedestal subtracted?
        int adcCount = (m_DBNonlADC && m_usePrediction
                        && numMCParticles == 0) ? m_DBNonlADC->getCorrectedADC(adcbaseCount, currentLayer) : adcbaseCount;

        double hitCharge = translator.getCharge(adcCount, wireID, false, pocaOnWire.Z(), pocaMom.Phi());
        int driftT = cdcHit->getTDCCount();

        // we want electrons to be one, so artificially scale the adcCount
        double dadcCount = 1.0 * adcCount;
        if (dedxTrack->m_scale != 0) dadcCount /= dedxTrack->m_scale;

        RealisticTDCCountTranslator realistictdc;
        double driftDRealistic = realistictdc.getDriftLength(driftT, wireID, 0, true, pocaOnWire.Z(), pocaMom.Phi(), pocaMom.Theta());
        double driftDRealisticRes = realistictdc.getDriftLengthResolution(driftDRealistic, wireID, true, pocaOnWire.Z(), pocaMom.Phi(),
                                    pocaMom.Theta());

        // now calculate the path length for this hit
        double celldx = c.dx(doca, entAng);
        if (c.isValid()) {
          // get the wire gain constant
          double wiregain = (m_DBWireGains && m_usePrediction && numMCParticles == 0) ? m_DBWireGains->getWireGain(iwire) : 1.0;

          //normalization of rescaled doca wrt cell size for layer dependent 2D corrections
          double normDocaRS = docaRS / cellHalfWidth;
          double twodcor = (m_DB2DCell && m_usePrediction
                            && numMCParticles == 0) ? m_DB2DCell->getMean(currentLayer, normDocaRS, entAngRS) : 1.0;

          // get the 1D cleanup correction
          double onedcor = (m_DB1DCell && m_usePrediction && numMCParticles == 0) ? m_DB1DCell->getMean(currentLayer, entAngRS) : 1.0;

          // apply the calibration to dE to propagate to both hit and layer measurements
          // Note: could move the sin(theta) here since it is common accross the track
          //       It is applied in two places below (hit level and layer level)
          double correction = dedxTrack->m_runGain * dedxTrack->m_cosCor * dedxTrack->m_cosEdgeCor * dedxTrack->m_timeGain * wiregain *
                              twodcor * onedcor;

          // --------------------
          // save individual hits (even for dead wires)
          // --------------------
          if (correction != 0) dadcCount = dadcCount / correction;
          else dadcCount = 0;

          double cellDedx = (dadcCount / celldx);

          // correct for path length through the cell
          if (nomom) cellDedx *= std::sin(std::atan(1 / fitResult->getCotTheta()));
          else cellDedx *= std::sin(trackMom.Theta());

          if (m_enableDebugOutput)
            dedxTrack->addHit(wire, iwire, currentLayer, doca, docaRS, entAng, entAngRS,
                              adcCount, adcbaseCount, hitCharge, celldx, cellDedx, cellHeight, cellHalfWidth, driftT,
                              driftDRealistic, driftDRealisticRes, wiregain, twodcor, onedcor,
                              foundByTrackFinder, weightPionHypo, weightKaonHypo, weightProtHypo);

          // --------------------
          // save layer hits only with active wires
          // --------------------
          if (correction != 0) {

            layerdE += dadcCount;
            layerdx += celldx;

            if (celldx > longesthit) {
              longesthit = celldx;
              wirelongesthit = iwire;
            }
            nhitscombined++;
          }
        }
      } catch (genfit::Exception&) {
        B2WARNING("Track: " << mtrack << ": genfit::MeasuredStateOnPlane exception...");
        continue;
      }

      // check if there are any more hits in this layer
      if (lastHitInCurrentLayer) {
        if (layerdx > 0) {
          double totalDistance;
          if (nomom) totalDistance = layerdx / std::sin(std::atan(1 / fitResult->getCotTheta()));
          else  totalDistance = layerdx / std::sin(trackMom.Theta());
          double layerDedx = layerdE / totalDistance;

          // save the information for this layer
          if (layerDedx > 0) {
            dedxTrack->addDedx(nhitscombined, wirelongesthit, currentLayer, totalDistance, layerDedx);
            // save the PID information if using individual hits
            if (m_useIndividualHits) {
              // use the momentum valid in the cdc
              saveLookupLogl(dedxTrack->m_cdcLogl, dedxTrack->m_pCDC, layerDedx);
            }
          }
        }
        // stop when the track starts to curl
        if (!m_backHalfCurlers && nextLayer < currentLayer) break;

        layerdE = 0;
        layerdx = 0;
        nhitscombined = 0;
        wirelongesthit = 0;
        longesthit = 0;
      }
    } // end of loop over CDC hits for this track

    // Determine the number of hits to be used
    //m_lDedx is a ector for layerDedx and created w/ ->adddedx method above
    if (dedxTrack->m_lDedx.empty()) {
      B2DEBUG(29, "Found track with no hits, ignoring.");
      continue;
    } else {
      // determine the number of hits for this track (used below)
      const int numDedx = dedxTrack->m_lDedx.size();
      // add a factor of 0.51 here to make sure we are rounding appropriately...
      const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.51);
      const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.51);
      dedxTrack->m_lNHitsUsed = highEdgeTrunc - lowEdgeTrunc;
    }

    // Get the truncated mean
    //
    // If using track-level MC, get the predicted mean and resolution and throw a random
    // number to get the simulated dE/dx truncated mean. Otherwise, calculate the truncated
    // mean from the simulated hits (hit-level MC).
    if (!m_useIndividualHits or m_enableDebugOutput) {
      calculateMeans(&(dedxTrack->m_dedxAvg),
                     &(dedxTrack->m_dedxAvgTruncatedNoSat),
                     &(dedxTrack->m_dedxAvgTruncatedErr),
                     dedxTrack->m_lDedx);

      // apply the "hadron correction" only to data
      if (numMCParticles == 0)
        dedxTrack->m_dedxAvgTruncated = D2I(costh, I2D(costh, 1.00) / 1.00 * dedxTrack->m_dedxAvgTruncatedNoSat);
      else dedxTrack->m_dedxAvgTruncated = dedxTrack->m_dedxAvgTruncatedNoSat;
    }

    // If this is a MC track, get the track-level dE/dx
    if (numMCParticles != 0 && dedxTrack->m_mcmass > 0 && dedxTrack->m_pTrue != 0) {
      // determine the predicted mean and resolution
      double mean = getMean(dedxTrack->m_pCDC / dedxTrack->m_mcmass);
      double sigma = getSigma(mean, dedxTrack->m_lNHitsUsed,
                              dedxTrack->m_cosTheta, dedxTrack->m_timeReso);
      dedxTrack->m_simDedx = gRandom->Gaus(mean, sigma);
      while (dedxTrack->m_simDedx < 0)
        dedxTrack->m_simDedx = gRandom->Gaus(mean, sigma);
      if (m_trackLevel)
        dedxTrack->m_dedxAvgTruncated = dedxTrack->m_simDedx;
    }

    // save the PID information for both lookup tables and parameterized means and resolutions
    saveChiValue(dedxTrack->m_cdcChi, dedxTrack->m_predmean, dedxTrack->m_predres, dedxTrack->m_pCDC, dedxTrack->m_dedxAvgTruncated,
                 dedxTrack->m_cosTheta, dedxTrack->m_lNHitsUsed, dedxTrack->m_timeReso);
    if (!m_useIndividualHits)
      saveLookupLogl(dedxTrack->m_cdcLogl, dedxTrack->m_pCDC, dedxTrack->m_dedxAvgTruncated);

    // save CDCDedxLikelihood
    // use parameterized method if called or if pdf file for lookup tables are empty
    double pidvalues[Const::ChargedStable::c_SetSize];
    Const::ParticleSet set = Const::chargedStableSet;
    if (m_usePrediction) {
      for (const Const::ChargedStable pdgIter : set) {
        pidvalues[pdgIter.getIndex()] = -0.5 * dedxTrack->m_cdcChi[pdgIter.getIndex()] * dedxTrack->m_cdcChi[pdgIter.getIndex()];
      }
    } else {
      for (const Const::ChargedStable pdgIter : set) {
        pidvalues[pdgIter.getIndex()] = dedxTrack->m_cdcLogl[pdgIter.getIndex()];
      }
    }

    CDCDedxLikelihood* likelihoodObj = m_dedxLikelihoods.appendNew(pidvalues);
    track.addRelationTo(likelihoodObj);

    if (m_enableDebugOutput) {
      // book the information for this track
      CDCDedxTrack* newCDCDedxTrack = m_dedxTracks.appendNew(*dedxTrack);
      track.addRelationTo(newCDCDedxTrack);
    }

  } // end of loop over tracks
}

void CDCDedxPIDModule::terminate()
{

  B2DEBUG(29, "CDCDedxPIDModule exiting");
}

void CDCDedxPIDModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr,
                                      const std::vector<double>& dedx) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());
  sortedDedx.erase(std::remove(sortedDedx.begin(), sortedDedx.end(), 0), sortedDedx.end());
  sortedDedx.shrink_to_fit();

  double truncatedMeanTmp = 0.0;
  double meanTmp = 0.0;
  double sumOfSquares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();

  // add a factor of 0.51 here to make sure we are rounding appropriately...
  const int lowEdgeTrunc = int(numDedx * m_removeLowest + 0.51);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest) + 0.51);
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

void CDCDedxPIDModule::saveLookupLogl(double(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx)
{
  //all pdfs have the same dimensions
  const TH2F* pdf = m_DBDedxPDFs->getCDCPDF(0, !m_useIndividualHits);
  const Int_t binX = pdf->GetXaxis()->FindFixBin(p);
  const Int_t binY = pdf->GetYaxis()->FindFixBin(dedx);

  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    pdf = m_DBDedxPDFs->getCDCPDF(iPart, !m_useIndividualHits);
    if (pdf->GetEntries() == 0) { //might be NULL if m_ignoreMissingParticles is set
      B2WARNING("NO CDC PDFS...");
      continue;
    }
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

double CDCDedxPIDModule::D2I(const double cosTheta, const double D) const
{
  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];
  if (projection == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return D;
  }

  double chargeDensity = D / projection;
  double numerator     = 1 + m_hadronpars[0] * chargeDensity;
  double denominator   = 1 + m_hadronpars[1] * chargeDensity;

  if (denominator == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return D;
  }

  double I = D * m_hadronpars[4] * numerator / denominator;
  return I;
}

double CDCDedxPIDModule::I2D(const double cosTheta, const double I) const
{
  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_hadronpars[3]) + m_hadronpars[2];

  if (projection == 0 || m_hadronpars[4] == 0) {
    B2WARNING("Something wrong with dE/dx hadron constants!");
    return I;
  }

  double a =  m_hadronpars[0] / projection;
  double b =  1 - m_hadronpars[1] / projection * (I / m_hadronpars[4]);
  double c = -1.0 * I / m_hadronpars[4];

  if (b == 0 && a == 0) {
    B2WARNING("both a and b coefficiants for hadron correction are 0");
    return I;
  }

  double discr = b * b - 4.0 * a * c;
  if (discr < 0) {
    B2WARNING("negative discriminant; return uncorrectecd value");
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(discr)) / (2.0 * a) : -c / b;
  if (D < 0) {
    B2WARNING("D is less 0! will try another solution");
    D = (a != 0) ? (-b - sqrt(discr)) / (2.0 * a) : -c / b;
    if (D < 0) {
      B2WARNING("D is still less 0! just return uncorrectecd value");
      return I;
    }
  }

  return D;
}

double CDCDedxPIDModule::meanCurve(double* x, double* par, int version) const
{
  // calculate the predicted mean value as a function of beta-gamma (bg)
  // this is done with a different function depending on the value of bg
  double f = 0;

  if (version == 0) {
    if (par[0] == 1)
      f = par[1] * std::pow(std::sqrt(x[0] * x[0] + 1), par[3]) / std::pow(x[0], par[3]) *
          (par[2] - par[5] * std::log(1 / x[0])) - par[4] + std::exp(par[6] + par[7] * x[0]);
    else if (par[0] == 2)
      f = par[1] * std::pow(x[0], 3) + par[2] * x[0] * x[0] + par[3] * x[0] + par[4];
    else if (par[0] == 3)
      f = -1.0 * par[1] * std::log(par[4] + std::pow(1 / x[0], par[2])) + par[3];
  }

  return f;
}

double CDCDedxPIDModule::getMean(double bg) const
{
  // define the section of the mean to use
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
    if (i < 7) parsA[i + 1] = m_meanpars[i];
    else if (i < 11) parsB[i % 7 + 1] = m_meanpars[i];
    else parsC[i % 11 + 1] = m_meanpars[i];
  }

  // calculate dE/dx from the Bethe-Bloch mean
  double partA = meanCurve(x, parsA, 0);
  double partB = meanCurve(x, parsB, 0);
  double partC = meanCurve(x, parsC, 0);

  return (A * partA + B * partB + C * partC);
}

double CDCDedxPIDModule::sigmaCurve(double* x, const double* par, int version) const
{
  // calculate the predicted mean value as a function of beta-gamma (bg)
  // this is done with a different function depending dE/dx, nhit, and sin(theta)
  double f = 0;

  if (version == 0) {
    if (par[0] == 1) { // return dedx parameterization
      f = par[1] + par[2] * x[0];
    } else if (par[0] == 2) { // return nhit or sin(theta) parameterization
      f = par[1] * std::pow(x[0], 4) + par[2] * std::pow(x[0], 3) +
          par[3] * x[0] * x[0] + par[4] * x[0] + par[5];
    } else if (par[0] == 3) { // return cos(theta) parameterization
      f = par[1] * exp(-0.5 * pow(((x[0] - par[2]) / par[3]), 2)) +
          par[4] * pow(x[0], 6) + par[5] * pow(x[0], 5) + par[6] * pow(x[0], 4) +
          par[7] * pow(x[0], 3) + par[8] * x[0] * x[0] + par[9] * x[0] + par[10];
    }
  }

  return f;
}


double CDCDedxPIDModule::getSigma(double dedx, double nhit, double cos, double timereso) const
{

  double x[1];
  double dedxpar[3];
  double nhitpar[6];
  double cospar[11];

  dedxpar[0] = 1; nhitpar[0] = 2; cospar[0] = 3;
  for (int i = 0; i < 10; ++i) {
    if (i < 2) dedxpar[i + 1] = m_sigmapars[i];
    if (i < 5) nhitpar[i + 1] = m_sigmapars[i + 2];
    cospar[i + 1] = m_sigmapars[i + 7];
  }

  // determine sigma from the parameterization
  x[0] = dedx;
  double corDedx = sigmaCurve(x, dedxpar, 0);

  x[0] = nhit;
  double corNHit;
  int nhit_min = 8, nhit_max = 37;

  if (nhit <  nhit_min) {
    x[0] = nhit_min;
    corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_min / nhit);
  } else if (nhit > nhit_max) {
    x[0] = nhit_max;
    corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_max / nhit);
  } else corNHit = sigmaCurve(x, nhitpar, 0);

  x[0] = cos;
  double corCos = sigmaCurve(x, cospar, 0);

  return (corDedx * corCos * corNHit * timereso);
}

void CDCDedxPIDModule::saveChiValue(double(&chi)[Const::ChargedStable::c_SetSize],
                                    double(&predmean)[Const::ChargedStable::c_SetSize], double(&predsigma)[Const::ChargedStable::c_SetSize], double p, double dedx,
                                    double cos, int nhit, double timereso) const
{
  // determine a chi value for each particle type
  Const::ParticleSet set = Const::chargedStableSet;
  for (const Const::ChargedStable pdgIter : set) {
    double bg = p / pdgIter.getMass();

    // determine the predicted mean and resolution
    double mean = getMean(bg);
    double sigma = getSigma(mean, nhit, cos, timereso);

    predmean[pdgIter.getIndex()] = mean;
    predsigma[pdgIter.getIndex()] = sigma;

    // fill the chi value for this particle type
    if (sigma != 0) chi[pdgIter.getIndex()] = ((dedx - mean) / (sigma));
  }
}
