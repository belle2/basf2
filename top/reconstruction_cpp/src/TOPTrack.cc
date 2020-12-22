/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/RaytracerBase.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/datastore/StoreArray.h>
#include <cmath>
#include <vector>
#include <algorithm>

namespace Belle2 {
  namespace TOP {

    TOPTrack::TrackAngles::TrackAngles(const TVector3& dir):
      cosTh(dir.Z()), sinTh(sqrt(1 - cosTh * cosTh)), cosFi(dir.X() / sinTh), sinFi(dir.Y() / sinTh)
    {}


    TOPTrack::TOPTrack(const Track& track, const Const::ChargedStable& chargedStable,
                       std::string digitsName)
    {
      // require fitResult

      const auto* fitResult = track.getTrackFitResultWithClosestMass(chargedStable);
      if (not fitResult) {
        B2ERROR("No TrackFitResult available for: "
                << LogVar("PDG code", chargedStable.getPDGCode()));
        return;
      }

      // require hits in CDC

      if (fitResult->getHitPatternCDC().getNHits() == 0) return;

      // find extHit and set pointers

      m_track = &track;

      Const::EDetector myDetID = Const::EDetector::TOP;
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      int numModules = geo->getNumModules();
      int pdgCode = abs(chargedStable.getPDGCode());

      RelationVector<ExtHit> extHits = track.getRelationsWith<ExtHit>();
      double tmin = 1e10; // some large time
      for (const auto& extHit : extHits) {
        if (abs(extHit.getPdgCode()) != pdgCode) continue;
        if (extHit.getDetectorID() != myDetID) continue;
        if (extHit.getCopyID() < 1 or extHit.getCopyID() > numModules) continue;
        if (extHit.getTOF() < tmin) {
          tmin = extHit.getTOF();
          m_extHit = &extHit;
        }
      }
      if (not m_extHit) return;

      m_mcParticle = track.getRelated<MCParticle>();
      if (m_mcParticle) {
        const auto barHits = m_mcParticle->getRelationsWith<TOPBarHit>();
        for (const auto& barHit : barHits) {
          if (barHit.getModuleID() == m_extHit->getCopyID()) m_barHit = &barHit;
        }
      }

      // set track parameters and helix

      m_moduleID = m_extHit->getCopyID();
      m_momentum = m_extHit->getMomentum().Mag();
      m_charge = fitResult->getChargeSign();
      m_TOFLength = m_extHit->getTOF() * Const::speedOfLight * getBeta(chargedStable);
      m_valid = setHelix(m_alignment->getRotation(m_moduleID), m_alignment->getTranslation(m_moduleID));
      if (not m_valid) return;

      // selection of photon hits belonging to this track

      double minTime = TOPRecoManager::getMinTime();
      double maxTime = TOPRecoManager::getMaxTime();
      const auto& pixelMasks = TOPRecoManager::getYScanner(m_moduleID)->getPixelMasks();

      StoreArray<TOPDigit> digits(digitsName);
      for (const auto& digit : digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        int pixelID = digit.getPixelID();
        if (not pixelMasks.isActive(pixelID)) continue;
        double time = digit.getTime();
        if (time < minTime or time > maxTime) continue;
        if (digit.getModuleID() == m_moduleID) {
          m_selectedHits.push_back(SelectedHit(pixelID, time, digit.getTimeError()));
        } else {
          m_numHitsOtherSlots++;
        }
      }

      // selected photon hits mapped to pixel columns

      unsigned numCols = TOPRecoManager::getYScanner(m_moduleID)->getPixelPositions().getNumPixelColumns();
      for (const auto& hit : m_selectedHits) {
        unsigned col = (hit.pixelID - 1) % numCols;
        m_columnHits.emplace(col, &hit);
      }

    }

    bool TOPTrack::setHelix(const TRotation& rotation, const TVector3& translation)
    {
      m_emissionPoints.clear();

      // helix in module nominal frame (z-axis still parallel to magnetic field)

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& module = geo->getModule(m_moduleID);
      auto globalPosition = m_extHit->getPosition();
      auto position = module.pointGlobalToNominal(globalPosition);
      auto momentum = module.momentumGlobalToNominal(m_extHit->getMomentum());
      double Bz = BFieldManager::getField(globalPosition).Z();
      m_helix.set(position, momentum, m_charge, Bz);
      m_helix.setTransformation(rotation, translation);

      // geometry data

      const RaytracerBase::BarSegment bar(module);
      const RaytracerBase::Mirror mirror(module);

      // bar surfaces in module nominal frame

      std::vector<TVector3> points;
      std::vector<TVector3> normals;
      points.push_back(rotation * TVector3(0, -bar.B / 2, 0) + translation); // lower
      normals.push_back(rotation * TVector3(0, -1, 0));

      points.push_back(rotation * TVector3(0, bar.B / 2, 0) + translation); // upper
      normals.push_back(rotation * TVector3(0, 1, 0));

      points.push_back(rotation * TVector3(-bar.A / 2, 0, 0) + translation); // left side
      normals.push_back(rotation * TVector3(-1, 0, 0));

      points.push_back(rotation * TVector3(bar.A / 2, 0, 0) + translation); // right side
      normals.push_back(rotation * TVector3(1, 0, 0));

      // intersection with quartz bar

      std::vector<double> lengths; // w.r.t extHit position
      std::vector<TVector3> positions; // in module local frame
      for (size_t i = 0; i < 2; i++) {
        double t = m_helix.getDistanceToPlane(points[i], normals[i]);
        if (isnan(t)) return false;
        auto r = m_helix.getPosition(t);
        if (abs(r.X()) > bar.A / 2) {
          auto k = (r.X() > bar.A / 2) ? 3 : 2;
          t = m_helix.getDistanceToPlane(points[k], normals[k]);
          if (isnan(t)) return false;
          r = m_helix.getPosition(t);
          if (abs(r.Y()) > bar.B / 2) return false;
        }
        lengths.push_back(t);
        positions.push_back(r);
      }
      if (lengths.size() != 2) return false;

      // crossing prism?

      if (positions[0].Z() < bar.zL or positions[1].Z() < bar.zL) {
        // TODO: track crossing prism needs special treatment
        return false;
      }

      // crossing mirror surface?

      std::vector<bool> outOfBar;
      TVector3 rc(mirror.xc, mirror.yc, mirror.zc);
      double Rsq = mirror.R * mirror.R;
      for (const auto& r : positions) {
        outOfBar.push_back((r - rc).Mag2() > Rsq);
      }
      if (outOfBar[0] and outOfBar[1]) return false;

      if (outOfBar[0] or outOfBar[1]) { // track crosses mirror surface, where?
        if (outOfBar[0]) std::reverse(lengths.begin(), lengths.end());
        double t1 = lengths[0];
        double t2 = lengths[1];
        for (int i = 0; i < 20; i++) {
          double t = (t1 + t2) / 2;
          auto r = m_helix.getPosition(t);
          if ((r - rc).Mag2() > Rsq) {
            t2 = t;
          } else {
            t1 = t;
          }
        }
        lengths[1] = (t1 + t2) / 2;
      }

      // set track length in quartz and full length from IP to the average emission point

      m_length = abs(lengths[1] - lengths[0]);
      double length = (lengths[0] + lengths[1]) / 2;
      m_trackLength = m_TOFLength + length;

      // finally move reference position to average emission point

      m_helix.moveReferencePosition(length);

      return m_length > bar.B / 2; // require minimal track lenght inside quartz (more than half of bar thickness)
    }

    const TOPTrack::AssumedEmission& TOPTrack::getEmissionPoint(double dL) const
    {
      auto& emissionPoint = m_emissionPoints[dL];
      if (not emissionPoint.isSet) {
        emissionPoint.position = m_helix.getPosition(dL);
        emissionPoint.trackAngles = TrackAngles(m_helix.getDirection(dL));
        emissionPoint.isSet = true;
      }
      return emissionPoint;
    }

    bool TOPTrack::isScanRequired(unsigned col, double time, double wid) const
    {
      const auto& tts = TOPGeometryPar::Instance()->getGeometry()->getTTS(0); // PMT independent TTS should be fine here
      const auto& range = m_columnHits.equal_range(col);
      for (auto it = range.first; it != range.second; ++it) {
        const auto hit = it->second;
        for (const auto& gaus : tts.getTTS()) {
          double sigsq = wid + pow(gaus.sigma, 2) + pow(hit->timeErr, 2);
          double x = pow(hit->time - time - gaus.position, 2) / sigsq;
          if (x < 10) return true;
        }
      }

      return false;
    }


  } // end top namespace
} // end Belle2 namespace

