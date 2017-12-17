/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/cry/CRY.h>

#include <geometry/GeometryManager.h>
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"

#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/IOIntercept.h>

#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TRandom3.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>  // For Ubuntu Linux

using namespace std;
using namespace Belle2;

CRY::CRY()
{
  m_totalTrials = 0;
  m_maxTrialsRun = 0;
  m_checkAcceptance = false;
  m_checkKeep = false;
  m_acceptLength = 0.;
  m_acceptWidth = 0.;
  m_acceptHeight = 0.;
  m_keepLength = 0.;
  m_keepWidth = 0.;
  m_keepHeight = 0.;
  m_kineticEnergyThreshold = 0.;
  m_maxTrials = 0.;
  m_crySetup = NULL;
  m_cryGenerator = NULL;
  m_cryRNG = NULL;
  m_xlow = 0.;
  m_xhigh = 0.;
  m_ylow = 0.;
  m_yhigh = 0.;
  m_zlow = 0.;
  m_zhigh = 0.;
  m_startTime = 0.;
  m_newvtx[0] = 0.;
  m_newvtx[1] = 0.;
  m_newvtx[2] = 0.;
  m_timeOffset = 0.;

  setDefaultSettings();
}

CRY::~CRY()
{

}

void CRY::setDefaultSettings()
{

}

void CRY::init()
{
  // Parse the contents of the setup file and the cosmic data
  std::ifstream inputFile;
  inputFile.open(m_setupFile, std::ios::in);
  char buffer[2000];

  // check if the setup file is found
  if (! inputFile.is_open()) {
    B2FATAL("[CRY]: Could not open setup file: " << m_setupFile);
  }

  std::string setupString("");
  while (!inputFile.getline(buffer, 2000).eof()) {
    setupString.append(buffer);
    setupString.append(" ");
  }


  IOIntercept::OutputToLogMessages initLogCapture("CRY", LogConfig::c_Debug, LogConfig::c_Warning, 100, 100);
  initLogCapture.start();

  // CRY setup
  m_crySetup = new CRYSetup(setupString, m_cosmicDataDir);

  // Get the framework random generator and wrap it to the CRY needs
  m_cryRNG = new CRYRndm();
  RNGWrapper<CRYRndm>::set(m_cryRNG, &CRYRndm::rndm);

  // Set the random generator to the setup
  m_crySetup->setRandomFunction(RNGWrapper<CRYRndm>::rng);

  // Set up the generator
  m_cryGenerator = new CRYGenerator(m_crySetup);

  // Get top volume of the geometry
  FillTopVolumeCoordinates();

  // Set the starting time
  m_startTime = m_cryGenerator->timeSimulated() * Belle2::Unit::s;
  initLogCapture.finish();

  // set up the acceptance box
  if (m_acceptWidth > 0. && m_acceptHeight > 0. && m_acceptLength > 0.) {
    m_checkAcceptance = true;
    TVector3 B1a(-m_acceptWidth * Belle2::Unit::m / 2., -m_acceptHeight * Belle2::Unit::m / 2., -m_acceptLength * Belle2::Unit::m / 2.);
    TVector3 B2a(m_acceptWidth * Belle2::Unit::m / 2.,  m_acceptHeight * Belle2::Unit::m / 2.,  m_acceptLength * Belle2::Unit::m / 2.);
    m_B1 = B1a;
    m_B2 = B2a;
  }

  // set up the keep box
  if (m_keepWidth > 0. && m_keepHeight > 0. && m_keepLength > 0.) {
    m_checkKeep = true;
    TVector3 B1a(-m_keepWidth * Belle2::Unit::m / 2., -m_keepHeight * Belle2::Unit::m / 2., -m_keepLength * Belle2::Unit::m / 2.);
    TVector3 B2a(m_keepWidth * Belle2::Unit::m / 2.,  m_keepHeight * Belle2::Unit::m / 2.,  m_keepLength * Belle2::Unit::m / 2.);
    m_B1keep = B1a;
    m_B2keep = B2a;
  }

  applySettings();
}


void CRY::FillTopVolumeCoordinates()
{
  G4VPhysicalVolume* volume = geometry::GeometryManager::getInstance().getTopVolume();
  if (!volume) {
    B2FATAL("No geometry found -> Add the Geometry module to the path before the CRY module.");
    return;
  }

  G4Box* topbox = (G4Box*) volume->GetLogicalVolume()->GetSolid();
  if (!topbox) {
    B2FATAL("No G4Box found -> Check the logical volume of the geometry.");
    return;
  }

  // Wrap the world coordinates (G4 coordinates are mm, Belle 2 units are currently cm)
  m_xlow  = -topbox->GetXHalfLength() * Belle2::Unit::mm;
  m_xhigh =  topbox->GetXHalfLength() * Belle2::Unit::mm;
  m_ylow  = -topbox->GetYHalfLength() * Belle2::Unit::mm;
  m_yhigh =  topbox->GetYHalfLength() * Belle2::Unit::mm;
  m_zlow  = -topbox->GetZHalfLength() * Belle2::Unit::mm;
  m_zhigh =  topbox->GetZHalfLength() * Belle2::Unit::mm;

  B2DEBUG(100, "m_xlow=" << m_xlow << ", m_xhigh=" << m_xhigh);
  B2DEBUG(100, "m_ylow=" << m_ylow << ", m_xhigh=" << m_yhigh);
  B2DEBUG(100, "m_zlow=" << m_zlow << ", m_xhigh=" << m_zhigh);

  //check if the acceptance volume is inside top volume
  if (-m_acceptWidth * Belle2::Unit::m / 2. < m_xlow || m_acceptWidth * Belle2::Unit::m / 2. > m_xhigh ||
      -m_acceptHeight * Belle2::Unit::m / 2. < m_ylow || m_acceptHeight * Belle2::Unit::m / 2. > m_yhigh ||
      -m_acceptLength * Belle2::Unit::m / 2. < m_zlow
      || m_acceptLength * Belle2::Unit::m / 2. > m_zhigh) B2FATAL("Acceptance volume larger than topvolume!");

  //check if the keep volume is inside top volume
  if (-m_keepWidth * Belle2::Unit::m / 2. < m_xlow || m_keepWidth * Belle2::Unit::m / 2. > m_xhigh ||
      -m_keepHeight * Belle2::Unit::m / 2. < m_ylow || m_keepHeight * Belle2::Unit::m / 2. > m_yhigh ||
      -m_keepLength * Belle2::Unit::m / 2. < m_zlow
      || m_keepLength * Belle2::Unit::m / 2. > m_zhigh) B2FATAL("Keep volume larger than topvolume!");

  //check if the acceptance volume is inside keep volume
  if (-m_acceptWidth * Belle2::Unit::m / 2. < -m_keepWidth * Belle2::Unit::m / 2.
      || m_acceptWidth * Belle2::Unit::m / 2. > m_keepWidth * Belle2::Unit::m / 2. ||
      -m_acceptHeight * Belle2::Unit::m / 2. < -m_keepHeight * Belle2::Unit::m / 2.
      || m_acceptHeight * Belle2::Unit::m / 2. > m_keepHeight * Belle2::Unit::m / 2. ||
      -m_acceptLength * Belle2::Unit::m / 2. < -m_keepLength * Belle2::Unit::m / 2.
      || m_acceptLength * Belle2::Unit::m / 2. > m_keepLength * Belle2::Unit::m / 2.)
    B2FATAL("Acceptance volume larger than keep volume!");

}

void CRY::generateEvent(MCParticleGraph& mcGraph)
{

  int eventInAcceptance = 0;

  //force at least particle in acceptance box
  int iTrial = 0;
  for (iTrial = 0; iTrial < m_maxTrials; iTrial++) {
    m_totalTrials++;

//     std::vector<CRYParticle*> *ev = new std::vector<CRYParticle*>;
    std::vector<CRYParticle*>* ev = new std::vector<CRYParticle*>;
    ev->clear();

    // Generate one event
    m_cryGenerator->genEvent(ev);

    for (unsigned j = 0; j < ev->size(); j++) {
      int pdg     = (*ev)[j]->PDGid();
      double ke   = (*ev)[j]->ke() * Belle2::Unit::MeV;
      if (ke < m_kineticEnergyThreshold) continue;

      double mass = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
      double etot = ke + mass;
      double ptot = etot * etot - mass * mass;
      if (ptot > 0.0) ptot = sqrt(ptot);
      else ptot   = 0.0;

      // Momentum
      // We have x horizontal,  y up and z along the beam. So uvw -> zxy, xc yc zc -> zxy
      double px = ptot * (*ev)[j]->v();
      double py = ptot * (*ev)[j]->w();
      double pz = ptot * (*ev)[j]->u();

      // Vertex
      double vx = (*ev)[j]->y() * Belle2::Unit::m;
      double vy = (*ev)[j]->z() * Belle2::Unit::m; // + m_yhigh;
      double vz = (*ev)[j]->x() * Belle2::Unit::m;

      // Project to edges of world box
      double vtx[3]    = { vx, vy, vz };
      double p[3]      = { -px, -py, -pz };
      ProjectToTopVolume(vtx, p, m_newvtx);

      if (m_checkAcceptance) {
        TVector3 Direction(px, py, pz);
        Direction.Unit();
        TVector3 L1(m_newvtx[0],  m_newvtx[1],  m_newvtx[2]);
        TVector3 L2(0., 0., 0.);
        L2 = L1 + 1.e9 * Direction;
        TVector3 Hit(0., 0., 0.);
        int check = CheckLineBox(m_B1, m_B2, L1, L2, Hit);
        if (check > 0) eventInAcceptance++;
      }

    }

    if ((m_checkAcceptance == true && eventInAcceptance > 0) || m_checkAcceptance == false) {
      // all secondaries
      for (unsigned j = 0; j < ev->size(); j++) {

        int pdg     = (*ev)[j]->PDGid();
        double ke   = (*ev)[j]->ke() * Belle2::Unit::MeV;
        if (ke < m_kineticEnergyThreshold) continue;

        double mass = TDatabasePDG::Instance()->GetParticle(pdg)->Mass();
        double etot = ke + mass;
        double ptot = etot * etot - mass * mass;
        if (ptot > 0.0) ptot = sqrt(ptot);
        else ptot = 0.0;

        double ptime = (*ev)[j]->t() * Belle2::Unit::s - m_startTime;

        // Momentum
        // x horizontal, y up and z along e- beam direction. So uvw -> zxy, xc yc zc -> zxy
        double px = ptot * (*ev)[j]->v();
        double py = ptot * (*ev)[j]->w();
        double pz = ptot * (*ev)[j]->u();
//         double p4[4] = {px, py, pz, etot};

        // Vertex
        double vx = (*ev)[j]->y() * Belle2::Unit::m;
        double vy = (*ev)[j]->z() * Belle2::Unit::m; // + m_yhigh;
        double vz = (*ev)[j]->x() * Belle2::Unit::m;

        // Project to edges of world box
        double vtx[3]    = { vx, vy, vz };
        double p[3]      = { -px, -py, -pz };
        ProjectToTopVolume(vtx, p, m_newvtx);

        int check = 0;
        if (m_checkKeep) {
          TVector3 Direction(px, py, pz);
          Direction.Unit();
          TVector3 L1(m_newvtx[0],  m_newvtx[1],  m_newvtx[2]);
          TVector3 L2(0., 0., 0.);
          L2 = L1 + 1.e9 * Direction;
          TVector3 Hit(0., 0., 0.);
          check = CheckLineBox(m_B1keep, m_B2keep, L1, L2, Hit);
        } else check = 1;

        if (check > 0) {
          double p4[4] = {px, py, pz, etot};
          storeParticle(mcGraph, p4, m_newvtx, ptime, pdg, false, false);
        }

        delete(*ev)[j];
      }

      break;
    }

    else {
      for (unsigned j = 0; j < ev->size(); j++) delete(*ev)[j];
    }
  }

  // check if the number of trials exceeds the maximum
  if (iTrial + 1 >= m_maxTrials) {
    B2ERROR("Number of trials (" << iTrial + 1 << ") exceeds maxTrials (" << m_maxTrials << "), increase number of maxTrials");
  }

}

void CRY::ProjectToTopVolume(const double vtx[], const double p[], double newvtx[])
{

  // inside or outside the world volume
  if (vtx[0] < m_xlow || vtx[0] > m_xhigh ||
      vtx[1] < m_ylow || vtx[1] > m_yhigh ||
      vtx[2] < m_zlow || vtx[2] > m_zhigh) B2FATAL("Particle starts outside top volume!");

  // Compute the distances to the walls
  double dx = 1.e30;
  double dy = 1.e30;
  double dz = 1.e30;
  if (p[0] > 0.0) {
    dx = (m_xhigh - vtx[0]) / p[0];
  } else if (p[0] < 0.0) {
    dx = (m_xlow - vtx[0]) / p[0];
  }

  if (p[1] > 0.0) {
    dy = (m_yhigh - vtx[1]) / p[1];
  } else if (p[1] < 0.0) {
    dy = (m_ylow - vtx[1]) / p[1];
  }

  if (p[2] > 0.0) {
    dz = (m_zhigh - vtx[2]) / p[2];
  } else if (p[2] < 0.0) {
    dz = (m_zlow - vtx[2]) / p[2];
  }

  // Choose the shortest distance
  double d = 0.0;
  if (dx < dy && dx < dz) d = dx;
  else if (dy < dz && dy < dx) d = dy;
  else if (dz < dx && dz < dy) d = dz;

  // Make the step, reduced by 0.1% to avoid edge effects in GEANT
  for (int i = 0; i < 3; ++i) {
    newvtx[i] = vtx[i] + p[i] * d * 0.999;
  }

}

bool CRY::CheckLineBox(TVector3 B1, TVector3 B2, TVector3 L1, TVector3 L2, TVector3& hit)
{
  if (L2.x() < B1.x() && L1.x() < B1.x()) return false;
  if (L2.x() > B2.x() && L1.x() > B2.x()) return false;
  if (L2.y() < B1.y() && L1.y() < B1.y()) return false;
  if (L2.y() > B2.y() && L1.y() > B2.y()) return false;
  if (L2.z() < B1.z() && L1.z() < B1.z()) return false;
  if (L2.z() > B2.z() && L1.z() > B2.z()) return false;
  if (L1.x() > B1.x() && L1.x() < B2.x() &&
      L1.y() > B1.y() && L1.y() < B2.y() &&
      L1.z() > B1.z() && L1.z() < B2.z()) {
    hit = L1;
    return true;
  }
  if ((GetIntersection(L1.x() - B1.x(), L2.x() - B1.x(), L1, L2, hit) && InBox(hit, B1, B2, 1))
      || (GetIntersection(L1.y() - B1.y(), L2.y() - B1.y(), L1, L2, hit) && InBox(hit, B1, B2, 2))
      || (GetIntersection(L1.z() - B1.z(), L2.z() - B1.z(), L1, L2, hit) && InBox(hit, B1, B2, 3))
      || (GetIntersection(L1.x() - B2.x(), L2.x() - B2.x(), L1, L2, hit) && InBox(hit, B1, B2, 1))
      || (GetIntersection(L1.y() - B2.y(), L2.y() - B2.y(), L1, L2, hit) && InBox(hit, B1, B2, 2))
      || (GetIntersection(L1.z() - B2.z(), L2.z() - B2.z(), L1, L2, hit) && InBox(hit, B1, B2, 3)))
    return true;

  return false;
}

bool CRY::GetIntersection(float fDst1, float fDst2, TVector3 P1, TVector3 P2, TVector3& hit)
{
  if ((fDst1 * fDst2) >= 0.0) return false;
  if (fDst1 == fDst2) return false;
  hit = P1 + (P2 - P1) * (-fDst1 / (fDst2 - fDst1));
  return true;
}

bool CRY::InBox(TVector3 hit, TVector3 B1, TVector3 B2, int axis)
{
  if (axis == 1 && hit.z() > B1.z() && hit.z() < B2.z() && hit.y() > B1.y() && hit.y() < B2.y()) return true;
  if (axis == 2 && hit.z() > B1.z() && hit.z() < B2.z() && hit.x() > B1.x() && hit.x() < B2.x()) return true;
  if (axis == 3 && hit.x() > B1.x() && hit.x() < B2.x() && hit.y() > B1.y() && hit.y() < B2.y()) return true;
  return false;
}

void CRY::term()
{
//   double timeSimulated = m_cryGenerator->timeSimulated();
  B2RESULT("Total time simulated: " << m_cryGenerator->timeSimulated() << "s");
  B2RESULT("Total number of events simulated: " << m_totalTrials);

  delete m_crySetup;
  delete m_cryRNG;
  delete m_cryGenerator;

}

//=========================================================================
//                       Protected methods
//=========================================================================

void CRY::applySettings()
{

}


void CRY::storeParticle(MCParticleGraph& mcGraph, const double* mom, const double* vertex, const double ptime, const int pdg,
                        bool isVirtual, bool isInitial)
{

  // Create particle
  MCParticleGraph::GraphParticle& part = mcGraph.addParticle();
  if (isVirtual) {
    part.setStatus(MCParticle::c_IsVirtual);
  } else if (isInitial) {
    part.setStatus(MCParticle::c_Initial);
  }

  // all particle of a generator are primary
  part.addStatus(MCParticle::c_PrimaryParticle);

  // all particle of CRY are stable
  part.addStatus(MCParticle::c_StableInGenerator);

  part.setPDG(pdg);
  part.setFirstDaughter(0);
  part.setLastDaughter(0);
  part.setMomentum(TVector3(mom[0], mom[1], mom[2]));
  part.setMass(TDatabasePDG::Instance()->GetParticle(pdg)->Mass());
  part.setEnergy(mom[3]);
  part.setProductionVertex(TVector3(vertex[0], vertex[1], vertex[2]));

  // correct the time back such that t=0 at y=0
  const TVector3 vertexVector(vertex[0], vertex[1], vertex[2]);
  const TVector3 momentumVector(mom[0], mom[1], mom[2]);
  const TVector3 normalVector(0., 1., 0.); // normal to the global generation plane
  const double beta = momentumVector.Mag() / mom[3]; // lorentz beta factor of this particle
  const double speed = beta  * Const::speedOfLight; // speed of this particle

  const double d = ((TVector3(0, 0, 0) - vertexVector) * normalVector) / (momentumVector * normalVector);
  TVector3 intersectionVector(0., 0., 0.);
  intersectionVector = d * momentumVector + vertexVector; //intersection of extrapolated particle with global plane
  const double trackLength = (vertexVector - intersectionVector).Mag();
  const double flightTime = trackLength / speed; // flighttime from virtual starting point to y=0

  part.setProductionTime(m_timeOffset - flightTime);

  B2DEBUG(100, "Original time from CRY: " << ptime);
}


//-----------------------------------------------------------------
//                 random generator for CRY
//-----------------------------------------------------------------
CRYRndm::CRYRndm()
{

}

double CRYRndm::rndm()
{
  double value = gRandom->Rndm();
  return value;
}

