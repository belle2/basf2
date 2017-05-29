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
  REG_MODULE(TOPDataQualityOnline)

  TOPDataQualityOnlineModule::TOPDataQualityOnlineModule() : HistoModule(), m_iEvent(0)
  {
    setDescription("TOP online monitoring module");
    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("TOP"));
    addParam("ADCCutLow", m_ADC_cut_low, "lower bound of ADC cut", 100);
    addParam("ADCCutHigh", m_ADC_cut_high, "higher bound of ADC cut", 2048);
    addParam("PulseWidthCutLow",  m_PulseWidth_cut_low, "lower bound of PulseWidth cut", 3);
    addParam("PulseWidthCutHigh", m_PulseWidth_cut_high, "higher bound of PulseWidth cut", 10);
    addParam("TDCParticleLow", m_TDC_particle_low, "lower bound of particle TDC", -300);
    addParam("TDCParticleHigh", m_TDC_particle_high, "Higher bound of particle TDC", -250);
    addParam("TDCCalLow", m_TDC_cal_low, "lower bound of cal TDC", -50);
    addParam("TDCCalHigh", m_TDC_cal_high, "Higher bound of cal TDC", 50);
    addParam("TDCLaserLow", m_TDC_laser_low, "lower bound of laser TDC", 50);
    addParam("TDCLaserHigh", m_TDC_laser_high, "Higher bound of laser TDC", 100);
  }


  void TOPDataQualityOnlineModule::defineHisto()
  {
    // Create a separate histogram directory and cd into it.
    TDirectory* oldDir = gDirectory;
    oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

    m_all_hits = new TH1F("all_hits", "Number of all hits per bar", 16, 0.5, 16.5);
    m_good_hits = new TH1F("good_hits", "Number of good hits per bar", 16, 0.5, 16.5);
    m_bad_hits = new TH1F("bad_hits", "Number of bad hits per bar", 16, 0.5, 16.5);
    m_good_hits_mean = new TH1F("good_hits_mean", "Mean of number of good hits per bar", 16, 0.5, 16.5);
    m_bad_hits_mean = new TH1F("bad_hits_mean", "Mean of number of bad hits per bar", 16, 0.5, 16.5);
    m_good_hits_rms = new TH1F("good_hits_rms", "RMS of number of good hits per bar", 16, 0.5, 16.5);
    m_bad_hits_rms = new TH1F("bad_hits_rms", "RMS of number of bad hits per bar", 16, 0.5, 16.5);

    m_particle_hits = new TH1F("particle_hits", "Number of particle hits per bar", 16, 0.5, 16.5);
    m_laser_hits = new TH1F("laser_hits", "Number of laser hits per bar", 16, 0.5, 16.5);
    m_cal_hits = new TH1F("cal_hits", "Number of cal hits per bar", 16, 0.5, 16.5);
    m_other_hits = new TH1F("other_hits", "Number of other hits per bar", 16, 0.5, 16.5);

    m_particle_hits_mean = new TH1F("particle_hits_mean", "Mean of number of particle hits per bar", 16, 0.5, 16.5);
    m_laser_hits_mean = new TH1F("laser_hits_mean", "Mean of number of laser hits per bar", 16, 0.5, 16.5);
    m_cal_hits_mean = new TH1F("cal_hits_mean", "Mean of number of cal hits per bar", 16, 0.5, 16.5);
    m_other_hits_mean = new TH1F("other_hits_mean", "Mean of number of other hits per bar", 16, 0.5, 16.5);

    m_particle_hits_rms = new TH1F("particle_hits_rms", "RMS of number of particle hits per bar", 16, 0.5, 16.5);
    m_laser_hits_rms = new TH1F("laser_hits_rms", "RMS of number of laser hits per bar", 16, 0.5, 16.5);
    m_cal_hits_rms = new TH1F("cal_hits_rms", "RMS of number of cal hits per bar", 16, 0.5, 16.5);
    m_other_hits_rms = new TH1F("other_hits_rms", "RMS of number of other hits per bar", 16, 0.5, 16.5);

    for (int i = 0; i < 16; i++) {
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
      m_slot_all_hits.push_back(h);
      m_slot_good_hits.push_back(h1);
      m_slot_bad_hits.push_back(h2);
    }
    for (int i = 0; i < 16; i++) {
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
      m_slot_particle_hits.push_back(h);
      m_slot_laser_hits.push_back(h1);
      m_slot_cal_hits.push_back(h2);
      m_slot_other_hits.push_back(h3);
    }
    for (int i = 0; i < 16; i++) {
      int module = i + 1;
      string name = str(format("all_channel_hits_%1%") % (module));
      string title = str(format("Number of all hits by channel of module #%1%") % (module));
      string name1 = str(format("good_channel_hits_%1%") % (module));
      string title1 = str(format("Number of good hits by channel of module #%1%") % (module));
      string name2 = str(format("bad_channel_hits_%1%") % (module));
      string title2 = str(format("Number of bad hits by channel of module #%1%") % (module));
      string name3 = str(format("particle_channel_hits_%1%") % (module));
      string title3 = str(format("Number of particle hits by channel of module #%1%") % (module));
      TH1F* h = new TH1F(name.c_str(), title.c_str(), 512, 0.5, 512.5);
      TH1F* h1 = new TH1F(name1.c_str(), title1.c_str(), 512, 0.5, 512.5);
      TH1F* h2 = new TH1F(name2.c_str(), title2.c_str(), 512, 0.5, 512.5);
      TH1F* h3 = new TH1F(name3.c_str(), title2.c_str(), 512, 0.5, 512.5);
      m_channel_all_hits.push_back(h);
      m_channel_good_hits.push_back(h1);
      m_channel_bad_hits.push_back(h2);
      m_channel_particle_hits.push_back(h3);
    }
    for (int i = 0; i < 16; i++) {
      int module = i + 1;
      string name = str(format("hit_quality_%1%") % (module));
      string title = str(format("Hit quality of module #%1%") % (module));
      TH1F* h = new TH1F(name.c_str(), title.c_str(), 8, 0.5, 8.5);
      m_hit_quality.push_back(h);
    }
    for (int i = 0; i < 16; i++) {
      int module = i + 1;
      string name = str(format("all_hits_xy_%1%") % (module));
      string title = str(format("Number of hits in x-y for module #%1%") % (module));
      string name1 = str(format("laser_hits_xy_%1%") % (module));
      string title1 = str(format("Number of laser hits in x-y for module #%1%") % (module));
      TH2F* h = new TH2F(name.c_str(), title.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      TH2F* h1 = new TH2F(name1.c_str(), title1.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      m_all_hits_xy.push_back(h);
      m_laser_hits_xy.push_back(h1);
    }
    for (int i = 0; i < 16; i++) {
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
      m_all_ADC.push_back(h);
      m_all_ADC_mean.push_back(h1);
      m_all_ADC_RMS.push_back(h2);
    }
    for (int i = 0; i < 16; i++) {
      int module = i + 1;
      string name = str(format("all_TDC_%1%") % (module));
      string title = str(format("TDC distribution for module #%1%") % (module));
      string name1 = str(format("all_TDC_mean_%1%") % (module));
      string title1 = str(format("TDC mean distribution for module #%1%") % (module));
      string name2 = str(format("all_TDC_RMS_%1%") % (module));
      string title2 = str(format("TDC RMS distribution for module #%1%") % (module));
      TH1F* h = new TH1F(name.c_str(), title.c_str(), 1000, -499.5, 500.5);
      TH2F* h1 = new TH2F(name1.c_str(), title1.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      TH2F* h2 = new TH2F(name2.c_str(), title2.c_str(), 64, 0.5, 64.5, 8, 0.5, 8.5);
      m_all_TDC.push_back(h);
      m_all_TDC_mean.push_back(h1);
      m_all_TDC_RMS.push_back(h2);
    }
  }

  void TOPDataQualityOnlineModule::initialize()
  {
    REG_HISTOGRAM;
    StoreArray<TOPDigit>::required();
  }

  void TOPDataQualityOnlineModule::beginRun()
  {
  }

  int TOPDataQualityOnlineModule::flag_hit(const TOPDigit& digit)
  {
    int flag = 0;
    if (digit.getPulseHeight() < m_ADC_cut_low || digit.getPulseHeight() > m_ADC_cut_high) flag += 1;
    if (digit.getPulseWidth() < m_PulseWidth_cut_low || digit.getPulseWidth() > m_PulseWidth_cut_high) flag += 2;
    return flag;
  }

  void TOPDataQualityOnlineModule::event()
  {
    StoreArray<TOPDigit> digits;
    vector<int> all_hit(16, 0), good_hit(16, 0), bad_hit(16, 0);
    vector<int> particle_hit(16, 0), laser_hit(16, 0), cal_hit(16, 0), other_hit(16, 0);
    for (const auto& digit : digits) {
      m_all_hits->Fill(digit.getModuleID());
      int i = digit.getModuleID() - 1;
      int col = digit.getPixelCol();
      int row = digit.getPixelRow();
      if (i < 0 || i >= 16) {
        B2ERROR("Invalid module ID found in TOPDigits: ID = " << i);
        continue;
      }
      all_hit[i]++;
      m_channel_all_hits[i]->Fill(digit.getPixelID());
      m_all_ADC[i]->Fill(digit.getPulseHeight());
      m_all_TDC[i]->Fill(digit.getRawTime());

      double nhits = m_all_hits_xy[i]->GetBinContent(col, row);
      double adc_mean = m_all_ADC_mean[i]->GetBinContent(col, row);
      double adc_rms = m_all_ADC_RMS[i]->GetBinContent(col, row);
      double adc_mean2 = adc_rms * adc_rms + adc_mean * adc_mean;
      double tdc_mean = m_all_TDC_mean[i]->GetBinContent(col, row);
      double tdc_rms = m_all_TDC_RMS[i]->GetBinContent(col, row);
      double tdc_mean2 = tdc_rms * tdc_rms + tdc_mean * tdc_mean;

      adc_mean = (nhits * adc_mean + digit.getPulseHeight()) / (nhits + 1);
      adc_mean2 = (nhits * adc_mean2 + digit.getPulseHeight() * digit.getPulseHeight()) / (nhits + 1);
      tdc_mean = (nhits * tdc_mean + digit.getRawTime()) / (nhits + 1);
      tdc_mean2 = (nhits * tdc_mean2 + digit.getRawTime() * digit.getRawTime()) / (nhits + 1);

      m_all_ADC_mean[i]->SetBinContent(col, row, adc_mean);
      m_all_ADC_RMS[i]->SetBinContent(col, row, sqrt(adc_mean2 - adc_mean * adc_mean));
      m_all_TDC_mean[i]->SetBinContent(col, row, tdc_mean);
      m_all_TDC_RMS[i]->SetBinContent(col, row, sqrt(tdc_mean2 - tdc_mean * tdc_mean));
      m_all_hits_xy[i]->Fill(col, row);

      int flag = flag_hit(digit);
      if (flag == 0) { // good hit
        m_good_hits->Fill(digit.getModuleID());
        good_hit[i]++;
        m_channel_good_hits[i]->Fill(digit.getPixelID());
      } else { // bad hit
        m_bad_hits->Fill(digit.getModuleID());
        bad_hit[i]++;
        m_channel_bad_hits[i]->Fill(digit.getPixelID());
        m_hit_quality[i]->Fill(flag);
      }
      double digit_tdc = digit.getRawTime();
      if (digit_tdc > m_TDC_particle_low && digit_tdc < m_TDC_particle_high) { // particle hits
        m_particle_hits->Fill(digit.getModuleID());
        particle_hit[i]++;
        m_channel_particle_hits[i]->Fill(digit.getPixelID());
      } else if (digit_tdc > m_TDC_laser_low && digit_tdc < m_TDC_laser_high) { // laser hits
        m_laser_hits->Fill(digit.getModuleID());
        laser_hit[i]++;
        m_laser_hits_xy[i]->Fill(col, row);
      } else if (digit_tdc > m_TDC_cal_low && digit_tdc < m_TDC_cal_high) { // cal hits
        m_cal_hits->Fill(digit.getModuleID());
        cal_hit[i]++;
      } else {
        m_other_hits->Fill(digit.getModuleID());
        other_hit[i]++;
      }
    }
    m_iEvent++;
    for (int i = 0; i < 16; i++) {
      if (all_hit[i] > 0) m_slot_all_hits[i]->Fill(all_hit[i]);
      if (good_hit[i] > 0) m_slot_good_hits[i]->Fill(good_hit[i]);
      if (bad_hit[i] > 0) m_slot_bad_hits[i]->Fill(bad_hit[i]);
      if (particle_hit[i] > 0) m_slot_particle_hits[i]->Fill(particle_hit[i]);
      if (laser_hit[i] > 0) m_slot_laser_hits[i]->Fill(laser_hit[i]);
      if (cal_hit[i] > 0) m_slot_cal_hits[i]->Fill(cal_hit[i]);
      if (other_hit[i] > 0) m_slot_other_hits[i]->Fill(other_hit[i]);

      m_good_hits_mean->SetBinContent(i + 1, m_slot_good_hits[i]->GetMean());
      m_bad_hits_mean->SetBinContent(i + 1, m_slot_bad_hits[i]->GetMean());

      m_good_hits_rms->SetBinContent(i + 1, m_slot_good_hits[i]->GetRMS());
      m_bad_hits_rms->SetBinContent(i + 1, m_slot_bad_hits[i]->GetRMS());

      m_particle_hits_mean->SetBinContent(i + 1, m_slot_particle_hits[i]->GetMean());
      m_laser_hits_mean->SetBinContent(i + 1, m_slot_laser_hits[i]->GetMean());
      m_cal_hits_mean->SetBinContent(i + 1, m_slot_cal_hits[i]->GetMean());
      m_other_hits_mean->SetBinContent(i + 1, m_slot_other_hits[i]->GetMean());

      m_particle_hits_rms->SetBinContent(i + 1, m_slot_particle_hits[i]->GetRMS());
      m_laser_hits_rms->SetBinContent(i + 1, m_slot_laser_hits[i]->GetRMS());
      m_cal_hits_rms->SetBinContent(i + 1, m_slot_cal_hits[i]->GetRMS());
      m_other_hits_rms->SetBinContent(i + 1, m_slot_other_hits[i]->GetRMS());
    }
    return;
  }


  void TOPDataQualityOnlineModule::endRun()
  {
  }


  void TOPDataQualityOnlineModule::terminate()
  {
  }
} // end Belle2 namespace
