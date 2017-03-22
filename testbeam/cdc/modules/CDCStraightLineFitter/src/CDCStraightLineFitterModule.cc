/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/cdc/modules/CDCStraightLineFitter/CDCStraightLineFitterModule.h>
#include <testbeam/cdc/modules/CDCStraightLineFitter/StraightLine.h>



// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <genfit/TrackCand.h>
#include <cdc/dataobjects/CDCHit.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <tracking/dataobjects/ExtHit.h>
#include <testbeam/top/dataobjects/TOPTBSimHit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <framework/dataobjects/EventT0.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <top/geometry/TOPGeometryPar.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <iostream>
#include <iomanip>
#include <algorithm>

#include <TRandom3.h>

extern "C" {
  void tema_clear_();
  void tema_set_rseg_(float*, float*);
  void tema_add_hits_(float*, float*, float*, float*);
  void tema_fit_3d_();
  int tema_get_status_();
  float tema_get_chi2_();
  int tema_get_ndf_();
  void tema_get_dir_(float*);
  void tema_get_rf_(float*);
  void tema_get_re_(float*);
}


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(CDCStraightLineFitter)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  CDCStraightLineFitterModule::CDCStraightLineFitterModule() : Module(),
    m_startTime(0)

  {
    // set module description (e.g. insert text)
    setDescription("Performs straight line fit of CDC track candidates. Useful only for Earth-like magnetic field.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("alpha", m_alpha, "Rotation angle of trigger counters around z [deg]", 0.0);
    addParam("momentumEstimate", m_momentumEstimate,
             "Estimate for the momenta of cosmis muons", 3.0);
    addParam("smearDriftLength", m_smearDriftLength,
             "sigma for additional drift length smearing", 0.0);
    addParam("removeLowest", m_removeLowest,
             "portion of lowest dE/dx measurements to discard", 0.05);
    addParam("removeHighest", m_removeHighest,
             "portion of highest dE/dx measurements to discard", 0.25);
    addParam("smearDedx", m_smearDedx,
             "relative sigma for dEdx smearing of individual cell measurements", 0.0);
  }

  CDCStraightLineFitterModule::~CDCStraightLineFitterModule()
  {
  }

  void CDCStraightLineFitterModule::initialize()
  {

    // input

    StoreArray<genfit::TrackCand> trackCandidates;
    trackCandidates.isRequired();

    StoreArray<CDCHit> cdcHits;
    cdcHits.isRequired();

    StoreArray<TOPTBSimHit> trigSimHits; // to be replaced with equivalent digits
    trigSimHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    // output

    StoreArray<Track> tracks;
    tracks.registerInDataStore();

    StoreArray<TrackFitResult> fitResults;
    fitResults.registerInDataStore();

    StoreArray<ExtHit> extHits;
    extHits.registerInDataStore();

    StoreArray<CDCDedxTrack> dedxTracks;
    dedxTracks.registerInDataStore();

    StoreObjPtr<TOPRecBunch> recBunch;
    recBunch.registerInDataStore();

    StoreObjPtr<EventT0> eventT0;
    eventT0.registerInDataStore();

    trackCandidates.registerRelationTo(fitResults);
    trackCandidates.registerRelationTo(mcParticles);
    tracks.registerRelationTo(mcParticles);
    tracks.registerRelationTo(extHits);
    tracks.registerRelationTo(dedxTracks);

  }


  void CDCStraightLineFitterModule::beginRun()
  {
  }


  void CDCStraightLineFitterModule::event()
  {

    StoreArray<genfit::TrackCand> trackCandidates;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> fitResults;
    StoreArray<MCParticle> mcParticles;
    StoreArray<ExtHit> extHits;
    StoreArray<CDCDedxTrack> dedxTracks;
    RelationArray trackCandToFitResults(trackCandidates, fitResults);
    RelationArray trackCandToMCParticles(trackCandidates, mcParticles);

    setStartTime();
    StoreObjPtr<TOPRecBunch> recBunch;
    if (!recBunch.isValid()) recBunch.create();
    recBunch->setReconstructed(0, -m_startTime, 0, 0);

    double Bfield = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();

    for (int icand = 0; icand < trackCandidates.getEntries(); icand++) {

      // track fit

      const auto& trackCandidate = *trackCandidates[icand];
      bool ok = fitTrackCandidate(trackCandidate);
      if (!ok) continue;

      // store fitted track

      TVector3 momFit = getMomentum();
      const TVector3 posFit = getStartPoint();
      if (momFit.Y() > 0) momFit = -momFit;

      TMatrixDSym cov(6);
      float chi = getChi2() / getNDF();
      fitResults.appendNew(posFit, momFit, cov, 1, Const::muon, chi, Bfield, 0, 0);
      auto* track = tracks.appendNew();
      track->setTrackFitResultIndex(Const::muon, fitResults.getEntries() - 1);
      trackCandToFitResults.add(icand, fitResults.getEntries() - 1);

      // set MC truth

      vector<int> hitIDs = trackCandidate.getHitIDs(Const::CDC);
      const auto* mcParticle = getMCTruth(hitIDs);
      if (mcParticle) {
        track->addRelationTo(mcParticle);
        trackCandToMCParticles.add(icand, mcParticle->getArrayIndex());
      }

      // extrapolate to TOP

      CDCTB::StraightLine trackGlobal(posFit, momFit);
      trackGlobal.crossPlaneAlpha(m_alpha);
      trackGlobal.setReferencePoint();
      const TVector3 startPoint = trackGlobal.getPosition();
      double mass = Const::pion.getMass();
      double p = momFit.Mag();
      double beta = p / sqrt(p * p + mass * mass);
      const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
      for (unsigned moduleID = 1; moduleID <= geo->getNumModules(); moduleID++) {
        const auto& module = geo->getModule(moduleID);
        auto posLocal = module.pointToLocal(startPoint);
        auto momLocal = module.momentumToLocal(momFit);
        CDCTB::StraightLine trackLocal(posLocal, momLocal);
        vector<double> paths;
        double yPlane = -module.getBarThickness() / 2;
        for (int k = 0; k < 2; k++) {
          if (!trackLocal.crossPlaneY(yPlane)) break;
          auto position = trackLocal.getPosition();
          if (fabs(position.X()) > module.getBarWidth() / 2) break;
          if (fabs(position.Z()) > module.getBarLength() / 2) break;
          paths.push_back(trackLocal.getPath());
          yPlane += module.getBarThickness();
        }
        if (paths.size() != 2) continue;
        std::sort(paths.begin(), paths.end());
        ExtHitStatus status = EXT_ENTER;
        for (const auto& path : paths) {
          trackGlobal.setPath(path);
          auto position = trackGlobal.getPosition();
          if (!onTheSameSide(position, posFit)) break;
          double tof = path / (beta * Const::speedOfLight);
          auto* extHit = extHits.appendNew(Const::pion.getPDGCode(), Const::TOP, moduleID,
                                           status, tof, position, momFit, cov);
          track->addRelationTo(extHit);
          status = EXT_EXIT;
        }
      }

      // determine dE/dx and save to CDCDedxTracks

      auto* dedxTrack = dedxTracks.appendNew();
      setDedx(trackCandidate, momFit, posFit, dedxTrack);
      track->addRelationTo(dedxTrack);

    }

  }


  bool CDCStraightLineFitterModule::onTheSameSide(const TVector3& point1,
                                                  const TVector3& point2)
  {
    TVector3 normal(-sin(m_alpha), cos(m_alpha), 0);
    return (normal * point1) * (normal * point2) > 0;
  }



  void CDCStraightLineFitterModule::endRun()
  {
  }

  void CDCStraightLineFitterModule::terminate()
  {

  }


  bool CDCStraightLineFitterModule::fitTrackCandidate(const genfit::TrackCand& trackCand)
  {

    StoreArray<CDCHit> hits;
    const auto& cdcgp = CDC::CDCGeometryPar::Instance();
    CDC::RealisticTDCCountTranslator translator(true); // use in wire propagation delay
    // translator.setEventTime(m_startTime); // deprecated

    tema_clear_();

    TVector3 posSeed = trackCand.getPosSeed();
    TVector3 dirSeed = trackCand.getMomSeed().Unit();
    if (dirSeed.Y() > 0) dirSeed = -dirSeed; // upside down tracks
    CDCTB::StraightLine trackSeed(posSeed, dirSeed);
    if (!trackSeed.crossPlaneAlpha(m_alpha)) {
      B2ERROR("CDCStraightLineFitter: no X-sect");
      return false;
    }
    trackSeed.setReferencePoint();

    double theta = dirSeed.Theta();
    double phi = dirSeed.Phi();
    TRotation rotate;
    rotate.RotateYEulerAngles(phi, M_PI / 2, -M_PI / 2.);
    m_rotateBack = rotate.Inverse();

    TVector3 posRot = rotate * posSeed;
    TVector3 dirRot = rotate * dirSeed;
    float r0[3] = {(float) posRot.X(), (float) posRot.Y(), (float) posRot.Z()};
    float dir[3] = {(float) dirRot.X(), (float) dirRot.Y(), (float) dirRot.Z()};
    tema_set_rseg_(r0, dir);

    vector<int> hitIDs = trackCand.getHitIDs(Const::CDC);
    for (int hitID : hitIDs) {
      const auto* hit = hits[hitID];
      WireID wire(hit->getID());
      TVector3 posFor = cdcgp.wireForwardPosition(wire);
      TVector3 posBack = cdcgp.wireBackwardPosition(wire);
      CDCTB::StraightLine wireLine(posBack, posFor - posBack);
      if (!trackSeed.closestApproach(wireLine)) continue;

      double flightTime = trackSeed.getPath() / Const::speedOfLight;
      double z = wireLine.getPosition().Z();
      double alpha = cdcgp.getAlpha(wireLine.getPosition(), dirSeed);

      float drift[2] = {0, 0};
      float sigmaSq[2] = {0, 0};
      for (int i = 0; i < 2; i++) {
        drift[i] = translator.getDriftLength(hit->getTDCCount(), wire, flightTime,
                                             i, z, alpha, theta);
        if (drift[i] < 0) drift[i] = 0; // ali to?
        sigmaSq[i] = translator.getDriftLengthResolution(drift[i], wire, i,
                                                         z, alpha, theta);
      }
      if (drift[0] < 0 or drift[1] < 0) continue; // ali pa to?
      float driftLen = (drift[0] + drift[1]) / 2;
      float driftDiff = drift[0] - drift[1];
      float sigma = (sigmaSq[0] + sigmaSq[1]) / 2 + driftDiff * driftDiff / 12;
      if (m_smearDriftLength > 0) {
        driftLen += gRandom->Gaus(0., m_smearDriftLength);
        sigma += m_smearDriftLength * m_smearDriftLength;
      }
      sigma = sqrt(sigma);

      posFor.Transform(rotate);
      posBack.Transform(rotate);
      float r_fw[3] = {(float) posFor.X(), (float) posFor.Y(), (float) posFor.Z()};
      float r_bk[3] = {(float) posBack.X(), (float) posBack.Y(), (float) posBack.Z()};
      tema_add_hits_(r_fw, r_bk, &driftLen, &sigma);
    }

    tema_fit_3d_();

    return tema_get_status_();

  }


  bool CDCStraightLineFitterModule::setDedx(const genfit::TrackCand& trackCand,
                                            const TVector3& momFit,
                                            const TVector3& posFit,
                                            CDCDedxTrack* dedxTrack)
  {

    const auto& cdcgp = CDC::CDCGeometryPar::Instance();
    int nLayers = cdcgp.nWireLayers();
    const double* innerRadius = cdcgp.innerRadiusWireLayer();
    const double* outerRadius = cdcgp.outerRadiusWireLayer();
    CDC::LinearGlobalADCCountTranslator translator;

    CDCTB::StraightLine trackLine(posFit, momFit);

    StoreArray<CDCHit> hits;
    std::vector<int> hitIDs = trackCand.getHitIDs(Const::CDC);
    std::vector<double> dEdx;
    for (int hitID : hitIDs) {
      const auto* hit = hits[hitID];
      WireID wire(hit->getID());
      int layer = wire.getICLayer();
      if (layer >= nLayers) {
        B2ERROR("invalid CDC layer number");
        continue;
      }
      TVector3 posFor = cdcgp.wireForwardPosition(wire);
      TVector3 posBack = cdcgp.wireBackwardPosition(wire);
      CDCTB::StraightLine wireLine(posBack, posFor - posBack);
      if (!trackLine.closestApproach(wireLine)) continue;
      double phi = wireLine.getPosition().Phi();
      double Dphi = M_PI / cdcgp.nWiresInLayer(layer);
      double length = trackLine.getLengthInCell(innerRadius[layer], outerRadius[layer],
                                                phi - Dphi, phi + Dphi);
      if (length > 0) {
        double charge = translator.getCharge(hit->getADCCount(), wire, false, 0, M_PI / 2);
        if (m_smearDedx > 0) {
          double u = gRandom->Gaus(0.0, m_smearDedx);
          charge *= u > 0 ? 1.0 + u : 1.0 / (1.0 - u);
        }
        double dedx = charge / length;
        if (dedx > 0) dEdx.push_back(dedx);
      }
    }

    double mean = 0;
    for (auto& dedx : dEdx) mean += dedx;
    int size = dEdx.size();
    if (size > 0) mean /= size;

    std::sort(dEdx.begin(), dEdx.end());
    unsigned i1 = int(size * m_removeLowest);
    unsigned i2 = int(size * (1.0 - m_removeHighest));
    double truncMean = 0;
    double truncSigma = 0;
    int n = 0;
    for (unsigned i = i1; i < i2; i++) {
      truncMean += dEdx[i];
      truncSigma += dEdx[i] * dEdx[i];
      n++;
    }
    if (n > 0) {
      truncMean /= n;
      truncSigma = sqrt((truncSigma / n - truncMean * truncMean) / n);
    }

    dedxTrack->setMean(mean);
    dedxTrack->setTruncatedMean(truncMean);
    dedxTrack->setError(truncSigma);
    return true;
  }


  double CDCStraightLineFitterModule::getChi2()
  {
    return tema_get_chi2_();
  }


  int CDCStraightLineFitterModule::getNDF()
  {
    return tema_get_ndf_();
  }

  TVector3 CDCStraightLineFitterModule::getMomentum()
  {
    float dir[3] = {0, 0, 0};
    tema_get_dir_(dir);
    TVector3 vec(dir[0], dir[1], dir[2]);
    vec.Transform(m_rotateBack);
    return m_momentumEstimate * vec;
  }

  TVector3 CDCStraightLineFitterModule::getStartPoint()
  {
    float r[3] = {0, 0, 0};
    tema_get_rf_(r);
    TVector3 vec(r[0], r[1], r[2]);
    vec.Transform(m_rotateBack);
    return vec;
  }

  TVector3 CDCStraightLineFitterModule::getEndPoint()
  {
    float r[3] = {0, 0, 0};
    tema_get_re_(r);
    TVector3 vec(r[0], r[1], r[2]);
    vec.Transform(m_rotateBack);
    return vec;
  }


  void CDCStraightLineFitterModule::setStartTime()
  {
    StoreArray<TOPTBSimHit> trigSimHits;

    const double maxTime = 1e10;
    double time[2] = {maxTime, maxTime};
    for (const auto& trigHit : trigSimHits) {
      int id = trigHit.getCounterID();
      if (id == 1 or id == 2) {
        double t = trigHit.getTime();
        if (t < time[id - 1]) time[id - 1] = t;
      }
    }
    m_startTime = 0;
    int n = 0;
    for (int i = 0; i < 2; i++) {
      if (time[i] < maxTime) {m_startTime += time[i]; n++;}
    }
    if (n > 0) {
      m_startTime /= n;
    } else {
      B2ERROR("No trigger found to determine start time");
    }

    StoreObjPtr<EventT0> eventT0;
    if (!eventT0) eventT0.create();
    eventT0->addEventT0(m_startTime, 0., Const::TEST);

  }

  const MCParticle* CDCStraightLineFitterModule::getMCTruth(const vector<int>& hitIDs,
                                                            double fraction)
  {

    StoreArray<CDCHit> hits;

    std::map<const MCParticle*, double> posWeightSum;
    double otherWeightSum = 0;

    for (int hitID : hitIDs) {
      const auto* hit = hits[hitID];
      auto mcParticles = hit->getRelationsWith<MCParticle>();
      if (mcParticles.size() == 0) {
        otherWeightSum += 1;
        continue;
      }
      for (unsigned i = 0;  i < mcParticles.size(); i++) {
        double weight = mcParticles.weight(i);
        if (weight > 0) {
          const auto* mcParticle = mcParticles[i];
          posWeightSum[mcParticle] += weight;
        } else {
          otherWeightSum += 1;
        }
      }
    }

    const MCParticle* mcParticle = 0;
    double weight = 0;
    double sum = otherWeightSum;
    for (const auto& element : posWeightSum) {
      sum += element.second;
      if (element.second > weight) {
        mcParticle = element.first;
        weight = element.second;
      }
    }

    if (weight / sum > fraction) return mcParticle;
    return 0;

  }



} // end Belle2 namespace

