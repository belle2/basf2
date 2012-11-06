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
               "now not used - to be removed", 0.3);
      addParam("T0jitter", m_T0jitter, "r.m.s of T0 jitter [ns]", 25e-3);
      addParam("ELjitter", m_ELjitter, "r.m.s of electronics jitter [ns]", 50e-3);

    }

    TOPDigitizerModule::~TOPDigitizerModule()
    {
    }

    void TOPDigitizerModule::initialize()
    {
      // data store registration
      StoreArray<TOPDigit>::registerPersistent(m_outColName);
      RelationArray::registerPersistent<TOPSimHit, TOPDigit>(m_inColName, m_outColName);

      // print parameters
      printModuleParams();

      // store electronics jitter to make it known for reconstruction
      m_topgp->setELjitter(m_ELjitter);
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
      topDigits.create();

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
      for (int iHit = 0; iHit < nHits; iHit++) {
        TOPSimHit* aSimHit = topSimHits[iHit];

        // Do spatial digitization
        double x = aSimHit->getX();
        double y = aSimHit->getY();
        int pmtID = aSimHit->getPmtID();
        int channelID = m_topgp->getChannelID(x, y, pmtID);
        if (channelID == 0) continue;

        // add T0 jitter, TTS and electronic jitter to photon time
        double tts = PMT_TTS();
        double tel = gRandom->Gaus(0., m_ELjitter);
        double time = t_beam + aSimHit->getTime() + tts + tel;

        // convert to TDC digits
        if (time < 0) continue;
        if (time > Tmax) time = Tmax;
        int TDC = int(time / TDCwidth);
        if (TDC > maxTDC) TDC = maxTDC;

        // store result
        new(topDigits.nextFreeAddress()) TOPDigit(aSimHit->getBarID(), channelID, TDC);

        // make relations
        int iDigit = topDigits.getEntries() - 1;
        relSimHitToDigit.add(iHit, iDigit);

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

