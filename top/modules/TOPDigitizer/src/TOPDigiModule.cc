/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <top/modules/TOPDigitizer/TOPDigiModule.h>

#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPDigiHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

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

    REG_MODULE(TOPDigi)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPDigiModule::TOPDigiModule() : Module(),
      m_topgp(TOPGeometryPar::Instance())
    {
      // Set description()
      setDescription("TOPDigitizer");

      // Add parameters
      //      addParam("InputColName", m_inColName, "Input collection name",
      //         string("TOPSimHitArray"));
      //      addParam("OutputColName", m_outColName, "Output collection name",
      //         string("TOPDigiHitArray"));
      addParam("PhotonFraction", m_photonFraction,
               "The fraction of Cerenkov photons propagated in FullSim.", 1.0);

    }

    TOPDigiModule::~TOPDigiModule()
    {
    }

    void TOPDigiModule::initialize()
    {
      // Initialize variables
      m_nRun    = 0 ;
      m_nEvent  = 0 ;

      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      StoreArray<TOPSimHit> topSimHits;
      StoreArray<TOPDigiHit> topDigiHits;
      RelationArray relSimHitToDigiHit(topSimHits, topDigiHits);
    }

    void TOPDigiModule::beginRun()
    {
      // Print run number
      B2INFO("TOPDigi: Processing run: " << m_nRun);

    }

    void TOPDigiModule::event()
    {

      StoreArray<TOPSimHit> topSimHits;
      if (!topSimHits) {
        B2ERROR("TOPDigi: Cannot find TOPSimHit data store.");
        return;
      }
      StoreArray<TOPDigiHit> topDigiHits;
      RelationArray relSimHitToDigiHit(topSimHits, topDigiHits);

      m_topgp->setBasfUnits();

      //! simulate interaction time relative to RF clock
      double sig_beam = 25.e-3;
      double t_beam = gRandom->Gaus(0., sig_beam);

      //! TDC
      int NTDC = m_topgp->getTDCbits();
      int maxTDC = 1 << NTDC;
      double TDCwidth = m_topgp->getTDCbitwidth();

      int nHits = topSimHits->GetEntries();
      for (int i = 0; i < nHits; i++) {
        TOPSimHit* aSimHit = topSimHits[i];

        //! Apply quantum efficiency
        double energy = aSimHit->getEnergy();
        if (!DetectorQE(energy)) continue;

        //! Do spatial digitization
        double x = aSimHit->getPosition().X();
        double y = aSimHit->getPosition().Y();
        int pmtID = aSimHit->getModuleID();
        int channelID = m_topgp->getChannelID(x, y, pmtID);
        if (channelID == 0) continue;

        //! add T0 jitter and TTS to the photon time, and convert to TDC digits
        double tts = PMT_TTS();
        double time = t_beam + aSimHit->getTime() + tts;
        if (time < 0) continue;
        int TDC = int(time / TDCwidth);
        if (TDC > maxTDC) TDC = maxTDC;

        //! store result
        int nentr = topDigiHits->GetEntries();
        new(topDigiHits->AddrAt(nentr)) TOPDigiHit(aSimHit->getBarID(),
                                                   channelID, TDC, energy,
                                                   aSimHit->getParentID(),
                                                   aSimHit->getTrackID());

        //! make relations
        relSimHitToDigiHit.add(i, nentr);

      }

      m_nEvent++;
      B2INFO("nHits: " << nHits << " digitized hits: " << topDigiHits->GetEntries());

    }

    void TOPDigiModule::endRun()
    {
      m_nRun++;
    }

    void TOPDigiModule::terminate()
    {
      // CPU time end
      m_timeCPU = clock() * Unit::us - m_timeCPU;

      // Announce
      B2INFO("TOPDigi finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");

    }

    void TOPDigiModule::printModuleParams() const
    {
      cout << "TOPDigi: PhotonFraction=" << m_photonFraction << endl;
    }

    bool TOPDigiModule::DetectorQE(double energy)
    {
      double rnd = gRandom->Rndm() * m_photonFraction;
      double eff = m_topgp->QE(energy) * m_topgp->ColEffi();
      return (rnd < eff);
    }

    double TOPDigiModule::PMT_TTS()
    {
      double prob = gRandom->Rndm();
      double s = 0;
      for (int i = 0; i < m_topgp->NgaussTTS(); i++) {
        s = s + m_topgp->TTSfrac(i);
        if (prob < s) {
          return gRandom->Gaus(m_topgp->TTSmean(i), m_topgp->TTSsigma(i));
        }
      }
      return 0;
    }


  } // end top namespace
} // end Belle2 namespace
