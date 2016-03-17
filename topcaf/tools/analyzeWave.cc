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

bool analyzeWave(const char* filename, int firstEvent = 0, int nEvents = 1, const char* outfilename = "plot.root")
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");

  int iEvent = -1;

  //TFile plotFile("plot.root", "RECREATE");
  TFile plotFile(outfilename, "RECREATE");
  map<topcaf_channel_id_t, TH1F*> channelNoise; // stores the histogram of vector differences for each channel
  map<topcaf_channel_id_t, int> channelEventMap; // stores the first occurence of a channel

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
    for (EventWaveformPacket v : eventRV) {
      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      auto channelID = v.GetChannelID();
      channelLabels.push_back(to_string(channelID) + string(";") + to_string(v.GetASICWindow()));
      if (channelNoise.find(channelID) == channelNoise.end()) {
        string idName = string("noise_") + to_string(channelID);
        channelNoise.insert(make_pair(channelID, new TH1F(idName.c_str(), idName.c_str(), 200, -100, 100)));
        channelEventMap.insert(make_pair(channelID, iEvent));
      }
      TH1F* noise = channelNoise[channelID];
      if (nSamples == 0) {
        nSamples = y.size();
      }

      TH1D* h = new TH1D(Form("%s_%s", evtnum.c_str(), to_string(channelID).c_str()), "h", y.size(), 0, y.size());
      h->SetTitle(h->GetName());
      for (unsigned int c = 0; c < y.size(); c++) {
        h->SetBinContent(c, y[c]);
      }

      h->Write();

      vector<double> x;
      // create the x axis
      for (size_t i = 0; i < y.size(); ++i) {
        if (i < y.size() - 1) {
          noise->Fill(y[i] - y[i + 1]);
        }
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
  for (map<topcaf_channel_id_t, TH1F*>::iterator it = channelNoise.begin(); it != channelNoise.end(); ++it) {
    plotFile.WriteTObject((*it).second);
    // if more than 1% of ADC values differ by a large number from the previous value, consider channel noisy.
    if ((*it).second->Integral(50, 150) < 0.98 * (*it).second->GetEntries()) {
      cout << "Channel " << (*it).first << " (e.g. in event " << channelEventMap[(*it).first] << ") is noisy." << endl;
    }
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
  } else if (argc == 5) {
    analyzeWave(argv[1], atoi(argv[2]), atoi(argv[3]), argv[4]);
  }
  return 0;
}
