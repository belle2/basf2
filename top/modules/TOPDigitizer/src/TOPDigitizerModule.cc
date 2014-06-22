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
#include <mdst/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace TOP {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(TOPDigitizer)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPDigitizerModule::TOPDigitizerModule() : Module(),
      m_topgp(TOPGeometryPar::Instance())
    {
      // Set description()
      setDescription("Digitize TOPSimHits");
      setPropertyFlags(c_ParallelProcessingCertified);

      // Add parameters
      addParam("inputSimHits", m_inputSimHits, "Input collection name (TOPSimHits)",
               string(""));
      addParam("outputDigits", m_outputDigits, "Output collection name (TOPDigits)",
               string(""));
      addParam("timeZeroJitter", m_timeZeroJitter, "r.m.s of T0 jitter [ns]", 25e-3);
      addParam("electronicJitter", m_electronicJitter,
               "r.m.s of electronic jitter [ns]", 50e-3);
      addParam("electronicEfficiency", m_electronicEfficiency,
               "electronic efficiency", 1.0);
      addParam("darkNoise", m_darkNoise,
               "uniformly distributed dark noise (hits per bar)", 0.0);

    }

    TOPDigitizerModule::~TOPDigitizerModule()
    {
    }

    void TOPDigitizerModule::initialize()
    {
      // data store registration
      StoreArray<TOPDigit>::registerPersistent(m_outputDigits);
      RelationArray::registerPersistent<TOPDigit, TOPSimHit>(m_outputDigits, m_inputSimHits);
      RelationArray::registerPersistent<TOPDigit, MCParticle>(m_outputDigits, "");

      StoreArray<TOPSimHit>::required();
      StoreArray<MCParticle>::optional();

      // store electronics jitter and efficiency to make it known for reconstruction
      m_topgp->setELjitter(m_electronicJitter);
      m_topgp->setELefficiency(m_electronicEfficiency);
    }

    void TOPDigitizerModule::beginRun()
    {

    }

    void TOPDigitizerModule::event()
    {

      // input: simulated hits
      StoreArray<TOPSimHit> topSimHits(m_inputSimHits);

      // output: digitized hits
      StoreArray<TOPDigit> topDigits(m_outputDigits);

      m_topgp->setBasfUnits();

      // simulate interaction time relative to RF clock
      double t_beam = gRandom->Gaus(0., m_timeZeroJitter);

      // TDC
      int NTDC = m_topgp->getTDCbits();
      int maxTDC = 1 << NTDC;
      double TDCwidth = m_topgp->getTDCbitwidth();
      double Tmax = maxTDC * TDCwidth;

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

        // add T0 jitter, TTS and electronic jitter to photon time
        double tts = PMT_TTS();
        double tel = gRandom->Gaus(0., m_electronicJitter);
        double time = t_beam + simHit->getTime() + tts + tel;

        // convert to TDC digits
        if (time < 0) continue;
        if (time > Tmax) continue;
        int TDC = int(time / TDCwidth);
        if (TDC > maxTDC) TDC = maxTDC;

        // store result and add relations
        TOPDigit* digit = topDigits.appendNew(simHit->getBarID(), channelID, TDC);
        digit->addRelationTo(simHit);
        const MCParticle* particle = simHit->getRelatedFrom<MCParticle>();
        digit->addRelationTo(particle); //TODO: add relation weight

      }

      // add randomly distributed electronic noise
      if (m_darkNoise <= 0) return;

      int Nbars = m_topgp->getNbars();
      int Nchannels = m_topgp->getNpmtx() * m_topgp->getNpmty() *
                      m_topgp->getNpadx() * m_topgp->getNpady();

      for (int barID = 1; barID < Nbars + 1; barID++) {
        int NoiseHits = gRandom->Poisson(m_darkNoise);
        for (int i = 0; i < NoiseHits; i++) {
          int channelID = int(gRandom->Rndm() * Nchannels) + 1;
          int TDC = int(gRandom->Rndm() * maxTDC);
          topDigits.appendNew(barID, channelID, TDC);
        }
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


  } // end top namespace
} // end Belle2 namespace

