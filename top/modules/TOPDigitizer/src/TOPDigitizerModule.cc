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
#include <top/modules/TOPDigitizer/TimeDigitizer.h>

#include <framework/core/ModuleManager.h>

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

#include <map>

using namespace std;

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
    StoreArray<TOPSimHit> simHits;
    simHits.isRequired();
    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    // output
    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();
    digits.registerRelationTo(simHits);
    digits.registerRelationTo(mcParticles);
    StoreObjPtr<TOPRecBunch> recBunch;
    recBunch.registerInDataStore();

    // store electronics jitter and efficiency to make it known to reconstruction
    m_topgp->setELjitter(m_electronicJitter);
    m_topgp->setELefficiency(m_electronicEfficiency);

    // bunch separation in time
    if (m_trigT0Sigma > 0) {
      GearDir superKEKB("/Detector/SuperKEKB/");
      double circumference = superKEKB.getLength("circumference");
      int numofBunches = superKEKB.getInt("numofBunches");
      m_bunchTimeSep = circumference / Const::speedOfLight / numofBunches;
    }

    // set pile-up and double hit resolution times (needed for BG overlay)
    TOPDigit::setDoubleHitResolution(m_topgp->getDoubleHitResolution());
    TOPDigit::setPileupTime(m_topgp->getPileupTime());

  }

  void TOPDigitizerModule::beginRun()
  {

  }

  void TOPDigitizerModule::event()
  {

    // input: simulated hits
    StoreArray<TOPSimHit> simHits;

    // output: digitized hits
    StoreArray<TOPDigit> digits;

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

    // pixels with time digitizers
    std::map<unsigned, TimeDigitizer> pixels;
    typedef std::map<unsigned, TimeDigitizer>::iterator Iterator;

    // add simulated hits
    for (const auto& simHit : simHits) {
      // simulate electronic efficiency
      if (gRandom->Rndm() > m_electronicEfficiency) continue;

      // Do spatial digitization
      double x = simHit.getX();
      double y = simHit.getY();
      int pmtID = simHit.getPmtID();
      int pixelID = m_topgp->getPixelID(x, y, pmtID);
      if (pixelID == 0) continue;

      // add TTS to photon time and make it relative to start time
      double time = simHit.getTime() + generateTTS() - startTime;

      // add time to digitizer of a given pixel
      TimeDigitizer digitizer(simHit.getBarID(), pixelID);
      unsigned id = digitizer.getUniqueID();
      Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
      it->second.addTimeOfHit(time, &simHit);
    }

    // add randomly distributed dark noise
    if (m_darkNoise > 0) {
      int numBars = m_topgp->getNbars();
      int numPixels = m_topgp->getNpmtx() * m_topgp->getNpmty() *
                      m_topgp->getNpadx() * m_topgp->getNpady();
      double timeMin = m_topgp->getTime(0);
      double timeMax = m_topgp->getTime(m_topgp->TDCoverflow() - 1);
      for (int barID = 1; barID <= numBars; barID++) {
        int numHits = gRandom->Poisson(m_darkNoise);
        for (int i = 0; i < numHits; i++) {
          int pixelID = int(gRandom->Rndm() * numPixels) + 1;
          double time = (timeMax - timeMin) * gRandom->Rndm() + timeMin;
          TimeDigitizer digitizer(barID, pixelID);
          unsigned id = digitizer.getUniqueID();
          Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
          it->second.addTimeOfHit(time);
        }
      }
    }

    // digitize in time
    for (auto& pixel : pixels) {
      pixel.second.digitize(digits, m_electronicJitter);
    }

  }


  void TOPDigitizerModule::endRun()
  {

  }

  void TOPDigitizerModule::terminate()
  {

  }

  double TOPDigitizerModule::generateTTS()
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

