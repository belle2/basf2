/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/datastore/StoreArray.h>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;
using namespace ROOT::Math;

namespace Belle2 {
  namespace TOP {

    TOPTrack::TrackAngles::TrackAngles(const ROOT::Math::XYZVector& dir):
      cosTh(dir.Z()), sinTh(sqrt(1 - cosTh * cosTh)), cosFi(dir.X() / sinTh), sinFi(dir.Y() / sinTh)
    {}


    TOPTrack::TOPTrack(const Track& track, const std::string& digitsName, const Const::ChargedStable& chargedStable)
    {
      // find extrapolated track hit at TOP

      Const::EDetector myDetID = Const::EDetector::TOP;
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      int numModules = geo->getNumModules();
      int pdgCode = std::abs(chargedStable.getPDGCode());

      RelationVector<ExtHit> extHits = track.getRelationsWith<ExtHit>();
      double tmin = 1e10; // some large time
      for (const auto& extHit : extHits) {
        if (std::abs(extHit.getPdgCode()) != pdgCode) continue;
        if (extHit.getDetectorID() != myDetID) continue;
        if (extHit.getCopyID() < 1 or extHit.getCopyID() > numModules) continue;
        if (extHit.getTOF() < tmin) {
          tmin = extHit.getTOF();
          m_extHit = &extHit;
        }
      }
      if (not m_extHit) return;

      // set the object

      set(track, digitsName, chargedStable);
    }


    TOPTrack::TOPTrack(const ExtHit* extHit, const std::string& digitsName)
    {
      if (not extHit) return;
      m_extHit = extHit;

      const auto* track = extHit->getRelated<Track>();
      if (not track) {
        B2ERROR("TOPTrack: no related Track found for valid ExtHit");
        return;
      }

      auto chargedStable = Const::chargedStableSet.find(std::abs(extHit->getPdgCode()));
      if (chargedStable == Const::invalidParticle) {
        B2ERROR("TOPTrack: extrapolation hypothesis of ExtHit is not ChargedStable");
        return;
      }

      // set the object

      set(*track, digitsName, chargedStable);
    }


    void TOPTrack::set(const Track& track, const std::string& digitsName, const Const::ChargedStable& chargedStable)
    {
      // require fitResult

      const auto* fitResult = track.getTrackFitResultWithClosestMass(chargedStable);
      if (not fitResult) {
        B2ERROR("TOPTrack: no TrackFitResult available for PDGCode = " << chargedStable.getPDGCode());
        return;
      }
      m_pT = fitResult->getTransverseMomentum();

      // require hits in CDC

      if (fitResult->getHitPatternCDC().getNHits() == 0) return;

      // set pointers

      m_track = &track;
      m_mcParticle = track.getRelated<MCParticle>();
      if (m_mcParticle) {
        const auto barHits = m_mcParticle->getRelationsWith<TOPBarHit>();
        for (const auto& barHit : barHits) {
          if (barHit.getModuleID() == m_extHit->getCopyID()) m_barHit = &barHit;
        }
      }

      // set track parameters and helix

      m_moduleID = m_extHit->getCopyID();
      m_momentum = m_extHit->getMomentum().R();
      m_charge = fitResult->getChargeSign();
      m_TOFLength = m_extHit->getTOF() * Const::speedOfLight * getBeta(chargedStable);
      m_valid = setHelix(m_alignment->getTransformation(m_moduleID));
      if (not m_valid) return;

      // selection of photon hits belonging to this track

      unsigned numHitsOtherSlots = 0;
      StoreArray<TOPDigit> digits(digitsName);
      for (const auto& digit : digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getModuleID() == m_moduleID) {
          m_selectedHits.push_back(SelectedHit(digit.getPixelID(), digit.getTime(), digit.getTimeError()));
        } else {
          numHitsOtherSlots++;
        }
      }

      // background rate estimation (TODO to be improved ...)

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& tdc = geo->getNominalTDC();
      double timeWindow = m_feSetting->getReadoutWindows() * tdc.getSyncTimeBase() / static_cast<double>(TOPNominalTDC::c_syncWindows);

      const auto& backgroundPDFs = TOPRecoManager::getBackgroundPDFs();
      unsigned k = m_moduleID - 1;
      double effi = (k < backgroundPDFs.size()) ? backgroundPDFs[k].getEfficiency() : 0;
      double effiSum = 0;
      for (const auto& bkg : backgroundPDFs) effiSum += bkg.getEfficiency();
      m_bkgRate = (effiSum > effi) ? numHitsOtherSlots * effi / (effiSum - effi) / timeWindow : 0;

      // selected photon hits mapped to pixel columns

      const auto* yScanner = TOPRecoManager::getYScanner(m_moduleID);
      if (not yScanner) {
        B2ERROR("TOPTrack: YScanner for given module not found (must be a bug!)" << LogVar("slot", m_moduleID));
        m_valid = false;
        return;
      }
      unsigned numCols = yScanner->getPixelPositions().getNumPixelColumns();
      for (const auto& hit : m_selectedHits) {
        unsigned col = (hit.pixelID - 1) % numCols;
        m_columnHits.emplace(col, &hit);
      }

      m_valid = effi > 0; // no sense to provide PID for the track if the module is fully inefficient
    }


    bool TOPTrack::setHelix(const Transform3D& transform)
    {
      m_emissionPoints.clear();

      // helix in module nominal frame (z-axis still parallel to magnetic field)

      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      const auto& module = geo->getModule(m_moduleID);
      auto globalPosition = m_extHit->getPosition();
      auto position = module.pointGlobalToNominal(static_cast<XYZPoint>(globalPosition));
      auto momentum = module.momentumGlobalToNominal(m_extHit->getMomentum());
      double Bz = BFieldManager::getField(globalPosition).Z();
      m_helix.set(position, momentum, m_charge, Bz);
      m_helix.setTransformation(transform);

      // geometry data

      const RaytracerBase::BarSegment bar(module);
      const RaytracerBase::Mirror mirror(module);

      // bar surfaces in module nominal frame

      std::vector<XYZPoint> points;
      std::vector<XYZVector> normals;
      points.push_back(transform * XYZPoint(0, -bar.B / 2, 0)); // lower
      normals.push_back(transform * XYZVector(0, -1, 0));

      points.push_back(transform * XYZPoint(0, bar.B / 2, 0)); // upper
      normals.push_back(transform * XYZVector(0, 1, 0));

      points.push_back(transform * XYZPoint(-bar.A / 2, 0, 0)); // left side
      normals.push_back(transform * XYZVector(-1, 0, 0));

      points.push_back(transform * XYZPoint(bar.A / 2, 0, 0)); // right side
      normals.push_back(transform * XYZVector(1, 0, 0));

      // intersection with quartz bar

      std::vector<double> lengths; // w.r.t extHit position
      std::vector<XYZPoint> positions; // in module local frame
      for (size_t i = 0; i < 2; i++) {
        double t = m_helix.getDistanceToPlane(points[i], normals[i]);
        if (isnan(t)) return false;
        auto r = m_helix.getPosition(t);
        if (std::abs(r.X()) > bar.A / 2) {
          auto k = (r.X() > 0) ? 3 : 2;
          t = m_helix.getDistanceToPlane(points[k], normals[k]);
          if (isnan(t)) return false;
          r = m_helix.getPosition(t);
          if (r.Z() >= bar.zL and std::abs(r.Y()) > bar.B / 2) return false;
        }
        lengths.push_back(t);
        positions.push_back(r);
      }

      // crossing prism?

      if (positions[0].Z() < bar.zL or positions[1].Z() < bar.zL) {
        const RaytracerBase::Prism prism(module);
        bool ok = xsecPrism(lengths, positions, prism, transform);
        if (not ok) return false;
      }

      // crossing mirror surface?

      std::vector<bool> outOfBar;
      XYZPoint rc(mirror.xc, mirror.yc, mirror.zc);
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

      m_length = std::abs(lengths[1] - lengths[0]);
      double length = (lengths[0] + lengths[1]) / 2;
      m_trackLength = m_TOFLength + length;

      // finally move reference position to average emission point

      m_helix.moveReferencePosition(length);

      return m_length > bar.B / 2; // require minimal track lenght inside quartz (more than half of bar thickness)
    }


    bool TOPTrack::xsecPrism(std::vector<double>& lengths, std::vector<XYZPoint>& positions,
                             const RaytracerBase::Prism& prism, const Transform3D& transform)
    {
      std::vector<XYZPoint> points;
      std::vector<XYZVector> normals;

      points.push_back(transform * XYZPoint(0, prism.yDown, 0)); // lower-most surface
      normals.push_back(transform * XYZVector(0, -1, 0));

      points.push_back(transform * XYZPoint(0, prism.yUp, 0)); // upper surface
      normals.push_back(transform * XYZVector(0, 1, 0));

      points.push_back(transform * XYZPoint(-prism.A / 2, 0, 0)); // left-side surface
      normals.push_back(transform * XYZVector(-1, 0, 0));

      points.push_back(transform * XYZPoint(prism.A / 2, 0, 0)); // right-side surface
      normals.push_back(transform * XYZVector(1, 0, 0));

      for (size_t i = 0; i < 2; i++) {
        if (positions[i].Z() < prism.zR) {
          double t = m_helix.getDistanceToPlane(points[i], normals[i]);
          if (isnan(t)) return false;
          auto r = m_helix.getPosition(t);
          if (i == 0 and r.Z() > prism.zFlat) { // intersection with slanted surface -> find it using bisection
            auto point = transform * XYZPoint(0, -prism.B / 2, 0);
            double t1 = m_helix.getDistanceToPlane(point, normals[0]);
            if (isnan(t1)) return false;
            double t2 = t;
            for (int iter = 0; iter < 20; iter++) {
              t = (t1 + t2) / 2;
              r = m_helix.getPosition(t);
              double ySlanted = prism.yDown + prism.slope * (r.Z() - prism.zFlat);
              if (r.Y() < ySlanted) t2 = t;
              else t1 = t;
            }
            t = (t1 + t2) / 2;
          }
          if (std::abs(r.X()) > prism.A / 2) { // intersection on the side surface
            auto k = (r.X() > 0) ? 3 : 2;
            t = m_helix.getDistanceToPlane(points[k], normals[k]);
            if (isnan(t)) return false;
            r = m_helix.getPosition(t);
            if (r.Z() < prism.zR) { // yes, it's on the prism side
              if (r.Y() > prism.yUp or r.Y() < prism.yDown) return false;
              double ySlanted = prism.yDown + prism.slope * (r.Z() - prism.zFlat);
              if (r.Y() < ySlanted) return false;
            } else { // no, it's on the prism entrance but outside the bar exit window -> find it using bisection
              double t1 = lengths[i];
              double t2 = t;
              for (int iter = 0; iter < 20; iter++) {
                t = (t1 + t2) / 2;
                r = m_helix.getPosition(t);
                if (r.Z() < prism.zR) t1 = t;
                else t2 = t;
              }
              t = (t1 + t2) / 2;
            }
          }
          lengths[i] = t;
          positions[i] = m_helix.getPosition(t);
        }
      }

      if (positions[0].Z() < prism.zL and positions[1].Z() < prism.zL) return false;

      if (positions[0].Z() < prism.zL or positions[1].Z() < prism.zL) { // intersection is on exit window
        int i0 = (positions[0].Z() < prism.zL) ? 0 : 1;
        double t1 = lengths[i0];
        double t2 = lengths[(i0 + 1) % 2];
        for (int iter = 0; iter < 20; iter++) {
          double t = (t1 + t2) / 2;
          auto r = m_helix.getPosition(t);
          if (r.Z() < prism.zL) t1 = t;
          else t2 = t;
        }
        double t = (t1 + t2) / 2;
        lengths[i0] = t;
        positions[i0] = m_helix.getPosition(t);
      }

      return true;
    }


    const TOPTrack::AssumedEmission& TOPTrack::getEmissionPoint(double dL) const
    {
      if (m_emissionPoints.size() > 1000) m_emissionPoints.clear(); // prevent blow-up

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

