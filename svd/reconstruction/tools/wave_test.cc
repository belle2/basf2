#include <iostream>
#include <random>
#include <svd/reconstruction/WaveFitter.h>
#include <TString.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TVectorD.h>

using namespace std;
using namespace Belle2::SVD;


const double sigma1 = 0.8;
const double sigma2 = 0.7;
const double amplitude1 = 2.0;
const double amplitude2 = 2.0;
const int nSamples = 100000;
const double t0_true = 0;
const double dt = 31.44;

const TCanvas* plotCurves(WaveFitter& fitter, double true_t0, const vector<double>& true_amplitudes)
{
  // Make canvas and divide it.
  // We plot
  // - data with true and fitted curves in -dt, +5*dt
  // - chi2 as a function of time shift in -dt, +dt
  // - likelihood as a function of time shift in -dt, dt,
  //   together with signal and background priors
  TCanvas* cEvent = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("cEvent");
  if (cEvent) { delete cEvent; cEvent = nullptr; }
  cEvent = new TCanvas("cEvent", "Cluster time fit", 1200, 400);
  cEvent->cd();
  cEvent->Divide(3, 1);
  cEvent -> cd(1);
  TMultiGraph* mgData = new TMultiGraph("mgData", "Time fit data, true and fitted curves");
  TLegend* leg = new TLegend(0.55, 0.12, 0.87, 0.42);
  mgData->SetTitle("Cluster Data; time [ns]; signal [ADU]");
  // Plot truth and fitted curves
  TVectorD curveTimes(100);
  for (int i = 0; i < 100; ++i)
    curveTimes[i] = -50.0 + 180.0 / 99 * i;
  TVectorD trueSamples(100);
  TVectorD fittedSamples(100);
  for (unsigned int row = 0; row < true_amplitudes.size(); ++row) {
    double fitted_t0 = fitter.getFittedTime();
    double fitted_amplitude = fitter.getFittedAmplitudes()[row];
    for (int i = 0; i < 100; ++i) {
      trueSamples[i] = true_amplitudes[row] * fitter.wave(curveTimes[i] - true_t0);
      fittedSamples[i] = fitted_amplitude * fitter.wave(curveTimes[i] - fitted_t0);
    }
    // add graphs to mgData
    TGraph* gTrue = new TGraph(curveTimes, trueSamples);
    gTrue->SetLineColor(9 - row);
    gTrue->SetLineStyle(2); // short dash
    TString legTitleTrue(Form("MC truth strip %d", row + 1));
    leg->AddEntry(gTrue, legTitleTrue.Data(), "l");
    mgData->Add(gTrue, "L");
    TGraph* gFit = new TGraph(curveTimes, fittedSamples);
    gFit->SetLineColor(9 - row);
    TString legTitleFit(Form("Fit strip %d", row + 1));
    leg->AddEntry(gFit, legTitleFit.Data(), "l");
    mgData->Add(gFit, "L");
  }
  // Plot samples

  TVectorD sampleTimes(6);
  for (int i = 0; i < 6; ++i)
    sampleTimes[i] = fitter.getTimes()[i];
  const WaveFitter::strip_data_type& data = fitter.getData();
  TVectorD samples(6);
  for (unsigned int row = 0; row < data.size(); ++row) {
    for (int i = 0; i < 6; ++i) samples[i] = data[row][i];
    TGraph* gData = new TGraph(sampleTimes, samples);
    gData->SetMarkerStyle(21);
    gData->SetMarkerColor(9 - row);
    mgData->Add(gData, "P");
    TString legTitleData(Form("Samples strip %d", row + 1));
    leg->AddEntry(gData, legTitleData.Data(), "p");
  }
  mgData->Draw("A");
  leg->Draw();
  // Plot chisquare/ndf and likelihood
  double dt = fitter.getDt();
  for (int i = 0; i < 100; ++i)
    curveTimes[i] = -dt + 2.0 * i / 99 * dt;
  TVectorD chiValues(100);
  TVectorD likValues(100);
  for (int i = 0; i < 100; ++i) {
    chiValues[i] = fitter.Chi(curveTimes[i]);
    likValues[i] = exp(-fitter.negLogLikelihood(curveTimes[i]));
  }
  cEvent->cd(2);
  TLegend* legChi = new TLegend(0.6, 0.7, 0.87, 0.87);
  TGraph* gChi = new TGraph(curveTimes, chiValues);
  gChi->SetLineColor(kBlue);
  gChi->Draw("AL");
  gChi->SetTitle("Chi2/ndf");
  gChi->GetXaxis()->SetTitle("time shift [ns]");
  gChi->GetYaxis()->SetTitle("chi^2/ndf");
  double true_chi = fitter.Chi(true_t0);
  TGraph* gChiTrue = new TGraph(1, &true_t0, &true_chi);
  gChiTrue->SetMarkerColor(kRed);
  gChiTrue->SetMarkerStyle(20);
  gChiTrue->Draw("P");
  legChi->AddEntry(gChiTrue, "True t0", "p");
  double fit_time = fitter.getFittedTime();
  double fit_chi = fitter.Chi(fit_time);
  TGraph* gChiFit = new TGraph(1, &fit_time, &fit_chi);
  gChiFit->SetMarkerColor(kBlue);
  gChiFit->SetMarkerStyle(20);
  gChiFit->Draw("P");
  legChi->AddEntry(gChiFit, "Fitted t0", "p");
  legChi->Draw();

  cEvent->cd(3);
  TGraph* gLik = new TGraph(curveTimes, likValues);
  TLegend* legLik = new TLegend(0.6, 0.7, 0.87, 0.87);
  gLik->SetLineColor(kBlue);
  gLik->Draw("AL");
  gLik->SetTitle("Marginal likelihood");
  gLik->GetXaxis()->SetTitle("time shift [ns]");
  gLik->GetYaxis()->SetTitle("P(Data | time shift)");
  double true_lik = exp(-fitter.negLogLikelihood(true_t0));
  TGraph* gLikTrue = new TGraph(1, &true_t0, &true_lik);
  gLikTrue->SetMarkerColor(kRed);
  gLikTrue->SetMarkerStyle(20);
  gLikTrue->Draw("P");
  legLik->AddEntry(gLikTrue, "True t0", "p");
  double fit_lik = exp(-fitter.negLogLikelihood(fit_time));
  TGraph* gLikFit = new TGraph(1, &fit_time, &fit_lik);
  gLikFit->SetMarkerColor(kBlue);
  gLikFit->SetMarkerStyle(20);
  gLikFit->Draw("P");
  legLik->AddEntry(gLikFit, "Fitted t0", "p");
  legLik->Draw();

  cEvent->Modified();
  cEvent->Update();
  cout << "s to save, any other char to continue..." << endl;
  char inp;
  cin >> inp;
  if (inp == 's' || inp == 'S')
    cEvent->SaveAs("TimeFitter.png");
  return cEvent;
}

int main(int argc, char** argv)
{
  TApplication timeApp("wave_test", &argc, argv);

  mt19937_64 generator;
  normal_distribution<double> noise_distribution(0.0, 1.0);

  TFile* f = new TFile("time-histo.root", "RECREATE");
  // Create time axis
  std::array<double, 6> times;
  for (int i = 0; i < 6; ++i)times[i] = (i - 1) * dt;
  std::vector<double> true_amplitudes(2);
  true_amplitudes[0] = amplitude1;
  true_amplitudes[1] = amplitude2;
  // Create the fitter
  DefaultWave mywave;
  WaveFitter fitter(
    [&mywave](double t)->double {return mywave.getValue(t);},
    times
  );
  TH1F* h = new TH1F("hDiff", "Fitted", 100, -dt, +dt);
  for (int i = 0; i < nSamples; ++i) {
    fitter.reset();
    WaveFitter::strip_data_type mydata;
    array<double, 6> data;
    for (int i = 0; i < 6; ++i)
      data[i] = amplitude1 * mywave.getValue(times[i] - t0_true)
                + sigma1 * noise_distribution(generator);
    fitter.addData(data, sigma1);
    mydata.push_back(data);
    for (int i = 0; i < 6; ++i)
      data[i] = amplitude2 * mywave.getValue(times[i] - t0_true)
                + sigma2 * noise_distribution(generator);
    fitter.addData(data, sigma2);
    mydata.push_back(data);
    double time = fitter.getFittedTime();
    h->Fill(time);
    //cout << "Time: " << time << " +/- " << fitter.getFittedTimeError() << endl;
    auto amplitudes = fitter.getFittedAmplitudes();
    auto amplitudeErrors = fitter.getFittedAmplitudeErrors();
    //cout << "Ampl1: " << amplitudes[0] << " +/- " << amplitudeErrors[0] << endl;
    //cout << "Ampl2: " << amplitudes[1] << " +/- " << amplitudeErrors[1] << endl;
    //cout << "Chi2: " << fitter.getFitChi() << endl << endl;
    plotCurves(fitter, t0_true, true_amplitudes);
    cout << fitter.pSignal() << endl;
  }
  h->Write();
  f->Close();

  return 0;
}

