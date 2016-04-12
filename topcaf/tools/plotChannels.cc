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
#include <set>
#include <utility>

using namespace std;
using namespace Belle2;

// const int CALIBRATIONCHANNEL = 7;
const int ASIC_PER_SCROD = 16;

bool plotChannels(const char* filename, const char* outputname)
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");

  int iEvent = -1;

  TFile plotFile(outputname, "RECREATE");

  map<int, map<int, TMultiGraph*>> channels; // per board stack, per asic
  map<int, map<int, set<int>>> channelLabels; // per board stack, per asic
  size_t nSamples = 0;
  while (theReader.Next()) {
    iEvent += 1;
    for (EventWaveformPacket v : eventRV) {
      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      //   auto channelID = v.GetChannelID();
      if (y.size() > nSamples) {
        nSamples = y.size();
      }
      int scrodid = v.GetScrodID();
      // skip broken events
      if (scrodid == 0) {
        continue;
      }
      //if (scrodid!=37 && scrodid!=38 && scrodid!=52 && scrodid!=66) {
      //continue;
      //}
      int asicid = v.GetASICRow() + 4 * v.GetASIC();
      if (channelLabels[scrodid].find(asicid) == channelLabels[scrodid].end()) {
        // channelLabels[scrodid].insert(make_pair(asicid, vector<string>()));
        string gname = string("channels") + to_string(scrodid) + string("_") + to_string(asicid);
        channels[scrodid].insert(make_pair(asicid, new TMultiGraph(gname.c_str(), gname.c_str())));
      }
      // for plotting purposes, only look at one waveform per channel
      // The data doesn't come in order, so we need to make sure we haven's see this particular channel, yet
      int iChannel = v.GetASICChannel();
      if (channelLabels[scrodid][asicid].find(iChannel) != channelLabels[scrodid][asicid].end()) {
        continue;
      }
      channelLabels[scrodid][asicid].insert(iChannel);
      TMultiGraph* mg = channels[scrodid][asicid];

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

    // check all scrods: if we have not seen 16 asics from each, keep going, else we're done
    bool done = true;
    // four scrods
    if (channelLabels.size() != 4) {
      done = false;
    }
    for (auto scrod_it : channelLabels) {
      if (scrod_it.second.size() != ASIC_PER_SCROD) {
        done = false;
        break;
      }
      for (auto channelID_it : scrod_it.second) {
        // 8 channels per asic
        if (channelID_it.second.size() != 8) {
          done = false;
          break;
        }
      }
      // if we found one reason to continue, that's enough
      if (not done) {
        break;
      }
    }
    // if we have enough channels, stop reading more frames
    if (done) {
      break;
    }
  }

  for (auto scrod_it : channels) {
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
    plotFile.WriteTObject(c);
  }
  return true;
}


bool plotChannels(const char* filename)
{
  return plotChannels(filename, "channels.root");
}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    plotChannels(argv[1]);
  } else if (argc == 3) {
    plotChannels(argv[1], argv[2]);
  } else {
    cerr << "Usage: plotCalibrationChannel <filename> [output.root]" << endl;
  }
  return 0;
}
