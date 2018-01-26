/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Jan Strube, Sam Cunliffe                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager


// Own include
#include <top/modules/TOPReconstruction_PDF/TOPReconstructorPDFModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPPDFCollection.h>
#include <top/dataobjects/TOPSmallestPullCollection.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>

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

  REG_MODULE(TOPReconstructorPDF)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPReconstructorPDFModule::TOPReconstructorPDFModule() : Module()
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
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use full TDC range)", 51.2);
    addParam("PDGCode", m_PDGCode,
             "PDG code of hypothesis to construct pulls (0 means: use MC truth)",
             211);
    addParam("writeNPdfs", m_writeNPdfs,
             "Write out the PDF for the first N events. -1 is for all.", 0);
    addParam("writeNPulls", m_writeNPulls,
             "Write out pulls for the first N events. -1 is for all.", 0);

    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_masses[i] = 0;}
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_pdgCodes[i] = 0;}

  }


  TOPReconstructorPDFModule::~TOPReconstructorPDFModule()
  {
  }


  void TOPReconstructorPDFModule::initialize()
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

    StoreArray<TOPPDFCollection> pdfCollection;
    pdfCollection.registerInDataStore();
    tracks.registerRelationTo(pdfCollection);

    StoreArray<TOPSmallestPullCollection> pullCollection;
    pullCollection.registerInDataStore();
    tracks.registerRelationTo(pullCollection);

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

  void TOPReconstructorPDFModule::beginRun()
  {
  }

  void TOPReconstructorPDFModule::event()
  {
    // output: log likelihoods
    StoreArray<TOPLikelihood> likelihoods;
    StoreArray<TOPPDFCollection> pdfCollection;
    StoreArray<TOPSmallestPullCollection> pullCollection;

    // create reconstruction object
    TOPreco reco(Const::ChargedStable::c_SetSize, m_masses, m_minBkgPerBar, m_scaleN0);
    reco.setHypID(Const::ChargedStable::c_SetSize, m_pdgCodes);

    // set time limit for photons lower than that given by TDC range (optional)
    if (m_maxTime > 0) reco.setTmax(m_maxTime);

    // add photons
    StoreArray<TOPDigit> digits;
    for (const auto& digit : digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good) {
        int returnCode = reco.addData(digit.getModuleID(), digit.getPixelID(),
                                      digit.getTime(), digit.getTimeError());
        if (returnCode == 0)
          B2ERROR("TOPReconstructor: Could not add module ID:" << digit.getModuleID()
                  << " pixel: " << digit.getPixelID()
                  << " time: " << digit.getTime());
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

      // normal reconstruction
      reco.getLogL(Const::ChargedStable::c_SetSize, logl, estPhot, nphot);
      double estBkg = reco.getExpectedBG();

      // store results
      TOPLikelihood* topL = likelihoods.appendNew(reco.getFlag(), nphot,
                                                  logl, estPhot, estBkg);
      // make relations:
      track.addRelationTo(topL);
      topL->addRelationTo(trk.getExtHit());
      topL->addRelationTo(trk.getBarHit());

      // write out the pdf if needed
      if (m_writeNPdfs < 0 or m_iEvent < m_writeNPdfs) {
        // add this vector of vector of triplets to the TOPPDFCollection
        TOPPDFCollection* topPDFColl = pdfCollection.appendNew();
        for (size_t ihypothesis = 0; ihypothesis < Const::ChargedStable::c_SetSize; ++ihypothesis) {
          double iMass = m_masses[ihypothesis];
          int iPDGCode = m_pdgCodes[ihypothesis];
          reco.redoPDF(iMass);

          // collection of gaussian_t's for each pixel
          vector<vector<TOPPDFCollection::gaussian_t>> channelPDFCollection(512);

          // the mean, width and normalisation for each peak in the pdf
          float position = 0, width = 0, numPhotons = 0;
          for (int pixelID = 1; pixelID <= 512; pixelID++) {
            for (int k = 0; k < reco.getNumOfPDFPeaks(pixelID); k++) {

              // get this peak
              reco.getPDFPeak(pixelID, k, position, width, numPhotons);

              // save the triplet of values using nice pythonic c++11
              auto tp = vector<float> {position, width, numPhotons};
              channelPDFCollection.at(pixelID - 1).push_back(tp);

            } // end loop over peaks in the pdf for this pixel
          } // end loop over pixels

          topPDFColl->addHypothesisPDFSample(channelPDFCollection, iPDGCode);
        }
        track.addRelationTo(topPDFColl);
      } // closes if statement about optional writing out pdfs

      // write out pulls for each digit if needed
      if (m_writeNPulls < 0 or m_iEvent < m_writeNPdfs) {
        reco.redoPDF(m_masses[1]);
        // FIXME as above

        float position = 0, width = 0, numPhotons = 0; //, this_pull = 0;
        vector<float> smallest_pulls(digits.getEntries(), 10000);
        vector<float> pulls_from_smallest_diffs(digits.getEntries(), 10000);
        vector<double> pulls_from_weightedAve(digits.getEntries(), 10000);

        // FIXME this does not loop over the digits for this track under
        // whatever hypothesis but ALL digits...
        for (int i = 0; i < digits.getEntries(); i++) {
          int pixelID = digits[i]->getPixelID();
          double t = digits[i]->getTime();
          // Computes a weighted average for the pdf
          double avePos = 0;
          double aveWidth = 0;
          double sumPhotons = 0;
          for (int k = 0; k < reco.getNumOfPDFPeaks(pixelID); k++) {
            // get this peak, calculate pull
            reco.getPDFPeak(pixelID, k, position, width, numPhotons);
            sumPhotons += numPhotons;
            avePos += numPhotons * position;
            aveWidth += numPhotons * width;

            float this_diff = (t - position);
            float this_pull = this_diff / width;

            // check to see if this is the smallest diff
            if (abs(this_diff) < abs(pulls_from_smallest_diffs[i])) {
              pulls_from_smallest_diffs[i] = this_pull;
            }
            // check to see if this is the smallest pull
            if (abs(this_pull) < abs(smallest_pulls[i])) {
              smallest_pulls[i] = this_pull;
            }
          } // end loop over peaks
          avePos /= sumPhotons;
          aveWidth /= sumPhotons;
          pulls_from_weightedAve[i] = (t - avePos) / aveWidth;
        } // end loop over (ALL) digits

        // FIXME should split this up for each TOPDigit and register reltation
        TOPSmallestPullCollection* tspc = pullCollection.appendNew();
        tspc->setSmallestPulls(smallest_pulls);
        tspc->setPullsFromSmallestDiff(pulls_from_smallest_diffs);
        tspc->setPullsFromWeightedAve(pulls_from_weightedAve);
        track.addRelationTo(tspc);

      } // closes if statement about optional writing out pulls

    } // end loop over tracks
    ++m_iEvent;
  }

  void TOPReconstructorPDFModule::endRun()
  {
  }

  void TOPReconstructorPDFModule::terminate()
  {
  }



} // end Belle2 namespace

