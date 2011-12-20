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
  namespace TOP {
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
      //      if (m_topgp) delete m_topgp;
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

      TOPconfigure();

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


      double Masses[5] = {.511E-3, .10566, .13957, .49368, .93827};
      int Nhyp = 5;
      TOPreco reco(Nhyp, Masses);

      // Get number of hits in this event
      int nHits = topDigiHits->GetEntries();
      B2INFO("nHits: " << nHits);

      // Get number of hits in this event
      int nQHits = topQuartzHits->GetEntries();
      B2INFO("n tracks: " << nQHits);

      for (int track = 0; track < nQHits; ++track) {
        TOPQuartzHit* atrack = topQuartzHits[track];

        TVector3 pos = atrack->getPosition();
        TVector3 mom = atrack->getMomentum();

        TOPtrack tr(pos.X() / 10., pos.Y() / 10., pos.Z() / 10., mom.X() / 1000., mom.Y() / 1000., mom.Z() / 1000., atrack->getLength() / 10., atrack->getCharge(), atrack->getParticleID());
        tr.toTop();
        tr.Dump();

        reco.Clear();
        for (int hit = 0; hit < nHits; ++hit) {
          TOPDigiHit* pmthit = topDigiHits[hit];

          reco.AddData(pmthit->getBarID() - 1, pmthit->getChannelID(), pmthit->getTDC());
        }

        reco.Reconstruct(tr);
        reco.DumpHit(Local);
        reco.DumpHit(Global);
        reco.DumpLogL(Nhyp);
      }

    }

    void TOPRecoModule::TOPconfigure()
    {

      TOPvolume(116.5, 125.0, -85.0, 192.0);

      setBfield(1.5);

      setPMT(m_topgp->getMsizex() / 10.0, m_topgp->getMsizey() / 10.0, m_topgp->getAsizex() / 10.0, m_topgp->getAsizey() / 10.0, m_topgp->getNpmtx(), m_topgp->getNpmty());


      double frac[3] = {0.5815, 0.2870, 0.1315};
      double mean[3] = { -13.59e-3, 29.03e-3, 273.0e-3};
      double sigma[3] = {31.97e-3, 53.39e-3, 340.2e-3};
      setTTS(3, frac, mean, sigma);

      const int size = 62;
      double QE[size] = {0.0, 0.11, 0.11, 0.18, 0.18, 0.21, 0.21, 0.23, 0.23, 0.23, 0.23, 0.24, 0.24, 0.25, 0.25, 0.23, 0.23, 0.21, 0.21, 0.2, 0.2, 0.17, 0.17, 0.14, 0.14, 0.12, 0.12, 0.11, 0.11, 0.095, 0.095, 0.081, 0.081, 0.07, 0.07, 0.06, 0.06, 0.05, 0.05, 0.042, 0.042, 0.036, 0.036, 0.03, 0.03, 0.023, 0.023, 0.019, 0.019, 0.013, 0.013, 0.009, 0.009, 0.006, 0.006, 0.003, 0.003, 0.002, 0.002, 0.001, 0.001, 0.0};

      double Wavelength[size] = {250, 260, 270, 280, 290, 300, 310, 320, 330, 340, 350, 360, 370, 380, 390, 400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500, 510, 520, 530, 540, 550, 560, 570, 580, 590, 600, 610, 620, 630, 640, 650, 660, 670, 680, 690, 700, 710, 720, 730, 740, 750, 760, 770, 780, 790, 800, 810, 820, 830, 840, 850, 860};

      setQE(Wavelength, QE, size, 0.6);

      setTDC(m_topgp->getTDCbits(), m_topgp->getTDCbitwidth());

      int n = m_topgp->getNbars();           // number of bars in phi
      double Dphi = 2 * M_PI / n;
      double Phi = - 0.5 * M_PI;

      int id;
      double R = m_topgp->getRadius() / 10.0;     // innner bar surface radius
      double MirR = m_topgp->getMirradius() / 10.0; //Mirror radious
      double A = m_topgp->getQwidth() / 10.0;      // bar width
      double B = m_topgp->getQthickness() / 10.0;       // bar thickness
      double z1 = m_topgp->getZ1() / 10.0; // backward, forward bar position
      double z2 = m_topgp->getZ2() / 10.0;   // backward, forward bar position
      double DzExp = m_topgp->getWLength() / 10.0;  // expansion volume length
      double YsizExp = (m_topgp->getWextdown() + m_topgp->getQthickness()) / 10.0;  // expansion volume height
      double YsizPMT = (m_topgp->getNpmty() * m_topgp->getMsizey() + m_topgp->getYgap()) / 10.0; // height to arrange 2 rows of PMT
      double XsizPMT = (m_topgp->getNpmtx() * m_topgp->getMsizex() + (m_topgp->getNpmtx() - 1) * m_topgp->getXgap()) / 10.0; // height to arrange 2 rows of PMT
      // B2INFO("z1 = "<<z1<<"  z2 = "<<z2);


      //! No edge roughness
      setEdgeRoughness(0);

      for (int i = 0; i < n; i++) {
        id = setQbar(A, B, z1, z2, R, 0, Phi, PMT, SphericM);
        setMirrorRadius(id, MirR);
        addExpansionVolume(id, Left, Prism, DzExp, B / 2, - YsizExp);
        arrangePMT(id, Left, XsizPMT, YsizPMT);
        Phi += Dphi;
      }


      TOPfinalize();


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
