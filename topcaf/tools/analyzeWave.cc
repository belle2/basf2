#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TH2F.h>
#include <iostream>
#include <string>

using namespace std;

bool analyzeWave(const char* filename, int firstEvent = 0, int nEvents = 1)
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");
  // TTreeReaderArray<Belle2::EventHeaderPacket> eventHDR(theReader, "EventHeaderPacket");
  int iEvent = -1;

  TFile plotFile("plot.root", "RECREATE");
  while (theReader.Next()) {
    iEvent += 1;
    if (iEvent < firstEvent) {
      continue;
    }
    if (iEvent >= firstEvent + nEvents) {
      break;
    }
    string evtnum = string("graph") + to_string(iEvent);
    TMultiGraph* mg = new TMultiGraph(evtnum.c_str(), evtnum.c_str());
    TCanvas* c = new TCanvas(evtnum.c_str(), evtnum.c_str());
    int iChannel = 0;
    int nSamples = 0;
    vector <string> channelLabels;
    for (Belle2::EventWaveformPacket v : eventRV) {
      channelLabels.push_back(to_string(v.GetChannelID()) + string(";") + to_string(v.GetASICWindow()));
      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      if (nSamples == 0) {
        nSamples = y.size();
      }
      vector<double> x;
      // create the x axis
      for (size_t i = 0; i < y.size(); ++i) {
        y[i] += iChannel * 1000;
        x.push_back(i);
      }
      TGraph* g = new TGraph(y.size(), &x[0], &y[0]);
      g->SetMarkerStyle(7);
      mg->Add(g);
      iChannel += 1;
    }
    if (iChannel == 0 or nSamples == 0) {
      continue;
    }
    // abort the loop after plotting the first waveform
    TH2F* h = new TH2F(evtnum.c_str(), evtnum.c_str(), nSamples, 0, nSamples, iChannel, -500, -500 + iChannel * 1000);
    for (int ibin = 0; ibin < iChannel; ibin++) {
      h->GetYaxis()->SetBinLabel(ibin + 1, channelLabels[ibin].c_str());
    }
    h->GetYaxis()->SetTickSize(0);
    h->GetYaxis()->SetTickLength(0);
    h->SetStats(0);
    h->Draw();
    mg->Draw("P");
    plotFile.WriteTObject(c);
  }
  return true;
}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    analyzeWave(argv[1]);
  } else if (argc == 3) {
    analyzeWave(argv[1], atoi(argv[2]));
  } else if (argc == 4) {
    analyzeWave(argv[1], atoi(argv[2]), atoi(argv[3]));
  }
  return 0;
}
