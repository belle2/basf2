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
#include <GFTrack.h>
#include <GFTrackCand.h>
#include <tracking/dataobjects/ExtRecoHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihoods.h>
#include <generators/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPTrack.h>

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
      m_debugLevel(0),
      m_topgp(TOPGeometryPar::Instance())
    {
      // Set description
      setDescription("Reconstruction for TOP counter. Uses reconstructed tracks extrapolated to TOP and TOPDigits to calculate log likelihoods for e, mu, pi, K, p.");

      // Set property flags
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

      // Add parameters
      addParam("GFTracksColName", m_gfTracksColName, "GF tracks",
               string(""));
      addParam("ExtTrackCandsColName", m_extTrackCandsColName, "Ext track candidates",
               string("ExtTrackCands"));
      addParam("ExtRecoHitsColName", m_extRecoHitsColName, "Ext reconstructed hits",
               string("ExtRecoHits"));
      addParam("TOPDigitColName", m_topDigitColName, "TOP digits",
               string(""));
      addParam("TOPLikelihoodsColName", m_topLogLColName, "TOP likelihoods",
               string(""));
      addParam("TOPTrackColName", m_topTrackColName, "MCParticle hits at bars",
               string(""));
      addParam("DebugLevel", m_debugLevel, "Debug level", 0);
      addParam("minBkgPerBar", m_minBkgPerQbar,
               "minimal number of background photons per bar", 0.0);
      addParam("scaleN0", m_ScaleN0, "scale factor for N0", 1.0);

    }

    TOPRecoModule::~TOPRecoModule()
    {
    }

    void TOPRecoModule::initialize()
    {
      // Initialize masses (PDG 2010, hard coding -> to be removed in future (?))
      m_Masses[0] = 0.510998910E-3;
      m_Masses[1] = 0.105658367;
      m_Masses[2] = 0.13957018;
      m_Masses[3] = 0.493677;
      m_Masses[4] = 0.938272013;

      // Print set parameters
      printModuleParams();

      // Data store
      StoreArray<TOPDigit> topDigits;
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);
      StoreArray<GFTrackCand> extTrackCands(m_extTrackCandsColName);
      StoreArray<ExtRecoHit> extRecoHits(m_extRecoHitsColName);
      StoreArray<TOPTrack> topTracks;
      StoreArray<TOPLikelihoods> toplogL;
      RelationArray gfTrackLogL(gfTracks, toplogL);
      RelationArray LogLextHit(toplogL, extRecoHits);
      RelationArray LogLextTrackCand(toplogL, extTrackCands);
      RelationArray topTrackLogL(topTracks, toplogL);

      // Configure TOP detector
      TOPconfigure();

    }

    void TOPRecoModule::beginRun()
    {

    }

    void TOPRecoModule::event()
    {
      // input: digitized photons

      StoreArray<TOPDigit> topDigits;

      // input: reconstructed tracks

      StoreArray<GFTrack> gfTracks(m_gfTracksColName);
      StoreArray<GFTrackCand> extTrackCands(m_extTrackCandsColName);
      StoreArray<ExtRecoHit> extRecoHits(m_extRecoHitsColName);
      StoreArray<TOPTrack> topTracks;

      // output: log likelihoods

      StoreArray<TOPLikelihoods> toplogL;
      toplogL->Clear();

      // output: relations

      RelationArray gfTrackLogL(gfTracks, toplogL);
      gfTrackLogL.clear();
      RelationArray LogLextHit(toplogL, extRecoHits);
      LogLextHit.clear();
      RelationArray LogLextTrackCand(toplogL, extTrackCands);
      LogLextTrackCand.clear();
      RelationArray topTrackLogL(topTracks, toplogL);
      topTrackLogL.clear();

      // collect reconstructed tracks extrapolated to TOP

      std::vector<TOPtrack> tracks; // extrapolated tracks
      getTracks(tracks, 2); // use pion hypothesis
      if (tracks.size() == 0) return;

      // create reconstruction object

      TOPreco reco(Nhyp, m_Masses, m_minBkgPerQbar, m_ScaleN0);

      // clear reconstruction object

      reco.Clear();

      // add photons

      int nHits = topDigits.getEntries();
      for (int i = 0; i < nHits; ++i) {
        TOPDigit* data = topDigits[i];
        reco.AddData(data->getBarID() - 1, data->getChannelID() - 1, data->getTDC());
      }

      // reconstruct track-by-track and store the results

      for (unsigned int i = 0; i < tracks.size(); i++) {
        // reconstruct
        reco.Reconstruct(tracks[i]);
        if (m_debugLevel != 0) {
          tracks[i].Dump();
          reco.DumpHit(Local);
          reco.DumpLogL(Nhyp);
        }
        // get results
        double logl[Nhyp], expPhot[Nhyp];
        int nphot;
        reco.GetLogL(Nhyp, logl, expPhot, nphot);
        // store results
        int nentr = toplogL.getEntries();
        new(toplogL->AddrAt(nentr)) TOPLikelihoods(reco.Flag(), logl, nphot, expPhot);
        // make relations
        gfTrackLogL.add(tracks[i].Label(LgfTrack), nentr);
        LogLextHit.add(nentr, tracks[i].Label(LextHit));
        LogLextTrackCand.add(nentr, tracks[i].Label(LextTrackCand));
        int iTopTrack = tracks[i].Label(LtopTrack);
        if (iTopTrack >= 0) topTrackLogL.add(iTopTrack, nentr);
      }

      // consolidate relatons

      gfTrackLogL.consolidate();
      LogLextHit.consolidate();
      LogLextTrackCand.consolidate();
      topTrackLogL.consolidate();

    }


    void TOPRecoModule::endRun()
    {

    }

    void TOPRecoModule::terminate()
    {

    }

    void TOPRecoModule::printModuleParams() const
    {

    }


    void TOPRecoModule::TOPconfigure()
    {
      m_topgp->setBasfUnits();

      // space for bars including wedges
      m_R1 = m_topgp->getRadius() - m_topgp->getWextdown();
      double x = m_topgp->getQwidth() / 2.0;
      double y = m_topgp->getRadius() + m_topgp->getQthickness();
      m_R2 = sqrt(x * x + y * y);
      m_Z1 = m_topgp->getZ1() - m_topgp->getWLength();
      m_Z2 = m_topgp->getZ2();

      TOPvolume(m_R1, m_R2, m_Z1, m_Z2);

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


    const MCParticle* TOPRecoModule::getMCParticle(const GFTrack* track)
    {
      if (! track) return 0;

      StoreArray<MCParticle> mcParticles;
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);

      RelationArray rel(gfTracks, mcParticles);
      if (! rel) return 0;

      RelationIndex<GFTrack, MCParticle> irel(gfTracks, mcParticles);
      if (irel.getFirstTo(track)) {
        return irel.getFirstTo(track)->to;
      }
      return 0;
    }


    int TOPRecoModule::getTOPTrackIndex(const MCParticle* particle)
    {
      if (! particle) return -1;

      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPTrack> topTracks;

      RelationArray rel(mcParticles, topTracks);
      if (! rel) return -1;

      RelationIndex<MCParticle, TOPTrack> irel(mcParticles, topTracks);
      if (irel.getFirstTo(particle)) {
        return irel.getFirstTo(particle)->indexTo;
      }
      return -1;
    }


    void TOPRecoModule::getTracks(std::vector<TOPtrack> & tracks, int hypothesis)
    {
      unsigned int myDetID = 3; // TOP
      unsigned int NumBars = m_topgp->getNbars();
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);
      StoreArray<GFTrackCand> extTrackCands(m_extTrackCandsColName);
      StoreArray<ExtRecoHit> extRecoHits(m_extRecoHitsColName);

      for (int itra = 0; itra < gfTracks.getEntries(); ++itra) {
        GFTrack* track = gfTracks[itra];
        int charge = (int) track->getCharge();
        const MCParticle* particle = getMCParticle(track);
        int Lund = 0;
        if (particle) Lund = particle->getPDG();
        int iTopTrack = getTOPTrackIndex(particle);
        int iTrackCand = itra * 5 + hypothesis;
        GFTrackCand* cand = extTrackCands[iTrackCand];
        if (! cand) continue;
        std::vector<double> TOFs = cand->GetRhos();
        for (unsigned int j = 0; j < cand->getNHits(); ++j) {
          unsigned int detID;
          unsigned int hitID;
          unsigned int planeID;
          cand->getHitWithPlane(j, detID, hitID, planeID);
          if (detID != myDetID) continue;
          if (planeID == 0 || planeID > NumBars) continue;
          ExtRecoHit* hit = extRecoHits[hitID];
          ExtHitStatus status = hit->getStatus();
          if (status != 0) continue;
          TMatrixD point = hit->getRawHitCoord();
          double x = point(0, 0);
          double y = point(1, 0);
          double z = point(2, 0);
          if (z < m_Z1 || z > m_Z2) continue;
          double r = sqrt(x * x + y * y);
          if (r < m_R1 || r > m_R2) continue;
          double px = point(3, 0);
          double py = point(4, 0);
          double pz = point(5, 0);
          TOPtrack trk(x, y, z, px, py, pz, 0.0, charge, Lund);
          double tof = TOFs.at(j);
          trk.setTrackLength(tof, m_Masses[hypothesis]);
          trk.setLabel(LgfTrack, itra);
          trk.setLabel(LextTrackCand, iTrackCand);
          trk.setLabel(LextHit, hitID);
          trk.setLabel(LtopTrack, iTopTrack);
          tracks.push_back(trk);
        }
      }

    }


  } // end top namespace
} // end Belle2 namespace
