/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Jan Strube, Sam Cunliffe                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Module manager


// Own include
#include <top/modules/TOPPDFDebugger/TOPPDFDebuggerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// Hit classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPPDFCollection.h>
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

  REG_MODULE(TOPPDFDebugger)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPDFDebuggerModule::TOPPDFDebuggerModule() : Module()
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
    addParam("minTime", m_minTime,
             "lower limit for photon time [ns] (default if minTime >= maxTime)", 0.0);
    addParam("maxTime", m_maxTime,
             "time limit for photons [ns] (0 = use full TDC range)", 0.0);
    addParam("writeNPdfs", m_writeNPdfs,
             "Write out the PDF for the first N events. -1 is for all.", 0);
    addParam("writeNPulls", m_writeNPulls,
             "Write out pulls for the first N events. -1 is for all.", 0);

    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_masses[i] = 0;}
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {m_pdgCodes[i] = 0;}

  }


  TOPPDFDebuggerModule::~TOPPDFDebuggerModule()
  {
  }


  void TOPPDFDebuggerModule::initialize()
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
    StoreArray<TOPPDFCollection> pdfCollection;
    pdfCollection.registerInDataStore();
    tracks.registerRelationTo(pdfCollection);

    // check for module debug level
    if (getLogConfig().getLogLevel() == LogConfig::c_Debug) {
      m_debugLevel = getLogConfig().getDebugLevel();
    }

    // Initialize masses
    for (const auto& part : Const::chargedStableSet) {
      m_masses[part.getIndex()] = part.getMass();
      m_pdgCodes[part.getIndex()] = abs(part.getPDGCode());
    }

    // Configure TOP detector
    TOPconfigure config;
    if (m_debugLevel > 0) config.print();
  }

  void TOPPDFDebuggerModule::beginRun()
  {
  }

  void TOPPDFDebuggerModule::event()
  {
    // output: pdfs
    StoreArray<TOPPDFCollection> pdfCollection;

    // create reconstruction object
    TOPreco reco(Const::ChargedStable::c_SetSize, m_masses, m_minBkgPerBar, m_scaleN0);
    reco.setHypID(Const::ChargedStable::c_SetSize, m_pdgCodes);
    reco.setPDFoption(TOPreco::c_Fine);

    // set time limit for photons lower than that given by TDC range (optional)
    if (m_maxTime > m_minTime) {
      reco.setTimeWindow(m_minTime, m_maxTime);
    }

    // add photons
    StoreArray<TOPDigit> digits;
    for (const auto& digit : digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good) {
        reco.addData(digit.getModuleID(), digit.getPixelID(),
                     digit.getTime(), digit.getTimeError());
      }
    }

    // reconstruct track-by-track and store the results
    StoreArray<Track> tracks;
    for (const auto& track : tracks) {
      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // add this vector of vector of triplets to the TOPPDFCollection
      TOPPDFCollection* topPDFColl = pdfCollection.appendNew();
      for (size_t ihypothesis = 0; ihypothesis < Const::ChargedStable::c_SetSize; ++ihypothesis) {
        double iMass = m_masses[ihypothesis];
        int iPDGCode = m_pdgCodes[ihypothesis];
        reco.setMass(iMass);
        reco.reconstruct(trk); // will run reconstruction only for this mass hypothesis
        if (reco.getFlag() != 1) break; // track is not in the acceptance of TOP

        // collection of gaussian_t's for each pixel
        TOPPDFCollection::modulePDF_t channelPDFCollection;

        // the mean, width and normalisation for each peak in the pdf
        float position = 0, width = 0, numPhotons = 0;
        for (int pixelID = 1; pixelID <= static_cast<int>(channelPDFCollection.size()); pixelID++) {
          for (int k = 0; k < reco.getNumofPDFPeaks(pixelID); k++) {

            // get this peak
            reco.getPDFPeak(pixelID, k, position, width, numPhotons);

            auto tp = make_tuple(position, width, numPhotons);
            channelPDFCollection.at(pixelID - 1).push_back(tp);

          } // end loop over peaks in the pdf for this pixel
        } // end loop over pixels

        topPDFColl->addHypothesisPDF(channelPDFCollection, iPDGCode);
      }
      track.addRelationTo(topPDFColl);
    } // end loop over tracks
    ++m_iEvent;
  }

  void TOPPDFDebuggerModule::endRun()
  {
  }

  void TOPPDFDebuggerModule::terminate()
  {
  }



} // end Belle2 namespace

