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

  map<int, TMultiGraph*> calibrationChannels;
  map<int, vector <string>> channelLabels;
  size_t nSamples = 0;
  while (theReader.Next()) {
    iEvent += 1;
    for (EventWaveformPacket v : eventRV) {
      if (v.GetASICChannel() != CALIBRATIONCHANNEL) {
        continue;
      }
      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      auto channelID = v.GetChannelID();
      if (y.size() > nSamples) {
        nSamples = y.size();
      }

      int scrodid = v.GetScrodID();
      if (calibrationChannels.find(scrodid) == calibrationChannels.end()) {
        string gname = string("calibChannels") + to_string(scrodid);
        calibrationChannels.insert(make_pair(scrodid, new TMultiGraph(gname.c_str(), gname.c_str())));
      }
      channelLabels[scrodid].push_back(to_string(channelID));
      TMultiGraph* mg = calibrationChannels[scrodid];
      int iChannel = mg->GetListOfGraphs() ? mg->GetListOfGraphs()->GetEntries() : 0;

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

    // check all scrods: if we have not seen 16 calibration channels from each, keep going, else we're done
    bool done = true;
    for (auto it = channelLabels.begin(); it != channelLabels.end(); ++it) {
      if ((*it).second.size() < 16) {
        done = false;
        break;
      }
    }
    if (done) {
      break;
    }
  }

  for (auto graph_it = calibrationChannels.begin(); graph_it != calibrationChannels.end(); ++graph_it) {
    int scrodid = (*graph_it).first;
    string name = string("scrod_") + to_string(scrodid) + string("calib");
    TMultiGraph* mg = (*graph_it).second;
    TCanvas* c = new TCanvas(name.c_str(), name.c_str());
    name += string("Channels");
    int iChannel = mg->GetListOfGraphs() ? mg->GetListOfGraphs()->GetEntries() : 0;
    TH2F* h = new TH2F(name.c_str(), name.c_str(), nSamples, 0, nSamples, iChannel, -500, -500 + iChannel * 1000);
    for (int ibin = 0; ibin < iChannel; ibin++) {
      h->GetYaxis()->SetBinLabel(ibin + 1, channelLabels[scrodid][ibin].c_str());
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
    plotCalibrationChannel(argv[1]);
  } else {
    cerr << "Usage: plotCalibrationChannel <filename>" << endl;
  }
  return 0;
}
