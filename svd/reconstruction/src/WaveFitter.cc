/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <framework/logging/Logger.h>
#include <svd/reconstruction/WaveFitter.h>
#include <iostream>
#include <math.h>
#include <utility>
#include <boost/math/tools/minima.hpp>

using namespace Belle2::SVD;
using namespace std;

double DefaultWave::s_default_tau = 50;

int WaveFitter::s_minimizer_precision = 6;

void WaveFitter::setPrecision(int ndigits) { s_minimizer_precision = ndigits; }

double WaveFitter::Chi(double t0)
{
  // This we only need to calculate once
  array<double, 6> waves;
  double sumTemplateByTemplate = 0;
  for (int i = 0; i < 6; ++i) {
    waves[i] = m_wave(m_times[i] - t0);
    sumTemplateByTemplate += waves[i] * waves[i];
  }
  // Calculate amplitudes
  vector<double> amplitudes(m_data.size());
  for (unsigned int row = 0; row < m_data.size(); ++row) {
    double sumDataByTemplate = 0;
    for (int i = 0; i < 6; i++) {
      sumDataByTemplate += m_data[row][i] * waves[i];
    }
    amplitudes[row] = sumDataByTemplate / sumTemplateByTemplate;
  }
  double result = 0;
  for (unsigned int row = 0; row < m_data.size(); ++row)
    for (int i = 0; i < 6; i++) {
      double diff =
        (m_data[row][i] - amplitudes[row] * waves[i]) / m_noises[row];
      result += diff * diff;
    }
  return result / m_ndf;
}

double WaveFitter::negLogLikelihood(double t)
{
  // wave values
  array<double, 6> w;
  for (int i = 0; i < 6; ++i) w[i] = m_wave(m_times[i] - t);
  double sum_w2 = 0;
  for (int i = 0; i < 6; ++i) sum_w2 += w[i] * w[i];
  double lognorm = 0.5 * m_data.size() * log(sum_w2);
  double sum_ksi = 0.0;
  for (unsigned int row = 0; row < m_data.size(); ++row) {
    double ksi = 0;
    for (int i = 1; i < 6; ++i)
      for (int j = 0; j < i; ++j) {
        double cross = w[i] * m_data[row][j] - w[j] * m_data[row][i];
        ksi += cross * cross;
      }
    sum_ksi += ksi;
  }
  return lognorm + 0.5 * sum_ksi / sum_w2;
}

void WaveFitter::doFit()
{
  // Degrees of freedom:
  // # samples - # strips (amplitudes) - 1 (timeshift) - 1 (sum = 0)
  m_ndf = 6 * m_data.size() - m_data.size() - 2;
  if (m_ndf < 1) { // We have no data, so there'll be no fit and an error
    B2ERROR("SVD::WaveFitterter : Fit required with no data provided.");
    m_fittedTime = 0;
    m_fittedTimeError = 0;
    m_fittedAmplitudes.clear();
    m_fittedAmplitudeErrors.clear();
    m_fittedLik = 1.0e10;
    m_fittedData.clear();
    // We don't need to repeat this
    m_hasFit = true;
    return;
  }
  // Normal operation: minimize wrt. t0 in (-dt,0) and (0,dt),
  // take the solution giving a better fit
  auto result_low = boost::math::tools::brent_find_minima(
                      [this](double t)->double { return negLogLikelihood(t); }, -m_dt, 0.0, s_minimizer_precision
                    );
  double t0_low = result_low.first;
  double lik_low = result_low.second;
  auto result_hi = boost::math::tools::brent_find_minima(
                     [this](double t)->double { return negLogLikelihood(t); }, 0.0, m_dt, s_minimizer_precision
                   );
  double t0_hi = result_hi.first;
  double lik_hi = result_hi.second;
  if (lik_low < lik_hi) {
    m_fittedTime = t0_low;
    m_fittedLik = lik_low;
  } else {
    m_fittedTime = t0_hi;
    m_fittedLik = lik_hi;
  }
  calculateAmplitudes();
  calculateFitErrors();
  calculateFittedData();
  m_hasFit = true;
}

void WaveFitter::calculateAmplitudes()
{
  // This we only need to calculate once
  array<double, 6> waves;
  double sumTemplateByTemplate = 0;
  for (int i = 0; i < 6; ++i) {
    waves[i] = m_wave(m_times[i] - m_fittedTime);
    sumTemplateByTemplate += waves[i] * waves[i];
  }
  // Calculate amplitudes
  for (unsigned int row = 0; row < m_data.size(); ++row) {
    double sumDataByTemplate = 0;
    for (int i = 0; i < 6; i++) {
      sumDataByTemplate += m_data[row][i] * waves[i];
    }
    m_fittedAmplitudes.push_back(sumDataByTemplate / sumTemplateByTemplate);
  }
}

void WaveFitter::calculateFitErrors()
{
  // Errors on amplitudes - simple from the linear model
  double sum_w2 = 0;
  for (int i = 0; i < 6; ++i) {
    double wave = m_wave(m_times[i] - m_fittedTime);
    sum_w2 += wave * wave;
  }
  double one_by_sqrtSumw2 = 1.0 / sqrt(sum_w2);
  for (unsigned int row = 0; row < m_data.size(); ++row)
    m_fittedAmplitudeErrors.push_back(one_by_sqrtSumw2 * m_noises[row]);
  // For time: ignore correlation with amplitudes
  // We will need derivatives, calculate them numerically
  double timestep = 0.1 * m_dt;
  double dwave_sq = 0;
  for (int i = 0; i < 6; ++i) {
    double dwave = (wave(m_times[i] - m_fittedTime + 0.5 * timestep) - wave(m_times[i] - m_fittedTime - 0.5 * timestep)) / timestep;
    dwave_sq += dwave * dwave;
  }
  double sn_sq = 0;
  for (unsigned int row = 0; row < m_data.size(); ++row) {
    double sn = m_fittedAmplitudes[row] / m_noises[row];
    sn_sq += sn * sn;
  }
  m_fittedTimeError = 1.0 / sqrt(sn_sq * dwave_sq);
}

void WaveFitter::calculateFittedData()
{
  m_fittedData.resize(m_data.size());
  for (int i = 0; i < 6; ++i) {
    double wave = m_wave(m_times[i] - m_fittedTime);
    for (unsigned int row = 0; row < m_data.size(); ++row)
      m_fittedData[row][i] = m_fittedAmplitudes[row] * wave;
  }
}

double WaveFitter::integral12(double lower, double upper, std::function<double(double)> f)
{
  const unsigned int half_order = 6;
  const double knots[half_order] = {
    0.1252334085114689154724414,
    0.3678314989981801937526915,
    0.5873179542866174472967024,
    0.7699026741943046870368938,
    0.9041172563704748566784659,
    0.9815606342467192506905491
  };
  const double weights[half_order] = {
    0.2491470458134027850005624,
    0.2334925365383548087608499,
    0.2031674267230659217490645,
    0.1600783285433462263346525,
    0.1069393259953184309602547,
    0.0471753363865118271946160
  };
  double span = 0.5 * (upper - lower);
  double center = 0.5 * (upper + lower);
  double result = 0;
  for (unsigned int iknot = 0; iknot < half_order; ++iknot) {
    result += weights[iknot] * f(center + span * knots[iknot]);
    result += weights[iknot] * f(center - span * knots[iknot]);
  }
  result *= span;
  return result;
}

double WaveFitter::integral20(double lower, double upper, std::function<double(double)> f)
{
  const unsigned int half_order = 10;
  const double knots[half_order] = {
    0.0765265211,
    0.2277858511,
    0.3737060887,
    0.510867002,
    0.6360536807,
    0.7463319065,
    0.8391169718,
    0.9122344283,
    0.9639719273,
    0.9931285992
  };
  const double weights[half_order] = {
    0.1527533871,
    0.1491729865,
    0.1420961093,
    0.1316886384,
    0.118194532,
    0.1019301198,
    0.0832767416,
    0.0626720483,
    0.0406014298,
    0.0176140071
  };
  double span = 0.5 * (upper - lower);
  double center = 0.5 * (upper + lower);
  double result = 0;
  for (unsigned int iknot = 0; iknot < half_order; ++iknot) {
    result += weights[iknot] * f(center + span * knots[iknot]);
    result += weights[iknot] * f(center - span * knots[iknot]);
  }
  result *= span;
  return result;
}

double WaveFitter::pSignal()
{
  const double outer = m_dt;
  const double inner = 5; // ns
  if (!m_hasFit) doFit();
  auto integrand = [this](double t)->double { return exp(-negLogLikelihood(t));};

  double i1 = integral20(-outer, -inner, integrand);
  double i2 = integral20(-inner, 0, integrand);
  double i3 = integral20(0, inner, integrand);
  double i4 = integral20(inner, outer, integrand);
  double L_background = 1.0 / (2 * outer) * (i1 + i2 + i3 + i4);
  double L_signal = 1.0 / (2 * inner) * (i2 + i3);
  double prob = (L_signal / (L_background + L_signal));
  //cout << L_background << " <><> " << L_signal << " <> " << prob << endl;
  return prob;
}

double WaveFitter::lrSignal(double low, double high)
{
  const double outer = m_dt;
  if (!m_hasFit) doFit();
  auto integrand = [this](double t)->double { return exp(-negLogLikelihood(t));};
  // Calculate total integral and integral between a and b:
  double int_total = integral20(-outer, 0, integrand);
  int_total += integral20(0, outer, integrand);
  double int_inner = 0;
  if (low * high < 0) {
    int_inner += integral20(low, 0, integrand);
    int_inner += integral20(0, high, integrand);
  } else {
    int_inner += integral20(low, high, integrand);
  }
  double int_outer = int_total - int_inner;
  double LR = int_inner / int_outer;
  //cout << L_background << " <><> " << L_signal << " <> " << prob << endl;
  return LR;
}

