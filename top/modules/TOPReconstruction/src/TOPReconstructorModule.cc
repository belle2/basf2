/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <top/modules/TOPReconstruction/TOPReconstructorModule.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/geometry/TOPGeometryPar.h>

// framework aux
#include <framework/logging/Logger.h>
#include <set>
#include <map>

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPReconstructor);

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

    // sort tracks by module ID

    std::unordered_multimap<int, const TOPTrack*> topTracks; // tracks sorted by top modules
    for (const auto& track : m_tracks) {
      auto* trk = new TOPTrack(track, m_topDigitCollectionName);
      if (trk->isValid() and trk->getTransverseMomentum() > m_pTCut) {
        topTracks.emplace(trk->getModuleID(), trk);
      } else {
        delete trk;
      }
    }

    // reconstruct module-by-module

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    for (unsigned moduleID = 1; moduleID <= geo->getNumModules(); moduleID++) {

      // make a vector of PDF collections for a given TOP module

      std::vector<PDFCollection> pdfCollections;
      const auto& range = topTracks.equal_range(moduleID);
      for (auto it = range.first; it != range.second; ++it) {
        const auto* trk = it->second;
        PDFCollection collection(*trk, m_deltaRayModeling);
        if (collection.isValid) pdfCollections.push_back(collection);
      }
      if (pdfCollections.empty()) continue;

      // add most probable PDF's of other tracks if present

      if (pdfCollections.size() > 1) {
        std::vector<int> lastMostProbables;
        for (int iter = 0; iter < 10; iter++) {
          std::vector<int> mostProbables;
          for (auto& collection : pdfCollections) { // set most probable PDF
            collection.setMostProbable();
            mostProbables.push_back(collection.mostProbable->getHypothesis().getPDGCode());
          }
          if (mostProbables == lastMostProbables) break;
          else lastMostProbables = mostProbables;

          for (auto& collection : pdfCollections) collection.clearPDFOther(); // reset
          for (auto& collection : pdfCollections) { // append
            for (auto& other : pdfCollections) {
              if (&other == &collection) continue;
              collection.appendPDFOther(other.mostProbable);
            }
          }
        } // loop: iter
      }

      // determine and save log likelihoods

      for (auto& collection : pdfCollections) {
        auto* topLL = m_likelihoods.appendNew();
        const auto* trk = collection.topTrack;
        const auto* track = trk->getTrack();
        track->addRelationTo(topLL);
        topLL->addRelationTo(trk->getExtHit());
        topLL->addRelationTo(trk->getBarHit());

        int pdgCode = (m_PDGCode != 0) ? m_PDGCode : trk->getPDGCode(); // PDG code used for providing pulls
        std::set<int> nfotSet;    // to x-check if the number of photons is the same for all particle hypotheses
        std::set<double> nbkgSet; // to x-check if the background is the same for all particle hypotheses

        for (const auto* pdfConstructor : collection.PDFs) {
          const auto& chargedStable = pdfConstructor->getHypothesis();
          auto LL = pdfConstructor->getLogL();
          auto expBkgPhotons = pdfConstructor->getExpectedBkgPhotons();
          topLL->set(chargedStable, LL.numPhotons, LL.logL, LL.expPhotons, expBkgPhotons);

          nfotSet.insert(LL.numPhotons);
          nbkgSet.insert(expBkgPhotons);

          if (abs(chargedStable.getPDGCode()) == abs(pdgCode)) {
            for (const auto& p : pdfConstructor->getPulls()) {
              auto* pull = m_topPulls.appendNew(p.pixelID, p.time, p.peakT0 + p.ttsT0, p.sigma, p.phiCer, p.wt);
              track->addRelationTo(pull);
            }
          }
        }
        topLL->setFlag(1);

        if (nfotSet.size() > 1) B2ERROR("Bug in TOP::PDFConstructor: number of photons differs between particle hypotheses");
        if (nbkgSet.size() > 1) B2ERROR("Bug in TOP::PDFConstructor: estimated background differs between particle hypotheses");
      }

      for (auto& collection : pdfCollections) collection.deletePDFs();

    } // loop: moduleID

    for (auto& x : topTracks) delete x.second;

  }

} // end Belle2 namespace

