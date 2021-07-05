/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/modules/collectors/TOPCommonT0LLCollectorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction_cpp/TOPTrack.h>
#include <top/reconstruction_cpp/PDFConstructor.h>
#include <top/reconstruction_cpp/TOPRecoManager.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// root
#include <TRandom.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2F.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCommonT0LLCollector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCommonT0LLCollectorModule::TOPCommonT0LLCollectorModule()
  {
    // set module description and processing properties
    setDescription("A collector for common T0 calibration with dimuons or Bhabha's using "
                   "neg. log likelihood minimization (method LL)");
    setPropertyFlags(c_ParallelProcessingCertified);

    // module parameters
    addParam("bunchesPerSSTclk", m_bunchesPerSSTclk,
             "number of bunches per SST clock period", 24);
    addParam("numBins", m_numBins, "number of bins of the search region", 200);
    addParam("timeRange", m_timeRange,
             "time range in which to search for the minimum [ns]", 10.0);
    addParam("sigmaSmear", m_sigmaSmear,
             "sigma in [ns] for additional smearing of PDF", 0.0);
    addParam("sample", m_sample,
             "sample type: one of dimuon or bhabha", std::string("dimuon"));
    addParam("deltaEcms", m_deltaEcms,
             "c.m.s energy window (half size) if sample is dimuon or bhabha", 0.1);
    addParam("dr", m_dr, "cut on POCA in r", 2.0);
    addParam("dz", m_dz, "cut on POCA in abs(z)", 4.0);
    addParam("minZ", m_minZ,
             "minimal local z of extrapolated hit", -130.0);
    addParam("maxZ", m_maxZ,
             "maximal local z of extrapolated hit", 130.0);
    addParam("pdfOption", m_pdfOption,
             "PDF option, one of 'rough', 'fine', 'optimal'", std::string("rough"));
  }


  void TOPCommonT0LLCollectorModule::prepare()
  {
    // input collections

    m_digits.isRequired();
    m_tracks.isRequired();
    m_extHits.isRequired();
    m_recBunch.isRequired();

    // bunch time separation

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_bunchTimeSep = geo->getNominalTDC().getSyncTimeBase() / m_bunchesPerSSTclk;

    // Parse PDF option

    if (m_pdfOption == "rough") {
      m_PDFOption = PDFConstructor::c_Rough;
    } else if (m_pdfOption == "fine") {
      m_PDFOption = PDFConstructor::c_Fine;
    } else if (m_pdfOption == "optimal") {
      m_PDFOption = PDFConstructor::c_Optimal;
    } else {
      B2ERROR("Unknown PDF option '" << m_pdfOption << "'");
    }

    // set track selector

    if (m_sample == "dimuon" or m_sample == "bhabha") {
      m_selector = TrackSelector(m_sample);
      m_selector.setDeltaEcms(m_deltaEcms);
      m_selector.setCutOnPOCA(m_dr, m_dz);
      m_selector.setCutOnLocalZ(m_minZ, m_maxZ);
    } else {
      B2ERROR("Invalid sample type '" << m_sample << "'");
    }

    // create and register histograms

    double tmin = -m_timeRange / 2;
    double tmax =  m_timeRange / 2;
    for (unsigned i = 0; i < c_numSets; i++) {
      string name = "chi2_set" + to_string(i);
      auto h = new TH1D(name.c_str(), "chi2 scan; t0 [ns]; chi2",  m_numBins, tmin, tmax);
      registerObject<TH1D>(name, h);
      m_names.push_back(name);
    }

    auto h1 = new TH1F("tracks_per_set", "tracks per sample; sample number; num tracks",
                       c_numSets, 0, c_numSets);
    registerObject<TH1F>("tracks_per_set", h1);

    auto h2 = new TH1F("numHits", "Number of photons per slot",
                       c_numModules, 0.5, c_numModules + 0.5);
    h2->SetXTitle("slot number");
    h2->SetYTitle("hits per slot");
    registerObject<TH1F>("numHits", h2);

    auto h3 = new TH2F("timeHits", "Photon times vs. boardstacks",
                       c_numModules * 4, 0.5, c_numModules + 0.5, 200, 0.0, 20.0);
    h3->SetXTitle("slot number");
    h3->SetYTitle("time [ns]");
    registerObject<TH2F>("timeHits", h3);

    // this one is needed primarely to pass bunch time separation to the algorithm,
    // since DB interface doesn't work there
    auto h4 = new TH1F("offset", "current offset from input files; offset [ns]",
                       200, -m_bunchTimeSep / 2, m_bunchTimeSep / 2);
    registerObject<TH1F>("offset", h4);

  }


  void TOPCommonT0LLCollectorModule::collect()
  {
    // bunch must be reconstructed

    if (not m_recBunch.isValid()) return;
    if (not m_recBunch->isReconstructed()) return;

    TOPRecoManager::setDefaultTimeWindow();
    double timeMin = TOPRecoManager::getMinTime();
    double timeMax = TOPRecoManager::getMaxTime();

    // correct-back digits for common T0

    double T0 = 0;
    if (m_commonT0->isCalibrated()) T0 = m_commonT0->getT0();
    for (auto& digit : m_digits) {
      if (digit.isCommonT0Calibrated()) digit.subtractT0(-T0);
      if (digit.hasStatus(TOPDigit::c_BunchOffsetSubtracted)) digit.subtractT0(-m_recBunch->getAverageOffset());
    }

    // loop over reconstructed tracks, make a selection and accumulate log likelihoods

    int ntra = 0;
    for (const auto& track : m_tracks) {

      // track selection
      TOPTrack trk(track);
      if (not trk.isValid()) continue;

      if (not m_selector.isSelected(trk)) continue;

      // construct PDF
      PDFConstructor pdfConstructor(trk, m_selector.getChargedStable(), m_PDFOption);
      if (not pdfConstructor.isValid()) continue;

      // minimization procedure: accumulate
      int sub = gRandom->Integer(c_numSets); // generate sub-sample number
      auto h = getObjectPtr<TH1D>(m_names[sub]);
      for (int ibin = 0; ibin < h->GetNbinsX(); ibin++) {
        double t0 = h->GetBinCenter(ibin + 1);
        double chi = h->GetBinContent(ibin + 1);
        chi += -2 * pdfConstructor.getLogL(t0, m_sigmaSmear).logL;
        h->SetBinContent(ibin + 1, chi);
      }
      auto h1 = getObjectPtr<TH1F>("tracks_per_set");
      h1->Fill(sub);

      // fill histograms of hits
      auto h2 = getObjectPtr<TH1F>("numHits");
      auto h3 = getObjectPtr<TH2F>("timeHits");
      for (const auto& digit : m_digits) {
        if (digit.getHitQuality() != TOPDigit::c_Good) continue;
        if (digit.getModuleID() != trk.getModuleID()) continue;
        if (digit.getTime() < timeMin) continue;
        if (digit.getTime() > timeMax) continue;
        h2->Fill(digit.getModuleID());
        int bs = digit.getBoardstackNumber();
        h3->Fill((digit.getModuleID() * 4 + bs - 1.5) / 4.0 , digit.getTime());
      }
      ntra++;
    }

    // fill just another control histogram

    if (ntra > 0) {
      auto h4 = getObjectPtr<TH1F>("offset");
      h4->Fill(m_recBunch->getCurrentOffset());
    }

  }

}
