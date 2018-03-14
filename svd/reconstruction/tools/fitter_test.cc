#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <array>
#include <vector>
#include <tuple>
#include <algorithm>
#include <random>
#include <numeric>
#include <functional>
#include <chrono>
#include <random>

#include <svd/reconstruction/NNWaveFitter.h>
#include <svd/reconstruction/NNWaveFitTool.h>
#include <svd/simulation/SVDSimulationTools.h>

using namespace std;
using namespace Belle2::SVD;

int main()
{
  // Initialize random seed generation
  typedef chrono::high_resolution_clock myclock;
  myclock::time_point beginning = myclock::now();

// Create fitter
  NNWaveFitter fitter("svd/data/SVDTimeNet.xml");
  auto fitTool = fitter.getFitTool();

  // Initialize random generators
  myclock::duration d = myclock::now() - beginning;
  unsigned seed = d.count();
  default_random_engine randn(seed);
  normal_distribution<double> gaussian(0.0, 1.0);

  // Generate samples
  auto generator = WaveGenerator();
  size_t n_amplitudes = 3;
  vector< tuple<double, double, double> > components(n_amplitudes);
  components = {{6, -20, 300}, {6, -20, 300}, {6, -20, 300}};
  vector<apvSamples> samples_arr(n_amplitudes);
  vector<shared_ptr<nnFitterBinData> > parr(n_amplitudes);
  nnFitterBinData p(fitter.getBinCenters().size());
  fill(p.begin(), p.end(), double(1.0));
  double amplitude, t0, width;
  double t0_fit, t0_err;
  double a_fit,  a_err, chi2;
  cout << " >>>>>>>>>> SEPARRATE COMPONENTS <<<<<<<<<< " << endl;
  for (size_t icomp = 0; icomp < n_amplitudes; ++icomp) {
    tie(amplitude, t0, width) = components[icomp];
    apvSamples u = generator(t0, width);
    do {
      transform(u.begin(), u.end(), samples_arr[icomp].begin(),
      [&](double x)->double {
        double w = amplitude * x + gaussian(randn);
        return (int(w) > 3 ? int(w) : 0.0);
      });
    } while (!pass3Samples(samples_arr[icomp], 3));
    copy(samples_arr[icomp].begin(), samples_arr[icomp].end(), ostream_iterator<double>(cout, " "));
    cout << endl;
    parr[icomp] =  fitter.getFit(samples_arr[icomp], width);
    copy(parr[icomp]->begin(), parr[icomp]->end(), ostream_iterator<double>(cout, " "));
    cout << endl;
    fitTool.multiply(p, *(parr[icomp]));
    tie(t0_fit, t0_err) = fitTool.getTimeShift(*(parr[icomp]));
    tie(a_fit, a_err, chi2) = fitTool.getAmplitudeChi2(u, t0_fit, width);
    cout << "Component: " << icomp << endl;
    cout << "Time: " << t0_fit << " +/- " << t0_err << endl;
    cout << "Amplitude: " << a_fit << " +/- " << a_err << endl;
    cout << "Chi-square " << chi2 << endl;
  }
  cout << " >>>>>>>>>> POOLED COMPONENTS <<<<<<<<<< " << endl;
  tie(t0_fit, t0_err) = fitTool.getTimeShift(p);
  cout << "Time: " << t0_fit << " +/- " << t0_err << endl;
  for (size_t icomp = 0; icomp < n_amplitudes; ++icomp) {
    tie(a_fit, a_err, chi2) = fitTool.getAmplitudeChi2(samples_arr[icomp], t0_fit, width);
    cout << "Component: " << icomp << endl;
    cout << "Amplitude: " << a_fit << " +/- " << a_err << endl;
    cout << "Chi-square " << chi2 << endl;
  }
  cout << " >>>>>>>>>> SUM OF SIGNALS COMPONENTS <<<<<<<<<< " << endl;
  width = 300;
  apvSamples u_sum;
  fill(u_sum.begin(), u_sum.end(), double(0.0));
  for (size_t i = 0; i < nAPVSamples; ++i)
    for (size_t j = 0; j < n_amplitudes; ++j)
      u_sum[i] += samples_arr[j][i];
  shared_ptr<nnFitterBinData> p_sum = fitter.getFit(u_sum, width);
  tie(t0_fit, t0_err) = fitTool.getTimeShift(*p_sum);
  cout << "Time: " << t0_fit << " +/- " << t0_err << endl;
  for (size_t icomp = 0; icomp < n_amplitudes; ++icomp) {
    tie(a_fit, a_err, chi2) = fitTool.getAmplitudeChi2(samples_arr[icomp], t0_fit, width);
    cout << "Component: " << icomp << endl;
    cout << "Amplitude: " << a_fit << " +/- " << a_err << endl;
    cout << "Chi-square " << chi2 << endl;
  }
  return 0;
}
