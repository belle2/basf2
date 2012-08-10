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
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

      // Add parameters
      addParam("InputColName", m_inColName, "Input collection name (TOPSimHits)",
               string(""));
      addParam("OutputColName", m_outColName, "Output collection name (TOPDigits)",
               string(""));
      addParam("PhotonFraction", m_photonFraction,
               "Fraction of Cerenkov photons propagated in FullSim.", 0.3);
      addParam("T0jitter", m_T0jitter, "r.m.s of T0 jitter", 25e-3);
      addParam("ELjitter", m_ELjitter, "r.m.s of electronics jitter", 0.0);

    }

    TOPDigitizerModule::~TOPDigitizerModule()
    {
    }

    void TOPDigitizerModule::initialize()
    {
      // Print set parameters
      printModuleParams();

      // data store
      StoreArray<TOPSimHit> topSimHits(m_inColName);
      StoreArray<TOPDigit> topDigits(m_outColName);
      RelationArray relSimHitToDigit(topSimHits, topDigits);
    }

    void TOPDigitizerModule::beginRun()
    {

    }

    void TOPDigitizerModule::event()
    {

      // input: simulated hits
      StoreArray<TOPSimHit> topSimHits(m_inColName);

      // output: digitized hits
      StoreArray<TOPDigit> topDigits(m_outColName);
      topDigits->Clear();
      RelationArray relSimHitToDigit(topSimHits, topDigits);
      relSimHitToDigit.clear();

      m_topgp->setBasfUnits();

      // simulate interaction time relative to RF clock
      double t_beam = gRandom->Gaus(0., m_T0jitter);

      // TDC
      int NTDC = m_topgp->getTDCbits();
      int maxTDC = 1 << NTDC;
      double TDCwidth = m_topgp->getTDCbitwidth();
      double Tmax = maxTDC * TDCwidth;

      int nHits = topSimHits.getEntries();
      for (int i = 0; i < nHits; i++) {
        TOPSimHit* aSimHit = topSimHits[i];

        // Apply quantum efficiency
        double energy = aSimHit->getEnergy();
        if (!DetectorQE(energy)) continue;

        // Do spatial digitization
        double x = aSimHit->getPosition().X();
        double y = aSimHit->getPosition().Y();
        int pmtID = aSimHit->getPmtID();
        int channelID = m_topgp->getChannelID(x, y, pmtID);
        if (channelID == 0) continue;

        // add T0 jitter, TTS and electronic jitter to photon time
        // and convert to TDC digits
        double tts = PMT_TTS();
        double tel = gRandom->Gaus(0., m_ELjitter);
        double time = t_beam + aSimHit->getTime() + tts + tel;
        if (time < 0) continue;
        if (time > Tmax) time = Tmax;
        int TDC = int(time / TDCwidth);
        if (TDC > maxTDC) TDC = maxTDC;

        // store result
        int nentr = topDigits.getEntries();
        new(topDigits->AddrAt(nentr)) TOPDigit(aSimHit->getBarID(), channelID, TDC);

        // make relations
        relSimHitToDigit.add(i, nentr);

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
      cout << "TOPDigi: PhotonFraction=" << m_photonFraction << endl;
      cout << "TOPDigi: T0jitter (rms)=" << m_T0jitter << endl;
      cout << "TOPDigi: ELjitter (rms)=" << m_ELjitter << endl;
    }

    bool TOPDigitizerModule::DetectorQE(double energy)
    {
      double rnd = gRandom->Rndm() * m_photonFraction;
      double eff = m_topgp->QE(energy) * m_topgp->getColEffi();
      return (rnd < eff);
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

