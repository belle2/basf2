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
#include <tracking/dataobjects/ExtHit.h>
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
    setDescription("This module makes an analytic PDF available in "
                   "a store array TOPPDFCollections, related from Tracks");

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
             "upper limit for photon time [ns] (default if minTime >= maxTime)", 0.0);
    /* not implemented
    addParam("writeNPdfs", m_writeNPdfs,
       "Write out the PDF for the first N events. -1 is for all.", 0);
    addParam("writeNPulls", m_writeNPulls,
       "Write out pulls for the first N events. -1 is for all.", 0);
    */
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("fine"));
    addParam("pdgCodes", m_pdgCodes,
             "PDG codes of charged stable particles for which to construct PDF. "
             "Empty list means all charged stable particles.",
             m_pdgCodes);
  }


  TOPPDFDebuggerModule::~TOPPDFDebuggerModule()
  {
  }


  void TOPPDFDebuggerModule::initialize()
  {
    // input
    m_digits.isRequired();
    m_tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();

    // output
    m_pdfCollection.registerInDataStore();
    m_tracks.registerRelationTo(m_pdfCollection);

    // check for module debug level
    if (getLogConfig().getLogLevel() == LogConfig::c_Debug) {
      m_debugLevel = getLogConfig().getDebugLevel();
    }

    // particle hypotheses
    if (m_pdgCodes.empty()) {
      for (const auto& part : Const::chargedStableSet) {
        m_pdgCodes.push_back(abs(part.getPDGCode()));
        m_masses.push_back(part.getMass());
      }
    } else {
      for (auto& pdg : m_pdgCodes) pdg = abs(pdg);
      for (auto pdg : m_pdgCodes) {
        auto part = Const::ChargedStable(pdg); //throws runtime error for invalid pdg
        m_masses.push_back(part.getMass());
      }
    }

    // Configure TOP detector
    TOPconfigure config;
    if (m_debugLevel > 0) config.print();

    // Parse PDF option
    if (m_pdfOption == "rough") {
      m_PDFOption = TOPreco::c_Rough;
    } else if (m_pdfOption == "fine") {
      m_PDFOption = TOPreco::c_Fine;
    } else if (m_pdfOption == "optimal") {
      m_PDFOption = TOPreco::c_Optimal;
    } else {
      B2ERROR("TOPPDFDebuggerModule: unknown PDF option '" << m_pdfOption << "'");
    }

  }

  void TOPPDFDebuggerModule::beginRun()
  {
  }

  void TOPPDFDebuggerModule::event()
  {
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // create reconstruction object
    TOPreco reco(m_masses.size(), m_masses.data(), m_minBkgPerBar, m_scaleN0);
    reco.setHypID(m_pdgCodes.size(), m_pdgCodes.data());
    reco.setPDFoption(m_PDFOption);

    // set time limit for photons lower than that given by TDC range (optional)
    if (m_maxTime > m_minTime) {
      reco.setTimeWindow(m_minTime, m_maxTime);
    }

    // add photons
    for (const auto& digit : m_digits) {
      if (digit.getHitQuality() == TOPDigit::EHitQuality::c_Good) {
        reco.addData(digit.getModuleID(), digit.getPixelID(),
                     digit.getTime(), digit.getTimeError());
      }
    }

    // reconstruct track-by-track and store the results
    for (const auto& track : m_tracks) {
      // construct TOPtrack from mdst track
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;

      // add this vector of vector of triplets to the TOPPDFCollection
      TOPPDFCollection* topPDFColl = m_pdfCollection.appendNew();
      const auto& module = geo->getModule(trk.getModuleID());
      topPDFColl->setLocalPositionMomentum(
        module.pointToLocal(trk.getPosition()),
        module.momentumToLocal(trk.getMomentum()),
        trk.getModuleID()
      );
      for (unsigned i = 0; i < m_pdgCodes.size(); i++) {
        double mass = m_masses[i];
        int iPDGCode = m_pdgCodes[i];
        reco.setMass(mass);
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

            auto tp = TOPPDFCollection::Gaussian(position, width, numPhotons);
            channelPDFCollection.at(pixelID - 1).push_back(tp);

          } // end loop over peaks in the pdf for this pixel
        } // end loop over pixels

        topPDFColl->addHypothesisPDF(channelPDFCollection, iPDGCode);
      }
      if (reco.getFlag() == 1) track.addRelationTo(topPDFColl);
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

