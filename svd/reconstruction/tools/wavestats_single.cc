#include <iostream>
#include <random>
#include <svd/reconstruction/WaveFitter.h>
#include <TString.h>
#include <TCanvas.h>
#include <TTree.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TVectorD.h>

using namespace std;
using namespace Belle2::SVD;

const int samplesPerSec = 10000;
const double timestep = 0.01;
const int nSamples = int(timestep* samplesPerSec);
const double t0_range = 35; // ns - range of probed true time shifts
const double sigma = 1.0;
const double dt = 31.44;

int main(int argc, char** argv)
{
  TApplication timeApp("wavestats_single", &argc, argv);

  mt19937_64 generator;
  normal_distribution<double> noise_distribution(0.0, 1.0);
  const vector<double> sn_values = {2, 3, 5, 7, 10, 15, 20};

  TFile* f = new TFile("time-stats.root", "RECREATE");
  TTree* tree = new TTree("tree", "SVD time fitter toy simulation for a single strip");
  double true_amp;
  tree->Branch("true_amp", &true_amp, "true_amp/D");
  double true_t0;
  tree->Branch("true_t0", &true_t0, "true_t0/D");
  double fit_amp;
  tree->Branch("fit_amp", &fit_amp, "fit_amp/D");
  double fit_amp_error;
  tree->Branch("fit_amp_error", &fit_amp_error, "fit_amp_error/D");
  double pull_amp;
  tree->Branch("pull_amp", &pull_amp, "pull_amp/D");
  double fit_t0;
  tree->Branch("fit_t0", &fit_t0, "fit_t0/D");
  double fit_t0_error;
  tree->Branch("fit_t0_error", &fit_t0_error, "fit_t0_error/D");
  double pull_t0;
  tree->Branch("pull_t0", &pull_t0, "pull_t0/D");
  double p_signal;
  tree->Branch("p_signal", &p_signal, "p_signal/D");
  double lr_signal;
  tree->Branch("lr_signal", &lr_signal, "lr_signal/D");
  unsigned short accept;
  tree->Branch("accept", &accept, "accept/s");
  double lik_fit;
  tree->Branch("lik_fit", &lik_fit, "lik_fit/D");
  double lik_0;
  tree->Branch("lik_0", &lik_0, "lik_0/D");
  // Create a vector of sample times
  std::array<double, 6> times;
  for (int i = 0; i < 6; ++i)times[i] = (i - 1) * dt;
  // Create the fitter
  DefaultWave mywave;
  WaveFitter fitter(
    [&mywave](double t)->double {return mywave.getValue(t);},
    times
  );
  // Cycle through s/n ratios
  for (double amplitude : sn_values) {
    true_amp = amplitude;
    cout << "Amplitude: " << amplitude << endl;
    true_t0 = - t0_range;
    while (true_t0 < t0_range) {
      for (int i = 0; i < nSamples; ++i) {
        fitter.reset();
        WaveFitter::strip_data_type mydata;
        array<double, 6> data;
        for (int i = 0; i < 6; ++i)
          data[i] = amplitude * mywave.getValue(times[i] - true_t0)
                    + sigma * noise_distribution(generator);
        fitter.addData(data, sigma);
        mydata.push_back(data);
        fit_t0 = fitter.getFittedTime();
        fit_t0_error = fitter.getFittedTimeError();
        pull_t0 = (fit_t0 - true_t0) / fit_t0_error;
        auto fit_amps = fitter.getFittedAmplitudes();
        fit_amp = fit_amps[0];
        auto fit_amp_errors = fitter.getFittedAmplitudeErrors();
        fit_amp_error = fit_amp_errors[0];
        pull_amp = (fit_amp - true_amp) / fit_amp_error;
        p_signal = fitter.pSignal();
        lr_signal = fitter.lrSignal(-5, 5);
        lr_signal = lr_signal / (1.0 + lr_signal);
        accept = (fitter.negLogLikelihood(0) - fitter.negLogLikelihood(fit_t0) < 1.92);
        lik_fit = fitter.negLogLikelihood(fit_t0);
        lik_0 = fitter.negLogLikelihood(0);
        tree->Fill();
      } // samples
      true_t0 += timestep;
    } // times
  } // amplitudes
  f->Write();
  f->Close();

  return 0;
}

