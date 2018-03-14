/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jan Strube                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// basf2
#include <framework/core/HistoModule.h>
#include <top/modules/TOPWaveformQualityPlotter/TOPWaveformQualityPlotterModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RbTuple.h>

// stl
#include <utility>

//ROOT
#include "TDirectory.h"
#include "TCanvas.h"
#include "TGraphErrors.h"

using namespace std;

namespace Belle2 {

  REG_MODULE(TOPWaveformQualityPlotter)

  TOPWaveformQualityPlotterModule::TOPWaveformQualityPlotterModule()
    : HistoModule()
  {
    setDescription("TOP DQM histogram module");
    addParam("histogramDirectoryName", m_histogramDirectoryName,
             "histogram directory in ROOT file", string("TOP"));
    addParam("drawWaves", m_DRAWWAVES, "option to draw waveforms", true);
    addParam("debugHistos", m_DEBUGGING, "option to draw debug histograms", true);
    addParam("noisemaps", m_NOISE, "option to draw noisemaps", false);
  }


  void TOPWaveformQualityPlotterModule::defineHisto()
  {
    TDirectory* oldDir = gDirectory;
    m_directory = oldDir->mkdir(m_histogramDirectoryName.c_str());
    m_directory->cd();
    m_samples = new TH1F("ADCvalues", "ADC values ", 100, -50, 50);
    m_samples->GetXaxis()->SetTitle("ADC Value");
    m_samples->GetYaxis()->SetTitle("Number of Samples");
    m_scrod_id = new TH1F("scrodID", "scrodID", 100, 0, 100);
    m_asic = new TH1F("IRSX", "IRSX", 4, 0, 4);
    m_carrier = new TH1F("carrier", "asic col", 4, 0, 4);
    m_asic_ch = new TH1F("asicCh", "channel", 8, 0, 8);
    m_errorFlag = new TH1F("errorFlag", "errorFlag", 1000, 0, 1000);
    m_asic_win = new TH1F("window", "window", 4, 0, 4);
    m_entries = new TH1F("entries", "entries", 100, 0, 2600);
    m_moduleID = new TH1F("moduleID", "moduleID", 16, 1, 17);
    m_pixelID = new TH1F("pixelID", "pixelID", 512, 1, 513);
    oldDir->cd();
  }


  void TOPWaveformQualityPlotterModule::initialize()
  {
    // Register histograms (calls back defineHisto)
    REG_HISTOGRAM;
    //Get Waveform from datastore
    m_waveform.isRequired();
  }


  void TOPWaveformQualityPlotterModule::basicDebuggingPlots(const TOPRawWaveform& v)
  {
    int scrodid = v.getScrodID();
    int asicid = v.getASICNumber();
    int channelid = v.getASICChannel();
    int carrierid = v.getCarrierNumber();
    m_scrod_id->Fill(scrodid);
    m_asic_ch->Fill(channelid);
    m_asic->Fill(asicid);
    m_carrier->Fill(carrierid);
    m_asic_win->Fill(v.getStorageWindow());
    m_entries->Fill(v.getWaveform().size());
    m_moduleID->Fill(v.getModuleID());
    m_pixelID->Fill(v.getPixelID());

    if (m_hitmap.find(scrodid) == m_hitmap.end()) {
      m_hitmap[scrodid] = new TH2F((string("scrod ") + to_string(scrodid) + string("Hitmap")).c_str(),
                                   (string("scrod ") + to_string(scrodid) + string("carrier vs. asic;asic;carrier")).c_str(), 4, 0, 4, 4, 0, 4);
    }
    m_hitmap[scrodid]->Fill(asicid, carrierid);
    const vector<short>& waveform = v.getWaveform();
    if (waveform.empty()) {
      return;
    }
    for (short adc : waveform) {
      m_samples->Fill(adc);
    }
  }

  void
  TOPWaveformQualityPlotterModule::drawWaveforms(const TOPRawWaveform& v)
  {
    vector<short> waveform = v.getWaveform();
    if (waveform.empty()) {
      return;
    }
    int scrodid = v.getScrodID();
    // skip broken events
    if (scrodid == 0) {
      return;
    }
    int asicNumber = v.getASICNumber();
    int carrierNumber = v.getCarrierNumber();
    int iChannel = v.getASICChannel();
    string gname = string("scrod_") + to_string(scrodid) + string("_carrier") + to_string(carrierNumber) + string("_asic") + to_string(
                     asicNumber) + to_string(iChannel);
    if (m_waveformHists[scrodid][carrierNumber][asicNumber].find(iChannel) ==
        m_waveformHists[scrodid][carrierNumber][asicNumber].end()) {
      // FIXME ? This assumes exactly 256 samples in a waveform (FE data)
      auto h = new TProfile(gname.c_str(), gname.c_str(), 256, 0, 256);
      h->Sumw2(false); // unweighted filling.
      m_waveformHists[scrodid][carrierNumber][asicNumber][iChannel] = h;
    }
    // FIXME assumes 256 samples in a waveform
    for (size_t i = 0; i < 256; ++i) {
      // exit broken waveforms early
      if (i >= waveform.size()) {
        break;
      }
      m_waveformHists[scrodid][carrierNumber][asicNumber][iChannel]->Fill(i + 0.5, iChannel * 1500 + waveform[i]);
    }
  }


  void TOPWaveformQualityPlotterModule::event()
  {
    if (not m_waveform) {
      return;
    }
    for (auto evtwave : m_waveform) {
      if (m_DRAWWAVES) {
        drawWaveforms(evtwave);
      }
      if (m_DEBUGGING) {
        basicDebuggingPlots(evtwave);
      }
      if (m_NOISE) {
        auto channelID = evtwave.getChannel();
        const vector<short> v_samples = evtwave.getWaveform();
        size_t nsamples = v_samples.size();
        if (m_channelNoiseMap.find(channelID) == m_channelNoiseMap.end()) {
          string idName = string("noise_") + to_string(channelID);
          m_channelNoiseMap.insert(make_pair(channelID, new TH1F(idName.c_str(), idName.c_str(), 200, -100, 100)));
          m_channelEventMap.insert(make_pair(channelID, m_iEvent));
        }
        TH1F* noise = m_channelNoiseMap[channelID];
        // Plot all samples in common histogram for quick sanity check
        for (size_t s = 0; s < nsamples; s++) {
          double adc = v_samples.at(s);
          m_samples->Fill(adc);
          if (s < nsamples - 1) {
            noise->Fill(v_samples[s + 1] - adc);
          }
        }
      }
    }
    m_iEvent += 1;
    return;
  }

  void TOPWaveformQualityPlotterModule::endRun()
  {
    if (m_DRAWWAVES) {
      // Each waveform was stored in a TH1F
      // This now gets transformed to a TGraph
      // All 8 channels for a given ASIC are put in the same TMultiGraph
      // Then we make one canvas of several TMultigraphs for each scrod
      // FIXME: This is going to get called at the end of the run; the mem leak I am introducing here should be fixed by somebody more patient with ROOT memory management than me.
      for (auto scrod_it : m_waveformHists) {
        int scrodid = scrod_it.first;
        string name = string("scrod_") + to_string(scrodid);
        TCanvas* c = new TCanvas(name.c_str(), name.c_str());
        c->Divide(4, 4);
        int canvasPad = 0;
        for (auto carrier_it : scrod_it.second) {
          int carrierNumber = carrier_it.first;
          for (auto asic_it : carrier_it.second) {
            int asicNumber = asic_it.first;
            canvasPad += 1;
            string gname = string("scrod_") + to_string(scrodid) + string("_carrier") + to_string(carrierNumber) + string("_asic") + to_string(
                             asicNumber);
            TMultiGraph* mg = new TMultiGraph(gname.c_str(), gname.c_str());
            for (auto channel_it : asic_it.second) {
              // read the data from the TProfile
              TGraphErrors* g = new TGraphErrors(channel_it.second);
              g->SetMarkerStyle(7);
              mg->Add(g);
            }
            // FIXME nSamples is hard-coded to 256
            TH2F* h = new TH2F(gname.c_str(), gname.c_str(), 256, 0, 256, 8, -500, -500 + 8 * 1500);
            for (int ibin = 0; ibin < 8; ibin++) {
              h->GetYaxis()->SetBinLabel(ibin + 1, to_string(ibin).c_str());
            }
            h->GetYaxis()->SetTickSize(0);
            h->GetYaxis()->SetTickLength(0);
            h->SetStats(0);
            c->cd(canvasPad);
            h->Draw();
            mg->Draw("P");
          }
        }
        m_directory->WriteTObject(c);
      }
    }
  }

} // end Belle2 namespace
