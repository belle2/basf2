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
#include <tracking/dataobjects/Track.h>
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
#include <geometry/bfieldmap/BFieldMap.h>

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
      m_smearTrack(false),
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
      addParam("inputTracks", m_inputTracks, "Mdst tracks", string(""));
      addParam("inputExtHits", m_inputExtHits, "Extrapolated tracks", string(""));
      addParam("inputDigits", m_inputDigits, "TOP digits", string(""));
      addParam("inputBarHits", m_inputBarHits, "MCParticle hits at bars", string(""));
      addParam("outputLikelihoods", m_outputLikelihoods, "TOP likelihoods", string(""));
      addParam("debugLevel", m_debugLevel, "Debug level", 0);
      addParam("minBkgPerBar", m_minBkgPerBar,
               "Minimal number of background photons per bar", 0.0);
      addParam("scaleN0", m_scaleN0, "Scale factor for N0", 1.0);
      addParam("sigmaRphi", m_sigmaRphi , "track smearing sigma in Rphi [cm]", 0.0);
      addParam("sigmaZ", m_sigmaZ , "track smearing sigma in Z [cm]", 0.0);
      addParam("sigmaTheta", m_sigmaTheta , "track smearing sigma in Theta [radians]",
               0.0);
      addParam("sigmaPhi", m_sigmaPhi , "track smearing sigma in Phi [radians]", 0.0);

      for (int i = 0; i < c_Nhyp; i++) {m_Masses[i] = 0;}

    }

    TOPReconstructorModule::~TOPReconstructorModule()
    {
    }

    void TOPReconstructorModule::initialize()
    {
      // Initialize masses
      m_Masses[0] = Const::electron.getMass();
      m_Masses[1] = Const::muon.getMass();
      m_Masses[2] = Const::pion.getMass();
      m_Masses[3] = Const::kaon.getMass();
      m_Masses[4] = Const::proton.getMass();

      m_smearTrack = m_sigmaRphi > 0 || m_sigmaZ > 0 || m_sigmaTheta > 0 ||
                     m_sigmaPhi > 0;

      // Data store registration
      StoreArray<TOPLikelihood>::registerPersistent(m_outputLikelihoods);
      RelationArray::registerPersistent<Track, TOPLikelihood>
      (m_inputTracks, m_outputLikelihoods);
      RelationArray::registerPersistent<TOPLikelihood, ExtHit>
      (m_outputLikelihoods, m_inputExtHits);
      RelationArray::registerPersistent<TOPBarHit, TOPLikelihood>
      (m_inputBarHits, m_outputLikelihoods);

      // Configure TOP detector
      TOPconfigure();

    }

    void TOPReconstructorModule::beginRun()
    {

    }

    void TOPReconstructorModule::event()
    {
      // input: digitized photons

      StoreArray<TOPDigit> topDigits(m_inputDigits);

      // input: reconstructed tracks

      StoreArray<Track> Tracks(m_inputTracks);
      StoreArray<ExtHit> extHits(m_inputExtHits);
      StoreArray<TOPBarHit> barHits(m_inputBarHits);

      // output: log likelihoods

      StoreArray<TOPLikelihood> toplogL(m_outputLikelihoods);
      toplogL.create();

      // output: relations

      RelationArray TrackLogL(Tracks, toplogL);
      TrackLogL.clear();
      RelationArray LogLextHit(toplogL, extHits);
      LogLextHit.clear();
      RelationArray barHitLogL(barHits, toplogL);
      barHitLogL.clear();

      // collect reconstructed tracks extrapolated to TOP

      std::vector<TOPtrack> tracks; // extrapolated tracks
      getTracks(tracks, Const::pion); // use pion hypothesis
      if (tracks.empty()) return;

      // optional track smearing (needed for some MC studies)
      if (m_smearTrack) {
        for (unsigned int i = 0; i < tracks.size(); i++) {
          tracks[i].smear(m_sigmaRphi, m_sigmaZ, m_sigmaTheta, m_sigmaPhi);
        }
        B2INFO("TOPReconstructor: additional smearing of track parameters done");
      }

      // create reconstruction object

      TOPreco reco(c_Nhyp, m_Masses, m_minBkgPerBar, m_scaleN0);

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
        TrackLogL.add(tracks[i].Label(c_LTrack), last);
        LogLextHit.add(last, tracks[i].Label(c_LextHit));
        int ibarHit = tracks[i].Label(c_LbarHit);
        if (ibarHit >= 0) barHitLogL.add(ibarHit, last);
      }

      // consolidate relatons

      TrackLogL.consolidate();
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

      // get magnetic field at TOP
      TVector3 point(0, m_topgp->getRadius(), 0);
      TVector3 Bfield = BFieldMap::Instance().getBField(point);
      setBfield(-Bfield.Z());

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
        setQE(Wavelength, QE, size, m_topgp->getColEffi() * m_topgp->getELefficiency());
      }

      setTDC(m_topgp->getTDCbits(), m_topgp->getTDCbitwidth());

      int n = m_topgp->getNbars();           // number of bars in phi
      double Dphi = 2 * M_PI / n;
      double Phi = m_topgp->getPhi0() - 0.5 * M_PI;

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
      double Wwidth = m_topgp->getWwidth();     // expansion volume width
      double Wflat = m_topgp->getWflat();     // expansion volume flat part
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
        addExpansionVolume(id, Left, Prism, DzExp - Wflat, B / 2, B / 2 - YsizExp,
                           0, 0, Wwidth);
        setBBoxWindow(id, Wflat);
        arrangePMT(id, Left, XsizPMT, YsizPMT);
        Phi += Dphi;
      }

      bool ok = TOPfinalize(m_debugLevel);
      if (!ok) B2ERROR("TOP configuration failed");
    }


    void TOPReconstructorModule::getTracks(std::vector<TOPtrack>& tracks,
                                           Const::ChargedStable chargedStable)
    {
      ExtDetectorID myDetID = EXT_TOP; // TOP
      int NumBars = m_topgp->getNbars();
      int pdgCode = abs(chargedStable.getPDGCode());
      double mass = chargedStable.getMass();

      StoreArray<Track> Tracks(m_inputTracks);

      for (int itra = 0; itra < Tracks.getEntries(); ++itra) {
        const Track* track = Tracks[itra];
        const TrackFitResult* fitResult = track->getTrackFitResult(chargedStable);
        if (!fitResult) {
          B2ERROR("No TrackFitResult for " << chargedStable.getPDGCode());
          continue;
        }
        int charge = fitResult->getCharge();
        const MCParticle* particle = DataStore::getRelated<MCParticle>(track);
        const TOPBarHit* barHit = DataStore::getRelated<TOPBarHit>(particle);
        int iBarHit = -1;
        if (barHit) iBarHit = barHit->getArrayIndex();
        int truePDGCode = 0;
        if (particle) truePDGCode = particle->getPDG();

        RelationVector<ExtHit> extHits = DataStore::getRelationsWithObj<ExtHit>(track);
        for (unsigned i = 0; i < extHits.size(); i++) {
          const ExtHit* extHit = extHits[i];
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
          trk.setLabel(c_LbarHit, iBarHit);
          tracks.push_back(trk);
        }
      }
    }


  } // end top namespace
} // end Belle2 namespace
