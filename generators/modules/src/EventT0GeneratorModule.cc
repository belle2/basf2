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
                   "would give as the interaction time.");

    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("coreGaussWidth", m_coreGaussWidth, "sigma of core gaussian [ns]", 10.0);
    addParam("tailGaussWidth", m_tailGaussWidth, "sigma of tail gaussian [ns]", 20.0);
    addParam("tailGaussFraction", m_tailGaussFraction,
             "fraction (by area) of tail gaussian", 0.0);

  }


  void EventT0GeneratorModule::initialize()
  {
    m_mcParticles.isRequired();

    // bunch time separation: every second bunch is filled
    m_bunchTimeSep = 2 * 1.96516 * Unit::ns; //TODO: get it from DB (which object?)

  }


  void EventT0GeneratorModule::event()
  {

    double sigma = m_coreGaussWidth;
    if (gRandom->Rndm() < m_tailGaussFraction) sigma = m_tailGaussWidth;

    int relBunchNo = round(gRandom->Gaus(0., sigma) / m_bunchTimeSep);
    double trueT0 = relBunchNo * m_bunchTimeSep; // time at which interaction happens

    for (auto& particle : m_mcParticles) {
      particle.setProductionTime(particle.getProductionTime() + trueT0);
      particle.setDecayTime(particle.getDecayTime() + trueT0);
    }

    // t = 0 is from now on the time L1 thinks the interaction happened,
    // but in fact it happened at t = trueT0

  }


} // end Belle2 namespace

