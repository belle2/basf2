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
#include <top/modules/TOPReconstruction/TOPReconstructorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

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

  }


  TOPReconstructorModule::~TOPReconstructorModule()
  {
  }


  void TOPReconstructorModule::initialize()
  {
    // input

    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_barHits.isOptional();
    m_recBunch.isOptional();

    // output

    m_likelihoods.registerInDataStore();
    m_likelihoods.registerRelationTo(m_extHits);
    m_likelihoods.registerRelationTo(m_barHits);
    m_tracks.registerRelationTo(m_likelihoods);

    m_topPulls.registerInDataStore(DataStore::c_DontWriteOut);
    m_tracks.registerRelationTo(m_topPulls, DataStore::c_Event, DataStore::c_DontWriteOut);

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

    // clear output objects

    m_likelihoods.clear();
    m_topPulls.clear();

    // check bunch reconstruction status and do the reconstruction:
    // - if object exists and bunch is found (collision data w/ bunch finder in the path)
    // - if object doesn't exist (cosmic data and other cases w/o bunch finder in the path)

    if (m_recBunch.isValid()) {
      if (!m_recBunch->isReconstructed()) return;
    }

    // create reconstruction object

    TOPreco reco(Const::ChargedStable::c_SetSize, m_masses, m_minBkgPerBar, m_scaleN0);
    reco.setHypID(Const::ChargedStable::c_SetSize, m_pdgCodes);

    // set time window if given, otherwise use the default one from TOPNominalTDC
    if (m_maxTime > m_minTime) {
      reco.setTimeWindow(m_minTime, m_maxTime);
    }

    // add photons

    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good) {
        reco.addData(digit.getModuleID(), digit.getPixelID(), digit.getTime(),
                     digit.getTimeError());
      }
    }

    // reconstruct track-by-track and store the results

    for (const auto& track : m_tracks) {

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
      TOPLikelihood* topL = m_likelihoods.appendNew(reco.getFlag(), nphot,
                                                    logl, estPhot, estBkg);
      // make relations:
      track.addRelationTo(topL);
      topL->addRelationTo(trk.getExtHit());
      topL->addRelationTo(trk.getBarHit());

      // store pulls
      int pixelID; float t, t0, wid, fic, wt;
      for (int k = 0; k < reco.getPullSize(); k++) {
        reco.getPull(k, pixelID, t, t0, wid, fic, wt);
        auto* pull = m_topPulls.appendNew(pixelID, t, t0, wid, fic, wt);
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

