/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <generators/modules/EventT0GeneratorModule.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// root
#include <TRandom.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(EventT0Generator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  EventT0GeneratorModule::EventT0GeneratorModule() : Module()

  {
    // set module description
    setDescription("Module generates discrete event t0 in ~4ns steps (bunch spacing) "
                   "according to double gaussian distribution and adds it to the "
                   "production and decay times of MCParticles. This means that after "
                   "this module the time origin (t = 0) is set to what L1 trigger "
                   "would give as the collision time.");

    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("coreGaussWidth", m_coreGaussWidth, "sigma of core gaussian [ns]", 10.0);
    addParam("tailGaussWidth", m_tailGaussWidth, "sigma of tail gaussian [ns]", 20.0);
    addParam("tailGaussFraction", m_tailGaussFraction,
             "fraction (by area) of tail gaussian", 0.0);
    addParam("fixedT0",  m_fixedT0,
             "If set, a fixed event t0 is used instead of simulating the bunch timing.", m_fixedT0);
    addParam("maximumT0",  m_maximumT0,
             "If set, randomize between -maximum and maximum.",
             m_maximumT0);

  }


  void EventT0GeneratorModule::initialize()
  {
    m_mcParticles.isRequired();
    m_initialParticles.registerInDataStore();

    // bunch time separation: every second bunch is filled
    m_bunchTimeSep = 2 * 1.96516 * Unit::ns; //TODO: get it from DB (which object?)

    if (not std::isnan(m_maximumT0) and not std::isnan(m_fixedT0)) {
      B2ERROR("You can not set both the maximum T0 and the fixed T0 option.");
    }
  }


  void EventT0GeneratorModule::event()
  {
    double collisionTime = 0.0f;

    if (not std::isnan(m_maximumT0)) {
      collisionTime = -m_maximumT0 + (2 * m_maximumT0) * gRandom->Rndm();
    } else if (not std::isnan(m_fixedT0)) {
      collisionTime = m_fixedT0;
    } else {
      // generate collision time
      double sigma = m_coreGaussWidth;
      if (gRandom->Rndm() < m_tailGaussFraction) sigma = m_tailGaussWidth;

      collisionTime = gRandom->Gaus(0., sigma);
    }
    const int relBunchNo = round(collisionTime / m_bunchTimeSep);
    collisionTime = relBunchNo * m_bunchTimeSep;

    // correct MC particles times according to generated collision time
    for (auto& particle : m_mcParticles) {
      particle.setProductionTime(particle.getProductionTime() + collisionTime);
      particle.setDecayTime(particle.getDecayTime() + collisionTime);
    }

    // store collision time to MC initial particles (e.g. beam particles)
    if (!m_initialParticles.isValid()) m_initialParticles.create();
    m_initialParticles->setTime(collisionTime);

    // t = 0 is from now on the time L1 thinks the collision happened,
    // but in fact it happened at t = collisionTime

  }


} // end Belle2 namespace

