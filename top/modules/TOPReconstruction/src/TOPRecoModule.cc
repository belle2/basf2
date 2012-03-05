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
#include "top/modules/TOPReconstruction/TOPRecoModule.h"
#include "top/modules/TOPReconstruction/TOPreco.h"
#include "top/modules/TOPReconstruction/TOPtrack.h"
#include "top/modules/TOPReconstruction/TOPutil.h"
#include "top/modules/TOPReconstruction/TOPconfig.h"

#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes
// #include <generators/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPTrack.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihoods.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

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
      m_topgp(TOPGeometryPar::Instance())
    {
      // Set description()
      setDescription("TOPReconstruction");

      // Add parameters
      //      addParam("InputColName", m_inColName, "Input collection name",
      //         string("TOPDigitArray"));
      //      addParam("OutputColName", m_outColName, "Output collection name",
      //         string("TOPQuartzHitArray"));
    }

    TOPRecoModule::~TOPRecoModule()
    {
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

      StoreArray<TOPDigit> topDigits;
      StoreArray<TOPTrack> topTracks;
      StoreArray<TOPLikelihoods> toplogL;
      RelationArray relTrackLikelihoods(topTracks, toplogL);

      TOPconfigure();

    }

    void TOPRecoModule::beginRun()
    {
      B2INFO("TOPReco: Processing run: " << m_nRun);
    }

    void TOPRecoModule::event()
    {
      // input: digitized photons and tracks (currently MC tracks at TOP)
      StoreArray<TOPDigit> topDigits;
      StoreArray<TOPTrack> topTracks;

      // output: log likelihoods
      StoreArray<TOPLikelihoods> toplogL;
      toplogL->Clear();
      RelationArray relTrackLikelihoods(topTracks, toplogL);
      relTrackLikelihoods.clear();

      // create reconstruction object
      double Masses[5] = {.511E-3, .10566, .13957, .49368, .93827};
      int Nhyp = 5;
      TOPreco reco(Nhyp, Masses);

      // clear reconstruction object
      reco.Clear();

      // add photons
      int nHits = topDigits->GetEntries();
      for (int i = 0; i < nHits; ++i) {
        TOPDigit* data = topDigits[i];
        reco.AddData(data->getBarID() - 1, data->getChannelID() - 1, data->getTDC());
      }

      // reconstruct track-by-track and store results
      int nTracks = topTracks->GetEntries();
      for (int i = 0; i < nTracks; ++i) {
        TOPTrack* track = topTracks[i]; // MC particle at TOP
        if (track->getCharge() == 0) continue;
        TVector3 vpos = track->getVPosition();
        if (vpos.Perp() > 50.0) continue; // emulate track reco (acceptance in rho)
        TVector3 pos = track->getPosition();
        TVector3 mom = track->getMomentum();
        TOPtrack trk(pos.X(), pos.Y(), pos.Z(), mom.X(), mom.Y(), mom.Z(),
                     track->getLength(), track->getCharge(), track->getParticleID());
        trk.smear(1.0e-1, 1.4e-1, 1.5e-3, 1.5e-3); // emulate track reco (resolution)
        reco.Reconstruct(trk);
        double logl[5], expPhot[5];
        int nphot;
        reco.GetLogL(5, logl, expPhot, nphot);
        int nentr = toplogL->GetEntries();
        new(toplogL->AddrAt(nentr)) TOPLikelihoods(reco.Flag(), logl, nphot, expPhot);
        relTrackLikelihoods.add(i, nentr);
      }

    }

    void TOPRecoModule::TOPconfigure()
    {
      m_topgp->setBasfUnits();

      TOPvolume(116.5, 125.0, -85.0, 192.0);

      setBfield(-1.5);

      setPMT(m_topgp->getMsizex(), m_topgp->getMsizey(),
             m_topgp->getAsizex(), m_topgp->getAsizey(),
             m_topgp->getNpadx(), m_topgp->getNpady());

      int ng = m_topgp->getNgaussTTS();
      if (ng > 0) {
        double frac[ng], mean[ng], sigma[ng];
        for (int i = 0; i < ng; i++) {
          frac[i] = m_topgp->getTTSfrac(i);
          mean[i] = m_topgp->getTTSmean(i);
          sigma[i] = m_topgp->getTTSsigma(i);
        }
        setTTS(ng, frac, mean, sigma);
      }

      int size = m_topgp->getNpointsQE();
      if (size > 0) {
        double Wavelength[size], QE[size];
        for (int i = 0; i < size; i++) {
          QE[i] = m_topgp->getQE(i);
          Wavelength[i] = m_topgp->getLambdaFirst() + m_topgp->getLambdaStep() * i;
        }
        setQE(Wavelength, QE, size, m_topgp->getColEffi());
      }

      setTDC(m_topgp->getTDCbits(), m_topgp->getTDCbitwidth());

      int n = m_topgp->getNbars();           // number of bars in phi
      double Dphi = 2 * M_PI / n;
      double Phi = - 0.5 * M_PI;

      int id;
      double R = m_topgp->getRadius();          // innner bar surface radius
      double MirR = m_topgp->getMirradius();    // Mirror radious
      double A = m_topgp->getQwidth();          // bar width
      double B = m_topgp->getQthickness();      // bar thickness
      double z1 = m_topgp->getZ1();             // backward bar position
      double z2 = m_topgp->getZ2();             // forward bar position
      double DzExp = m_topgp->getWLength();     // expansion volume length
      double YsizExp = m_topgp->getWextdown() + m_topgp->getQthickness();
      double XsizPMT = m_topgp->getNpmtx() * m_topgp->getMsizex() +
                       (m_topgp->getNpmtx() - 1) * m_topgp->getXgap();
      double YsizPMT = m_topgp->getNpmty() * m_topgp->getMsizey() + m_topgp->getYgap();

      //! No edge roughness
      setEdgeRoughness(0);

      for (int i = 0; i < n; i++) {
        id = setQbar(A, B, z1, z2, R, 0, Phi, PMT, SphericM);
        setMirrorRadius(id, MirR);
        addExpansionVolume(id, Left, Prism, DzExp, B / 2, B / 2 - YsizExp);
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
