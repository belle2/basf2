/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPCosmicGun/TOPCosmicGunModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <mdst/dataobjects/MCParticle.h>

// ROOT
#include <TRandom3.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCosmicGun)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCosmicGunModule::TOPCosmicGunModule() : Module()

  {
    // set module description
    setDescription("Cosmic ray gun for TOP cosmics tests");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    vector<double> defaultPad;
    addParam("upperPad", m_upperPad, "upper rectangular surface (z,x,Dz,Dx,y) in cm",
             defaultPad);
    addParam("lowerPad", m_lowerPad, "lower rectangular surface (z,x,Dz,Dx,y) in cm",
             defaultPad);
    addParam("startTime", m_startTime, "start time in nsec (time at upperPad)", 0.0);
  }

  TOPCosmicGunModule::~TOPCosmicGunModule()
  {
  }

  void TOPCosmicGunModule::initialize()
  {
    // data store objects registration
    StoreArray<MCParticle>::registerPersistent();

    // parameter checks
    if (m_upperPad.size() != 5) B2FATAL("upperPad not defined, 5 parameters needed");
    if (m_lowerPad.size() != 5) B2FATAL("lowerPad not defined, 5 parameters needed");
    if (m_lowerPad[4] >= m_upperPad[4]) B2FATAL("lowerPad is not below UpperPad");

  }

  void TOPCosmicGunModule::beginRun()
  {
  }

  void TOPCosmicGunModule::event()
  {
    // generate points on upper and lower surfaces
    double z1 = m_upperPad[0] + m_upperPad[2] * (gRandom->Rndm() - 0.5);
    double x1 = m_upperPad[1] + m_upperPad[3] * (gRandom->Rndm() - 0.5);
    double y1 = m_upperPad[4];
    TVector3 r1(x1, y1, z1);
    double z2 = m_lowerPad[0] + m_lowerPad[2] * (gRandom->Rndm() - 0.5);
    double x2 = m_lowerPad[1] + m_lowerPad[3] * (gRandom->Rndm() - 0.5);
    double y2 = m_lowerPad[4];
    TVector3 r2(x2, y2, z2);

    // generate momentum
    double p = 4.0; // yet to be coded!

    // calculate momentum vector
    TVector3 dr = r2 - r1;
    TVector3 momentum = (p / dr.Mag()) * dr;

    // create MCParticles data store
    StoreArray<MCParticle> MCParticles;
    MCParticles.create();

    // store generated muon
    MCParticle* part = MCParticles.appendNew();
    part->setPDG(13);
    part->setMassFromPDG();
    part->setStatus(MCParticle::c_PrimaryParticle);
    part->addStatus(MCParticle::c_StableInGenerator);
    part->setProductionVertex(r1);
    part->setProductionTime(m_startTime);
    part->setMomentum(momentum);
    double mass = part->getMass();
    double energy = sqrt(p * p + mass * mass);
    part->setEnergy(energy);

  }


  void TOPCosmicGunModule::endRun()
  {
  }

  void TOPCosmicGunModule::terminate()
  {
  }


} // end Belle2 namespace

