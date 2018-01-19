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
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <utility>
#include <iostream>

using namespace std;
using namespace Belle2;

// register module
REG_MODULE(CDCCosmicSelectorAfterFullSim)
CDCCosmicSelectorAfterFullSimModule::CDCCosmicSelectorAfterFullSimModule() : Module()
{
  // Set description
  setDescription("Modify CDCSimHits and MCParticles for cosmics so that the global time is (approximately) zero at y=0 using FullSim output (=CDCSimHits). And select cosmics passing through a user-specified region at y=0. This module works only for the event with the no. of primary charged MC particles=1. Please place this module after FullSim and before CDCDigitizer. This module is not needed for normal MC; only needed when you want to evaluate performance of T0 extraction module.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("xOfRegion", m_xOfRegion, "x of center position of region at y=0 (cm)", 0.);
  addParam("zOfRegion", m_zOfRegion, "z of center position of region at y=0 (cm)", 0.);
  addParam("wOfRegion", m_wOfRegion, "full-width  (x-direction) of region at y=0 (cm)",  10.);
  addParam("lOfRegion", m_lOfRegion, "full-length (z-direction) of region at y=0 (cm)", 100.);
}

void CDCCosmicSelectorAfterFullSimModule::initialize()
{
  m_mcParticles.isRequired();
  m_simHits.isRequired();
}


void CDCCosmicSelectorAfterFullSimModule::event()
{
  // Get relation array
  RelationArray mcParticlesToCDCSimHits(m_mcParticles, m_simHits);

  bool returnVal = false;

  //  const double c = 29.9792458; //light speed (cm/ns)
  //  double mass = 0.1056583715;

  int nHits = m_simHits.getEntries();
  if (nHits <= 1) {
    B2DEBUG(250, "No .of CDCSimHits <= 1 in the current event.");
    //    B2WARNING("No .of CDCSimHits=0 in the current event= " << nHits);
    setReturnValue(returnVal);
    return;
  }

  int nMCPs = m_mcParticles.getEntries();
  if (nMCPs == 0) {
    B2ERROR("No. of MCParticles=0 in the current event.");
    setReturnValue(returnVal);
    return;
  }

  // Loop over prim. charged MC particle
  unsigned nPrimChgds = 0;
  for (int iMCP = 0; iMCP < nMCPs; ++iMCP) {
    MCParticle* m_P = m_mcParticles[iMCP];

    //    B2INFO("isPrimaryParticle= " << m_P->isPrimaryParticle());
    if (!m_P->isPrimaryParticle()) continue;

    unsigned pid = abs(m_P->getPDG());
    if (pid ==   13) {
      ++nPrimChgds;
    } else if (pid ==   11) {
      ++nPrimChgds;
      //      mass = 0.000510998928;
    } else if (pid ==  211) {
      ++nPrimChgds;
      //      mass = 0.13957018;
    } else if (pid ==  321) {
      ++nPrimChgds;
      //      mass = 0.493677;
    } else if (pid == 2212) {
      ++nPrimChgds;
      //      mass = 0.938272046;
    } else {
      continue;
    }

    //    B2INFO("No .of prim. charged MC particles= " << nPrimChgds);
    if (nPrimChgds > 1) continue;
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

    //try to calculate cdc hit with tof_up and tof_dn
    double tof_up = -9999.;
    double tof_dn =  9999.;
    double ihit_up = -1;
    double ihit_dn = -1;
    RelationIndex<MCParticle, CDCSimHit> mcp_to_hit(m_mcParticles, m_simHits);
    if (!mcp_to_hit) B2FATAL("No MCParticle->CDCSimHit relation found!");
    typedef RelationIndex<MCParticle, CDCSimHit>::Element RelationElement;
    //    std::cout <<" "<< std::endl;

    unsigned ntry = 0;
    double   yold    = 0.;
    double   tofold  = 0.;
    unsigned iHitold = 0 ;
    bool crossfind = false;

    for (int iHit = 0; iHit < nHits; ++iHit) {
      if (crossfind) break;
      for (const RelationElement& rel : mcp_to_hit.getElementsTo(m_simHits[iHit])) {
        //  std::cout <<"iHit,iMCP,rfromindex= " << iHit <<" "<< iMCP <<" "<< rel.from->getIndex()-1 << std::endl;
        if ((rel.from->getIndex() - 1) != iMCP) continue;
        //  std::cout << "weight,pdginsimhit= " << rel.weight <<" "<< simHits[iHit]->getPDGCode() << std::endl;
        if (rel.weight < 0.) continue;  //reject 2ndary particle
        const double y   = m_simHits[iHit]->getPosTrack().Y();
        const double tof = m_simHits[iHit]->getFlightTime();
        //        const double py  = simHits[iHit]->getMomentum().Y();

        ++ntry;

        if (ntry == 1) {
          yold    = y;
          tofold  = tof;
          iHitold = iHit;
        } else {
          //    std::cout <<"yold,y= " << yold<<" "<< y << std::endl;
          if (y * yold >= 0.) {
            yold    = y;
            tofold  = tof;
            iHitold = iHit;
          } else {
            tof_up = tofold;
            ihit_up = iHitold;
            tof_dn = tof;
            ihit_dn = iHit;
            crossfind = true;
            break;
          }
        }
        /*
          if (y > 0. && py < 0. && tof > tof_up) {
                tof_up = tof;
                ihit_up = iHit;
          }
          if (y < 0. && py < 0. && tof < tof_dn) {
                tof_dn = tof;
                ihit_dn = iHit;
          }
        */
      }
    }

    //calculate flight time from y_up to y=0 plane (linear approx.)
    //    std::cout <<"ihit_up,dn= " << ihit_up <<" "<< ihit_dn << std::endl;
    if (ihit_up < 0 || ihit_dn < 0) continue;
    const TVector3 pos_up = m_simHits[ihit_up]->getPosTrack();
    const TVector3 pos_dn = m_simHits[ihit_dn]->getPosTrack();
    const TVector3 mom_up = m_simHits[ihit_up]->getMomentum();
    const TVector3 mom_dn = m_simHits[ihit_dn]->getMomentum();
    if (tof_up > tof_dn) B2WARNING("tof_up > tof_dn " << tof_up << " " << tof_dn);
    //    std::cout <<"tof_up,dn= " << tof_up <<" "<< tof_dn << std::endl;

    //    const TVector3 mom_mean = 0.5 * (mom_up + mom_dn);
    const TVector3 dpos = pos_dn - pos_up;
    double cx = dpos.X() / dpos.Mag();
    double cy = dpos.Y() / dpos.Mag();
    double cz = dpos.Z() / dpos.Mag();
    double vx = pos_up.X();
    double vy = pos_up.Y();
    double vz = pos_up.Z();
    if (tof_up > tof_dn) {
      cx *= -1.;
      cy *= -1.;
      cz *= -1.;
      vx = pos_dn.X();
      vy = pos_dn.Y();
      vz = pos_dn.Z();
    }

    const double yi = 0.;
    double xi = 0.;
    double zi = 0.;
    if (cy != 0.) {
      xi = (yi - vy) * (cx / cy) + vx;
      zi = (yi - vy) * (cz / cy) + vz;
    } else {
      zi = -vx * (cz / cx) + vz;
    }

    bool hitRegion = (fabs(xi - m_xOfRegion) < 0.5 * m_wOfRegion && fabs(zi - m_zOfRegion) <  0.5 * m_lOfRegion) ? true : false;

    //    std::cout <<"xi,zi= " << xi <<" "<< zi << std::endl;

    if (hitRegion) {
      returnVal = true;
      double fl = (xi - vx) * (xi - vx) + (yi - vy) * (yi - vy) + (zi - vz) * (zi - vz);
      fl = sqrt(fl);
      //      const double beta_mean = mom_mean.Mag() / sqrt(mom_mean.Mag2() + mass * mass);
      //      const double tofToxyPlane = fl / (c * beta_mean);

      //      if ((tof_up + tofToxyPlane) > tof_dn) {
      //        B2WARNING("TOF corr. stragne: " << tof_up << " " << tof_dn << " " << tofToxyPlane);
      //      continue;
      //      }

      //      const double deltaT4cry = fl4cry / (c * beta);
      //      double deltaT = tof_up + deltaT4cry + tofToxyPlane;
      //      const double dT = tof_up + tofToxyPlane;
      double               dT = tof_up + (tof_dn - tof_up) * fl / dpos.Mag();
      if (tof_up > tof_dn) dT = tof_dn + (tof_up - tof_dn) * fl / dpos.Mag();

      //      const double dT = tof_up + (tof_dn - tof_up) * fl / dpos.Mag();
      //      std::cout <<"deltaT4cry,tof_up,tofToxyPlane,corrt= "<< deltaT4cry <<" "<< tof_up <<" "<< tofToxyPlane <<" "<< deltaT4cry - deltaT << std::endl;
      const double pTime = m_P->getProductionTime();
      m_P->setProductionTime(pTime - dT);

      for (int iHit = 0; iHit < nHits; ++iHit) {
        //  for (const RelationElement& rel : mcp_to_hit.getElementsTo(simHits[iHit])) {
        //        if ((rel.from->getIndex() - 1) != iMCP) continue;
        const double oldgtime = m_simHits[iHit]->getGlobalTime();
        m_simHits[iHit]->setFlightTime(oldgtime - dT);
        m_simHits[iHit]->setGlobalTime(oldgtime - dT);
        //        }
      }
    } //end of hitRegion
  } //end of mcp loop

  setReturnValue(returnVal);

}
