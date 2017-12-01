/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Boqun Wang                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/HistoModule.h>
#include <top/modules/TOPDataQualityOnline/TOPDataQualityOnlineModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <framework/pcore/RbTuple.h>
#include <top/dataobjects/TOPDigit.h>
#include <utility>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "TDirectory.h"
#include "TCanvas.h"
#include "TGraph.h"
#include <boost/format.hpp>

using namespace std;
using boost::format;

namespace Belle2 {
  using namespace TOP;
  REG_MODULE(TOPDataQualityOnline)

  TOPDataQualityOnlineModule::TOPDataQualityOnlineModule() : HistoModule()
  {
    setDescription("TOP online monitoring module");
    setPropertyFlags(c_ParallelProcessingCertified);
    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("TOP"));
    addParam("ADCCutLow", m_ADCCutLow, "lower bound of ADC cut", 100);
    addParam("ADCCutHigh", m_ADCCutHigh, "higher bound of ADC cut", 2048);
    addParam("PulseWidthCutLow",  m_PulseWidthCutLow, "lower bound of PulseWidth cut", 3);
    addParam("PulseWidthCutHigh", m_PulseWidthCutHigh, "higher bound of PulseWidth cut", 10);
    addParam("TDCParticleLow", m_TDCParticleLow, "lower bound of particle TDC", -50);
    addParam("TDCParticleHigh", m_TDCParticleHigh, "Higher bound of particle TDC", 50);
    addParam("TDCCalLow", m_TDCCalLow, "lower bound of cal TDC", 0);
    addParam("TDCCalHigh", m_TDCCalHigh, "Higher bound of cal TDC", 0);
    addParam("TDCLaserLow", m_TDCLaserLow, "lower bound of laser TDC", -200);
    addParam("TDCLaserHigh", m_TDCLaserHigh, "Higher bound of laser TDC", -50);
    addParam("VerboseMode", m_verbose, "use more histograms", 0);
  }


  void TOPDataQualityOnlineModule::defineHisto()
  {
    // Create a separate histogram directory and cd into it.
    TDirectory* oldDir = gDirectory;
    TDirectory* newDir = NULL;
    newDir = oldDir->mkdir(m_histogramDirectoryName.c_str());
    newDir->cd();

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();
    m_numModules = geo->getNumModules();

    m_particleHits = new TH1F("particle_hits", "Number of particle hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_laserHits = new TH1F("laser_hits", "Number of laser hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_calHits = new TH1F("cal_hits", "Number of cal hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_otherHits = new TH1F("other_hits", "Number of other hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_particleHits->SetOption("LIVE");
    m_laserHits->SetOption("LIVE");
    m_calHits->SetOption("LIVE");
    m_otherHits->SetOption("LIVE");

    m_particleHitsMean = new TH1F("particle_hits_mean", "Mean of number of particle hits per bar", m_numModules, 0.5,
                                  m_numModules + 0.5);
    m_laserHitsMean = new TH1F("laser_hits_mean", "Mean of number of laser hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_calHitsMean = new TH1F("cal_hits_mean", "Mean of number of cal hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_otherHitsMean = new TH1F("other_hits_mean", "Mean of number of other hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_particleHitsMean->SetOption("LIVE");
    m_laserHitsMean->SetOption("LIVE");
    m_calHitsMean->SetOption("LIVE");
    m_otherHitsMean->SetOption("LIVE");

    m_particleHitsRMS = new TH1F("particle_hits_rms", "RMS of number of particle hits per bar", m_numModules, 0.5,
                                 m_numModules + 0.5);
    m_laserHitsRMS = new TH1F("laser_hits_rms", "RMS of number of laser hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_calHitsRMS = new TH1F("cal_hits_rms", "RMS of number of cal hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_otherHitsRMS = new TH1F("other_hits_rms", "RMS of number of other hits per bar", m_numModules, 0.5, m_numModules + 0.5);
    m_particleHitsRMS->SetOption("LIVE");
    m_laserHitsRMS->SetOption("LIVE");
    m_calHitsRMS->SetOption("LIVE");
    m_otherHitsRMS->SetOption("LIVE");

    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;
      string name = str(format("all_channel_hits_%1%") % (module));
      string title = str(format("Number of all hits by channel of module #%1%") % (module));
      string name1 = str(format("good_channel_hits_%1%") % (module));
      string title1 = str(format("Number of good hits by channel of module #%1%") % (module));
      string name2 = str(format("bad_channel_hits_%1%") % (module));
      string title2 = str(format("Number of bad hits by channel of module #%1%") % (module));
      string name3 = str(format("particle_channel_hits_%1%") % (module));
      string title3 = str(format("Number of particle hits by channel of module #%1%") % (module));
      TH1F* h3 = new TH1F(name3.c_str(), title2.c_str(), 512, 0.5, 512.5);
      h3->SetOption("LIVE");
      m_channelParticleHits.push_back(h3);
      if (m_verbose == 1) { // in verbose mode
        TH1F* h = new TH1F(name.c_str(), title.c_str(), 512, 0.5, 512.5);
        TH1F* h1 = new TH1F(name1.c_str(), title1.c_str(), 512, 0.5, 512.5);
        TH1F* h2 = new TH1F(name2.c_str(), title2.c_str(), 512, 0.5, 512.5);
        m_channelAllHits.push_back(h);
        m_channelGoodHits.push_back(h1);
        m_channelBadHits.push_back(h2);
      }
    }
    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;
      string name = str(format("all_hits_xy_%1%") % (module));
      string title = str(format("Number of hits in x-y for module #%1%") % (module));
      string name1 = str(format("laser_hits_xy_%1%") % (module));
      string title1 = str(format("Number of laser hits in x-y for module #%1%") % (module));
      TH2F* h1 = new TH2F(name1.c_str(), title1.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      h1->SetOption("LIVE");
      m_laserHitsXY.push_back(h1);
      if (m_verbose == 1) { // in verbose mode
        TH2F* h = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
        m_allHitsXY.push_back(h);
      }
    }
    for (int i = 0; i < m_numModules; i++) {
      int module = i + 1;
      string name = str(format("all_TDC_%1%") % (module));
      string title = str(format("TDC distribution for module #%1%") % (module));
      string name1 = str(format("all_TDC_mean_%1%") % (module));
      string title1 = str(format("TDC mean distribution for module #%1%") % (module));
      string name2 = str(format("all_TDC_RMS_%1%") % (module));
      string title2 = str(format("TDC RMS distribution for module #%1%") % (module));
      TH1F* h = new TH1F(name.c_str(), title.c_str(), 500, -250, 250);
      h->SetOption("LIVE");
      m_allTdc.push_back(h);
      if (m_verbose == 1) { // in verbose mode
        TH2F* h1 = new TH2F(name1.c_str(), title1.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
        TH2F* h2 = new TH2F(name2.c_str(), title2.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
        m_allTdcMean.push_back(h1);
        m_allTdcRMS.push_back(h2);
      }
    }

    if (m_verbose == 1) { // in verbose mode
      m_allHits = new TH1F("all_hits", "Number of all hits per bar", m_numModules, 0.5, m_numModules + 0.5);
      m_goodHits = new TH1F("good_hits", "Number of good hits per bar", m_numModules, 0.5, m_numModules + 0.5);
      m_badHits = new TH1F("bad_hits", "Number of bad hits per bar", m_numModules, 0.5, m_numModules + 0.5);
      m_goodHitsMean = new TH1F("good_hits_mean", "Mean of number of good hits per bar", m_numModules, 0.5, m_numModules + 0.5);
      m_badHitsMean = new TH1F("bad_hits_mean", "Mean of number of bad hits per bar", m_numModules, 0.5, m_numModules + 0.5);
      m_goodHitsRMS = new TH1F("good_hits_rms", "RMS of number of good hits per bar", m_numModules, 0.5, m_numModules + 0.5);
      m_badHitsRMS = new TH1F("bad_hits_rms", "RMS of number of bad hits per bar", m_numModules, 0.5, m_numModules + 0.5);

      for (int i = 0; i < m_numModules; i++) {
        int module = i + 1;
        string name = str(format("all_hits_%1%") % (module));
        string title = str(format("Number of all hits per event of module #%1%") % (module));
        string name1 = str(format("good_hits_%1%") % (module));
        string title1 = str(format("Number of good hits per event of module #%1%") % (module));
        string name2 = str(format("bad_hits_%1%") % (module));
        string title2 = str(format("Number of bad hits per event of module #%1%") % (module));
        TH1F* h = new TH1F(name.c_str(), title.c_str(), 200, 0, 400);
        TH1F* h1 = new TH1F(name1.c_str(), title1.c_str(), 200, 0, 400);
        TH1F* h2 = new TH1F(name2.c_str(), title2.c_str(), 200, 0, 400);
        m_slotAllHits.push_back(h);
        m_slotGoodHits.push_back(h1);
        m_slotBadHits.push_back(h2);
      }

      for (int i = 0; i < m_numModules; i++) {
        int module = i + 1;
        string name = str(format("hit_quality_%1%") % (module));
        string title = str(format("Hit quality of module #%1%") % (module));
        TH1F* h = new TH1F(name.c_str(), title.c_str(), 8, 0.5, 8.5);
        m_hitFlag.push_back(h);
      }

      for (int i = 0; i < m_numModules; i++) {
        int module = i + 1;
        string name = str(format("all_ADC_%1%") % (module));
        string title = str(format("ADC distribution for module #%1%") % (module));
        string name1 = str(format("all_ADC_mean_%1%") % (module));
        string title1 = str(format("ADC mean distribution for module #%1%") % (module));
        string name2 = str(format("all_ADC_RMS_%1%") % (module));
        string title2 = str(format("ADC RMS distribution for module #%1%") % (module));
        TH1F* h = new TH1F(name.c_str(), title.c_str(), 1000, 0.5, 1000.5);
        TH2F* h1 = new TH2F(name1.c_str(), title1.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
        TH2F* h2 = new TH2F(name2.c_str(), title2.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
        m_allAdc.push_back(h);
        m_allAdcMean.push_back(h1);
        m_allAdcRMS.push_back(h2);
      }
      for (int i = 0; i < m_numModules; i++) {
        int module = i + 1;
        string name = str(format("particle_hits_%1%") % (module));
        string title = str(format("Number of particle hits per event of module #%1%") % (module));
        string name1 = str(format("laser_hits_%1%") % (module));
        string title1 = str(format("Number of laser hits per event of module #%1%") % (module));
        string name2 = str(format("cal_hits_%1%") % (module));
        string title2 = str(format("Number of cal hits per event of module #%1%") % (module));
        string name3 = str(format("other_hits_%1%") % (module));
        string title3 = str(format("Number of other hits per event of module #%1%") % (module));
        TH1F* h = new TH1F(name.c_str(), title.c_str(), 200, 0, 400);
        TH1F* h1 = new TH1F(name1.c_str(), title1.c_str(), 200, 0, 400);
        TH1F* h2 = new TH1F(name2.c_str(), title2.c_str(), 200, 0, 400);
        TH1F* h3 = new TH1F(name3.c_str(), title3.c_str(), 200, 0, 400);
        m_slotParticleHits.push_back(h);
        m_slotLaserHits.push_back(h1);
        m_slotCalHits.push_back(h2);
        m_slotOtherHits.push_back(h3);
      }
    }

    oldDir->cd();
  }

  void TOPDataQualityOnlineModule::initialize()
  {
    REG_HISTOGRAM;
    m_digits.isRequired();
  }

  void TOPDataQualityOnlineModule::beginRun()
  {
    m_particleHits->Reset();
    m_laserHits->Reset();
    m_calHits->Reset();
    m_otherHits->Reset();

    m_particleHitsMean->Reset();
    m_laserHitsMean->Reset();
    m_calHitsMean->Reset();
    m_otherHitsMean->Reset();

    m_particleHitsRMS->Reset();
    m_laserHitsRMS->Reset();
    m_calHitsRMS->Reset();
    m_otherHitsRMS->Reset();

    for (int i = 0; i < m_numModules; i++) {
      m_channelParticleHits[i]->Reset();
      m_laserHitsXY[i]->Reset();
      m_allTdc[i]->Reset();
    }

    if (m_verbose == 1) { // in verbose mode
      m_allHits->Reset();
      m_goodHits->Reset();
      m_badHits->Reset();
      m_goodHitsMean->Reset();
      m_badHitsMean->Reset();
      m_goodHitsRMS->Reset();
      m_badHitsRMS->Reset();

      for (int i = 0; i < m_numModules; i++) {
        m_channelAllHits[i]->Reset();
        m_channelGoodHits[i]->Reset();
        m_channelBadHits[i]->Reset();
        m_allHitsXY[i]->Reset();
        m_allTdcMean[i]->Reset();
        m_allTdcRMS[i]->Reset();
        m_slotAllHits[i]->Reset();
        m_slotGoodHits[i]->Reset();
        m_slotBadHits[i]->Reset();
        m_hitFlag[i]->Reset();
        m_allAdc[i]->Reset();
        m_allAdcMean[i]->Reset();
        m_allAdcRMS[i]->Reset();
        m_slotParticleHits[i]->Reset();
        m_slotLaserHits[i]->Reset();
        m_slotCalHits[i]->Reset();
        m_slotOtherHits[i]->Reset();
      }
    }
  }

  int TOPDataQualityOnlineModule::flagHit(const TOPDigit& digit)
  {
    int flag = 0;
    if (digit.getPulseHeight() < m_ADCCutLow || digit.getPulseHeight() > m_ADCCutHigh) flag += 1;
    if (digit.getPulseWidth() < m_PulseWidthCutLow || digit.getPulseWidth() > m_PulseWidthCutHigh) flag += 2;
    return flag;
  }

  void TOPDataQualityOnlineModule::event()
  {
    vector<int> all_hit(m_numModules, 0), good_hit(m_numModules, 0), bad_hit(m_numModules, 0);
    vector<int> particle_hit(m_numModules, 0), laser_hit(m_numModules, 0), cal_hit(m_numModules, 0), other_hit(m_numModules, 0);

    int m_nhits = 0;
    map<int, double> refTdcMap;
    vector<int> pixelHit, moduleHit, colHit, rowHit, isCal;
    vector<double> rawTimeHit;

    for (const auto& digit : m_digits) {
      int i = digit.getModuleID() - 1;
      int col = digit.getPixelCol();
      int row = digit.getPixelRow();
      if (i < 0 || i >= m_numModules) {
        B2ERROR("Invalid module ID found in TOPDigits: ID = " << i + 1);
        continue;
      }
      all_hit[i]++;

      if (digit.getASICChannel() == 0) {
        isCal.push_back(1);
        int reducedPixelId = (digit.getPixelID() - 1) / 8 + digit.getModuleID() * 100;
        if (refTdcMap.count(reducedPixelId) == 0)
          refTdcMap[reducedPixelId] = digit.getRawTime();
      } else {
        isCal.push_back(0);
      }
      rawTimeHit.push_back(digit.getRawTime());
      pixelHit.push_back(digit.getPixelID());
      moduleHit.push_back(digit.getModuleID());
      colHit.push_back(digit.getPixelCol());
      rowHit.push_back(digit.getPixelRow());
      m_nhits++;

      if (m_verbose == 1) { // in verbose mode
        m_allHits->Fill(digit.getModuleID());
        m_channelAllHits[i]->Fill(digit.getPixelID());
        m_allAdc[i]->Fill(digit.getPulseHeight());
        double nhits = m_allHitsXY[i]->GetBinContent(col, row);
        double adc_mean = m_allAdcMean[i]->GetBinContent(col, row);
        double adc_rms = m_allAdcRMS[i]->GetBinContent(col, row);
        double adc_mean2 = adc_rms * adc_rms + adc_mean * adc_mean;
        double tdc_mean = m_allTdcMean[i]->GetBinContent(col, row);
        double tdc_rms = m_allTdcRMS[i]->GetBinContent(col, row);
        double tdc_mean2 = tdc_rms * tdc_rms + tdc_mean * tdc_mean;

        adc_mean = (nhits * adc_mean + digit.getPulseHeight()) / (nhits + 1);
        adc_mean2 = (nhits * adc_mean2 + digit.getPulseHeight() * digit.getPulseHeight()) / (nhits + 1);
        tdc_mean = (nhits * tdc_mean + digit.getRawTime()) / (nhits + 1);
        tdc_mean2 = (nhits * tdc_mean2 + digit.getRawTime() * digit.getRawTime()) / (nhits + 1);

        m_allAdcMean[i]->SetBinContent(col, row, adc_mean);
        m_allAdcRMS[i]->SetBinContent(col, row, sqrt(adc_mean2 - adc_mean * adc_mean));
        m_allTdcMean[i]->SetBinContent(col, row, tdc_mean);
        m_allTdcRMS[i]->SetBinContent(col, row, sqrt(tdc_mean2 - tdc_mean * tdc_mean));
        m_allHitsXY[i]->Fill(col, row);
        int flag = flagHit(digit);
        if (flag == 0) { // good hit
          m_goodHits->Fill(digit.getModuleID());
          m_channelGoodHits[i]->Fill(digit.getPixelID());
        } else { // bad hit
          m_badHits->Fill(digit.getModuleID());
          m_channelBadHits[i]->Fill(digit.getPixelID());
          m_hitFlag[i]->Fill(flag);
        }
      }

      int flag = flagHit(digit);
      if (flag == 0) good_hit[i]++;
      else bad_hit[i]++;
    }
    for (int i = 0; i < m_nhits; i++) {
      int reducedPixelId = (pixelHit[i] - 1) / 8 + moduleHit[i] * 100;
      double refTdc;
      if (refTdcMap.count(reducedPixelId) > 0)
        refTdc = refTdcMap[reducedPixelId];
      else
        refTdc = -99999;
      if (refTdc > 0 && isCal[i] == 0)
        m_allTdc[moduleHit[i] - 1]->Fill(rawTimeHit[i] - refTdc);

      double digit_tdc = rawTimeHit[i] - refTdc;
      if (isCal[i] == 0 && digit_tdc > m_TDCParticleLow && digit_tdc < m_TDCParticleHigh) { // particle hits
        m_particleHits->Fill(moduleHit[i]);
        particle_hit[moduleHit[i] - 1]++;
        m_channelParticleHits[moduleHit[i] - 1]->Fill(pixelHit[i]);
      } else if (isCal[i] == 0 && digit_tdc > m_TDCLaserLow && digit_tdc < m_TDCLaserHigh) { // laser hits
        m_laserHits->Fill(moduleHit[i]);
        laser_hit[moduleHit[i] - 1]++;
        m_laserHitsXY[moduleHit[i] - 1]->Fill(colHit[i], rowHit[i]);
      } else if (isCal[i] == 1) { // cal hits
        m_calHits->Fill(moduleHit[i]);
        cal_hit[moduleHit[i] - 1]++;
      } else {
        m_otherHits->Fill(moduleHit[i]);
        other_hit[moduleHit[i] - 1]++;
      }
    }
    for (int i = 0; i < m_numModules; i++) {
      if (m_verbose == 1) { // in verbose mode
        if (all_hit[i] > 0) m_slotAllHits[i]->Fill(all_hit[i]);
        if (good_hit[i] > 0) m_slotGoodHits[i]->Fill(good_hit[i]);
        if (bad_hit[i] > 0) m_slotBadHits[i]->Fill(bad_hit[i]);
        if (particle_hit[i] > 0) m_slotParticleHits[i]->Fill(particle_hit[i]);
        if (laser_hit[i] > 0) m_slotLaserHits[i]->Fill(laser_hit[i]);
        if (cal_hit[i] > 0) m_slotCalHits[i]->Fill(cal_hit[i]);
        if (other_hit[i] > 0) m_slotOtherHits[i]->Fill(other_hit[i]);
        m_goodHitsMean->SetBinContent(i + 1, m_slotGoodHits[i]->GetMean());
        m_badHitsMean->SetBinContent(i + 1, m_slotBadHits[i]->GetMean());
        m_goodHitsRMS->SetBinContent(i + 1, m_slotGoodHits[i]->GetRMS());
        m_badHitsRMS->SetBinContent(i + 1, m_slotBadHits[i]->GetRMS());
      }
      if (particle_hit[i] > 0) m_particleHitsCounter.Add(i, particle_hit[i]);
      if (laser_hit[i] > 0) m_laserHitsCounter.Add(i, laser_hit[i]);
      if (cal_hit[i] > 0) m_calHitsCounter.Add(i, cal_hit[i]);
      if (other_hit[i] > 0) m_otherHitsCounter.Add(i, other_hit[i]);

      m_particleHitsMean->SetBinContent(i + 1, m_particleHitsCounter.GetMean(i));
      m_laserHitsMean->SetBinContent(i + 1, m_laserHitsCounter.GetMean(i));
      m_calHitsMean->SetBinContent(i + 1, m_calHitsCounter.GetMean(i));
      m_otherHitsMean->SetBinContent(i + 1, m_otherHitsCounter.GetMean(i));

      m_particleHitsRMS->SetBinContent(i + 1, m_particleHitsCounter.GetRMS(i));
      m_laserHitsRMS->SetBinContent(i + 1, m_laserHitsCounter.GetRMS(i));
      m_calHitsRMS->SetBinContent(i + 1, m_calHitsCounter.GetRMS(i));
      m_otherHitsRMS->SetBinContent(i + 1, m_otherHitsCounter.GetRMS(i));
    }
    m_iEvent++;
    return;
  }


  void TOPDataQualityOnlineModule::endRun()
  {
  }


  void TOPDataQualityOnlineModule::terminate()
  {
  }
} // end Belle2 namespace
