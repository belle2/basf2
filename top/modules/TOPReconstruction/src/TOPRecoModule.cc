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
#include "top/modules/TOPReconstruction/TOPRecoModule.h"
#include "top/modules/TOPReconstruction/TOPreco.h"
#include "top/modules/TOPReconstruction/TOPtrack.h"
#include "top/modules/TOPReconstruction/TOPutil.h"
#include "top/modules/TOPReconstruction/TOPconfig.h"


#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
#include <top/dataobjects/TOPQuartzHit.h>
#include <top/dataobjects/TOPDigiHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector2.h>
#include <TVector3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace top {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(TOPReco)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPRecoModule::TOPRecoModule() : Module(),
        m_random(new TRandom1(0)), m_topgp(TOPGeometryPar::Instance())
    {
      // Set description()
      setDescription("TOPRecotizer");

      // Add parameters
      addParam("InputColName", m_inColName, "Input collection name", string("TOPDigiHitArray"));
      addParam("OutputColName", m_outColName, "Output collection name", string("TOPQuartzHitArray"));
    }

    TOPRecoModule::~TOPRecoModule()
    {
      if (m_random) delete m_random;
      if (m_topgp) delete m_topgp;
    }

    void TOPRecoModule::initialize()
    {
      // Initialize variables
      m_nRun    = 0 ;
      m_nEvent  = 0 ;

      // Print set parameters
      printModuleParams();

      // CPU time start
      m_timeCPU = clock() * Unit::us;

      StoreArray<TOPDigiHit> topDigiHits;
      StoreArray<TOPQuartzHit> topQuartzHits;
    }

    void TOPRecoModule::beginRun()
    {
      // Print run number
      B2INFO("TOPReco: Processing run: " << m_nRun);

    }

    void TOPRecoModule::event()
    {

      //! Get the collection of TOPSimHits from the Data store.

      StoreArray<TOPDigiHit> topDigiHits;
      if (!topDigiHits) B2ERROR("TOPDigitizerModule: Cannot find TOPDigiHit array.");

      //! Get the collection of TOPDigiHits from the Data store,

      StoreArray<TOPQuartzHit> topQuartzHits;
      if (!topQuartzHits) B2ERROR("TOPDigitizerModule: Cannot find TOPQuartzHit array.");


      //TOPconfigure();

      /*
      double Masses[5] = {.511E-3, .10566, .13957, .49368, .93827};
      int Nhyp = 5;
      TOPreco reco(Nhyp, Masses);

      // Get number of hits in this event
      int nHits = topHits->GetLast();
      B2INFO("nHits: " << nHits);

      // Get number of hits in this event
      int nQHits = topQuartzHits->GetLast();
      B2INFO("n tracks: " << nHits);

      for (int track = 0; track < nQHits; ++track) {
          TOPQuartzHit* atrack = topQuartzHits[track];

          TVector3 pos = atrack->getPosition();
          TVector3 mom = atrack->getMomentum();

          TOPtrack tr(pos.X(), pos.Y(), pos.Z(), mom.X(), mom.Y(), mom.Z(), atrack->getLength(), 1, atrack->getParticleID());
          tr.toTop();
          tr.Dump();

          reco.Clear();
          for (int hit; hit < nHits; ++nHits) {
              TOPHit* pmthit = topHits[hit];

              reco.AddData(pmthit->getBarID(), pmthit->getChannelID(), TDCdigi(pmthit->getGlobalTime()));
          }

          reco.Reconstruct(tr);
      }*/

    }

    void TOPRecoModule::TOPconfigure()
    {

      TOPvolume(116.5, 125.0, -85.0, 192.0);
      /*
      setBfield(1.5);

      setPMT(m_topgp->getMsizex(), m_topgp->getMsizey(), m_topgp->getAsizex(), m_topgp->getAsizey(), m_topgp->getNpmtx(), m_topgp->getNpmty());


      double frac[3] = {0.5815, 0.2870, 0.1315};
      double mean[3] = { -13.59e-3, 29.03e-3, 273.0e-3};
      double sigma[3] = {31.97e-3, 53.39e-3, 340.2e-3};
      setTTS(3, frac, mean, sigma);

      const int size=62;
      double QE[size] = {0.0, 0.11, 0.11, 0.18, 0.18, 0.21, 0.21, 0.23, 0.23, 0.23, 0.23, 0.24, 0.24, 0.25, 0.25, 0.23, 0.23, 0.21, 0.21, 0.2, 0.2, 0.17, 0.17, 0.14, 0.14, 0.12, 0.12, 0.11, 0.11, 0.095, 0.095, 0.081, 0.081, 0.07, 0.07, 0.06, 0.06, 0.05, 0.05, 0.042, 0.042, 0.036, 0.036, 0.03, 0.03, 0.023, 0.023, 0.019, 0.019, 0.013, 0.013, 0.009, 0.009, 0.006, 0.006, 0.003, 0.003, 0.002, 0.002, 0.001, 0.001, 0.0};

      double Wavelength[size] = {250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500, 510, 520, 530, 540, 550, 560, 570, 580, 590, 600, 610, 620, 630, 640, 650, 660, 670, 680, 690, 700, 710, 720, 730, 740, 750, 760, 770, 780, 790, 800, 810, 820, 830, 840, 850, 860};

      setQE(Wavelength, QE, size, 0.6);

      setTDC(m_topgp->getTDCbits(), m_topgp->getTDCbitwidth());


      double Pi = 4.*atan(1.);

      int n = 16;           // number of bars in phi
      double Dphi = 2 * Pi / n;
      double Phi = 0;

      int id;
      double R = 120.0;     // innner bar surface radius
      double A = 45.4;      // bar width
      double B = 2.0;       // bar thickness
      double z1, z2;        // backward, forward bar position
      double DzExp = 10.0;  // expansion volume length
      double YsizExp = 5.05;  // expansion volume height
      double YsizPMT = 2 * 2.8; // height to arrange 2 rows of PMT

      //! No edge roughness
      setEdgeRoughness(0);*/
    }


    void TOPRecoModule::endRun()
    {
      m_nRun++;
    }

    void TOPRecoModule::terminate()
    {
      // CPU time end
      m_timeCPU = clock() * Unit::us - m_timeCPU;

      // Announce
      B2INFO("TOPReco finished. Time per event: "/* << m_timeCPU / m_nEvent / Unit::ms << " ms."*/);

    }

    void TOPRecoModule::printModuleParams() const
    {

    }

  } // end top namespace
} // end Belle2 namespace
