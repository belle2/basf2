#include <TFile.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <topcaf/dataobjects/EventWaveformPacket.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <iostream>
#include <string>

using namespace std;

bool analyzeWave(const char* filename)
{
  TFile fileIn(filename);
  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::EventWaveformPacket> eventRV(theReader, "EventWaveformPackets");
  TCanvas c;
  while (theReader.Next()) {
    for (Belle2::EventWaveformPacket v : eventRV) {
      vector<double> y = v.GetSamples();
      if (y.empty()) {
        continue;
      }
      cout << y.size() << endl;
      vector<double> x;
      // create the x axis
      for (size_t i = 0; i < y.size(); ++i) {
        x.push_back(i);
      }
      TGraph* g = new TGraph(y.size(), &x[0], &y[0]);
      g->Draw("AL*");
      c.Update();
      c.SaveAs("plot.root");
      // abort the loop after plotting the first waveform
      break;
    }
    // abort the loop after plotting the first waveform
    break;
  }
  return true;
}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    analyzeWave(argv[1]);
  }
  return 0;
}
