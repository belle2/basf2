/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPPDFDebugger/TOPPDFDebuggerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>

// Hit classes
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>

// framework aux
#include <framework/logging/Logger.h>

#include <cmath>
#include <algorithm>

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
    addParam("minTime", m_minTime,
             "lower limit for photon time [ns] (default from DB used, if minTime >= maxTime)", 0.0);
    addParam("maxTime", m_maxTime,
             "upper limit for photon time [ns] (default from DB used, if minTime >= maxTime)", 0.0);
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("fine"));
    addParam("pdgCodes", m_pdgCodes,
             "PDG codes of charged stable particles for which to construct PDF. "
             "Empty list means all charged stable particles.",
             m_pdgCodes);
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
    m_associatedPDFs.registerInDataStore();
    m_digits.registerRelationTo(m_associatedPDFs);
    m_pixelData.registerInDataStore();
    m_tracks.registerRelationTo(m_pixelData);

    // particle hypotheses
    if (m_pdgCodes.empty()) {
      for (const auto& part : Const::chargedStableSet) {
        m_chargedStables.push_back(part);
      }
    } else {
      for (auto pdg : m_pdgCodes) {
        auto part = Const::ChargedStable(abs(pdg)); //throws runtime error for invalid pdg
        m_chargedStables.push_back(part);
      }
    }

    // Parse PDF option
    if (m_pdfOption == "rough") {
      m_PDFOption = PDFConstructor::c_Rough;
    } else if (m_pdfOption == "fine") {
      m_PDFOption = PDFConstructor::c_Fine;
    } else if (m_pdfOption == "optimal") {
      m_PDFOption = PDFConstructor::c_Optimal;
    } else {
      B2ERROR("TOPPDFDebuggerModule: unknown PDF option '" << m_pdfOption << "'");
    }

  }


  void TOPPDFDebuggerModule::event()
  {
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    TOPRecoManager::setTimeWindow(m_minTime, m_maxTime);

    // reconstruct track-by-track and store the results

    for (const auto& track : m_tracks) {
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      // add this vector of vector of triplets to the TOPPDFCollection
      TOPPDFCollection* topPDFColl = m_pdfCollection.appendNew();
      const auto& module = geo->getModule(trk.getModuleID());
      topPDFColl->setLocalPositionMomentum(module.pointToLocal(trk.getExtHit()->getPosition()),
                                           module.momentumToLocal(trk.getExtHit()->getMomentum()),
                                           trk.getModuleID());

      TOPPixelLikelihood* topPLkhs = m_pixelData.appendNew();
      topPLkhs->setModuleID(trk.getModuleID());

      for (const auto& chargedStable : m_chargedStables) {
        const PDFConstructor pdfConstructor(trk, chargedStable, m_PDFOption, PDFConstructor::c_Full);
        if (not pdfConstructor.isValid()) {
          B2ERROR("PDFConstructor found not valid - bug in reconstruction code?"
                  << LogVar("PDG", chargedStable.getPDGCode()));
          continue;
        }

        // associate PDF peaks with photons using S-plot technique
        associatePDFPeaks(pdfConstructor);

        // collection of gaussian_t's for each pixel
        TOPPDFCollection::modulePDF_t channelPDFCollection;

        // store PDF peaks in the collection
        for (const auto& signalPDF : pdfConstructor.getSignalPDF()) {
          int pixelID = signalPDF.getPixelID();
          for (const auto& peak : signalPDF.getPDFPeaks()) {
            float position = peak.t0;
            float width = sqrt(peak.wid);
            float numPhotons = pdfConstructor.getExpectedSignalPhotons() * peak.nph;
            auto tp = TOPPDFCollection::Gaussian(position, width, numPhotons);
            channelPDFCollection.at(pixelID - 1).push_back(tp);
          } // end loop over peaks in the pdf for this pixel
        } // end loop over pixels

        topPDFColl->addHypothesisPDF(channelPDFCollection, chargedStable.getPDGCode());

        // Initialize logL and sfot pixel arrays
        TOPPixelLikelihood::PixelArray_t pixLogLs, pixSigPhots;
        pixLogLs.fill(0);
        pixSigPhots.fill(0);

        const auto& pixelLogLs = pdfConstructor.getPixelLogLs(0);
        for (size_t i = 0; i < std::min(pixelLogLs.size(), pixLogLs.size()); i++) {
          pixLogLs[i] = pixelLogLs[i].logL;
          pixSigPhots[i] = pixelLogLs[i].expPhotons;
        }

        topPLkhs->addHypothesisLikelihoods(pixLogLs, chargedStable.getPDGCode());
        topPLkhs->addHypothesisSignalPhotons(pixSigPhots, chargedStable.getPDGCode());
      }
      track.addRelationTo(topPDFColl);
      track.addRelationTo(topPLkhs);

    } // end loop over tracks

  }


  void TOPPDFDebuggerModule::associatePDFPeaks(const PDFConstructor& pdfConstructor)
  {

    const auto& signalPDFs = pdfConstructor.getSignalPDF();
    auto signalPhotons = pdfConstructor.getExpectedSignalPhotons();
    auto bkgPhotons = pdfConstructor.getExpectedBkgPhotons();
    auto deltaPhotons = pdfConstructor.getExpectedDeltaPhotons();
    int PDGCode = pdfConstructor.getHypothesis().getPDGCode();

    for (const auto& digit : m_digits) {
      if (digit.getModuleID() != pdfConstructor.getModuleID()) continue;
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      int pixelID = digit.getPixelID();
      unsigned index = pixelID - 1;
      if (index >= signalPDFs.size()) continue;
      const auto& signalPDF = signalPDFs[index];
      const auto* tts = signalPDF.getTTS();
      const auto& pdfPeaks = signalPDF.getPDFPeaks();
      const auto& extraInfos = signalPDF.getPDFExtraInfo();
      if (extraInfos.size() != pdfPeaks.size()) {
        B2ERROR("associatePDFPeaks: sizes of PDFPeaks and ExtraInfo's differ");
        continue;
      }

      auto* associatedPDF = m_associatedPDFs.appendNew(PDGCode);
      digit.addRelationTo(associatedPDF);

      double bkgLevel = pdfConstructor.getBackgroundPDF()->getPDFValue(pixelID) * bkgPhotons;
      associatedPDF->setBackgroundWeight(bkgLevel);
      double deltaLevel = pdfConstructor.getDeltaRayPDF().getPDFValue(pixelID, digit.getTime()) * deltaPhotons;
      associatedPDF->setDeltaRayWeight(deltaLevel);

      float time = digit.getTime();
      float timeErr = digit.getTimeError();

      for (size_t k = 0; k < pdfPeaks.size(); k++) {
        const auto& pdfPeak = pdfPeaks[k];
        const auto& pdfExtra = extraInfos[k];
        TOPAssociatedPDF::PDFPeak peak;
        peak.position = pdfPeak.t0;
        peak.width = sqrt(pdfPeak.wid);
        peak.numPhotons = pdfPeak.nph * signalPhotons;
        float wt = 0;
        for (const auto& gaus : tts->getTTS()) {
          float sig2 = peak.width * peak.width + gaus.sigma * gaus.sigma + timeErr * timeErr;
          float x = pow(time - peak.position - gaus.position, 2) / sig2;
          if (x > 20) continue;
          wt += peak.numPhotons * gaus.fraction / sqrt(2 * M_PI * sig2) * exp(-x / 2);
        }
        if (wt > 0) {
          peak.fic = pdfPeak.fic;
          peak.e = pdfExtra.e;
          peak.sige = pdfExtra.sige;
          peak.nx = abs(pdfExtra.Nxm) + abs(pdfExtra.Nxb) + abs(pdfExtra.Nxe);
          peak.ny = abs(pdfExtra.Nym) + abs(pdfExtra.Nyb) + abs(pdfExtra.Nye);
          peak.nxm = abs(pdfExtra.Nxm);
          peak.nym = abs(pdfExtra.Nym);
          peak.nxe = abs(pdfExtra.Nxe);
          peak.nye = abs(pdfExtra.Nye);
          peak.xd = pdfExtra.xD;
          peak.yd = pdfExtra.yD;
          peak.type = pdfExtra.type;
          peak.kxe = pdfExtra.kxE;
          peak.kye = pdfExtra.kyE;
          peak.kze = pdfExtra.kzE;
          peak.kxd = pdfExtra.kxD;
          peak.kyd = pdfExtra.kyD;
          peak.kzd = pdfExtra.kzD;
          associatedPDF->appendPeak(peak, wt);
        }
      }
    }

  }

} // end Belle2 namespace

