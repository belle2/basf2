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
#include <top/modules/TOPCosmicT0Finder/TOPCosmicT0FinderModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPtrack.h>
#include <top/reconstruction/TOPconfigure.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>
#include <top/dataobjects/TOPBarHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPTimeZero.h>

// Root
#include <TH1F.h>

using namespace std;

namespace Belle2 {
  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCosmicT0Finder)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCosmicT0FinderModule::TOPCosmicT0FinderModule() : Module()

  {
    // set module description
    setDescription("Event T0 finder for global cosmic runs");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("useIncomingTrack", m_useIncomingTrack,
             "if true, use incoming track, otherwise use outcoming track", true);
    addParam("minHits", m_minHits,
             "minimal number of detected photons in a module", (unsigned) 10);
    addParam("applyT0", m_applyT0, "if true, subtract T0 in TOPDigits", true);
    addParam("numBins", m_numBins, "number of bins time range is divided to", 200);
    addParam("timeRange", m_timeRange, "time range in which to search [ns]", 10.0);
    addParam("sigma", m_sigma, "additional time spread added to PDF [ns]", 0.0);
    addParam("saveHistograms", m_saveHistograms,
             "save histograms to TOPTimeZero", false);
    addParam("bkgPerModule", m_bkgPerModule,
             "average number of background hits per module", 1.0);
  }

  TOPCosmicT0FinderModule::~TOPCosmicT0FinderModule()
  {
  }

  void TOPCosmicT0FinderModule::initialize()
  {

    // input

    StoreArray<TOPDigit> topDigits;
    topDigits.isRequired();

    StoreArray<Track> tracks;
    tracks.isRequired();

    StoreArray<ExtHit> extHits;
    extHits.isRequired();

    StoreArray<TOPBarHit> barHits;
    barHits.isOptional();

    // output

    StoreArray<TOPTimeZero> timeZeros;
    timeZeros.registerInDataStore();
    timeZeros.registerRelationTo(tracks);
    timeZeros.registerRelationTo(extHits);
    timeZeros.registerRelationTo(barHits);

    // Step size of the search region

    m_dt = m_timeRange / m_numBins;

    // Configure TOP detector

    TOPconfigure config;

  }

  void TOPCosmicT0FinderModule::beginRun()
  {
  }

  void TOPCosmicT0FinderModule::event()
  {

    // select track: if several, choose highest momentum track

    StoreArray<Track> tracks;
    const Track* selectedTrack = 0;
    double p0 = 0;
    int moduleID = 0;
    for (const auto& track : tracks) {
      const auto extHits = track.getRelationsWith<ExtHit>();
      const ExtHit* extHit0 = 0;
      for (const auto& extHit : extHits) {
        if (abs(extHit.getPdgCode()) != 13) continue;
        if (extHit.getDetectorID() != Const::EDetector::TOP) continue;
        double dot = extHit.getPosition() * extHit.getMomentum();
        if (m_useIncomingTrack) {
          if (dot > 0) continue;
          if (!extHit0) extHit0 = &extHit;
          if (extHit.getTOF() < extHit0->getTOF()) extHit0 = &extHit;
        } else {
          if (dot < 0) continue;
          if (!extHit0) extHit0 = &extHit;
          if (extHit.getTOF() > extHit0->getTOF()) extHit0 = &extHit;
        }
      }
      if (!extHit0) continue;
      double p = extHit0->getMomentum().Mag();
      if (p > p0) {
        p0 = p;
        selectedTrack = &track;
        moduleID = abs(extHit0->getCopyID());
      }
    }
    if (!selectedTrack) return;

    TOPtrack topTrack(selectedTrack, moduleID, Const::muon);
    if (!topTrack.isValid()) return;
    if (moduleID == 0) {
      B2ERROR("moduleID == 0");
      return;
    }

    // select photons: at least m_minHits

    StoreArray<TOPDigit> topDigits;
    std::vector<const TOPDigit*> selDigits;
    for (const auto& digit : topDigits) {
      if (digit.getModuleID() != topTrack.getModuleID()) continue;
      if (digit.getHitQuality() != TOPDigit::c_Good) continue;
      selDigits.push_back(&digit);
    }
    if (selDigits.empty() or selDigits.size() < m_minHits) return;

    // create reconstruction object and set various options

    int Nhyp = 1;
    double mass = Const::muon.getMass();
    TOPreco reco(Nhyp, &mass, m_bkgPerModule);
    reco.setPDFoption(TOPreco::c_Rough);

    // add photon hits to reconstruction object

    for (const auto& digit : selDigits) {
      if (digit->getHitQuality() == TOPDigit::c_Good)
        reco.addData(digit->getModuleID(), digit->getPixelID(), digit->getTime(),
                     digit->getTimeError());
    }

    // construct PDF

    reco.reconstruct(topTrack);
    if (reco.getFlag() != 1) return;

    // find rough t0

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    const auto& tdc = geo->getNominalTDC();
    double timeMin = tdc.getTimeMin();
    double timeMax = tdc.getTimeMax();
    double binSize = 1.0; // ns
    int nbins = int((timeMax - timeMin) / binSize);
    binSize = (timeMax - timeMin) / nbins;
    TH1F pdf("pdf", "PDF", nbins, timeMin, timeMax);
    double tminPDF = timeMax;
    for (int pix = 1; pix <= 512; pix++) {
      for (int k = 0; k < reco.getNumofPDFPeaks(pix); k++) {
        float pos = 0;
        float wid = 0;
        float nph = 0;
        reco.getPDFPeak(pix, k, pos, wid, nph);
        pdf.Fill(pos, nph);
        if (pos < tminPDF) tminPDF = pos;
      }
    }

    double tminFot = selDigits[0]->getTime();
    double tmaxFot = tminFot;
    for (const auto& digit : selDigits) {
      double t = digit->getTime();
      tminFot = std::min(tminFot, t);
      tmaxFot = std::max(tmaxFot, t);
    }
    double t0Sken = tminFot - tminPDF - 10.0;
    double t0Rough = 0;
    double chi2Min = 0;
    bool start = true;
    double bkg = std::max(m_bkgPerModule / (timeMax - timeMin) * binSize, 0.001);
    while (t0Sken < tmaxFot - tminPDF) {
      double chi2 = 0;
      for (const auto& digit : selDigits) {
        double t = digit->getTime();
        double f = pdf.GetBinContent(pdf.FindBin(t - t0Sken)) + bkg;
        chi2 += -2 * log(f);
      }
      if (start or chi2 < chi2Min) {
        start = false;
        chi2Min = chi2;
        t0Rough = t0Sken;
      }
      t0Sken += binSize;
    }

    // calculate log likelihoods

    m_chi2s.clear();
    m_t0.clear();
    double t0min = t0Rough - m_timeRange / 2;
    double t0max = t0Rough + m_timeRange / 2;
    m_t0.push_back(t0min + m_dt / 2);
    for (int i = 1; i < m_numBins; i++) {
      m_t0.push_back(m_t0.back() + m_dt);
    }
    timeMin += t0Rough;
    timeMax += t0Rough;
    for (double t0 : m_t0) {
      m_chi2s.push_back(-2 * reco.getLogL(t0, timeMin, timeMax, m_sigma));
    }

    // find minimum

    int i0 = 0;
    for (int i = 0; i < m_numBins; i++) {
      if (m_chi2s[i] < m_chi2s[i0]) i0 = i;
    }
    double chi2_min = m_chi2s[i0];
    for (auto& chi2 : m_chi2s) chi2 -= chi2_min;

    PointWithError t0 = getParabolicMinimum(i0);
    if (t0.value < t0min or t0.value > t0max or t0.error == 0) return; // out of range

    StoreArray<TOPTimeZero> timeZeros;
    auto* timeZero = timeZeros.appendNew(topTrack.getModuleID(), t0.value, t0.error,
                                         reco.getNumOfPhotons());
    timeZero->addRelationTo(topTrack.getTrack());
    timeZero->addRelationTo(topTrack.getExtHit());
    if (topTrack.getBarHit()) timeZero->addRelationTo(topTrack.getBarHit());

    // save histograms

    if (m_saveHistograms) {
      std::string name;
      name = "chi2_" + to_string(m_num);
      TH1F chi2(name.c_str(), "muon chi2 = -2 logL vs. t0", m_numBins, t0min, t0max);
      chi2.SetXTitle("t_{0} [ns]");
      chi2.SetYTitle("log L_{#mu}");
      for (int i = 0; i < m_numBins; i++) chi2.SetBinContent(i + 1, m_chi2s[i]);

      name = "hits_" + to_string(m_num);
      TH1F hits(name.c_str(), "time distribution of photons (t0-subtracted)",
                pdf.GetNbinsX(), pdf.GetXaxis()->GetXmin(), pdf.GetXaxis()->GetXmax());
      hits.SetXTitle("time [ns]");
      for (const auto& digit : selDigits) {
        if (digit->getHitQuality() == TOPDigit::c_Good)
          hits.Fill(digit->getTime() - t0.value);
      }

      name = "pdf_" + to_string(m_num);
      pdf.SetName(name.c_str());
      pdf.SetXTitle("time [ns]");
      pdf.SetLineColor(2);
      pdf.SetOption("hist c");

      timeZero->setHistograms(chi2, pdf, hits);
      m_num++;
    }

    // subtract T0 in digits

    if (m_applyT0) {
      for (auto& digit : topDigits) {
        digit.subtractT0(t0.value);
        digit.addStatus(TOPDigit::c_EventT0Subtracted);
      }
    }

  }


  void TOPCosmicT0FinderModule::endRun()
  {
  }

  void TOPCosmicT0FinderModule::terminate()
  {
  }


  PointWithError TOPCosmicT0FinderModule::getParabolicMinimum(unsigned i0)
  {
    if (i0 == 0) i0++;
    if (i0 == m_t0.size() - 1) i0--;

    PointWithError x = getParabolicMinimum(m_chi2s[i0 - 1],
                                           m_chi2s[i0],
                                           m_chi2s[i0 + 1]);
    return PointWithError(m_t0[i0] + x.value * m_dt, x.error * m_dt);
  }


  PointWithError TOPCosmicT0FinderModule::getParabolicMinimum(double yLeft,
                                                              double yCenter,
                                                              double yRight)
  {
    double D21 = yCenter - yLeft;
    double D32 = yRight - yCenter;
    double A = (D32 - D21) / 2;
    if (A <= 0) return PointWithError(0, 0);
    double B = (D32 + D21) / 2;
    return PointWithError(- B / 2 / A, sqrt(1 / A));
  }


} // end Belle2 namespace


