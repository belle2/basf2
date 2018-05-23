/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPPDFChecker/TOPPDFCheckerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// DataStore classes
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <mdst/dataobjects/MCParticle.h>
#include <top/dataobjects/TOPBarHit.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>
#include <iostream>


using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPPDFChecker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPDFCheckerModule::TOPPDFCheckerModule() : HistoModule()

  {
    // set module description (e.g. insert text)
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

  TOPPDFCheckerModule::~TOPPDFCheckerModule()
  {
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

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<MCParticle> mcParticles;
    mcParticles.isRequired();

    StoreArray<TOPBarHit> barHits;
    barHits.isRequired();

    // Configure TOP detector

    TOPconfigure config;

  }

  void TOPPDFCheckerModule::beginRun()
  {
  }

  void TOPPDFCheckerModule::event()
  {

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // create reconstruction object and set various options

    int Nhyp = 1;
    double mass = Const::pion.getMass(); // pion used just to initialize
    TOPreco reco(Nhyp, &mass);
    reco.setPDFoption(TOPreco::c_Fine);
    reco.setTimeWindow(m_minTime, m_maxTime);

    // loop over reconstructed tracks, call reconstruction and fill histograms

    StoreArray<Track> tracks;
    int numTrk = 0;
    for (const auto& track : tracks) {
      TOPtrack trk(&track);
      if (!trk.isValid()) continue;
      if (!trk.getMCParticle()) continue;
      if (!trk.getBarHit()) continue;
      mass = trk.getMCParticle()->getMass();
      reco.setMass(mass);
      reco.reconstruct(trk);
      if (reco.getFlag() != 1) continue; // track is not in the acceptance of TOP
      numTrk++;

      // average values - to print in terminate()
      const auto& module = geo->getModule(trk.getModuleID());
      m_avrgMomentum += module.momentumToLocal(trk.getMomentum());
      m_avrgPosition += module.pointToLocal(trk.getPosition());
      m_numTracks++;
      m_slotIDs.emplace(trk.getModuleID());
      m_PDGCodes.emplace(trk.getPDGcode());

      // histogram photons in a slot crossed by the track
      StoreArray<TOPDigit> digits;
      for (const auto& digit : digits) {
        if (digit.getModuleID() == trk.getModuleID() and digit.getTime() < m_maxTime) {
          if (!isFromThisParticle(digit, trk.getMCParticle())) continue;
          m_hits->Fill(digit.getPixelID(), digit.getTime());
          m_hitsCol->Fill(digit.getPixelCol(), digit.getTime());
        }
      }

      // histogram PDF using MC approach
      for (int pixelID = 1; pixelID <= 512; pixelID++) {
        for (int peak = 0; peak < reco.getNumofPDFPeaks(pixelID); peak++) {
          float t0 = 0;
          float sigma = 0;
          float numPhot = 0;
          reco.getPDFPeak(pixelID, peak, t0, sigma, numPhot);
          for (int i = 0; i < gRandom->Poisson(numPhot); i++) {
            double time = gRandom->Gaus(t0, sigma);
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


  void TOPPDFCheckerModule::endRun()
  {
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
    cout << " momentum: p = " << m_avrgMomentum.Mag()
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

