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

#include <cmath>
#include <algorithm>
#include <map>
#include <vector>
#include <iostream> //TODO rm

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
    m_hits.isRequired();
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

    /* TODO
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
    */

  }

  void CDCDedxPIDCreatorModule::event()
  {
    // clear output collections
    m_likelihoods.clear();
    m_dedxTracks.clear();

    // CDC geometry parameters and translators
    const auto& cdcgeo = CDCGeometryPar::Instance();
    LinearGlobalADCCountTranslator adcTranslator;
    RealisticTDCCountTranslator tdcTranslator;

    // is data or MC ?
    bool isData = m_mcParticles.getEntries() == 0; //TODO: get it from framework/core/Environment.h

    // track independent calibration constants
    double runGain = isData ? m_DBRunGain->getRunGain() : 1.0;
    double timeGain = 1;
    double timeReso = 1; // TODO default value?
    if (isData and m_TTDInfo.isValid() and m_TTDInfo->hasInjection()) {
      timeGain = m_DBInjectTime->getCorrection("mean", m_TTDInfo->isHER(), m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds());
      timeReso = m_DBInjectTime->getCorrection("reso", m_TTDInfo->isHER(), m_TTDInfo->getTimeSinceLastInjectionInMicroSeconds());
    }
    double scale = m_DBScaleFactor->getScaleFactor(); // scale factor to make electron dE/dx ~ 1
    if (scale == 0) {
      B2WARNING("Scale factor from DB is zero! Will be set to one");
      scale = 1;
    }

    // loop over tracks
    for (const auto& track : m_tracks) {
      // track fit result
      const auto* fitResult = track.getTrackFitResultWithClosestMass(Const::pion);
      if (not fitResult) {
        B2WARNING("No related fit for track ...");
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
        const auto* mcParticle = track.getRelated<MCParticle>();
        if (mcParticle) {
          dedxTrack->m_pdg = mcParticle->getPDG();
          dedxTrack->m_mcmass = mcParticle->getMass();
          const auto* mother = mcParticle->getMother();
          dedxTrack->m_motherPDG = mother ? mother->getPDG() : 0;
          const auto& trueMom = mcParticle->getMomentum();
          dedxTrack->m_pTrue = trueMom.R();
          dedxTrack->m_cosThetaTrue = cos(trueMom.Theta());
          /// dedxTrack->m_simDedx =
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
        double celldx = cell.dx(doca, entAng) / sinTheta; // length of track in the cell
        if (not cell.isValid()) continue;

        // wire gain calibration
        int wire = wireID.getIWire();
        int iwire = (superlayer == 0) ? 160 * layer + wire : m_nLayerWires[superlayer - 1] + (160 + 32 * (superlayer - 1)) * layer + wire;
        double wiregain = isData ? m_DBWireGains->getWireGain(iwire) : 1.0;

        // re-scaled (RS) doca and entAng variable: map to square cell
        double cellHalfWidth = M_PI * wireRadius / nWires;
        double cellHeight = topHeight + bottomHeight;
        double cellR = 2 * cellHalfWidth / cellHeight;
        double tana = 100.0; // TODO
        if (std::abs(2 * atan(1) - std::abs(entAng)) < 0.01) tana = 100 * (entAng / std::abs(entAng)); //avoid infinity at pi/2
        else tana =  std::tan(entAng);
        // double tana = std::max(std::min(tan(entAng), 1e10), -1e10);
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

        // dEdx measurements of single wires - active wires only
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

      // push dEdx values to vector
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

      // apply the "hadron correction" only to data
      double correctedMean = isData ? D2I(cosTheta, I2D(cosTheta, 1.00) * truncatedMean) : truncatedMean;

      // calculate log likelihoods
      double cdcLogL[Const::ChargedStable::c_SetSize] = {0};
      for (const auto& chargedStable : Const::chargedStableSet) {
        double betagamma = pCDC / chargedStable.getMass();
        double predictedMean = getMean(betagamma);
        double predictedSigma = getSigma(predictedMean, numValues, cosTheta, timeReso);
        double chi = predictedSigma != 0 ? (correctedMean - predictedMean) / predictedSigma : 0; // TODO sigma = 0 ?
        int index = chargedStable.getIndex();
        cdcLogL[index] = -0.5 * chi * chi; // TODO normalization ?
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


  // TODO --> move these functions to DB classes

  double CDCDedxPIDCreatorModule::D2I(double cosTheta, double D) const
  {
    const auto& params = m_DBHadronCor->getHadronPars();
    if (params.size() < 5) {
      B2WARNING("Vector of dE/dx hadron constants too short!");
      return D;
    }

    double projection = pow(fabs(cosTheta), params[3]) + params[2];
    if (projection == 0) {
      B2WARNING("Something wrong with dE/dx hadron constants!");
      return D;
    }

    double chargeDensity = D / projection;
    double numerator = 1 + params[0] * chargeDensity;
    double denominator = 1 + params[1] * chargeDensity;

    if (denominator == 0) {
      B2WARNING("Something wrong with dE/dx hadron constants!");
      return D;
    }

    double I = D * params[4] * numerator / denominator;
    return I;
  }


  double CDCDedxPIDCreatorModule::I2D(double cosTheta, double I) const
  {
    const auto& params = m_DBHadronCor->getHadronPars();
    if (params.size() < 5) {
      B2WARNING("Vector of dE/dx hadron constants too short!");
      return I;
    }

    double projection  = pow(fabs(cosTheta), params[3]) + params[2];
    if (projection == 0 or params[4] == 0) {
      B2WARNING("Something wrong with dE/dx hadron constants!");
      return I;
    }

    double a =  params[0] / projection;
    double b =  1 - params[1] / projection * (I / params[4]);
    double c = -1.0 * I / params[4];

    if (b == 0 and a == 0) {
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
      D = (a != 0) ? (-b - sqrt(discr)) / (2.0 * a) : -c / b;
      if (D < 0) {
        B2WARNING("D is less than 0; return uncorrectecd value");
        return I;
      }
    }

    return D;
  }


  double CDCDedxPIDCreatorModule::meanCurve(double x, const double* par, int version) const
  {
    // calculate the predicted mean value as a function of beta-gamma (bg)
    // this is done with a different function depending on the value of bg
    double f = 0;

    if (version == 0) {
      if (par[0] == 1)
        f = par[1] * std::pow(std::sqrt(x * x + 1), par[3]) / std::pow(x, par[3]) *
            (par[2] - par[5] * std::log(1 / x)) - par[4] + std::exp(par[6] + par[7] * x);
      else if (par[0] == 2)
        f = par[1] * std::pow(x, 3) + par[2] * x * x + par[3] * x + par[4];
      else if (par[0] == 3)
        f = -1.0 * par[1] * std::log(par[4] + std::pow(1 / x, par[2])) + par[3];
    }

    return f;
  }

  double CDCDedxPIDCreatorModule::getMean(double bg) const
  {
    // define the section of the mean to use
    double A = 0, B = 0, C = 0;
    if (bg < 4.5)
      A = 1;
    else if (bg < 10)
      B = 1;
    else
      C = 1;

    double parsA[9];
    double parsB[5];
    double parsC[5];

    const auto& params = m_DBMeanPars->getMeanPars();
    if (params.size() < 15) B2FATAL("getMean: vector of parameters too short");

    parsA[0] = 1; parsB[0] = 2; parsC[0] = 3;
    for (int i = 0; i < 15; ++i) {
      if (i < 7) parsA[i + 1] = params[i];
      else if (i < 11) parsB[i % 7 + 1] = params[i];
      else parsC[i % 11 + 1] = params[i];
    }

    // calculate dE/dx from the Bethe-Bloch mean
    double partA = meanCurve(bg, parsA, 0);
    double partB = meanCurve(bg, parsB, 0);
    double partC = meanCurve(bg, parsC, 0);

    return (A * partA + B * partB + C * partC);
  }

  double CDCDedxPIDCreatorModule::sigmaCurve(double x, const double* par, int version) const
  {
    // calculate the predicted mean value as a function of beta-gamma (bg)
    // this is done with a different function depending dE/dx, nhit, and sin(theta)
    double f = 0;

    if (version == 0) {
      if (par[0] == 1) { // return dedx parameterization
        f = par[1] + par[2] * x;
      } else if (par[0] == 2) { // return nhit or sin(theta) parameterization
        f = par[1] * std::pow(x, 4) + par[2] * std::pow(x, 3) +
            par[3] * x * x + par[4] * x + par[5];
      } else if (par[0] == 3) { // return cos(theta) parameterization
        f = par[1] * exp(-0.5 * pow(((x - par[2]) / par[3]), 2)) +
            par[4] * pow(x, 6) + par[5] * pow(x, 5) + par[6] * pow(x, 4) +
            par[7] * pow(x, 3) + par[8] * x * x + par[9] * x + par[10];
      }
    }

    return f;
  }

  double CDCDedxPIDCreatorModule::getSigma(double dedx, double nhit, double cos, double timereso) const
  {

    double x;
    double dedxpar[3];
    double nhitpar[6];
    double cospar[11];

    const auto& params = m_DBSigmaPars->getSigmaPars();
    if (params.size() < 17) B2FATAL("getSigma: vector of parameters too short");

    dedxpar[0] = 1; nhitpar[0] = 2; cospar[0] = 3;
    for (int i = 0; i < 10; ++i) {
      if (i < 2) dedxpar[i + 1] = params[i];
      if (i < 5) nhitpar[i + 1] = params[i + 2];
      cospar[i + 1] = params[i + 7];
    }

    // determine sigma from the parameterization
    x = dedx;
    double corDedx = sigmaCurve(x, dedxpar, 0);

    x = nhit;
    double corNHit;
    int nhit_min = 8, nhit_max = 37;

    if (nhit <  nhit_min) {
      x = nhit_min;
      corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_min / nhit);
    } else if (nhit > nhit_max) {
      x = nhit_max;
      corNHit = sigmaCurve(x, nhitpar, 0) * sqrt(nhit_max / nhit);
    } else corNHit = sigmaCurve(x, nhitpar, 0);

    x = cos;
    double corCos = sigmaCurve(x, cospar, 0);

    return (corDedx * corCos * corNHit * timereso);
  }




} // end Belle2 namespace

