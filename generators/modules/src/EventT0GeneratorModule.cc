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

// framework aux
#include <framework/gearbox/Unit.h>
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
    addParam("coreGaussWidth", m_coreGaussWidth, "sigma of core gaussian [ns]", double(5.6));
    addParam("tailGaussWidth", m_tailGaussWidth, "sigma of tail gaussian [ns]", double(14.5));
    addParam("tailGaussFraction", m_tailGaussFraction,
             "fraction (by area) of tail gaussian", double(0.08));
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
    m_simClockState.registerInDataStore();

    if (not std::isnan(m_maximumT0) and not std::isnan(m_fixedT0)) {
      B2ERROR("You can not set both the maximum T0 and the fixed T0 option.");
    }
  }


  void EventT0GeneratorModule::event()
  {

    // generate bucket number w.r.t revo9 marker

    int bucket = m_bunchStructure->generateBucketNumber(); // [RF clock]
    int beamRevo9Cycle = gRandom->Integer(9); // number of beam revolutions w.r.t revo9 marker
    int bucketRevo9 = bucket + beamRevo9Cycle * m_bunchStructure->getRFBucketsPerRevolution(); // [RF clock]

    // generate L1 time jitter

    double timeJitter = 0;
    if (not std::isnan(m_maximumT0)) {
      timeJitter = -m_maximumT0 + (2 * m_maximumT0) * gRandom->Rndm();
    } else if (not std::isnan(m_fixedT0)) {
      timeJitter = m_fixedT0;
    } else {
      double sigma = m_coreGaussWidth;
      if (gRandom->Rndm() < m_tailGaussFraction) sigma = m_tailGaussWidth;
      timeJitter = gRandom->Gaus(0., sigma);
    }

    // calculate revo9count (system clock ticks since revo9 marker as determined by L1 trigger)

    double bucketTimeSep = 1 / m_clockSettings->getAcceleratorRF();
    int relBucketNo = round(timeJitter / bucketTimeSep); // RF clock
    int revo9count = (bucketRevo9 + relBucketNo) / 4; // system clock

    // calculate collision time w.r.t L1 trigger

    double collisionTime = (bucketRevo9 - revo9count * 4) * bucketTimeSep;

    // correct MC particle times according to generated collision time

    for (auto& particle : m_mcParticles) {
      particle.setProductionTime(particle.getProductionTime() + collisionTime);
      particle.setDecayTime(particle.getDecayTime() + collisionTime);
    }

    // store collision time to MC initial particles (e.g. beam particles)

    if (not m_initialParticles.isValid()) m_initialParticles.create();
    m_initialParticles->setTime(collisionTime);

    // store revo9count (modulo range) in order to be distibuted to sub-detectors

    unsigned revo9range = (m_bunchStructure->getRFBucketsPerRevolution() / 4) * 9;
    revo9count %= revo9range;
    if (revo9count < 0) revo9count += revo9range;

    m_simClockState.create();
    m_simClockState->setRevo9Count(revo9count);
    m_simClockState->setBucketNumber(bucket);
    m_simClockState->setBeamCycleNumber(beamRevo9Cycle);

  }


} // end Belle2 namespace

