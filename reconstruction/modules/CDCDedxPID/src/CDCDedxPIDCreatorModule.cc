/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/CDCDedxPID/CDCDedxPIDCreatorModule.h>
#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <reconstruction/modules/CDCDedxPID/LineHelper.h>
#include <framework/core/Environment.h>
#include <TRandom.h>
#include <cmath>
#include <algorithm>
#include <map>
#include <vector>

using namespace std;

namespace Belle2 {

  using namespace CDC;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(CDCDedxPIDCreator);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  CDCDedxPIDCreatorModule::CDCDedxPIDCreatorModule() : Module()

  {
    // set module description
    setDescription("Module that creates PID likelihoods from CDC hit information stored in CDCDedxHits "
                   "using parameterized means and resolutions.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("removeLowest", m_removeLowest,
             "Portion of events with low dE/dx that should be discarded", double(0.05));
    addParam("removeHighest", m_removeHighest,
             "Portion of events with high dE/dx that should be discarded", double(0.25));
    addParam("useBackHalfCurlers", m_useBackHalfCurlers,
             "Whether to use the back half of curlers", false);
    addParam("trackLevel", m_trackLevel,
             "ONLY USEFUL FOR MC: Use track-level MC (generate truncated mean from predicted mean and sigma using MC truth). "
             "If false, use hit-level MC (use truncated mean determined from hits)", true);
    addParam("enableDebugOutput", m_enableDebugOutput,
             "Option to write out debugging information to CDCDedxTracks", true);
    addParam("likelihoodsName", m_likelihoodsName,
             "name of CDCDedxLikelihood collection", string(""));
    addParam("dedxTracksName", m_dedxTracksName,
             "name of CDCDedxTrack collection", string(""));
  }

  CDCDedxPIDCreatorModule::~CDCDedxPIDCreatorModule()
  {
  }

  void CDCDedxPIDCreatorModule::initialize()
  {
    m_tracks.isRequired();
    m_hits.isOptional(); // in order to run also with old cdst's where this collection doesn't exist
    m_mcParticles.isOptional();
    m_TTDInfo.isOptional();
    m_likelihoods.registerInDataStore(m_likelihoodsName);
    m_tracks.registerRelationTo(m_likelihoods);
    m_dedxTracks.registerInDataStore(m_dedxTracksName);
    m_tracks.registerRelationTo(m_dedxTracks);

    m_nLayerWires[0] = 1280;
    for (int i = 1; i < 9; ++i) {
      m_nLayerWires[i] = m_nLayerWires[i - 1] + 6 * (160 + (i - 1) * 32);
    }

    if (not m_trackLevel)
      B2WARNING("Hit-level MC still needs a precise calibration to perform well! Until then please use track-level MC.");

  }

  void CDCDedxPIDCreatorModule::event()
  {
    // check if CDCDedxHits are present; return if not.
    if (not m_hits.isValid()) {
      m_warnCount++;
      if (m_warnCount < 10) {
        B2WARNING("StoreArray 'CDCDedxHits' does not exist, returning. Probably running on old cdst.");
      } else if (m_warnCount == 10) {
        B2WARNING("StoreArray 'CDCDedxHits' does not exist, returning. ...message will be suppresed now.");
      }
      return;
    }

    // clear output collections
    m_likelihoods.clear();
    m_dedxTracks.clear();

    // CDC geometry parameters and translators
    const auto& cdcgeo = CDCGeometryPar::Instance();
    LinearGlobalADCCountTranslator adcTranslator;
    RealisticTDCCountTranslator tdcTranslator;

    // is data or MC ?
    bool isData = not Environment::Instance().isMC();

    // track independent calibration constants
    double runGain = isData ? m_DBRunGain->getRunGain() : 1.0;
    double timeGain = 1;
    double timeReso = 1; // this is multiplicative constant
    if (isData and m_TTDInfo.isValid() and m_TTDInfo->hasInjection()) {
      timeGain = m_DBInjectTime->getCorrection("mean", m_TTDInfo->isHER(), m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds());
      timeReso = m_DBInjectTime->getCorrection("reso", m_TTDInfo->isHER(), m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds());
    }
    double scale = m_DBScaleFactor->getScaleFactor(); // scale factor to make electron dE/dx ~ 1
    if (scale == 0) {
      B2ERROR("Scale factor from DB is zero! Will be set to one");
      scale = 1;
    }

    // loop over tracks
    for (const auto& track : m_tracks) {
      // track fit result
      const auto* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
      if (not fitResult) {
        B2WARNING("No related fit for track, skip it.");
        continue;
      }

      // hits of this track
      const auto hits = track.getRelationsTo<CDCDedxHit>();
      if (hits.size() == 0) continue;

      // track momentum
      const auto& trackMom = fitResult->getMomentum();
      double theta = trackMom.Theta();
      double cosTheta = cos(theta);
      double sinTheta = sin(theta);

      // track dependent calibration constants
      double cosCor = isData ? m_DBCosineCor->getMean(cosTheta) : 1.0;
      bool isEdge = std::abs(cosTheta + 0.860) < 0.010 or std::abs(cosTheta - 0.955) <= 0.005;
      double cosEdgeCor = (isData and isEdge) ? m_DBCosEdgeCor->getMean(cosTheta) : 1.0;

      // MC particle
      const auto* mcParticle = isData ? nullptr : track.getRelated<MCParticle>();

      // debug output
      CDCDedxTrack* dedxTrack = m_enableDebugOutput ? m_dedxTracks.appendNew() : nullptr;
      if (dedxTrack) {
        dedxTrack->m_track = track.getArrayIndex();
        dedxTrack->m_charge = fitResult->getChargeSign();
        dedxTrack->m_cosTheta = cosTheta;
        dedxTrack->m_p = trackMom.R();
        if (isData and m_TTDInfo.isValid() and m_TTDInfo->hasInjection()) {
          dedxTrack->m_injring = m_TTDInfo->isHER();
          dedxTrack->m_injtime = m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds();
        }
        if (mcParticle) {
          dedxTrack->m_pdg = mcParticle->getPDG();
          dedxTrack->m_mcmass = mcParticle->getMass();
          const auto* mother = mcParticle->getMother();
          dedxTrack->m_motherPDG = mother ? mother->getPDG() : 0;
          const auto& trueMom = mcParticle->getMomentum();
          dedxTrack->m_pTrue = trueMom.R();
          dedxTrack->m_cosThetaTrue = cos(trueMom.Theta());
        }
        dedxTrack->m_scale = scale;
        dedxTrack->m_cosCor = cosCor;
        dedxTrack->m_cosEdgeCor = cosEdgeCor;
        dedxTrack->m_runGain = runGain;
        dedxTrack->m_timeGain = timeGain;
        dedxTrack->m_timeReso = timeReso;
      }

      // loop over hits
      int lastLayer = -1;
      double pCDC = 0;
      std::map<int, DEDX> dedxWires;
      for (const auto& hit : hits) {
        // wire numbering: layer and superlayer
        const auto& wireID = hit.getWireID();
        int layer = wireID.getILayer(); // layer within superlayer
        int superlayer = wireID.getISuperLayer();
        int currentLayer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer); // continuous layer number
        if (not m_useBackHalfCurlers and currentLayer < lastLayer) break;
        lastLayer = currentLayer;

        // track momentum at the first hit
        if (pCDC == 0) pCDC = hit.getPOCAMomentum().R();

        // drift cell
        double innerRadius = cdcgeo.innerRadiusWireLayer()[currentLayer];
        double outerRadius = cdcgeo.outerRadiusWireLayer()[currentLayer];
        const ROOT::Math::XYZVector& wirePosF = cdcgeo.wireForwardPosition(wireID, CDCGeometryPar::c_Aligned);
        double wireRadius = wirePosF.Rho();
        int nWires = cdcgeo.nWiresInLayer(currentLayer);
        double topHeight = outerRadius - wireRadius;
        double bottomHeight = wireRadius - innerRadius;
        double topHalfWidth = M_PI * outerRadius / nWires;
        double bottomHalfWidth = M_PI * innerRadius / nWires;
        DedxDriftCell cell(DedxPoint(-topHalfWidth, topHeight),
                           DedxPoint(topHalfWidth, topHeight),
                           DedxPoint(bottomHalfWidth, -bottomHeight),
                           DedxPoint(-bottomHalfWidth, -bottomHeight));

        // length of a track within the drift cell
        double doca = hit.getSignedDOCAXY();
        double entAng = hit.getEntranceAngle();
        double celldx = cell.dx(doca, entAng) / sinTheta; // length of a track in the cell
        if (not cell.isValid()) continue;

        // wire gain calibration (iwire is a continuous wire number)
        int wire = wireID.getIWire();
        int iwire = (superlayer == 0) ? 160 * layer + wire : m_nLayerWires[superlayer - 1] + (160 + 32 * (superlayer - 1)) * layer + wire;
        double wiregain = isData ? m_DBWireGains->getWireGain(iwire) : 1.0;

        // re-scaled (RS) doca and entAng variable: map to square cell
        double cellHalfWidth = M_PI * wireRadius / nWires;
        double cellHeight = topHeight + bottomHeight;
        double cellR = 2 * cellHalfWidth / cellHeight;
        double tana = std::max(std::min(tan(entAng), 1e10), -1e10); // this fixes bug in CDCDedxPIDModule near +-pi/2
        double docaRS = doca * sqrt((1 + cellR * cellR * tana * tana) / (1 + tana * tana));
        double normDocaRS = docaRS / cellHalfWidth;
        double entAngRS = atan(tana / cellR);

        // one and two dimensional corrections
        double onedcor = isData ? m_DB1DCell->getMean(currentLayer, entAngRS) : 1.0;
        double twodcor = isData ? m_DB2DCell->getMean(currentLayer, normDocaRS, entAngRS) : 1.0;

        // total correction
        double correction = runGain * cosCor * cosEdgeCor * timeGain * wiregain * twodcor * onedcor;

        // calibrated ADC count
        double adcCount = isData ? m_DBNonlADC->getCorrectedADC(hit.getADCCount(), currentLayer) : hit.getADCCount();
        double adcCalibrated = correction != 0 ? adcCount / scale / correction : 0;

        // merge dEdx measurements on single wires; take active wires only
        if (correction != 0) dedxWires[iwire].add(hit, iwire, currentLayer, celldx, adcCalibrated);

        // debug output
        if (dedxTrack) {
          dedxTrack->m_pCDC = pCDC;
          const auto& pocaMom = hit.getPOCAMomentum();
          double pocaPhi = pocaMom.Phi();
          double pocaTheta = pocaMom.Theta();
          double pocaZ = hit.getPOCAOnWire().Z();
          double hitCharge = adcTranslator.getCharge(adcCount, wireID, false, pocaZ, pocaPhi);
          double driftDRealistic = tdcTranslator.getDriftLength(hit.getTDCCount(), wireID, 0, true, pocaZ, pocaPhi, pocaTheta);
          double driftDRealisticRes = tdcTranslator.getDriftLengthResolution(driftDRealistic, wireID, true, pocaZ, pocaPhi, pocaTheta);
          double cellDedx = adcCalibrated / celldx;

          dedxTrack->addHit(wire, iwire, currentLayer, doca, docaRS, entAng, entAngRS,
                            adcCount, hit.getADCCount(), hitCharge, celldx * sinTheta, cellDedx, cellHeight, cellHalfWidth,
                            hit.getTDCCount(), driftDRealistic, driftDRealisticRes, wiregain, twodcor, onedcor,
                            hit.getFoundByTrackFinder(), hit.getWeightPionHypo(), hit.getWeightKaonHypo(), hit.getWeightProtonHypo());
        }

      } // end of loop over hits

      // merge dEdx measurements in layers
      std::map<int, DEDX> dedxLayers;
      for (const auto& dedxWire : dedxWires) {
        const auto& dedx = dedxWire.second;
        dedxLayers[dedx.cLayer].add(dedx);
      }

      // push dEdx values to a vector
      std::vector<double> dedxValues;
      for (const auto& dedxLayer : dedxLayers) {
        const auto& dedx = dedxLayer.second;
        if (dedx.dx > 0 and dedx.dE > 0) {
          dedxValues.push_back(dedx.dE / dedx.dx);
          // debug output
          if (dedxTrack) dedxTrack->addDedx(dedx.nhits, dedx.cWire, dedx.cLayer, dedx.dx, dedxValues.back());
        }
      }
      if (dedxValues.empty()) continue;

      // sort dEdx values
      std::sort(dedxValues.begin(), dedxValues.end());

      // calculate mean
      double mean = 0;
      for (auto x : dedxValues) mean += x;
      mean /= dedxValues.size();

      // calculate truncated mean and error
      int lowEdgeTrunc = int(dedxValues.size() * m_removeLowest + 0.51);
      int highEdgeTrunc = int(dedxValues.size() * (1 - m_removeHighest) + 0.51);
      double truncatedMean = 0;
      double sumOfSquares = 0;
      int numValues = 0;
      for (int i = lowEdgeTrunc; i < highEdgeTrunc; i++) {
        double x = dedxValues[i];
        truncatedMean += x;
        sumOfSquares += x * x;
        numValues++;
      }
      if (numValues > 0) {
        truncatedMean /= numValues;
      } else {
        truncatedMean = mean;
        numValues = dedxValues.size();
      }
      double truncatedError = numValues > 1 ? sqrt(sumOfSquares / numValues - truncatedMean * truncatedMean) / (numValues - 1) : 0;

      // apply the saturation correction only to data (the so called "hadron correction")
      double correctedMean = isData ? m_DBHadronCor->getCorrectedMean(truncatedMean, cosTheta) : truncatedMean;

      // track level MC (e.g. replacing truncated mean with a generated one)
      if (m_trackLevel and mcParticle) {
        double mass = mcParticle->getMass();
        if (mass > 0) {
          double mcMean = m_DBMeanPars->getMean(pCDC / mass);
          double mcSigma = m_DBSigmaPars->getSigma(mcMean, numValues, cosTheta, timeReso);
          correctedMean = gRandom->Gaus(mcMean, mcSigma);
          while (correctedMean < 0) correctedMean = gRandom->Gaus(mcMean, mcSigma);
          // debug output
          if (dedxTrack) dedxTrack->m_simDedx = correctedMean;
        }
      }

      // calculate log likelihoods
      double cdcLogL[Const::ChargedStable::c_SetSize] = {0};
      for (const auto& chargedStable : Const::chargedStableSet) {
        double betagamma = pCDC / chargedStable.getMass();
        double predictedMean = m_DBMeanPars->getMean(betagamma);
        double predictedSigma = m_DBSigmaPars->getSigma(predictedMean, numValues, cosTheta, timeReso);
        if (predictedSigma <= 0) B2ERROR("Predicted sigma is not positive for PDG = " << chargedStable.getPDGCode());
        double chi = (correctedMean - predictedMean) / predictedSigma;
        int index = chargedStable.getIndex();
        cdcLogL[index] = -0.5 * chi * chi;
        // debug output
        if (dedxTrack) {
          dedxTrack->m_predmean[index] = predictedMean;
          dedxTrack->m_predres[index] = predictedSigma;
          dedxTrack->m_cdcChi[index] = chi;
          dedxTrack->m_cdcLogl[index] = cdcLogL[index];
        }
      }

      // save log likelihoods
      auto* likelihoods = m_likelihoods.appendNew(cdcLogL);
      track.addRelationTo(likelihoods);

      // debug output
      if (dedxTrack) {
        double fullLength = 0;
        for (const auto& dedxLayer : dedxLayers) fullLength += dedxLayer.second.dx;
        dedxTrack->m_length = fullLength;
        dedxTrack->m_dedxAvg = mean;
        dedxTrack->m_dedxAvgTruncatedNoSat = truncatedMean;
        dedxTrack->m_dedxAvgTruncatedErr = truncatedError;
        dedxTrack->m_dedxAvgTruncated = correctedMean;
        dedxTrack->m_lNHitsUsed = numValues;
        track.addRelationTo(dedxTrack);
      }

    } // end of loop over tracks

  }

} // end Belle2 namespace

