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

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector2.h>

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
      m_random(new TRandom1(0)), m_topgp(TOPGeometryPar::Instance())
    {
      // Set description()
      setDescription("TOPDigitizer");

      // Add parameters
      addParam("InputColName", m_inColName, "Input collection name", string("TOPSimHitArray"));
      addParam("OutputColName", m_outColName, "Output collection name", string("TOPDigiHitArray"));
    }

    TOPDigiModule::~TOPDigiModule()
    {
      if (m_random) delete m_random;
      if (m_topgp) delete m_topgp;
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
    }

    void TOPDigiModule::beginRun()
    {
      // Print run number
      B2INFO("TOPDigi: Processing run: " << m_nRun);

    }

    void TOPDigiModule::event()
    {

      //! Get the collection of TOPSimHits from the Data store.

      StoreArray<TOPSimHit> topSimHits;
      if (!topSimHits) B2ERROR("TOPDigitizerModule: Cannot find TOPSimHit array.");

      //! Get the collection of TOPDigiHits from the Data store,

      StoreArray<TOPDigiHit> topDigiHits;
      if (!topDigiHits) B2ERROR("TOPDigitizerModule: Cannot find TOPDigiHits array.");



      //! Get number of hits in this event
      int nHits = topSimHits->GetLast();

      //! count the numebr of digitised photons
      int dighit = 0;

      for (int i = 0; i < nHits; i++) {
        //! Get a simhit
        TOPSimHit* aSimHit = topSimHits[i];

        double energy = aSimHit->getEnergy();

        //! Apply quantum efficiency
        if (!DetectorQE(energy)) continue;

        //! Get local position
        TVector2 locpos(aSimHit->getPosition().X(), aSimHit->getPosition().Y());

        //! Convert local position to channel number - spatial digitisation
        int moduleID = aSimHit->getModuleID();
        int channelID = m_topgp->getChannelID(locpos, moduleID);

        if (channelID < 0) continue;

        //! get global time
        double globaltime = aSimHit->getTime();

        //! simulate interaction time relative to RF clock
        double sig_beam = 0.;
        double t_beam = Gaus(0., sig_beam);

        //! simulate TTS
        double t_tts = PMT_TTS();

        //! obtain actual time
        double time = t_beam + globaltime + t_tts;
        if (time < 0) continue;

        //convert time to TDC
        int NTDC = m_topgp->getTDCbits();
        double TDCwidth = m_topgp->getTDCbitwidth();

        int TDC = (int)(time / TDCwidth);
        int maxTDC = (int)pow(2., (double)NTDC);

        //! If TDC is in overflow correct to max
        if (TDC > maxTDC) {
          TDC = maxTDC;
        }

        int nentr = topDigiHits->GetEntries();
        new(topDigiHits->AddrAt(nentr)) TOPDigiHit(aSimHit->getBarID(), channelID, TDC, energy, aSimHit->getParentID(), aSimHit->getTrackID());
        dighit++;

      }

      m_nEvent++;
      B2INFO("nHits: " << nHits << " digitized hits: " << dighit);

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

    }

    double TOPDigiModule::QEMultiAlkali(double energy)
    {
      const float qe[62] = {0.0, 0.11, 0.11, 0.18, 0.18, 0.21, 0.21, 0.23, 0.23, 0.23, 0.23, 0.24, 0.24, 0.25, 0.25, 0.23, 0.23, 0.21, 0.21, 0.2, 0.2, 0.17, 0.17, 0.14, 0.14, 0.12, 0.12, 0.11, 0.11, 0.095, 0.095, 0.081, 0.081, 0.07, 0.07, 0.06, 0.06, 0.05, 0.05, 0.042, 0.042, 0.036, 0.036, 0.03, 0.03, 0.023, 0.023, 0.019, 0.019, 0.013, 0.013, 0.009, 0.009, 0.006, 0.006, 0.003, 0.003, 0.002, 0.002, 0.001, 0.001, 0.0};

      int ich = (int)((1239.85 / energy - 250) / 10.);
      if (ich < 0 || ich >= 62) return 0;
      return 4.0 * qe[ich]; // collection efficiency 0.8 and 4 denoes the fraction of photons killed by staking action

    }

    double TOPDigiModule::QESuperBialkali(double energy)
    {
      const float qe[45] = {0.0, 0.197, 0.251, 0.29, 0.313, 0.333, 0.343, 0.348, 0.35, 0.35, 0.348, 0.346, 0.343, 0.34, 0.335, 0.327, 0.317, 0.306, 0.294, 0.28, 0.263, 0.244, 0.22, 0.197, 0.174, 0.153, 0.133, 0.116, 0.099, 0.083, 0.07, 0.058, 0.048, 0.041, 0.033, 0.026, 0.019, 0.014, 0.01, 0.006, 0.004, 0.002, 0.001, 0.0, 0.0};

      int ich = (int)((1239.85 / energy - 260) / 10.);
      if (ich < 0 || ich >= 45) return 0;
      return 1 / 0.35 * qe[ich]; // collection efficiency 0.8 and 1/0.35 denoes the fraction of photons killed by staking action
    }

    bool TOPDigiModule::DetectorQE(double energy)
    {
      return (m_random->Rndm(0) < QEMultiAlkali(energy));
    }

    double TOPDigiModule::PMT_TTS()
    {

      double prob = 0;
      double s = 0;

      int TTS_NG = 3;
      double tts_frac[3] = {0.5815, 0.2870, 0.1315};
      double tts_t0[3] = { -13.59e-3, 29.03e-3, 273.0e-3};
      double tts_sig[3] = {31.97e-3, 53.39e-3, 340.2e-3};

      prob = m_random->Rndm();

      for (int i = 0; i < TTS_NG; i++) {
        s = s + tts_frac[i];
        if (prob < s) {
          return Gaus(tts_t0[i], tts_sig[i]);

        }
      }

      return -999999;
    }

    double TOPDigiModule::Gaus(double mean, double sigma)
    {
      const double kC1 = 1.448242853;
      const double kC2 = 3.307147487;
      const double kC3 = 1.46754004;
      const double kD1 = 1.036467755;
      const double kD2 = 5.295844968;
      const double kD3 = 3.631288474;
      const double kHm = 0.483941449;
      const double kZm = 0.107981933;
      const double kHp = 4.132731354;
      const double kZp = 18.52161694;
      const double kPhln = 0.4515827053;
      const double kHm1 = 0.516058551;
      const double kHp1 = 3.132731354;
      const double kHzm = 0.375959516;
      const double kHzmp = 0.591923442;

      const double kAs = 0.8853395638;
      const double kBs = 0.2452635696;
      const double kCs = 0.2770276848;
      const double kB  = 0.5029324303;
      const double kX0 = 0.4571828819;
      const double kYm = 0.187308492 ;
      const double kS  = 0.7270572718 ;
      const double kT  = 0.03895759111;

      double result;
      double rn, x, y, z;


      do {
        y = m_random->Rndm(0);

        if (y > kHm1) {
          result = kHp * y - kHp1; break;
        }

        else if (y < kZm) {
          rn = kZp * y - 1;
          result = (rn > 0) ? (1 + rn) : (-1 + rn);
          break;
        }

        else if (y < kHm) {
          rn = m_random->Rndm(0);
          rn = rn - 1 + rn;
          z = (rn > 0) ? 2 - rn : -2 - rn;
          if ((kC1 - y) * (kC3 + TMath::Abs(z)) < kC2) {
            result = z; break;
          } else {
            x = rn * rn;
            if ((y + kD1) * (kD3 + x) < kD2) {
              result = rn; break;
            } else if (kHzmp - y < exp(-(z * z + kPhln) / 2)) {
              result = z; break;
            } else if (y + kHzm < exp(-(x + kPhln) / 2)) {
              result = rn; break;
            }
          }
        }

        while (1) {
          x = m_random->Rndm(0);
          y = kYm * m_random->Rndm(0);
          z = kX0 - kS * x - y;
          if (z > 0)
            rn = 2 + y / x;
          else {
            x = 1 - x;
            y = kYm - y;
            rn = -(2 + y / x);
          }
          if ((y - kAs + x) * (kCs + x) + kBs < 0) {
            result = rn; break;
          } else if (y < x + kT)
            if (rn * rn < 4 * (kB - log(x))) {
              result = rn; break;
            }
        }
      } while (0);


      return mean + sigma * result;

    }

  } // end top namespace
} // end Belle2 namespace
