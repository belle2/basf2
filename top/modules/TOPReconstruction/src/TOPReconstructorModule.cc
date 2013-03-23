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
#include "top/modules/TOPReconstruction/TOPReconstructorModule.h"
#include "top/modules/TOPReconstruction/TOPreco.h"
#include "top/modules/TOPReconstruction/TOPtrack.h"
#include "top/modules/TOPReconstruction/TOPutil.h"
#include "top/modules/TOPReconstruction/TOPconfig.h"

#include <framework/core/ModuleManager.h>

// Hit classes
#include <GFTrack.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <generators/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace TOP {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(TOPReconstructor)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    TOPReconstructorModule::TOPReconstructorModule() : Module(),
      m_debugLevel(0),
      m_topgp(TOPGeometryPar::Instance()),
      m_R1(0),
      m_R2(0),
      m_Z1(0),
      m_Z2(0)
    {
      // Set description
      setDescription("Reconstruction for TOP counter. Uses reconstructed tracks extrapolated to TOP and TOPDigits to calculate log likelihoods for e, mu, pi, K, p.");

      // Set property flags
      setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

      // Add parameters
      addParam("GFTracksColName", m_gfTracksColName, "GF tracks",
               string(""));
      addParam("ExtHitsColName", m_extHitsColName, "Extrapolated tracks",
               string(""));
      addParam("TOPDigitColName", m_topDigitColName, "TOP digits",
               string(""));
      addParam("TOPLikelihoodColName", m_topLogLColName, "TOP likelihoods",
               string(""));
      addParam("TOPBarHitColName", m_barHitColName, "MCParticle hits at bars",
               string(""));
      addParam("DebugLevel", m_debugLevel, "Debug level", 0);
      addParam("minBkgPerBar", m_minBkgPerQbar,
               "minimal number of background photons per bar", 0.0);
      addParam("scaleN0", m_ScaleN0, "scale factor for N0", 1.0);

      for (int i = 0; i < c_Nhyp; i++) {m_Masses[i] = 0;}

    }

    TOPReconstructorModule::~TOPReconstructorModule()
    {
    }

    void TOPReconstructorModule::initialize()
    {
      // Initialize masses (PDG 2010) TODO: remove hard coding, check c_Nhyp)
      m_Masses[0] = 0.510998910E-3;
      m_Masses[1] = 0.105658367;
      m_Masses[2] = 0.13957018;
      m_Masses[3] = 0.493677;
      m_Masses[4] = 0.938272013;

      // Print module parameters
      printModuleParams();

      // Data store registration
      StoreArray<TOPLikelihood>::registerPersistent(m_topLogLColName);
      RelationArray::registerPersistent<GFTrack, TOPLikelihood>
      (m_gfTracksColName, m_topLogLColName);
      RelationArray::registerPersistent<TOPLikelihood, ExtHit>
      (m_topLogLColName, m_extHitsColName);
      RelationArray::registerPersistent<TOPBarHit, TOPLikelihood>
      (m_barHitColName, m_topLogLColName);

      // Configure TOP detector
      TOPconfigure();

    }

    void TOPReconstructorModule::beginRun()
    {

    }

    void TOPReconstructorModule::event()
    {
      // input: digitized photons

      StoreArray<TOPDigit> topDigits(m_topDigitColName);

      // input: reconstructed tracks

      StoreArray<GFTrack> gfTracks(m_gfTracksColName);
      StoreArray<ExtHit> extHits(m_extHitsColName);
      StoreArray<TOPBarHit> barHits(m_barHitColName);

      // output: log likelihoods

      StoreArray<TOPLikelihood> toplogL(m_topLogLColName);
      toplogL.create();

      // output: relations

      RelationArray gfTrackLogL(gfTracks, toplogL);
      gfTrackLogL.clear();
      RelationArray LogLextHit(toplogL, extHits);
      LogLextHit.clear();
      RelationArray barHitLogL(barHits, toplogL);
      barHitLogL.clear();

      // collect reconstructed tracks extrapolated to TOP

      std::vector<TOPtrack> tracks; // extrapolated tracks
      getTracks(tracks, Const::pion); // use pion hypothesis
      if (tracks.empty()) return;

      // create reconstruction object

      TOPreco reco(c_Nhyp, m_Masses, m_minBkgPerQbar, m_ScaleN0);

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
          reco.DumpLogL(c_Nhyp);
        }

        // get results
        double logl[c_Nhyp], expPhot[c_Nhyp];
        int nphot;
        reco.GetLogL(c_Nhyp, logl, expPhot, nphot);

        // store results
        new(toplogL.nextFreeAddress()) TOPLikelihood(reco.Flag(), logl, nphot, expPhot);

        // make relations
        int last = toplogL.getEntries() - 1;
        gfTrackLogL.add(tracks[i].Label(c_LTrack), last);
        LogLextHit.add(last, tracks[i].Label(c_LextHit));
        int ibarHit = tracks[i].Label(c_LbarHit);
        if (ibarHit >= 0) barHitLogL.add(ibarHit, last);
      }

      // consolidate relatons

      gfTrackLogL.consolidate();
      LogLextHit.consolidate();
      barHitLogL.consolidate();

    }


    void TOPReconstructorModule::endRun()
    {

    }

    void TOPReconstructorModule::terminate()
    {

    }

    void TOPReconstructorModule::printModuleParams() const
    {

    }


    void TOPReconstructorModule::TOPconfigure()
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

      setBfield(-1.5); //TODO get magnetic field from database

      setPMT(m_topgp->getMsizex(), m_topgp->getMsizey(),
             m_topgp->getAsizex(), m_topgp->getAsizey(),
             m_topgp->getNpadx(), m_topgp->getNpady());

      int ng = m_topgp->getNgaussTTS();
      double sigmaTDC = m_topgp->getELjitter();
      if (ng > 0) {
        double frac[ng], mean[ng], sigma[ng];
        for (int i = 0; i < ng; i++) {
          frac[i] = m_topgp->getTTSfrac(i);
          mean[i] = m_topgp->getTTSmean(i);
          sigma[i] = m_topgp->getTTSsigma(i);
          sigma[i] = sqrt(sigma[i] * sigma[i] + sigmaTDC * sigmaTDC);
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
      double MirrR = m_topgp->getMirradius();   // Mirror radius
      double MirrXc = m_topgp->getMirposx();    // Mirror X center of curvature
      double MirrYc = m_topgp->getMirposy();    // Mirror Y center of curvature
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
        setMirrorRadius(id, MirrR);
        setMirrorCenter(id, MirrXc, MirrYc);
        addExpansionVolume(id, Left, Prism, DzExp, B / 2, B / 2 - YsizExp);
        arrangePMT(id, Left, XsizPMT, YsizPMT);
        Phi += Dphi;
      }

      TOPfinalize(); //TODO: if not successfull exit with B2ERROR
    }


    const MCParticle* TOPReconstructorModule::getMCParticle(const GFTrack* track)
    {
      if (! track) return 0;

      StoreArray<MCParticle> mcParticles;
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);

      RelationArray rel(gfTracks, mcParticles);
      if (! rel) return 0;

      RelationIndex<GFTrack, MCParticle> irel(gfTracks, mcParticles);
      if (irel.getFirstElementFrom(track)) {
        return irel.getFirstElementFrom(track)->to;
      }
      return 0;
    }


    int TOPReconstructorModule::getTOPBarHitIndex(const MCParticle* particle)
    {
      if (! particle) return -1;

      StoreArray<MCParticle> mcParticles;
      StoreArray<TOPBarHit> barHits;

      RelationArray rel(mcParticles, barHits);
      if (! rel) return -1;

      RelationIndex<MCParticle, TOPBarHit> irel(mcParticles, barHits);
      if (irel.getFirstElementFrom(particle)) {
        return irel.getFirstElementFrom(particle)->indexTo;
      }
      return -1;
    }


    void TOPReconstructorModule::getTracks(std::vector<TOPtrack> & tracks,
                                           Const::ChargedStable chargedStable)
    {
      ExtDetectorID myDetID = EXT_TOP; // TOP
      int NumBars = m_topgp->getNbars();
      StoreArray<GFTrack> gfTracks(m_gfTracksColName);
      StoreArray<ExtHit> extHits(m_extHitsColName);
      RelationIndex<GFTrack, ExtHit> gfTracksToExtHits(gfTracks, extHits);

      int pdgCode = abs(chargedStable.getPDGCode());
      double mass = chargedStable.getMass();

      for (int itra = 0; itra < gfTracks.getEntries(); ++itra) {
        GFTrack* track = gfTracks[itra];
        int charge = (int) track->getCharge();
        const MCParticle* particle = getMCParticle(track);
        int iTopTrack = getTOPBarHitIndex(particle);
        int truePDGCode = 0;
        if (particle) truePDGCode = particle->getPDG();

        /*
         * this doesn't work for GFTrack, have to use BOOST_FOREACH
         RelationVector<ExtHit> extHits = track->getRelationsTo<ExtHit>();
         for(unsigned i = 0; i < extHits.size(); i++) {
            const ExtHit* extHit = extHits[i];
        */

        typedef RelationIndex<GFTrack, ExtHit>::Element relElement_t;
        BOOST_FOREACH(const relElement_t & rel, gfTracksToExtHits.getElementsFrom(track)) {
          const ExtHit* extHit = rel.to;
          if (abs(extHit->getPdgCode()) != pdgCode) continue;
          if (extHit->getDetectorID() != myDetID) continue;
          if (extHit->getCopyID() == 0 || extHit->getCopyID() > NumBars) continue;
          if (extHit->getStatus() != EXT_ENTER) continue;
          TVector3 point = extHit->getPosition();
          double x = point.X();
          double y = point.Y();
          double z = point.Z();
          if (z < m_Z1 || z > m_Z2) continue;
          double r = sqrt(x * x + y * y);
          if (r < m_R1 || r > m_R2) continue;
          TVector3 momentum = extHit->getMomentum();
          double px = momentum.X();
          double py = momentum.Y();
          double pz = momentum.Z();
          TOPtrack trk(x, y, z, px, py, pz, 0.0, charge, truePDGCode);
          double tof = extHit->getTOF();
          trk.setTrackLength(tof, mass);
          trk.setLabel(c_LTrack, itra);
          trk.setLabel(c_LextHit, extHit->getArrayIndex());
          trk.setLabel(c_LbarHit, iTopTrack);
          tracks.push_back(trk);
        }
      }
    }


  } // end top namespace
} // end Belle2 namespace
