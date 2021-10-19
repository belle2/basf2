/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPReconstruction/TOPReconstructorModule.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>

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
    addParam("minTime", m_minTime,
             "lower limit for photon time [ns] (if minTime >= maxTime use the default from DB)", 0.0);
    addParam("maxTime", m_maxTime,
             "upper limit for photon time [ns] (if minTime >= maxTime use the default from DB)", 0.0);
    addParam("PDGCode", m_PDGCode,
             "PDG code of hypothesis to construct pulls (0 means: use MC truth)", 211);
    addParam("deltaRayModeling", m_deltaRayModeling,
             "include (True) or exclude (False) delta-ray modeling in log likelihood calculation", false);
    addParam("pTCut", m_pTCut,
             "pT cut to suppress badly extrapolated tracks that cannot reach TOP counter", 0.27);
    addParam("TOPDigitCollectionName", m_topDigitCollectionName,
             "Name of the collection of TOPDigits", string(""));
    addParam("TOPLikelihoodCollectionName", m_topLikelihoodCollectionName,
             "Name of the produced collection of TOPLikelihoods", string(""));
    addParam("TOPPullCollectionName", m_topPullCollectionName, "Name of the collection of produced TOPPulls", string(""));
  }


  void TOPReconstructorModule::initialize()
  {
    // input

    m_digits.isRequired(m_topDigitCollectionName);
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_barHits.isOptional();
    m_recBunch.isOptional();

    // output

    m_likelihoods.registerInDataStore(m_topLikelihoodCollectionName);
    m_likelihoods.registerRelationTo(m_extHits);
    m_likelihoods.registerRelationTo(m_barHits);
    m_tracks.registerRelationTo(m_likelihoods);

    m_topPulls.registerInDataStore(m_topPullCollectionName, DataStore::c_DontWriteOut);
    m_tracks.registerRelationTo(m_topPulls, DataStore::c_Event, DataStore::c_DontWriteOut);
  }


  void TOPReconstructorModule::event()
  {
    // clear output collections

    m_likelihoods.clear();
    m_topPulls.clear();

    // check bunch reconstruction status and do the reconstruction:
    // - if object exists and bunch is found (collision data w/ bunch finder in the path)
    // - if object doesn't exist (cosmic data and other cases w/o bunch finder in the path)

    if (m_recBunch.isValid()) {
      if (not m_recBunch->isReconstructed()) return;
    }

    // set time window in which photon hits are accepted for likelihood determination

    TOPRecoManager::setTimeWindow(m_minTime, m_maxTime);

    // reconstruct track-by-track and store the results

    for (const auto& track : m_tracks) {

      const TOPTrack trk(track, m_topDigitCollectionName);
      if (not trk.isValid()) continue; // track missed the bars
      if (trk.getTransverseMomentum() < m_pTCut) continue; // badly extrapolated

      auto* topLL = m_likelihoods.appendNew();
      track.addRelationTo(topLL);
      topLL->addRelationTo(trk.getExtHit());
      topLL->addRelationTo(trk.getBarHit());

      int pdgCode = (m_PDGCode != 0) ? m_PDGCode : trk.getPDGCode(); // PDG code used for providing pulls
      std::set<int> nfotSet;    // to x-check if the number of photons differs between particle hypotheses
      std::set<double> nbkgSet; // to x-check if the background differs between particle hypotheses
      int flag = 1;
      for (const auto& chargedStable : Const::chargedStableSet) {
        const PDFConstructor pdfConstructor(trk, chargedStable);
        pdfConstructor.switchDeltaRayPDF(m_deltaRayModeling);
        if (not pdfConstructor.isValid()) {
          flag = -1;
          continue;
        }
        auto LL = pdfConstructor.getLogL();
        topLL->set(chargedStable, LL.numPhotons, LL.logL, LL.expPhotons, pdfConstructor.getExpectedBkgPhotons());

        nfotSet.insert(LL.numPhotons);
        nbkgSet.insert(pdfConstructor.getExpectedBkgPhotons());

        if (abs(chargedStable.getPDGCode()) == abs(pdgCode)) {
          for (const auto& p : pdfConstructor.getPulls()) {
            auto* pull = m_topPulls.appendNew(p.pixelID, p.time, p.peakT0 + p.ttsT0, p.sigma, p.phiCer, p.wt);
            track.addRelationTo(pull);
          }
        }
      }
      topLL->setFlag(flag);

      if (nfotSet.size() > 1) B2ERROR("Bug in TOP::PDFConstructor: number of photons differs between particle hypotheses");
      if (nbkgSet.size() > 1) B2ERROR("Bug in TOP::PDFConstructor: estimated background differs between particle hypotheses");
    }

  }

} // end Belle2 namespace

