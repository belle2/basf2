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
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticle.h>

#include <utility>
#include <iostream>

using namespace std;
using namespace Belle2;

// register module
REG_MODULE(CDCCosmicSelector)
CDCCosmicSelectorModule::CDCCosmicSelectorModule() : Module()
{
  // Set description
  setDescription("Select cosmics passing through the trigger counter");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("xOfCounter", m_xOfCounter, "x-position of trigger counter (cm)",  -0.6);
  addParam("yOfCounter", m_yOfCounter, "y-position of trigger counter (cm)", -13.25);
  addParam("zOfCounter", m_zOfCounter, "z-position of trigger counter (cm)",  17.3);
  addParam("wOfCounter", m_wOfCounter, "full-width  of trigger counter (cm)",  7.0);
  addParam("lOfCounter", m_lOfCounter, "full-length of trigger counter (cm)", 12.5);
  addParam("TOF", m_tof, "Tof=1(2): TOF from production point to trigger counter (IP) is subtracted", 1);
  addParam("cryGenerator", m_cryGenerator, "true: CRY generator; false Cosmics generator", true);
}

void CDCCosmicSelectorModule::initialize()
{
  StoreArray<MCParticle> mcParticles;
  mcParticles.required();
}


void CDCCosmicSelectorModule::event()
{
  // Get SimHit array, MCParticle array
  StoreArray<MCParticle> mcParticles;

  // Loop over all particles
  int nMCPs = mcParticles.getEntries();
  B2DEBUG(250, "Number of mcParticles in the current event: " << nMCPs);
  if (nMCPs != 1) B2WARNING("No. of mcparticle != 1 !");

  const double mass = 0.1056583715; //muon mass (GeV)

  /*
  const double    yOfCounter = -16.25 + 3.0;
  const double    wOfCounter =   3.5;
  const double zminOfCounter = -1.5 - 15.5;
  const double zmaxOfCounter = -1.5 + 15.5;
  */

  const double c = 29.9792458; //light speed (cm/ns)

  for (int iMCPs = 0; iMCPs < nMCPs; ++iMCPs) {
    MCParticle* m_P = mcParticles[iMCPs];

    if (abs(m_P->getPDG()) != 13) B2FATAL("Not muon !")

      const TVector3 vertex = m_P->getProductionVertex();
    const double vX = vertex.X();
    const double vY = vertex.Y();
    const double vZ = vertex.Z();

    //    std::cout <<" "<< std::endl;
    //    std::cout <<"vr,vx,vy,yz= "<< sqrt(vX*vX + vY*vY) <<" "<<vX <<" "<< vY <<" "<< vZ << std::endl;

    const TVector3 momentum = m_P->getMomentum();
    const TVector3 zeromom(0., 0., 0.);
    const double pX = momentum.X();
    const double pY = momentum.Y();
    const double pZ = momentum.Z();

    double xi = -999.;
    double yi = -999.;
    double zi = -999.;

    if (pY != 0.) {
      xi = (m_yOfCounter - vY) * (pX / pY) + vX;
      yi = m_yOfCounter;
      zi = (m_yOfCounter - vY) * (pZ / pY) + vZ;
    } else {
      xi = 0.;
      yi = m_yOfCounter;
      zi = -vX * (pZ / pX) + vZ;
    }

    //    std::cout << "xi,yi,zi= " << xi <<" "<< yi <<" "<< zi << std::endl;

    bool hitCounter = (fabs(xi - m_xOfCounter) < 0.5 * m_wOfCounter && fabs(zi - m_zOfCounter) <  0.5 * m_lOfCounter) ? true : false;

    //if hit, re-set the production time to zero at IP
    if (hitCounter) {
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
        if (pY != 0.) {
          xi4cry = (0. - vY) * (pX / pY) + vX;
          zi4cry = (0. - vY) * (pZ / pY) + vZ;
        } else {
          xi4cry = 0.;
          zi4cry = -vX * (pZ / pX) + vZ;
        }

        const double fl4cry = sqrt((xi4cry - vX) * (xi4cry - vX) + (yi4cry - vY) * (yi4cry - vY) + (zi4cry - vZ) *
                                   (zi4cry - vZ)); // fl to y=0 plane

        //reset production time (to the time which old CRY set) once
        pTime += fl4cry / (c * beta);
      }

      const double tofToCounter = fl / (c * beta);
      m_P->setProductionTime(pTime - tofToCounter);
      //      std::cout <<"org,mod= " << pTimeOrg << m_P->getProductionTime() << std::endl;
      //if not hit, reverse the momentum vector so that the particle will not be simulated
      //better to use condition in basf2...
    } else {
      m_P->setMomentum(-1. * momentum);
      //      m_P->setMomentum(zeromom);
    }

  } // end loop over SimHits.

}
