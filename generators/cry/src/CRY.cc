/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <generators/cry/CRY.h>

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/IOIntercept.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <TDatabasePDG.h>
#include <TRandom3.h>

#include <VecGeom/volumes/UnplacedBox.h>
#include <VecGeom/volumes/UnplacedOrb.h>
#include <VecGeom/volumes/UnplacedTube.h>
#include <VecGeom/base/LorentzVector.h>
#include <geometry/GeometryManager.h>
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"

#include <string>
#include <vector>
#include <sstream>

namespace Belle2 {
  void CRY::init()
  {
    std::stringstream setupString;
    setupString << " returnGammas " << m_returnGammas << std::endl;
    setupString << " returnKaons " << m_returnKaons << std::endl;
    setupString << " returnPions " << m_returnPions << std::endl;
    setupString << " returnProtons " << m_returnProtons << std::endl;
    setupString << " returnNeutrons " << m_returnNeutrons << std::endl;
    setupString << " returnElectrons " << m_returnElectrons << std::endl;
    setupString << " returnMuons " << m_returnMuons << std::endl;
    setupString << " date " << m_date << std::endl;
    setupString << " latitude " << 36.0 << std::endl;
    setupString << " altitude " << 0 << std::endl;
    setupString << " subboxLength " << m_subboxLength << std::endl;

    IOIntercept::OutputToLogMessages initLogCapture("CRY", LogConfig::c_Debug, LogConfig::c_Warning, 100, 100);
    initLogCapture.start();

    // CRY setup
    m_crySetup.reset(new CRYSetup(setupString.str(), m_cosmicDataDir));
    // Set the random generator to the setup
    m_crySetup->setRandomFunction([]()->double { return gRandom->Rndm(); });
    // Set up the generator
    m_cryGenerator.reset(new CRYGenerator(m_crySetup.get()));

    initLogCapture.finish();

    // set up the acceptance box
    const auto coords = m_acceptSize.size();
    if (coords == 1) {
      m_acceptance.reset(new vecgeom::UnplacedOrb(m_acceptSize[0]));
    } else if (coords == 2) {
      m_acceptance.reset(new vecgeom::SUnplacedTube<vecgeom::TubeTypes::NonHollowTube>(0, m_acceptSize[0], m_acceptSize[1], 0, 2 * M_PI));
    } else if (coords == 3) {
      m_acceptance.reset(new vecgeom::UnplacedBox(m_acceptSize[0], m_acceptSize[1], m_acceptSize[2]));
    } else {
      B2FATAL("Acceptance volume needs to have one, two or three values for sphere, cylinder and box respectively");
    }
    //get information from geometry and create the world box
    G4VPhysicalVolume* volume = geometry::GeometryManager::getInstance().getTopVolume();
    if (!volume) {
      B2FATAL("No geometry found -> Add the Geometry module to the path before the CRY module.");
    }
    G4Box* topbox = (G4Box*) volume->GetLogicalVolume()->GetSolid();
    if (!topbox) {
      B2FATAL("No G4Box found -> Check the logical volume of the geometry.");
    }

    // Wrap the world coordinates (G4 coordinates are mm, Belle 2 units are currently cm)
    double  halfLength_x  = topbox->GetXHalfLength() * Belle2::Unit::mm;
    double  halfLength_y  = topbox->GetYHalfLength() * Belle2::Unit::mm;
    double halfLength_z  = topbox->GetZHalfLength() * Belle2::Unit::mm;

    //    m_world.reset(new vecgeom::UnplacedBox(m_subboxLength / 2. * Unit::m, m_subboxLength / 2. * Unit::m,
    //                                           m_subboxLength / 2. * Unit::m));
    m_world.reset(new vecgeom::UnplacedBox(halfLength_x / Unit::cm, halfLength_y / Unit::cm,
                                           halfLength_z / Unit::cm));
    B2INFO("World size [" << halfLength_x / Unit::cm << ", " << halfLength_x / Unit::cm << ",  " << halfLength_x / Unit::cm << "]");

    const double maxSize = *std::max_element(m_acceptSize.begin(), m_acceptSize.end());
    const double minWorld = std::min({halfLength_x, halfLength_y, halfLength_z, });
    if (maxSize > (minWorld / Unit::cm)) {
      B2FATAL("Acceptance bigger than world volume " << LogVar("acceptance", maxSize)
              << LogVar("World size", minWorld / Unit::cm));
    }

  }

  void CRY::generateEvent(MCParticleGraph& mcGraph)
  {
    bool eventInAcceptance = 0;
    static std::vector<CRYParticle*> ev;

    //force at least particle in acceptance box
    for (int iTrial = 0; iTrial < m_maxTrials; ++iTrial) {
      m_totalTrials++;
      // Generate one event
      ev.clear();
      m_cryGenerator->genEvent(&ev);
      // check all particles
      for (auto* p : ev) {
        const int pdg = p->PDGid();
        const double kineticEnergy = p->ke() * Unit::MeV;
        if (kineticEnergy < m_kineticEnergyThreshold) continue;

        const double mass = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
        const double etot = kineticEnergy + mass;
        // since etot is at least mass this cannot be negative
        const double ptot = sqrt(etot * etot - mass * mass);

        // Momentum
        // We have x horizontal,  y up and z along the beam. So uvw -> zxy, xc yc zc -> zxy
        const double px = ptot * p->v();
        const double py = ptot * p->w();
        const double pz = ptot * p->u();
        // Vertex
        const double vx = p->y() * Unit::m;
        const double vy = p->z() * Unit::m;
        const double vz = p->x() * Unit::m;

        // Time
        /* In basf2, it is assumed that t = 0 when an event was produced,
           For the cosmic case, we set t = 0 when particle cross y=0 plane;
           The output time from CRY (p->t()) is too large (order of second) and  also
        increase as simulated time, so it is impossible to handle in basf2.
        For event which has more then one particle, the difference between their production
        times is also too large (> micro-second) to keep in basf2, so the time relation
        between particles in each event is also reset. Production of each particle in event is set t=0 at y=0.
        if one need production from CRY for a special study, you have to find a way to handle it...*/
        double time = 0;

        vecgeom::Vector3D<double> pos(vx, vy, vz);
        vecgeom::LorentzVector<double> mom(px, py, pz, etot);
        const double speed = (mass == 0 ? 1 : mom.Beta()) * Const::speedOfLight;

        // Project on the boundary of the world box
        auto inside = m_world->Inside(pos);
        if (inside == vecgeom::kInside) {
          // inside the world volume, go backwards in time to the world box
          const auto dir = -mom.vect().Unit();
          double dist = m_world->DistanceToOut(pos, dir);
          pos += dist * dir;
          time -= dist / speed;
        } else if (inside == vecgeom::kOutside) {
          // outside the world volume, go forwards in time to the world box
          // this should not happen but better safe then sorry ...
          const auto dir = mom.vect().Unit();
          double dist = m_world->DistanceToIn(pos, dir);
          if (dist == vecgeom::InfinityLength<double>()) continue;
          pos += dist * dir;
          time += dist / speed;
        }
        // Intersect with the acceptance box
        double dist = m_acceptance->DistanceToIn(pos, mom.vect().Unit());
        if (dist == vecgeom::InfinityLength<double>()) continue;

        // We want to keep this one
        auto& particle = mcGraph.addParticle();
        // all particle of a generator are primary
        particle.addStatus(MCParticle::c_PrimaryParticle);
        // all particle of CRY are stable
        particle.addStatus(MCParticle::c_StableInGenerator);
        particle.setPDG(pdg);
        particle.setFirstDaughter(0);
        particle.setLastDaughter(0);
        particle.setMomentum(ROOT::Math::XYZVector(mom.x(), mom.y(), mom.z()));
        particle.setMass(mass);
        particle.setEnergy(mom.e());
        particle.setProductionVertex(ROOT::Math::XYZVector(pos.x(), pos.y(), pos.z()));
        particle.setProductionTime(time);
        eventInAcceptance = true;
      }
      // clean up CRY event
      for (auto* p : ev) delete p;
      // and if we have something in the acceptance then we're done
      if (eventInAcceptance) {
        return;
      }

    }
    B2ERROR("Number of trials exceeds maxTrials increase number of maxTrials" << LogVar("maxTrials", m_maxTrials));
  }

  void CRY::term()
  {
    B2RESULT("Total time simulated: " << m_cryGenerator->timeSimulated() << " seconds");
    B2RESULT("Total number of events simulated: " << m_totalTrials);
  }


}
