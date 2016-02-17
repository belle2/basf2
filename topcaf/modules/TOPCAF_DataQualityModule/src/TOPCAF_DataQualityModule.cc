#include <framework/core/HistoModule.h>
#include <topcaf/modules/TOPCAF_DataQualityModule/TOPCAF_DataQualityModule.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/pcore/RbTuple.h>
#include <utility>
#include "TDirectory.h"
#include "TCanvas.h"
#include "TGraph.h"

using namespace Belle2;
using namespace std;

REG_MODULE(TOPCAF_DataQuality)

TOPCAF_DataQualityModule::TOPCAF_DataQualityModule()
  : HistoModule()
  , m_iEvent(0)
{
  setDescription("TOPCAF DQM histogram module");
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "histogram directory in ROOT file", string("TOPCAF"));
}


void TOPCAF_DataQualityModule::defineHisto()
{
  TDirectory* oldDir = gDirectory;
  m_directory = oldDir->mkdir(m_histogramDirectoryName.c_str());
  m_directory->cd();
  m_samples = new TH1F("ADCvalues", "ADC values ", 100, -50, 50);
  m_samples->GetXaxis()->SetTitle("ADC Value");
  m_samples->GetYaxis()->SetTitle("Number of Samples");
  m_hitmap = new TH2F("Hitmap", "carrier vs. row", 4, 0, 4, 4, 0, 4);
  oldDir->cd();
}


void TOPCAF_DataQualityModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM;
}


void TOPCAF_DataQualityModule::beginRun()
{
  m_DRAWWAVES = true;
}

void
TOPCAF_DataQualityModule::drawWaveforms(EventWaveformPacket* ewp)
{
  const EventWaveformPacket& v = *ewp;
  vector<double> y = v.GetSamples();
  if (y.empty()) {
    return;
  }
  //   auto channelID = v.GetChannelID();
  int scrodid = v.GetScrodID();
  // skip broken events
  if (scrodid == 0) {
    return;
  }
  int asicid = v.GetASICRow() + 4 * v.GetASICColumn();
  if (m_channelLabels[scrodid].find(asicid) == m_channelLabels[scrodid].end()) {
    string gname = string("channels") + to_string(scrodid) + string("_") + to_string(asicid);
    m_channels[scrodid].insert(make_pair(asicid, new TMultiGraph(gname.c_str(), gname.c_str())));
  }
  // for plotting purposes, only look at one waveform per channel
  // The data doesn't come in order, so we need to make sure we haven's see this particular channel, yet
  int iChannel = v.GetASICChannel();
  if (m_channelLabels[scrodid][asicid].find(iChannel) != m_channelLabels[scrodid][asicid].end()) {
    return;
  }
  m_channelLabels[scrodid][asicid].insert(iChannel);
  TMultiGraph* mg = m_channels[scrodid][asicid];

  vector<double> x;
  // create the x axis
  for (size_t i = 0; i < y.size(); ++i) {
    // spread out y values for better plotting
    y[i] += iChannel * 1000;
    x.push_back(i);
  }
  TGraph* g = new TGraph(y.size(), &x[0], &y[0]);
  g->SetMarkerStyle(7);
  mg->Add(g);
}


void TOPCAF_DataQualityModule::event()
{
  //Get Waveform from datastore
  StoreArray<EventWaveformPacket> evtwaves_ptr;
  evtwaves_ptr.isRequired();
  if (not evtwaves_ptr) {
    return;
  }
  for (int c = 0; c < evtwaves_ptr.getEntries(); c++) {
    EventWaveformPacket* evtwave_ptr = evtwaves_ptr[c];
    if (m_DRAWWAVES) {
      drawWaveforms(evtwave_ptr);
    }
    auto channelID = evtwave_ptr->GetChannelID();
    const vector<double> v_samples = evtwave_ptr->GetSamples();
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


void TOPCAF_DataQualityModule::endRun()
{
  if (m_DRAWWAVES) {
    for (auto scrod_it : m_channels) {
      int scrodid = scrod_it.first;
      string name = string("scrod_") + to_string(scrodid) + string("calib");
      TCanvas* c = new TCanvas(name.c_str(), name.c_str());
      c->Divide(4, 4);
      for (auto graph_it : scrod_it.second) {
        int asicid = graph_it.first;
        c->cd(asicid + 1);
        TMultiGraph* mg = graph_it.second;
        string gname = name + string("asic_") + to_string(asicid);
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
      }
      TDirectory* oldDir = gDirectory;
      m_directory->cd();
      m_directory->WriteTObject(c);
      oldDir->cd();
    }
  }
}


void TOPCAF_DataQualityModule::terminate()
{
}
