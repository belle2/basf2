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
#include <utility>

using namespace std;
using namespace Belle2;

const int CALIBRATIONCHANNEL = 7;

bool plotCalibrationChannel(const char* filename)
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");

  int iEvent = -1;

  TFile plotFile("calibrationChannels.root", "RECREATE");
  map<topcaf_channel_id_t, int> channelEventMap; // stores the first occurence of a channel

  TMultiGraph* mg = new TMultiGraph("calibChannels", "calibChannels");
  TCanvas* c = new TCanvas("calib", "calib");
  size_t nSamples = 0;
  int iChannel = 0;
  vector <string> channelLabels;
  while (theReader.Next()) {
    iEvent += 1;
    if (iEvent > 16) {
      break;
    }
    for (EventWaveformPacket v : eventRV) {
      if (v.GetASICChannel() != CALIBRATIONCHANNEL) {
        continue;
      }
      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      auto channelID = v.GetChannelID();
      channelLabels.push_back(to_string(channelID));
      if (channelEventMap.find(channelID) == channelEventMap.end()) {
        string idName = string("noise_") + to_string(channelID);
        channelEventMap.insert(make_pair(channelID, iEvent));
      }
      if (y.size() > nSamples) {
        nSamples = y.size();
      }

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
      iChannel += 1;
    }
    if (iChannel == 0 or nSamples == 0) {
      continue;
    }
  }
  TH2F* h = new TH2F("calibrationChannels", "calibrationChannels", nSamples, 0, nSamples, iChannel, -500, -500 + iChannel * 1000);
  for (int ibin = 0; ibin < iChannel; ibin++) {
    h->GetYaxis()->SetBinLabel(ibin + 1, channelLabels[ibin].c_str());
  }
  h->GetYaxis()->SetTickSize(0);
  h->GetYaxis()->SetTickLength(0);
  h->SetStats(0);
  h->Draw();
  mg->Draw("P");
  plotFile.WriteTObject(c);
  return true;
}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    plotCalibrationChannel(argv[1]);
  } else {
    cerr << "Usage: plotCalibrationChannel <filename>" << endl;
  }
  return 0;
}
