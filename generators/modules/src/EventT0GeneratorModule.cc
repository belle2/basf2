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

// dataobjects
#include <mdst/dataobjects/MCParticle.h>

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
    // set module description (e.g. insert text)
    setDescription("Simulation of event T0 given by L1 trigger: "
                   "t = 0 is defined by L1 trigger instead of interaction itself");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("coreGaussWidth", m_coreGaussWidth, "sigma of core gaussian [ns]", 10.0);
    addParam("tailGaussWidth", m_tailGaussWidth, "sigma of tail gaussian [ns]", 20.0);
    addParam("tailGaussFraction", m_tailGaussFraction,
             "fraction (by area) of tail gaussian", 0.0);

  }

  EventT0GeneratorModule::~EventT0GeneratorModule()
  {
  }

  void EventT0GeneratorModule::initialize()
  {
    StoreArray<MCParticle> mcParticles;
    mcParticles.isRequired();

    // bunch time separation: every second bunch is filled
    m_bunchTimeSep = 2 * 1.96516 * Unit::ns; //TODO: get it from DB (which object?)


  }

  void EventT0GeneratorModule::beginRun()
  {
  }

  void EventT0GeneratorModule::event()
  {

    double sigma = m_coreGaussWidth;
    if (gRandom->Rndm() < m_tailGaussFraction) sigma = m_tailGaussWidth;

    int relBunchNo = round(gRandom->Gaus(0., sigma) / m_bunchTimeSep);
    double trueT0 = relBunchNo * m_bunchTimeSep; // time at which interaction happens

    StoreArray<MCParticle> mcParticles;
    for (auto& particle : mcParticles) {
      particle.setProductionTime(particle.getProductionTime() + trueT0);
      particle.setDecayTime(particle.getDecayTime() + trueT0);
    }

    // t = 0 is from now on the time L1 thinks the interaction happened,
    // but in fact it happened at t = trueT0

  }


  void EventT0GeneratorModule::endRun()
  {
  }

  void EventT0GeneratorModule::terminate()
  {
  }


} // end Belle2 namespace

