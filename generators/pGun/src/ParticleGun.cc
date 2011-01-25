/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <generators/pGun/ParticleGun.h>

#include <TMath.h>

#include <string>
#include <stdexcept>
#include <cmath>


using namespace std;
using namespace Belle2;
using namespace Generators;

bool ParticleGun::generateEvent(MCParticleGraph &graph)
{
  int first = graph.size();
  //Make list of particles
  for (int i = 0; i < m_ntracks; i++) {
    graph.addParticle();

    MCParticleGraph::GraphParticle &p = graph[first+i];
    p.setStatus(MCParticle::PrimaryParticle);

    if (m_PIDcodes.size() == 1)// only one PIDcode available, always take this one
      p.setPDG(m_PIDcodes[0]);
    else {//else choose randomly one of the available codes
      int index = static_cast<int>(m_grand.Uniform(m_PIDcodes.size()));
      p.setPDG(m_PIDcodes[index]);
    }
    p.setMassFromPDG();
    p.setFirstDaughter(0);
    p.setLastDaughter(0);

    //lets generate the momentum vector:
    double ptot = 0;
    switch (m_genMom) {
      case gauss:
        ptot = m_grand.Gaus(m_p_par1, m_p_par2);
        break;
      case uniform:
      default:
        ptot = m_grand.Uniform(m_p_par1, m_p_par2);
        break;
      case none:
        ptot = m_p_par1;
        break;
    }
    double phi = 0;
    double theta = 0;
    switch (m_genAngle) {
      case gauss:
        phi = m_grand.Gaus(m_ph_par1, m_ph_par2);
        theta = m_grand.Gaus(m_th_par1, m_th_par2);
        break;
      case uniform:
      default:
        theta = m_grand.Uniform(m_th_par1, m_th_par2);
        phi = m_grand.Uniform(m_ph_par1, m_ph_par2);
        break;
      case none:
        phi = m_ph_par1;
        theta = m_th_par1;
        break;
    }

    double pz = ptot * cos(TMath::DegToRad() * theta);
    double pt = ptot * sin(TMath::DegToRad() * theta);
    double px = pt * cos(TMath::DegToRad() * phi);
    double py = pt * sin(TMath::DegToRad() * phi);
    p.setMomentum(px, py, pz);
    double m = p.getMass();
    double e = sqrt(ptot * ptot + m * m);
    p.setEnergy(e);

    //now the vertex:
    double vx = 0;
    double vy = 0;
    double vz = 0;

    switch (m_genVert) {
      case gauss:
        vx = m_grand.Gaus(m_x_par1, m_x_par2);
        vy = m_grand.Gaus(m_y_par1, m_y_par2);
        vz = m_grand.Gaus(m_z_par1, m_z_par2);
        break;
      case uniform:
      default:
        vx = m_grand.Uniform(m_x_par1, m_x_par2);
        vy = m_grand.Uniform(m_y_par1, m_y_par2);
        vz = m_grand.Uniform(m_z_par1, m_z_par2);
        break;
      case none:
        vx = m_x_par1;
        vy = m_y_par1;
        vz = m_z_par1;
        break;
    }
    p.setProductionVertex(vx, vy, vz);
  }// end loop over particles in event

  return true;
}

