#include <framework/core/HistoModule.h>
#include <top/modules/TOPDataQualityPlotter/TOPDataQualityPlotterModule.h>
//#include <top/modules/TOPDataQualityPlotterModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/pcore/RbTuple.h>

#include <utility>

#include "TDirectory.h"
#include "TCanvas.h"
#include "TGraph.h"

using namespace Belle2;
using namespace std;

REG_MODULE(TOPDataQualityPlotter)

TOPDataQualityPlotterModule::TOPDataQualityPlotterModule()
  : HistoModule()
  , m_iEvent(0)
{
  setDescription("TOP DQM histogram module");
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", string("TOP"));
}


void TOPDataQualityPlotterModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  m_directory = oldDir->mkdir(m_histogramDirectoryName.c_str());
  m_directory->cd();
  m_samples = new TH1F("ADCvalues", "ADC values ", 100, -50, 50);
  m_samples->GetXaxis()->SetTitle("ADC Value");
  m_samples->GetYaxis()->SetTitle("Number of Samples");
  m_hitmap = new TH2F("Hitmap", "carrier vs. row", 4, 0, 4, 4, 0, 4);
  m_scrod_id = new TH1F("scrodID", "scrodID", 100, 0, 100);
  m_asic_row = new TH1F("asicRow", "asic row", 4, 0, 4);
  m_asic_col = new TH1F("asicCol", "asic col", 4, 0, 4);
  m_asic_ch = new TH1F("asicCh", "channel", 8, 0, 8);
  m_errorFlag = new TH1F("errorFlag", "errorFlag", 1000, 0, 1000);
  m_flag = new TH1F("flag", "parser flag", 4, 0, 4);
  m_asic_win = new TH1F("window", "window", 4, 0, 4);
  m_entries = new TH1F("entries", "entries", 100, 0, 2600);
  oldDir->cd();
}


void TOPDataQualityPlotterModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM;
  //Get Waveform from datastore
  m_waveform.isRequired();
}


void TOPDataQualityPlotterModule::beginRun()
{
  m_DRAWWAVES = true;
  m_DEBUGGING = true;
}

void TOPDataQualityPlotterModule::basicDebuggingPlots(TOPRawWaveform* rawwave)
{
  m_scrod_id->Fill(rawwave->getScrodID());
  m_asic_ch->Fill(rawwave->getASICChannel());
  m_asic_row->Fill(rawwave->getASICRow());
  m_asic_col->Fill(rawwave->getASICNumber());
  m_asic_win->Fill(rawwave->getStorageWindow());
  m_entries->Fill(rawwave->getWaveform().size());
}

void
TOPDataQualityPlotterModule::drawWaveforms(TOPRawWaveform* rawwave)
{
  const TOPRawWaveform& v = *rawwave;
  vector<short> waveform = v.getWaveform();
  if (waveform.empty()) {
    return;
  }
  //   auto channelID = v.getChannelID();
  int scrodid = v.getScrodID();
  // skip broken events
  if (scrodid == 0) {
    return;
  }
  string gname = string("scrod_") + to_string(scrodid) + string("_carrier") + to_string(v.getASICRow()) + string("_asic") + to_string(
                   v.getASICNumber());
  if (m_channelLabels[scrodid].find(gname) == m_channelLabels[scrodid].end()) {
    m_channels[scrodid].insert(make_pair(gname, new TMultiGraph(gname.c_str(), gname.c_str())));
  }
  // for plotting purposes, only look at one waveform per channel
  // The data doesn't come in order, so we need to make sure we haven's see this particular channel, yet
  int iChannel = v.getASICChannel();
  // if (m_channelLabels[scrodid][asicid].find(iChannel) != m_channelLabels[scrodid][asicid].end()) {
  //   return;
  // }
  m_channelLabels[scrodid][gname].insert(iChannel);
  TMultiGraph* mg = m_channels[scrodid][gname];

  vector<double> x;
  vector<double> y;
  // create the x axis
  for (size_t i = 0; i < waveform.size(); ++i) {
    // spread out y values for better plotting
    y.push_back(waveform[i] + iChannel * 1000);
    x.push_back(i);
  }
  TDirectory* oldDir = gDirectory;
  m_directory->cd();
  TGraph* g = new TGraph(y.size(), &x[0], &y[0]);
  // m_directory->WriteTObject(g);
  g->SetMarkerStyle(7);
  mg->Add(g);
  oldDir->cd();
}


void TOPDataQualityPlotterModule::event()
{
  if (not m_waveform) {
    return;
  }
  for (int c = 0; c < m_waveform.getEntries(); c++) {
    TOPRawWaveform* evtwave_ptr = m_waveform[c];
    if (m_DRAWWAVES) {
      drawWaveforms(evtwave_ptr);
    }
    if (m_DEBUGGING) {
      basicDebuggingPlots(evtwave_ptr);
    }
    auto channelID = evtwave_ptr->getChannel();
    const vector<short> v_samples = evtwave_ptr->getWaveform();
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
  m_iEvent += 1;
  return;
}

void TOPDataQualityPlotterModule::endRun()
{
  if (m_DRAWWAVES) {
    for (auto scrod_it : m_channels) {
      int scrodid = scrod_it.first;
      string name = string("scrod_") + to_string(scrodid);
      TCanvas* c = new TCanvas(name.c_str(), name.c_str());
      c->Divide(4, 4);
      int asicid = 1;
      for (auto graph_it : scrod_it.second) {
        string gname = graph_it.first;
        c->cd(asicid);
        TMultiGraph* mg = graph_it.second;
        // 8 channels per asic
        int nSamples = 0;
        TIter next(mg->GetListOfGraphs());
        while (TGraph* g = static_cast<TGraph*>(next())) {
          if (g->GetN() > nSamples) {
            nSamples = g->GetN();
          }
        }
        TH2F* h = new TH2F(gname.c_str(), gname.c_str(), nSamples, 0, nSamples, 8, -500, -500 + 8 * 1000);
        for (int ibin = 0; ibin < 8; ibin++) {
          h->GetYaxis()->SetBinLabel(ibin + 1, to_string(ibin).c_str());
        }
        h->GetYaxis()->SetTickSize(0);
        h->GetYaxis()->SetTickLength(0);
        h->SetStats(0);
        h->Draw();
        mg->Draw("P");
        asicid += 1;
      }
      TDirectory* oldDir = gDirectory;
      m_directory->cd();
      m_directory->WriteTObject(c);
      oldDir->cd();
    }
  }
}


void TOPDataQualityPlotterModule::terminate()
{
}
