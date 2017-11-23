/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdlib.h>
#include <TRandom.h>
// top
#include <top/reconstruction/TOPtrack.h>
#include <top/geometry/TOPGeometryPar.h>
// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
// DataStore classes
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>

extern "C" {
  void track2top_(float*, float*, float*, float*, int*); // from top_geo.F
}

namespace Belle2 {
  namespace TOP {

    TOPtrack::TOPtrack(double x, double y, double z,
                       double px, double py, double pz,
                       double Tlen, int Q, int pdg):
      m_valid(true), m_position(x, y, z), m_momentum(px, py, pz),
      m_trackLength(Tlen), m_charge(Q), m_pdg(pdg),
      m_atTop(false), m_moduleID(0),
      m_track(0), m_extHit(0), m_mcParticle(0), m_barHit(0)
    {
      m_moduleID = findModule();
    }


    TOPtrack::TOPtrack(const Track* track,
                       const Const::ChargedStable& chargedStable):
      m_valid(false), m_trackLength(0.0), m_charge(0), m_pdg(0),
      m_atTop(false), m_moduleID(0),
      m_track(0), m_extHit(0), m_mcParticle(0), m_barHit(0)
    {

      if (!track) return;

      // set pointers first
      m_track = track;

      Const::EDetector myDetID = Const::EDetector::TOP;
      const auto* geo = TOPGeometryPar::Instance()->getGeometry();
      int numModules = geo->getNumModules();
      int pdgCode = abs(chargedStable.getPDGCode());

      RelationVector<ExtHit> extHits = track->getRelationsWith<ExtHit>();
      double tmin = 1e10; // some large time
      for (unsigned i = 0; i < extHits.size(); i++) {
        const ExtHit* extHit = extHits[i];
        if (abs(extHit->getPdgCode()) != pdgCode) continue;
        if (extHit->getDetectorID() != myDetID) continue;
        if (extHit->getCopyID() < 1 or extHit->getCopyID() > numModules) continue;
        if (extHit->getTOF() < tmin) {
          tmin = extHit->getTOF();
          m_extHit = extHit;
        }
      }
      if (!m_extHit) return;

      m_mcParticle = track->getRelated<MCParticle>();
      if (m_mcParticle) {
        const auto barHits = m_mcParticle->getRelationsWith<TOPBarHit>();
        for (const auto& barHit : barHits) {
          if (barHit.getModuleID() == m_extHit->getCopyID()) m_barHit = &barHit;
        }
      }

      // set track parameters
      m_position = m_extHit->getPosition();
      m_momentum = m_extHit->getMomentum();
      setTrackLength(m_extHit->getTOF(), chargedStable);
      const auto* fitResult = track->getTrackFitResultWithClosestMass(chargedStable);
      if (!fitResult) {
        B2ERROR("No TrackFitResult for " << chargedStable.getPDGCode());
        return;
      }
      m_charge = fitResult->getChargeSign();
      if (m_mcParticle) m_pdg = m_mcParticle->getPDG();
      m_moduleID = m_extHit->getCopyID();
      m_valid = true;
    }


    double TOPtrack::getTOF(double mass) const
    {
      double pmom = getP();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      return m_trackLength / beta / Const::speedOfLight;
    }

    void TOPtrack::setTrackLength(double tof, double mass)
    {
      double pmom = getP();
      double beta = pmom / sqrt(pmom * pmom + mass * mass);
      m_trackLength = tof * Const::speedOfLight * beta;
    }

    int TOPtrack::getHypID() const
    {
      int lundc[6] = {11, 13, 211, 321, 2212, 1000010020};
      for (int i = 0; i < 6; i++) {
        if (abs(m_pdg) == lundc[i]) return i + 1;
      }
      return 0;
    }

    int TOPtrack::toTop()
    {
      if (m_atTop) return m_moduleID;

      float r[3] = {(float) m_position.X(),
                    (float) m_position.Y(),
                    (float) m_position.Z()
                   };
      float p[3] = {(float) m_momentum.X(),
                    (float) m_momentum.Y(),
                    (float) m_momentum.Z()
                   };
      float q = (float) m_charge;
      float t; int m;
      track2top_(r, p, &q, &t, &m);
      m_position.SetXYZ(r[0], r[1], r[2]);
      m_momentum.SetXYZ(p[0], p[1], p[2]);
      m_trackLength += t * Const::speedOfLight;
      m_atTop = true;
      m_moduleID = m + 1;
      return m_moduleID;
    }

    void TOPtrack::smear(double sig_x, double sig_z,
                         double sig_theta, double sig_phi)
    {
      double p = getP();
      if (p == 0) return;
      double theta = getTheta() + gRandom->Gaus(0., sig_theta);
      double phi = getPhi() + gRandom->Gaus(0., sig_phi);
      m_momentum.SetX(p * cos(phi) * sin(theta));
      m_momentum.SetY(p * sin(phi) * sin(theta));
      m_momentum.SetZ(p * cos(theta));

      double rho = gRandom->Gaus(0., sig_x);
      if (m_position.Perp() != 0) phi = atan2(m_position.Y(), m_position.X());
      double x = m_position.X() + rho * sin(phi);
      double y = m_position.Y() - rho * cos(phi);
      double z = m_position.Z() + gRandom->Gaus(0., sig_z);
      m_position.SetXYZ(x, y, z);

    }

    void TOPtrack::dump() const
    {
      double pi = 4 * atan(1);
      using namespace std;
      cout << "TOPtrack::dump(): ";
      cout << " PDG=" << m_pdg;
      cout << " charge=" << m_charge << endl;
      cout << "  p=" << setprecision(3) << getP() << " GeV/c";
      cout << "  theta=" << setprecision(3) << getTheta() / pi * 180;
      cout << "  phi=" << setprecision(3) << getPhi() / pi * 180 << endl;
      cout << "  x=" << getX() << " cm";
      cout << "  y=" << getY() << " cm";
      cout << "  z=" << getZ() << " cm\n";
      cout << "  trackLength=" << setprecision(4) << m_trackLength << " cm";
      cout << "  atTop=" << m_atTop;
      cout << "  moduleID=" << m_moduleID;
      cout << endl;
    }

    int TOPtrack::findModule()
    {
      float r[3] = {(float) m_position.X(),
                    (float) m_position.Y(),
                    (float) m_position.Z()
                   };
      float p[3] = {(float) m_momentum.X(),
                    (float) m_momentum.Y(),
                    (float) m_momentum.Z()
                   };
      float q = (float) m_charge;
      float t;
      int m;
      track2top_(r, p, &q, &t, &m);
      return m + 1;
    }

  } // end top namespace
} // end Belle2 namespace

