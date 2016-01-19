#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <TCanvas.h>
#include <TProfile.h>
#include <TROOT.h>
#include <iostream>
#include <string>
#include <map>

using namespace std;
using namespace Belle2;

map<topcaf_channel_id_t, TProfile*> fillCorrectedChannels(const char* filename)
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");
  TCanvas c;
  map<topcaf_channel_id_t, TProfile*> channelProfiles;
  gROOT->cd();
  while (theReader.Next()) {
    for (Belle2::EventWaveformPacket& v : eventRV) {
      topcaf_channel_id_t channel_id = v.GetChannelID() + v.GetASICWindow();
      size_t nSamples = v.GetSamples().size();
      string channel_name = to_string(channel_id);
      if (channelProfiles.find(channel_id) == channelProfiles.end()) {
        channelProfiles[channel_id] = new TProfile(channel_name.c_str(),
                                                   channel_name.c_str(),
                                                   nSamples, 0, nSamples);
      }
      TProfile* prof = channelProfiles[channel_id];

      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      for (size_t i = 0; i < y.size(); ++i) {
        prof->Fill(i, y[i]);
      }
    }
  }
  return channelProfiles;
}

void plotDistribution(const map<topcaf_channel_id_t, TProfile*>& channelProfiles)
{
  TH1D spreadDistribution("spread", "spread", 100, -20, 20);
  TH1D meanDistribution("mean", "mean", 100, -10, 10);
  TH1D errorSpreadDistribution("errorSpread", "errorSpread", 100, -20, 20);
  TH1D errorMeanDistribution("errorMean", "errorMean", 100, 0, 3.5);

  for (const auto& it : channelProfiles) {
    meanDistribution.Fill(it.second->GetMean(2));
    cout << it.second->GetMean(2) << endl;
    errorMeanDistribution.Fill(it.second->GetMeanError(2));
    // for (int i=1; i<=it.second->GetNbinsX(); ++i) {
    //     it.second->GetBinContent(i);
    //     it.second->GetBinError(i);
    // }
  }

  TCanvas c;
  c.Divide(2);
  c.cd(1);
  meanDistribution.Draw();
  c.cd(2);
  errorMeanDistribution.Draw();
  c.SaveAs("plot.png");
  return;
}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    plotDistribution(fillCorrectedChannels(argv[1]));
  }
  return 0;
}
