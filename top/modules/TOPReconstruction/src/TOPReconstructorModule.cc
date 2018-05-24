/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager


// Own include
#include <top/modules/TOPReconstruction/TOPReconstructorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPPull.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  using namespace TOP;
  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPReconstructor)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPReconstructorModule::TOPReconstructorModule() : Module()
  {
    // Set description
    setDescription("Reconstruction for TOP counter. Uses reconstructed tracks "
                   "extrapolated to TOP and TOPDigits to calculate log likelihoods "
                   "for charged stable particles");

    // Set property flags
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("minBkgPerBar", m_minBkgPerBar,
             "Minimal number of background photons per bar", 0.0);
    addParam("scaleN0", m_scaleN0,
             "Scale factor for N0", 1.0);
    addParam("sigmaRphi", m_sigmaRphi,
             "track smearing sigma in Rphi [cm]", 0.0);
    addParam("sigmaZ", m_sigmaZ,
             "track smearing sigma in Z [cm]", 0.0);
    addParam("sigmaTheta", m_sigmaTheta,
             "track smearing sigma in Theta [radians]", 0.0);
    addParam("sigmaPhi", m_sigmaPhi,
             "track smearing sigma in Phi [radians]", 0.0);
    addParam("minTime", m_minTime,
             "lower limit for photon time [ns] (default if minTime >= maxTime)", 0.0);
    addParam("maxTime", m_maxTime,
             "upper limit for photon time [ns] (default if minTime >= maxTime)", 0.0);
    addParam("PDGCode", m_PDGCode,
             "PDG code of hypothesis to construct pulls (0 means: use MC truth)",
             211);

    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_masses[i] = 0;}
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_pdgCodes[i] = 0;}

  }


  TOPReconstructorModule::~TOPReconstructorModule()
  {
  }


  void TOPReconstructorModule::initialize()
  {
    // input

    StoreArray<TOPDigit> digits;
    digits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();

    // output

    StoreArray<TOPLikelihood> likelihoods;
    likelihoods.registerInDataStore();
    likelihoods.registerRelationTo(extHits);
    likelihoods.registerRelationTo(barHits);
    tracks.registerRelationTo(likelihoods);

    StoreArray<TOPPull> topPulls;
    topPulls.registerInDataStore(DataStore::c_DontWriteOut);
    tracks.registerRelationTo(topPulls, DataStore::c_Event, DataStore::c_DontWriteOut);

    // check for module debug level

    if (getLogConfig().getLogLevel() == LogConfig::c_Debug) {
      m_debugLevel = getLogConfig().getDebugLevel();
    }

    // Initialize masses

    for (const auto& part : Const::chargedStableSet) {
      m_masses[part.getIndex()] = part.getMass();
      m_pdgCodes[part.getIndex()] = abs(part.getPDGCode());
    }

    // set track smearing flag

    m_smearTrack = m_sigmaRphi > 0 || m_sigmaZ > 0 || m_sigmaTheta > 0 ||
                   m_sigmaPhi > 0;

    // Configure TOP detector

    TOPconfigure config;
    if (m_debugLevel > 0) config.print();
  }

  void TOPReconstructorModule::beginRun()
  {
  }

  void TOPReconstructorModule::event()
  {

    // output: log likelihoods

    StoreArray<TOPLikelihood> likelihoods;

    StoreArray<TOPPull> topPulls;

    // create reconstruction object

    TOPreco reco(Const::ChargedStable::c_SetSize, m_masses, m_minBkgPerBar, m_scaleN0);
    reco.setHypID(Const::ChargedStable::c_SetSize, m_pdgCodes);

    // set time window if given, otherwise use the default one from TOPNominalTDC
    if (m_maxTime > m_minTime) {
      reco.setTimeWindow(m_minTime, m_maxTime);
    }

    // add photons

    StoreArray<TOPDigit> digits;
    for (const auto& digit : digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good) {
        reco.addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                     digit.getTimeError());
      }
    }

    // reconstruct track-by-track and store the results

    StoreArray<Track> tracks;
    for (const auto& track : tracks) {

      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // optional track smearing (needed for some MC studies)
      if (m_smearTrack) {
        trk.smear(m_sigmaRphi, m_sigmaZ, m_sigmaTheta, m_sigmaPhi);
        B2INFO("TOPReconstructor: additional smearing of track parameters done");
      }

      // reconstruct
      reco.reconstruct(trk, m_PDGCode);
      if (m_debugLevel > 1) {
        trk.dump();
        reco.dumpTrackHit(c_Local);
        reco.dumpLogL(Const::ChargedStable::c_SetSize);
        cout << endl;
      }

      // get results
      double logl[Const::ChargedStable::c_SetSize];
      double estPhot[Const::ChargedStable::c_SetSize];
      int nphot = 0;
      reco.getLogL(Const::ChargedStable::c_SetSize, logl, estPhot, nphot);
      double estBkg = reco.getExpectedBG();

      // store results
      TOPLikelihood* topL = likelihoods.appendNew(reco.getFlag(), nphot,
                                                  logl, estPhot, estBkg);
      // make relations:
      track.addRelationTo(topL);
      topL->addRelationTo(trk.getExtHit());
      topL->addRelationTo(trk.getBarHit());

      // store pulls
      int pixelID; float t, t0, wid, fic, wt;
      for (int k = 0; k < reco.getPullSize(); k++) {
        reco.getPull(k, pixelID, t, t0, wid, fic, wt);
        auto* pull = topPulls.appendNew(pixelID, t, t0, wid, fic, wt);
        track.addRelationTo(pull);
      }

    }

  }


  void TOPReconstructorModule::endRun()
  {
  }

  void TOPReconstructorModule::terminate()
  {
  }



} // end Belle2 namespace

