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
                   "would give as the collision time. In case of cosmics, the L1 trigger"
                   "jitter is generated according to a continuos double gaussian distribution");

    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("coreGaussWidth", m_coreGaussWidth, "sigma of core gaussian [ns]", double(6));
    addParam("tailGaussWidth", m_tailGaussWidth, "sigma of tail gaussian [ns]", double(15));
    addParam("tailGaussFraction", m_tailGaussFraction,
             "fraction (by area) of tail gaussian", double(0.1));
    addParam("fixedT0",  m_fixedT0,
             "If set, a fixed event t0 is used instead of simulating the bunch timing.", m_fixedT0);
    addParam("maximumT0",  m_maximumT0,
             "If set, randomize between -maximum and maximum.",
             m_maximumT0);
    addParam("isCosmics", m_isCosmics,
             "if True simulate L1 jitter for cosmics", bool(false));
    addParam("coreGaussWidthCosmics", m_coreGaussWidthCosmics, "sigma of core gaussian for cosmics [ns]", double(5.6));
    addParam("tailGaussWidthCosmics", m_tailGaussWidthCosmics, "sigma of tail gaussian for cosmics [ns]", double(14.5));
    addParam("tailGaussFractionCosmics", m_tailGaussFractionCosmics,
             "fraction (by area) of tail gaussian for cosmics", double(0.08));

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


    double eventTime = 0;
    int bucket = 0;
    int beamRevo9Cycle = 0;
    int relBucketNo = 0;
    //if m_isCosmics we need to randomize revo9count,
    //at least for the SVD trigger bin
    //if !m_isCosmics the revo9Count value is overwritten
    unsigned revo9range = (m_bunchStructure->getRFBucketsPerRevolution() / 4) * 9;
    int revo9count = gRandom->Integer(revo9range);

    if (!m_isCosmics) {
      // generate bucket number w.r.t revo9 marker

      bucket = m_bunchStructure->generateBucketNumber(); // [RF clock]
      beamRevo9Cycle = gRandom->Integer(9); // number of beam revolutions w.r.t revo9 marker
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
      relBucketNo = round(timeJitter / bucketTimeSep); // RF clock
      revo9count = (bucketRevo9 + relBucketNo) / 4; // system clock

      // calculate collision time w.r.t L1 trigger

      eventTime = (bucketRevo9 - revo9count * 4) * bucketTimeSep;

    } else {

      //compute jitter for cosmics, .i.e. no filling pattern
      double sigma = m_coreGaussWidthCosmics;
      if (gRandom->Rndm() < m_tailGaussFractionCosmics) sigma = m_tailGaussWidthCosmics;
      eventTime = gRandom->Gaus(0., sigma);
    }

    // correct MC particle times according to generated collision time

    for (auto& particle : m_mcParticles) {
      particle.setProductionTime(particle.getProductionTime() + eventTime);
      particle.setDecayTime(particle.getDecayTime() + eventTime);
    }

    // store collision time to MC initial particles (e.g. beam particles)

    if (not m_initialParticles.isValid()) m_initialParticles.create();
    m_initialParticles->setTime(eventTime);

    // store revo9count (modulo range) in order to be distibuted to sub-detectors
    revo9count %= revo9range;
    if (revo9count < 0) revo9count += revo9range;

    m_simClockState.create();
    m_simClockState->setRevo9Count(revo9count);
    m_simClockState->setBucketNumber(bucket);
    m_simClockState->setBeamCycleNumber(beamRevo9Cycle);
    m_simClockState->setRelativeBucketNo(relBucketNo);
  }


} // end Belle2 namespace

