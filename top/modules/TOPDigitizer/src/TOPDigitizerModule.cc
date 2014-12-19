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
#include <top/modules/TOPDigitizer/TOPDigitizerModule.h>

#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <mdst/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom3.h>

using namespace std;
using namespace boost;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDigitizer)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDigitizerModule::TOPDigitizerModule() : Module(),
    m_bunchTimeSep(0),
    m_topgp(TOPGeometryPar::Instance())
  {
    // Set description()
    setDescription("Digitize TOPSimHits");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("timeZeroJitter", m_timeZeroJitter,
             "r.m.s of T0 jitter [ns]", 25e-3);
    addParam("electronicJitter", m_electronicJitter,
             "r.m.s of electronic jitter [ns]", 50e-3);
    addParam("electronicEfficiency", m_electronicEfficiency,
             "electronic efficiency", 1.0);
    addParam("darkNoise", m_darkNoise,
             "uniformly distributed dark noise (hits per bar)", 0.0);
    addParam("trigT0Sigma", m_trigT0Sigma,
             "trigger T0 resolution [ns]", 0.0);

  }

  TOPDigitizerModule::~TOPDigitizerModule()
  {
  }

  void TOPDigitizerModule::initialize()
  {
    // input

    StoreArray<TOPSimHit> topSimHits;
    topSimHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    // output

    StoreArray<TOPDigit> topDigits;
    topDigits.registerInDataStore();
    topDigits.registerRelationTo(topSimHits);
    topDigits.registerRelationTo(mcParticles);

    StoreObjPtr<TOPRecBunch> recBunch;
    recBunch.registerInDataStore();

    // store electronics jitter and efficiency to make it known to reconstruction

    m_topgp->setELjitter(m_electronicJitter);
    m_topgp->setELefficiency(m_electronicEfficiency);

    // bunch separation in time

    GearDir superKEKB("/Detector/SuperKEKB/");
    double circumference = superKEKB.getLength("circumference");
    int numofBunches = superKEKB.getInt("numofBunches");
    m_bunchTimeSep = circumference / Const::speedOfLight / numofBunches;

  }

  void TOPDigitizerModule::beginRun()
  {

  }

  void TOPDigitizerModule::event()
  {

    // input: simulated hits
    StoreArray<TOPSimHit> topSimHits;

    // output: digitized hits
    StoreArray<TOPDigit> topDigits;

    // output: simulated bunch values
    StoreObjPtr<TOPRecBunch> recBunch;
    if (!recBunch.isValid()) recBunch.create();

    m_topgp->setBasfUnits();

    // simulate trigger T0 accuracy in finding the right bunch crossing

    double trigT0 = 0;
    if (m_trigT0Sigma > 0) {
      trigT0 = gRandom->Gaus(0., m_trigT0Sigma);
      int relBunchNo = round(trigT0 / m_bunchTimeSep);
      trigT0 = relBunchNo * m_bunchTimeSep;
      recBunch->setSimulated(relBunchNo, trigT0);
    }

    // simulate start time (bunch time given by trigger smeared according to T0 jitter)
    double startTime = gRandom->Gaus(trigT0, m_timeZeroJitter);

    // TDC
    int overflow = m_topgp->TDCoverflow();

    int nHits = topSimHits.getEntries();
    for (int iHit = 0; iHit < nHits; iHit++) {

      // simulate electronic efficiency
      if (gRandom->Rndm() > m_electronicEfficiency) continue;

      // take simulated hit
      const TOPSimHit* simHit = topSimHits[iHit];

      // Do spatial digitization
      double x = simHit->getX();
      double y = simHit->getY();
      int pmtID = simHit->getPmtID();
      int channelID = m_topgp->getChannelID(x, y, pmtID);
      if (channelID == 0) continue;

      // add TTS and electronic jitter to photon time and make it relative to start time
      double tts = PMT_TTS();
      double tel = gRandom->Gaus(0., m_electronicJitter);
      double time = simHit->getTime() + tts + tel - startTime;

      // convert to TDC count
      int TDC = m_topgp->getTDCcount(time);
      if (TDC == overflow) continue;

      // store result and add relations
      TOPDigit* digit = topDigits.appendNew(simHit->getBarID(), channelID, TDC);
      digit->addRelationTo(simHit);
      RelationVector<MCParticle> particles = simHit->getRelationsFrom<MCParticle>();
      for (unsigned k = 0; k < particles.size(); ++k) {
        digit->addRelationTo(particles[k], particles.weight(k));
      }

    }

    // add randomly distributed electronic noise

    if (m_darkNoise > 0) {
      int Nbars = m_topgp->getNbars();
      int Nchannels = m_topgp->getNpmtx() * m_topgp->getNpmty() *
                      m_topgp->getNpadx() * m_topgp->getNpady();

      for (int barID = 1; barID < Nbars + 1; barID++) {
        int NoiseHits = gRandom->Poisson(m_darkNoise);
        for (int i = 0; i < NoiseHits; i++) {
          int channelID = int(gRandom->Rndm() * Nchannels) + 1;
          int TDC = int(gRandom->Rndm() * overflow);
          topDigits.appendNew(barID, channelID, TDC);
        }
      }
    }

    // set hardware channel ID

    for (auto & digit : topDigits) {
      unsigned chan = m_topgp->getHardwareChannelID(digit.getChannelID());
      digit.setHardwareChannelID(chan);
    }

  }


  void TOPDigitizerModule::endRun()
  {

  }

  void TOPDigitizerModule::terminate()
  {

  }

  void TOPDigitizerModule::printModuleParams() const
  {
  }

  double TOPDigitizerModule::PMT_TTS()
  {
    double prob = gRandom->Rndm();
    double s = 0;
    for (int i = 0; i < m_topgp->getNgaussTTS(); i++) {
      s = s + m_topgp->getTTSfrac(i);
      if (prob < s) {
        return gRandom->Gaus(m_topgp->getTTSmean(i), m_topgp->getTTSsigma(i));
      }
    }
    return 0;
  }


} // end Belle2 namespace

