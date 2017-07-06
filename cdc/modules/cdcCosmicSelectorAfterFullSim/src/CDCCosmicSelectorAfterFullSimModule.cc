/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcCosmicSelectorAfterFullSim/CDCCosmicSelectorAfterFullSimModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <cdc/dataobjects/CDCSimHit.h>

#include <utility>
#include <iostream>

using namespace std;
using namespace Belle2;

// register module
REG_MODULE(CDCCosmicSelectorAfterFullSim)
CDCCosmicSelectorAfterFullSimModule::CDCCosmicSelectorAfterFullSimModule() : Module()
{
  // Set description
  setDescription("Modify CDCSimHits and MCParticles for cosmics so that the global time to (approximately) zero at y=0 using FullSim output. And select cosmics passing through the user-specified region at y=0.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("xOfRegion", m_xOfRegion, "x of center position of region at y=0 (cm)", 0.);
  addParam("zOfRegion", m_zOfRegion, "z of center position of region at y=0 (cm)", 0.);
  addParam("wOfRegion", m_wOfRegion, "full-width  of region at y=0 (cm)",  10.);
  addParam("lOfRegion", m_lOfRegion, "full-length of region at y=0 (cm)", 100.);
}

void CDCCosmicSelectorAfterFullSimModule::initialize()
{
  StoreArray<MCParticle> mcParticles;
  mcParticles.required();
  StoreArray<CDCSimHit> simHits;
  simHits.required();
}


void CDCCosmicSelectorAfterFullSimModule::event()
{
  // Get SimHit array, MCParticle array
  StoreArray<MCParticle> mcParticles;
  StoreArray<CDCSimHit>  simHits;
  RelationArray mcParticlesToCDCSimHits(mcParticles, simHits);

  bool returnVal = false;

  const double mass = 0.1056583715; //muon mass (GeV)
  const double c = 29.9792458; //light speed (cm/ns)

  int nHits = simHits.getEntries();
  if (nHits == 0) {
    B2DEBUG(250, "No .of CDCSimHits=0 in the current event.");
    //    B2WARNING("No .of CDCSimHits=0 in the current event= " << nHits);
    setReturnValue(returnVal);
    return;
  }

  // Loop over all particles
  int nMCPs = mcParticles.getEntries();
  if (nMCPs == 0) {
    B2ERROR("No. of MCParticles=0 in the current event.");
    setReturnValue(returnVal);
    return;
  }

  for (int iMCP = 0; iMCP < nMCPs; ++iMCP) {
    MCParticle* m_P = mcParticles[iMCP];
    if (abs(m_P->getPDG()) != 13) {
      B2WARNING("Does nothing for non-muon !");
      continue;
    }

    /*
    const TVector3 vertex = m_P->getProductionVertex();
    const double vX0 = vertex.X();
    const double vY0 = vertex.Y();
    const double vZ0 = vertex.Z();
    const TVector3 momentum = m_P->getMomentum();
    const double pX0 = momentum.X();
    const double pY0 = momentum.Y();
    const double pZ0 = momentum.Z();
    const double p = momentum.Mag();
    const double beta = p / sqrt(p * p + mass * mass);

    //calculate an intersection with y=0 plane
    double xi4cry = -999.;
    double yi4cry =    0.;
    double zi4cry = -999.;
    if (pY0 != 0.) {
      xi4cry = (0. - vY0) * (pX0 / pY0) + vX0;
      zi4cry = (0. - vY0) * (pZ0 / pY0) + vZ0;
    } else {
      xi4cry = 0.;
      zi4cry = -vX0 * (pZ0 / pX0) + vZ0;
    }

    const double fl4cry = sqrt((xi4cry - vX0) * (xi4cry - vX0) + (yi4cry - vY0) * (yi4cry - vY0) + (zi4cry - vZ0) *
             (zi4cry - vZ0)); // fl to y=0 plane
    */

    //try to calculate cdc hit with y_up and y_dn
    double y_up = 9999.;
    double y_dn = -9999.;
    double ihit_up = -1;
    double ihit_dn = -1;
    RelationIndex<MCParticle, CDCSimHit> mcp_to_hit(mcParticles, simHits);
    if (!mcp_to_hit) B2FATAL("No MCParticle->CDCSimHit relation found!");
    typedef RelationIndex<MCParticle, CDCSimHit>::Element RelationElement;
    //    std::cout <<" "<< std::endl;

    for (int iHit = 0; iHit < nHits; ++iHit) {
      for (const RelationElement& rel : mcp_to_hit.getElementsTo(simHits[iHit])) {
        //  std::cout <<"iHit,iMCP,rfromindex= " << iHit <<" "<< iMCP <<" "<< rel.from->getIndex()-1 << std::endl;
        if ((rel.from->getIndex() - 1) != iMCP) continue;
        double y = simHits[iHit]->getPosTrack().Y();
        //  std::cout <<"y= " << y << std::endl;
        if (y > 0. && y < y_up) {
          y_up = y;
          ihit_up = iHit;
        }
        if (y < 0. && y > y_dn) {
          y_dn = y;
          ihit_dn = iHit;
        }
      }
    }

    //calculate flight time from y_up to y=0 plane (linear approx.)
    //    std::cout <<"ihit_up= " << ihit_up << std::endl;
    //    std::cout <<"ihit_dn= " << ihit_dn << std::endl;
    if (ihit_up < 0 || ihit_dn < 0) continue;
    const TVector3 pos_up = simHits[ihit_up]->getPosTrack();
    const TVector3 pos_dn = simHits[ihit_dn]->getPosTrack();
    const TVector3 mom_up = simHits[ihit_up]->getMomentum();
    const TVector3 mom_dn = simHits[ihit_dn]->getMomentum();
    const double tof_up   = simHits[ihit_up]->getFlightTime();
    const double tof_dn   = simHits[ihit_dn]->getFlightTime();

    const TVector3 mom_mean = 0.5 * (mom_up + mom_dn);
    const TVector3 dpos = pos_dn - pos_up;
    const double cx = dpos.X() / dpos.Mag();
    const double cy = dpos.Y() / dpos.Mag();
    const double cz = dpos.Z() / dpos.Mag();
    const double vx = pos_up.X();
    const double vy = pos_up.Y();
    const double vz = pos_up.Z();

    const double yi = 0.;
    double xi = 0.;
    double zi = 0.;
    if (cy != 0.) {
      xi = (0. - vy) * (cx / cy) + vx;
      zi = (0. - vy) * (cz / cy) + vz;
    } else {
      zi = -vx * (cz / cx) + vz;
    }

    bool hitRegion = (fabs(xi - m_xOfRegion) < 0.5 * m_wOfRegion && fabs(zi - m_zOfRegion) <  0.5 * m_lOfRegion) ? true : false;

    //    std::cout <<"xi= " << xi << std::endl;
    //    std::cout <<"zi= " << zi << std::endl;

    if (hitRegion) {
      returnVal = true;
      double fl = (xi - vx) * (xi - vx) + (yi - vy) * (yi - vy) + (zi - vz) * (zi - vz);
      fl = sqrt(fl);
      const double beta_mean = mom_mean.Mag() / sqrt(mom_mean.Mag2() + mass * mass);
      const double tofToxyPlane = fl / (c * beta_mean);

      if ((tof_up + tofToxyPlane) > tof_dn) {
        B2WARNING("TOF corr. stragne: " << tof_up << " " << tof_dn << " " << tofToxyPlane);
        continue;
      }

      //      const double deltaT4cry = fl4cry / (c * beta);
      //      double deltaT = tof_up + deltaT4cry + tofToxyPlane;
      const double dT = tof_up + tofToxyPlane;
      //      std::cout <<"deltaT4cry,tof_up,tofToxyPlane,corrt= "<< deltaT4cry <<" "<< tof_up <<" "<< tofToxyPlane <<" "<< deltaT4cry - deltaT << std::endl;
      const double pTime = m_P->getProductionTime();
      m_P->setProductionTime(pTime - dT);

      for (int iHit = 0; iHit < nHits; ++iHit) {
        for (const RelationElement& rel : mcp_to_hit.getElementsTo(simHits[iHit])) {
          if ((rel.from->getIndex() - 1) != iMCP) continue;
          const double oldtof = simHits[iHit]->getFlightTime();
          simHits[iHit]->setFlightTime(oldtof - dT);
          simHits[iHit]->setGlobalTime(oldtof - dT);
        }
      }
    } //end of hitRegion
  } //end of mcp loop

  setReturnValue(returnVal);

}
