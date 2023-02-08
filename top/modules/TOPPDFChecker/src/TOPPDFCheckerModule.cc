/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <top/modules/TOPPDFChecker/TOPPDFCheckerModule.h>

// TOP headers.
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>

// DataStore classes
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>
#include <iostream>


using namespace std;
using namespace ROOT::Math;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  ///                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPPDFChecker);

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPDFCheckerModule::TOPPDFCheckerModule() : HistoModule()

  {
    // set module description
    setDescription("Module for checking analytic PDF used in likelihood calculation");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("minTime", m_minTime,
             "histogram lower bound in time [ns]", 0.0);
    addParam("maxTime", m_maxTime,
             "histogram upper bound in time [ns]", 50.0);
    addParam("numBins", m_numBins,
             "histogram number of bins in time", 1000);

  }

  void TOPPDFCheckerModule::defineHisto()
  {

    // time vs. pixel ID
    m_hits = new TH2F("hits", "photon hits", 512, 0.5, 512.5,
                      m_numBins, m_minTime, m_maxTime);
    m_hits->SetXTitle("pixel ID");
    m_hits->SetYTitle("time [ns]");

    m_pdf = new TH2F("pdf", "PDF", 512, 0.5, 512.5,
                     m_numBins, m_minTime, m_maxTime);
    m_pdf->SetXTitle("pixel ID");
    m_pdf->SetYTitle("time [ns]");

    // time vs pixel column
    m_hitsCol = new TH2F("hitsCol", "photon hits", 64, 0.5, 64.5,
                         m_numBins, m_minTime, m_maxTime);
    m_hitsCol->SetXTitle("pixel column");
    m_hitsCol->SetYTitle("time [ns]");

    m_pdfCol = new TH2F("pdfCol", "PDF", 64, 0.5, 64.5,
                        m_numBins, m_minTime, m_maxTime);
    m_pdfCol->SetXTitle("pixel column");
    m_pdfCol->SetYTitle("time [ns]");

  }

  void TOPPDFCheckerModule::initialize()
  {
    // Register histograms (calls back defineHisto)
    REG_HISTOGRAM;

    // input

    m_digits.isRequired();
    m_tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isRequired();

    StoreArray<TOPBarHit> barHits;
    barHits.isRequired();

  }


  void TOPPDFCheckerModule::event()
  {
    TOPRecoManager::setTimeWindow(m_minTime, m_maxTime);
    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // loop over reconstructed tracks, call reconstruction and fill histograms

    int numTrk = 0;
    for (const auto& track : m_tracks) {
      TOPTrack trk(track);
      if (not trk.isValid()) continue;
      if (not trk.getMCParticle()) continue;
      if (not trk.getBarHit()) continue;

      auto chargedStable = Const::chargedStableSet.find(abs(trk.getPDGCode()));
      if (chargedStable == Const::invalidParticle) continue;

      PDFConstructor pdfConstructor(trk, chargedStable, PDFConstructor::c_Fine);
      if (not pdfConstructor.isValid()) continue;
      numTrk++;

      // average values - to print in terminate()
      const auto& module = geo->getModule(trk.getModuleID());
      m_avrgMomentum += module.momentumToLocal(trk.getExtHit()->getMomentum());
      m_avrgPosition += static_cast<XYZVector>(module.pointToLocal(static_cast<XYZPoint>(trk.getExtHit()->getPosition())));
      m_numTracks++;
      m_slotIDs.emplace(trk.getModuleID());
      m_PDGCodes.emplace(trk.getPDGCode());

      // histogram photons in a slot crossed by the track
      for (const auto& digit : m_digits) {
        if (digit.getModuleID() == trk.getModuleID() and digit.getTime() < m_maxTime) {
          if (not isFromThisParticle(digit, trk.getMCParticle())) continue;
          m_hits->Fill(digit.getPixelID(), digit.getTime());
          m_hitsCol->Fill(digit.getPixelCol(), digit.getTime());
        }
      }

      // histogram PDF using MC approach
      for (const auto& signalPDF : pdfConstructor.getSignalPDF()) {
        int pixelID = signalPDF.getPixelID();
        for (const auto& peak : signalPDF.getPDFPeaks()) {
          double numPhot = pdfConstructor.getExpectedSignalPhotons() * peak.nph;
          double sigma = sqrt(peak.wid);
          for (int i = 0; i < gRandom->Poisson(numPhot); i++) {
            double time = gRandom->Gaus(peak.t0, sigma);
            m_pdf->Fill(pixelID, time);
            int pixelCol = (pixelID - 1) % 64 + 1;
            m_pdfCol->Fill(pixelCol, time);
          }
        }
      }

    }

    if (numTrk == 0) {
      B2WARNING("No track hitting the bars");
    } else if (numTrk > 1) {
      B2WARNING("More than one track hits the bars");
    }

  }


  void TOPPDFCheckerModule::terminate()
  {
    m_avrgPosition *= 1.0 / m_numTracks;
    m_avrgMomentum *= 1.0 / m_numTracks;

    cout << "Average particle parameters at entrance to bar (in local frame):" << endl;
    cout << " slot ID: ";
    for (auto slot : m_slotIDs) cout << slot << " ";
    cout << endl;
    cout << " PDG code: ";
    for (auto pdg : m_PDGCodes) cout << pdg << " ";
    cout << endl;
    cout << " position: x = " << m_avrgPosition.X()
         << ", y = " << m_avrgPosition.Y()
         << ", z = " << m_avrgPosition.Z() << endl;
    cout << " momentum: p = " << m_avrgMomentum.R()
         << ", theta = " << m_avrgMomentum.Theta() / Unit::deg
         << ", phi = " << m_avrgMomentum.Phi() / Unit::deg << endl;
    cout << "Number of particles: " << m_numTracks << endl;
    cout << "Photons per particle: simulation = " << m_hits->GetSum() / m_numTracks
         << ", PDF = " << m_pdf->GetSum() / m_numTracks << endl;

  }


  bool TOPPDFCheckerModule::isFromThisParticle(const TOPDigit& digit,
                                               const MCParticle* particle)
  {
    const auto particles = digit.getRelationsWith<MCParticle>();
    for (unsigned i = 0; i < particles.size(); ++i) {
      if (particles[i] == particle and particles.weight(i) > 0) return true;
    }
    return false;
  }


} // end Belle2 namespace

