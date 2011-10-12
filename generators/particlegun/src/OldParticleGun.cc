/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Susanne Koblitz                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <generators/particlegun/OldParticleGun.h>

#include <TMath.h>

#include <string>
#include <stdexcept>
#include <cmath>


using namespace std;
using namespace Belle2;

OldParticleGun::OldParticleGun()
{
  B2WARNING("This version of the ParticleGun is deprecated, please switch to the ParticleGun module.");
}

bool OldParticleGun::generateEvent(MCParticleGraph &graph)
{
  int first = graph.size();

  //generate the event vertex (the same for all particles in event)
  double vx = 0;
  double vy = 0;
  double vz = 0;

  switch (m_genVert) {
    case c_gauss:
      vx = m_gRand.Gaus(m_xVertexPar1, m_xVertexPar2);
      vy = m_gRand.Gaus(m_yVertexPar1, m_yVertexPar2);
      vz = m_gRand.Gaus(m_zVertexPar1, m_zVertexPar2);
      break;
    case c_uniform:
    default:
      vx = m_gRand.Uniform(m_xVertexPar1, m_xVertexPar2);
      vy = m_gRand.Uniform(m_yVertexPar1, m_yVertexPar2);
      vz = m_gRand.Uniform(m_zVertexPar1, m_zVertexPar2);
      break;
    case c_none:
      vx = m_xVertexPar1;
      vy = m_yVertexPar1;
      vz = m_zVertexPar1;
      break;
  }


  //Make list of particles
  for (int i = 0; i < m_nTracks; i++) {
    graph.addParticle();

    MCParticleGraph::GraphParticle &p = graph[first+i];
    p.setStatus(MCParticle::c_PrimaryParticle);

    if (m_PIDcodes.size() == 1)// only one PIDcode available, always take this one
      p.setPDG(m_PIDcodes[0]);
    else {//else choose randomly one of the available codes
      int index = static_cast<int>(m_gRand.Uniform(m_PIDcodes.size()));
      p.setPDG(m_PIDcodes[index]);
    }
    p.setMassFromPDG();
    p.setChargeFromPDG();
    p.setFirstDaughter(0);
    p.setLastDaughter(0);

    //lets generate the momentum vector:
    double ptot = 0;
    switch (m_genMom) {
      case c_gauss:
        ptot = m_gRand.Gaus(m_pPar1, m_pPar2);
        break;
      case c_uniform:
      default:
        ptot = m_gRand.Uniform(m_pPar1, m_pPar2);
        break;
      case c_none:
        ptot = m_pPar1;
        break;
    }
    double phi = 0;
    double theta = 0;
    switch (m_genAngle) {
      case c_gauss:
        phi = m_gRand.Gaus(m_phiPar1, m_phiPar2);
        theta = m_gRand.Gaus(m_thetaPar1, m_thetaPar2);
        break;
      case c_uniform:
      default:
        theta = m_gRand.Uniform(m_thetaPar1, m_thetaPar2);
        phi = m_gRand.Uniform(m_phiPar1, m_phiPar2);
        break;
      case c_none:
        phi = m_phiPar1;
        theta = m_thetaPar1;
        break;
    }

    double pt = ptot * sin(theta);
    if (m_fixedPt) {//this means we are actually simulating the Pt and not the P
      pt = ptot;
      if (sin(theta) != 0) { ptot = pt / sin(theta);} else ptot = 20; //here 20GeV replace "infinitely large"
    }

    double pz = ptot * cos(theta);
    double px = pt * cos(phi);
    double py = pt * sin(phi);
    p.setMomentum(px, py, pz);
    double m = p.getMass();
    double e = sqrt(ptot * ptot + m * m);
    p.setEnergy(e);

    p.setProductionVertex(vx, vy, vz);

    p.addStatus(MCParticle::c_StableInGenerator);

  }// end loop over particles in event

  return true;
}

