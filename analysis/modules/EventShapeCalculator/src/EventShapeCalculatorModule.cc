/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <analysis/modules/EventShapeCalculator/EventShapeCalculatorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventShapeContainer.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>

#include <framework/logging/Logger.h>

#include <iostream>

#include <analysis/ContinuumSuppression/Thrust.h>
#include <analysis/ContinuumSuppression/MultipoleMoments.h>
#include <analysis/ContinuumSuppression/CleoCones.h>
#include <analysis/ContinuumSuppression/FoxWolfram.h>

#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>
#include <TVectorD.h>
#include <TVector3.h>
#include <TLorentzVector.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventShapeCalculator)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventShapeCalculatorModule::EventShapeCalculatorModule() : Module()
{
  // Set module properties
  setDescription("Module to compute event shape attributes starting from particlelists. The core algorithms are not implemented in this module, but in dedicated basf2 classes.");
  // Parameter definitions
  addParam("particleListNames", m_particleListNames, "List of the ParticleLists to be used for the calculation of the EventShapes.",
           vector<string>());
  addParam("enableThrust", m_enableThrust, "Enables the calculation of thust-related quantities.", true);
  addParam("enableCollisionAxis", m_enableCollisionAxis, "Enables the calculation of the  quantities related to the collision axis.",
           true);
  addParam("enableFoxWolfram", m_enableFW, "Enables the calculation of the Fox-Wolfram moments.", true);
  addParam("enableMultipoleMoments", m_enableMultipoleMoments, "Enables the calculation of the Multipole moments.", true);
  addParam("enableJets", m_enableJets, "Enables the calculation of jet-related quantities.", true);
  addParam("enableSphericity", m_enableSphericity, "Enables the calculation of the sphericity-related quantities.", true);
  addParam("enableCleoCones", m_enableCleoCones, "Enables the calculation of the CLEO cones.", true);
  addParam("enableAllMoments", m_enableAllMoments, "Enables the calculation of FW and multipole moments from 5 to 8", false);
}


void EventShapeCalculatorModule::initialize()
{
  StoreObjPtr<EventShapeContainer> evtShapeContainer;
  evtShapeContainer.registerInDataStore();
}


void EventShapeCalculatorModule::event()
{

  PCmsLabTransform T;
  double sqrtS = T.getCMSEnergy();

  StoreObjPtr<EventShapeContainer> eventShapeContainer;
  if (!eventShapeContainer) eventShapeContainer.create();

  parseParticleLists(m_particleListNames);


  // --------------------
  // Calculates the FW moments
  // --------------------
  if (m_enableFW) {
    FoxWolfram fw(m_p3List);
    if (m_enableAllMoments) {
      fw.calculateAllMoments();
      for (short i = 0; i < 9; i++) {
        eventShapeContainer->setFWMoment(i, fw.getH(i));
      }
    } else {
      fw.calculateBasicMoments();
      for (short i = 0; i < 5; i++) {
        eventShapeContainer->setFWMoment(i, fw.getH(i));
      }
    }
  }

  // --------------------
  // Calculates the sphericity quantities
  // --------------------
  if (m_enableSphericity) {
    SphericityEigenvalues Sph(m_p3List);
    Sph.calculateEigenvalues();
    for (short i = 0; i < 3; i++) {
      eventShapeContainer->setSphericityEigenvalue(i, Sph.getEigenvalue(i));
      eventShapeContainer->setSphericityEigenvector(i, Sph.getEigenvector(i));
    }
  }


  // --------------------
  // Calculates thrust and thrust-related quantities
  // --------------------
  if (m_enableThrust) {
    TVector3 thrust = Thrust::calculateThrust(m_p3List);
    float thrustness = thrust.Mag();
    thrust = (1. / thrustness) * thrust;
    eventShapeContainer->setThrustAxis(thrust);
    eventShapeContainer->setThrustness(thrustness);

    // --- If required, calculates the MultipoleMoments ---
    if (m_enableMultipoleMoments) {
      MultipoleMoments MM(m_p3List, thrust);
      if (m_enableAllMoments) {
        MM.calculateAllMoments();
        for (short i = 0; i < 9; i++) {
          auto moment = MM.getMoment(i, sqrtS);
          eventShapeContainer->setMultipoleMomentThrust(i, moment);
        }
      } else {
        MM.calculateBasicMoments();
        for (short i = 0; i < 5; i++) {
          auto moment = MM.getMoment(i, sqrtS);
          eventShapeContainer->setMultipoleMomentThrust(i, moment);
        }
      }
    }

    // --- If required, calculates the cleo cones w/ respect to the thrust axis ---
    if (m_enableCleoCones) {
      // Cleo cone class constructor. Unfortunately this class is designed
      // to use the ROE, so the constructor takes two std::vector of momenta ("all" and "ROE"),
      // then a vector to be used as axis, and finally two flags that determine if the cleo cones
      // are calculated using the ROE, all the particles or both. Here we use the m_p3List as dummy
      // list of the ROE momenta, that is however not used at all since the calculate only the
      // cones with all the particles. This whole class would need some heavy restructuring...
      CleoCones cleoCones(m_p3List, m_p3List, thrust, true, false);
      std::vector<float> cones;
      cones = cleoCones.cleo_cone_with_all();
      for (short i = 0; i < 10; i++) {
        eventShapeContainer->setCleoConeThrust(i, cones[i]);
      }
    } // end of if m_enableCleoCones


    // --- If required, calculates the jet 4-momentum using the thrust axis ---
    if (m_enableJets) {
      TLorentzVector p4FWD(0., 0., 0., 0.);
      TLorentzVector p4BKW(0., 0., 0., 0.);
      for (auto p4 : m_p4List) {
        if (p4.Vect().Dot(thrust) > 0)
          p4FWD += p4;
        else
          p4BKW += p4;
      }
      eventShapeContainer->setForwardEmisphere4Momentum(p4FWD);
      eventShapeContainer->setBackwardEmisphere4Momentum(p4BKW);
    } // end of if m_enableJets
  }// end of if m_enableThrust



  // --------------------
  // Calculates the collision axis quantities
  // --------------------
  if (m_enableCollisionAxis) {

    TVector3 collisionAxis(0., 0., 1.);

    // --- If required, calculates the cleo cones w/ respect to the collision axis ---
    if (m_enableCleoCones) {
      CleoCones cleoCones(m_p3List, m_p3List, collisionAxis, true, false);
      std::vector<float> cones;
      cones = cleoCones.cleo_cone_with_all();
      for (short i = 0; i < 10; i++) {
        eventShapeContainer->setCleoConeCollision(i, cones[i]);
      }
    }

    // --- If required, calculates the MultipoleMoments ---
    if (m_enableMultipoleMoments) {
      MultipoleMoments MM(m_p3List, collisionAxis);
      if (m_enableAllMoments) {
        MM.calculateAllMoments();
        for (short i = 0; i < 9; i++) {
          auto moment = MM.getMoment(i, sqrtS);
          eventShapeContainer->setMultipoleMomentCollision(i, moment);
        }
      } else {
        MM.calculateBasicMoments();
        for (short i = 0; i < 5; i++) {
          auto moment = MM.getMoment(i, sqrtS);
          eventShapeContainer->setMultipoleMomentCollision(i, moment);
        }
      }
    } // end of m_enableMultipoleMoments
  } // end of m_enableCollisionAxis
} // end of event()



int EventShapeCalculatorModule::parseParticleLists(vector<string> particleListNames)
{
  int nPart = 0; // number of particles

  PCmsLabTransform T;
  m_p4List.clear();

  unsigned short nParticleLists = particleListNames.size();
  if (nParticleLists == 0)
    B2ERROR("No particle lists found. EventShape calculation not performed.");

  // Loops over the number of particle lists
  for (unsigned short iList = 0; iList < nParticleLists; iList++) {
    string particleListName = particleListNames[iList];
    StoreObjPtr<ParticleList> particleList(particleListName);

    // Loops over the number of particles in the list
    for (unsigned int iPart = 0; iPart < particleList->getListSize(); iPart++) {
      const Particle* part = particleList->getParticle(iPart);
      TLorentzVector p4CMS = T.rotateLabToCms() * part->get4Vector();

      // it need to fill an std::vector of TVector3 to use the current FW rutines.
      // It will hopefully change in release 3
      m_p4List.push_back(p4CMS);
      m_p3List.push_back(p4CMS.Vect());
    }
  }
  return nPart;
}

