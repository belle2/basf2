/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcCosmicSelector/CDCCosmicSelectorModule.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <utility>
#include <iostream>

using namespace std;
using namespace Belle2;

// register module
REG_MODULE(CDCCosmicSelector)
CDCCosmicSelectorModule::CDCCosmicSelectorModule() : Module()
{
  // Set description
  setDescription("Modify MCParticles for cosmics so that the global time is zero at y=0 assuming a cosmic trajectory is a line. And select cosmics passing through a trigger counter. This module works only for the event with the no. of primary charged MC particles=1. Please place this module after the event-generator and before FullSim.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("xOfCounter", m_xOfCounter, "x-position of trigger counter (cm)",  -0.6);
  addParam("yOfCounter", m_yOfCounter, "y-position of trigger counter (cm)", -13.25);
  addParam("zOfCounter", m_zOfCounter, "z-position of trigger counter (cm)",  17.3);
  addParam("phiOfCounter", m_phiOfCounter, "phi-angle of trigger counter (deg)", 0.);
  addParam("wOfCounter", m_wOfCounter, "full-width  of trigger counter (cm)",  7.0);
  addParam("lOfCounter", m_lOfCounter, "full-length of trigger counter (cm)", 12.5);
  addParam("TOF", m_tof, "Tof=1(2): TOF from production point to trigger counter (IP) is subtracted", 1);
  addParam("TOP", m_top, "TOP from hit point to pmt in trigger counter is subtracted (assuming PMT is put at -z end of counter",
           false);
  addParam("propSpeed", m_propSpeed, "Light prop. speed in counter (cm/ns)", 14.4);
  addParam("cryGenerator", m_cryGenerator, "true: CRY generator; false Cosmics generator", true);
}

void CDCCosmicSelectorModule::initialize()
{
  m_mcParticles.isRequired();
}


void CDCCosmicSelectorModule::event()
{
  bool returnVal = false;

  int nMCPs = m_mcParticles.getEntries();
  if (nMCPs == 0) {
    B2ERROR("No. of MCParticles=0 in the current event.");
    setReturnValue(returnVal);
    return;
  }

  /*
  const double    yOfCounter = -16.25 + 3.0;
  const double    wOfCounter =   3.5;
  const double zminOfCounter = -1.5 - 15.5;
  const double zmaxOfCounter = -1.5 + 15.5;
  */

  // Loop over prim. charged MC particle
  const double c = 29.9792458; //light speed (cm/ns)
  double mass = 0.1056583715; //muon mass (GeV)
  unsigned nPrimChgds = 0;

  for (int iMCPs = 0; iMCPs < nMCPs; ++iMCPs) {
    MCParticle* m_P = m_mcParticles[iMCPs];

    //    B2INFO("isPrimaryParticle= " << m_P->isPrimaryParticle());
    if (!m_P->isPrimaryParticle()) continue;

    unsigned pid = abs(m_P->getPDG());
    if (pid ==   13) {
      ++nPrimChgds;
    } else if (pid ==   11) {
      ++nPrimChgds;
      mass = 0.000510998928;
    } else if (pid ==  211) {
      ++nPrimChgds;
      mass = 0.13957018;
    } else if (pid ==  321) {
      ++nPrimChgds;
      mass = 0.493677;
    } else if (pid == 2212) {
      ++nPrimChgds;
      mass = 0.938272046;
    } else {
      continue;
    }

    //    B2INFO("No .of prim. charged MC particles= " << nPrimChgds);
    if (nPrimChgds > 1) continue;

    const TVector3 vertex = m_P->getProductionVertex();
    const double vX0 = vertex.X();
    const double vY0 = vertex.Y();
    const double cosphi =  cos(m_phiOfCounter * M_PI / 180.);
    const double sinphi =  sin(m_phiOfCounter * M_PI / 180.);
    //(vx,vy) is prod. vertex in frame rotated in phi wrt lab. frame
    const double vX =  cosphi * vX0 + sinphi * vY0;
    const double vY = -sinphi * vX0 + cosphi * vY0;
    const double vZ = vertex.Z();

    //    std::cout <<" "<< std::endl;
    //    std::cout <<"vr,vx,vy,yz= "<< sqrt(vX*vX + vY*vY) <<" "<<vX <<" "<< vY <<" "<< vZ << std::endl;

    const TVector3 momentum = m_P->getMomentum();
    //(px,py) is momentum in frame rotated in phi wrt lab. frame
    const double pX0 = momentum.X();
    const double pY0 = momentum.Y();
    const double pX =  cosphi * pX0 + sinphi * pY0;
    const double pY = -sinphi * pX0 + cosphi * pY0;
    const double pZ = momentum.Z();

    double xi = -999.;
    double yi = -999.;
    double zi = -999.;

    //(x...,y...) is counter pos. in frame rotated in phi wrt lab. frame
    const double xOfCounter =  cosphi * m_xOfCounter + sinphi * m_yOfCounter;
    const double yOfCounter = -sinphi * m_xOfCounter + cosphi * m_yOfCounter;

    if (pY != 0.) {
      xi = (yOfCounter - vY) * (pX / pY) + vX;
      yi =  yOfCounter;
      zi = (yOfCounter - vY) * (pZ / pY) + vZ;
    } else {
      //todo improve the following 3 lines and add check accept or not also in y-direction
      xi = 0.;
      yi = yOfCounter;
      zi = -vX * (pZ / pX) + vZ;
    }

    //    std::cout << "xi,yi,zi= " << xi <<" "<< yi <<" "<< zi << std::endl;

    bool hitCounter = (fabs(xi - xOfCounter) < 0.5 * m_wOfCounter && fabs(zi - m_zOfCounter) <  0.5 * m_lOfCounter) ? true : false;

    //if hit, re-set the production time to zero at IP
    if (hitCounter) {
      //      std::cout <<"checkxiyizi " << cosphi*xi-sinphi*yi<<" "<< sinphi*xi+cosphi *yi <<" "<< zi << std::endl;
      returnVal = true;
      const double p = sqrt(pX * pX + pY * pY + pZ * pZ);
      const double cX = pX / p;
      const double cY = pY / p;

      double fl = 0.;
      if (m_tof == 1) {
        fl = sqrt((xi - vX) * (xi - vX) + (yi - vY) * (yi - vY) + (zi - vZ) * (zi - vZ)); // fl to counter
        //  std::cout <<"fl= " << fl << std::endl;
      } else if (m_tof == 2) {
        fl = -(cX * vX + cY * vY) / (cX * cX + cY * cY); //fl to origin
      } else {
        B2FATAL("invalid tof option !");
      }
      //      if (fl < 0.) B2FATAL("negative tof !");
      //      std::cout << "fl,flp,dif= " << fl <<" "<< flp <<" "<< fl-flp << std::endl;
      //      double dot = pX*(xi - vX) + pY*(yi - vY);
      //      if (dot < 0.) fl *= -1.;

      const double beta = p / sqrt(p * p + mass * mass);
      double pTime = m_P->getProductionTime();
      //      const double pTimeOrg = pTime;

      if (m_cryGenerator) {
        //calculate an intersection with y=0 plane
        double xi4cry = -999.;
        double yi4cry =    0.;
        double zi4cry = -999.;
        if (pY0 != 0.) {
          xi4cry = (0. - vY0) * (pX0 / pY0) + vX0;
          zi4cry = (0. - vY0) * (pZ  / pY0) + vZ;
        } else {
          xi4cry = 0.;
          zi4cry = -vX0 * (pZ / pX0) + vZ;
        }

        const double fl4cry = sqrt((xi4cry - vX0) * (xi4cry - vX0) + (yi4cry - vY0) * (yi4cry - vY0) + (zi4cry - vZ) *
                                   (zi4cry - vZ)); // fl to y=0 plane

        //reset production time (to the time which old CRY set) once
        pTime += fl4cry / (c * beta);
      }

      const double tofToCounter = fl / (c * beta);
      const double topToPMT = m_top ? hypot(xi - xOfCounter, zi - (m_zOfCounter - 0.5 * m_lOfCounter)) / m_propSpeed : 0.;
      /*
      std::cout << "xi          = " << xi << std::endl;
      std::cout << "xOfCounter  = " << xOfCounter << std::endl;
      std::cout << "zi          = " << zi << std::endl;
      std::cout << "m_zOfCounter= " << m_zOfCounter << std::endl;
      std::cout << "m_lOfCounter= " << m_lOfCounter << std::endl;
      std::cout << "m_propSpeed = " << m_propSpeed  << std::endl;
      std::cout << "topToPMT    = " << topToPMT << std::endl;
      */
      m_P->setProductionTime(pTime - tofToCounter - topToPMT);
      //      std::cout <<"org,mod= " << pTimeOrg << m_P->getProductionTime() << std::endl;
      //if not hit, reverse the momentum vector so that the particle will not be simulated
      //better to use condition in basf2...
    } else {
      //      m_P->setMomentum(-1. * momentum);

    }
  } // end loop over MCParticles
  setReturnValue(returnVal);
}
