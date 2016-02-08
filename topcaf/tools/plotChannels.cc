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
const int ASIC_PER_SCROD = 16;

bool plotCalibrationChannel(const char* filename)
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");

  int iEvent = -1;

  TFile plotFile("channels.root", "RECREATE");

  map<int, map<int, TMultiGraph*>> channels; // per board stack, per asic
  map<int, map<int, vector <string>>> channelLabels; // per board stack, per asic
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
      //FIXME the scrodID is not set properly... nor is the ASIC ID
      int scrodid = v.GetASIC();
      //   if (channels.find(scrodid) == channels.end()) {
      //       channels.insert(make_pair(scrodid, map<int, TMultiGraph*>));
      //   }
      int asicid = v.GetASICRow() + 4 * v.GetASICColumn();
      //   cerr << v.GetASIC() << "\t" << v.GetASICRow() << "\t" << v.GetASICColumn() << endl;
      if (channelLabels[scrodid].find(asicid) == channelLabels[scrodid].end()) {
        // channelLabels[scrodid].insert(make_pair(asicid, vector<string>()));
        string gname = string("channels") + to_string(scrodid) + string("_") + to_string(asicid);
        channels[scrodid].insert(make_pair(asicid, new TMultiGraph(gname.c_str(), gname.c_str())));
      }
      // for plotting purposes, only look at one waveform per channel
      if (channelLabels[scrodid][asicid].size() == ASIC_PER_SCROD) {
        continue;
      }
      int iChannel = v.GetASICChannel();
      channelLabels[scrodid][asicid].push_back(to_string(iChannel));
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
      cerr << "ASIC ID " << asicid << endl;
      TMultiGraph* mg = graph_it.second;
      name += string("Channels");
      // 8 channels per asic
      TH2F* h = new TH2F(name.c_str(), name.c_str(), nSamples, 0, nSamples, 8, -500, -500 + 8 * 1000);
      for (int ibin = 0; ibin < 8; ibin++) {
        h->GetYaxis()->SetBinLabel(ibin + 1, channelLabels[scrodid][asicid][ibin].c_str());
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

int main(int argc, char* argv[])
{
  if (argc == 2) {
    plotCalibrationChannel(argv[1]);
  } else {
    cerr << "Usage: plotCalibrationChannel <filename>" << endl;
  }
  return 0;
}
