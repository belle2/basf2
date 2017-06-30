#include <ecl/digitization/EclConfiguration.h>
#include <ecl/digitization/shaperdsp.h>
#include <vector>
#include <cmath>

using namespace Belle2;
using namespace Belle2::ECL;
using namespace std;

// define the constexpr here to make clang happy. Could be a bug or some strict
// adherence to the standard. I guess it's just a different interpretation of
// how to use these values by clang.
// http://stackoverflow.com/questions/28264279/undefined-reference-when-accessing-static-constexpr-float-member
constexpr double EclConfiguration::s_clock;
constexpr double EclConfiguration::m_rf;
constexpr double EclConfiguration::m_step;

void EclConfiguration::signalsample_t::InitSample(const double* MPd, double u)
{
  const int N = m_ns * m_nl;
  vector<double> MP(MPd, MPd + 10);
  ShaperDSP_t dsp(MP, u);
  dsp.settimestride(m_step / m_ns);
  dsp.fillarray(0.0, N, m_ft);

  double sum = 0;
  for (int i = 0; i < N; i++) sum += m_ft[i];
  m_sumscale = m_ns / sum;
}


void EclConfiguration::signalsample_t::InitSample(const float* MP, double u)
{
  double MPd[10];
  for (int i = 0; i < 10; i++) MPd[i] = MP[i];
  InitSample(MPd, u);
}

void EclConfiguration::adccounts_t::AddHit(const double a, const double t0, const EclConfiguration::signalsample_t& s)
{
  total += s.Accumulate(a, t0, c);
}

double EclConfiguration::signalsample_t::Accumulate(const double a, const double t0, double* s) const
{
  // input parameters
  // a -- signal amplitude
  // t -- signal offset
  // output parameter
  // s -- output array with added signal
  const double itick = m_rf / s_clock;  // reciprocal to avoid division in usec^-1 (has to be evaluated at compile time)
  const double  tlen = m_nl - 1.0 / m_ns;   // length of the sampled signal in ADC clocks units
  const double  tmax = m_tmin + m_nsmp - 1; // upper range of the fit region

  double t = t0 * itick; // rescale time in usec to ADC clocks
  double x0 = t, x1 = t + tlen;

  if (x0 > tmax) return 0; // signal starts after the upper range of output array -- do nothing
  if (x0 < m_tmin) {
    if (x1 < m_tmin) return 0; // signal ends before lower range of output array -- do nothing
    x0 = m_tmin; // adjust signal with range of output array
  }

  int imax = m_nsmp; // length of sampled signal is long enough so
  // the last touched element is the last element
  // of the output array
  if (x1 < tmax) { // if initial time is too early we need to adjust
    // the last touched element of output array to avoid
    // out-of-bound situation in m_ft
    imax = x1 - m_tmin; // imax is always positive so floor can be
    // replace by simple typecast
    imax += 1; // like s.end()
  }

  double imind = ceil(x0 - m_tmin); // store result in double to avoid int->double conversion below
  // the ceil function today at modern CPUs is surprisingly fast (before it was horribly slow)
  int imin = imind; // starting point to fill output array
  double w = ((m_tmin - t) + imind) * double(m_ns);
  int jmin = w; // starting point in sampled signal array
  w -= jmin;

  // use linear interpolation between samples. Since signal samples
  // are aligned with output samples only two weights are need to
  // calculate to fill output array
  const double w1 = a * w, w0 = a - w1;
  double sum = 0;
  for (int i = imin, j = jmin; i < imax; i++, j += m_ns) {
    double amp = w0 * m_ft[j] + w1 * m_ft[j + 1];
    s[i] += amp;
    sum  += amp;
  }
  return sum * m_sumscale;
}

/** unpack waveform for simulation */
int EclConfiguration::unpack(int* out, const unsigned int* a)
{
  const unsigned int mask = (1 << 18) - 1;
  out[ 0] = ((a[0] >> 18) | (a[1] << 14))&mask;
  out[ 1] = (a[1] >> 4)&mask;
  out[ 2] = ((a[1] >> 22) | (a[2] << 10))&mask;
  out[ 3] = (a[2] >> 8)&mask;
  out[ 4] = ((a[2] >> 26) | (a[3] << 6))&mask;
  out[ 5] = (a[3] >> 12)&mask;
  out[ 6] = ((a[3] >> 30) | (a[4] << 2))&mask;
  out[ 7] = ((a[4] >> 16) | (a[5] << 16))&mask;
  out[ 8] = (a[5] >> 2)&mask;
  out[ 9] = ((a[5] >> 20) | (a[6] << 12))&mask;
  out[10] = (a[6] >> 6)&mask;
  out[11] = ((a[6] >> 24) | (a[7] << 8))&mask;
  out[12] = (a[7] >> 10)&mask;
  out[13] = ((a[7] >> 28) | (a[8] << 4))&mask;
  out[14] = (a[8] >> 14)&mask;
  out[15] = (a[9] >> 0)&mask;
  out[16] = ((a[9] >> 18) | (a[10] << 14))&mask;
  out[17] = (a[10] >> 4)&mask;
  out[18] = ((a[10] >> 22) | (a[11] << 10))&mask;
  out[19] = (a[11] >> 8)&mask;
  out[20] = ((a[11] >> 26) | (a[12] << 6))&mask;
  out[21] = (a[12] >> 12)&mask;
  out[22] = ((a[12] >> 30) | (a[13] << 2))&mask;
  out[23] = ((a[13] >> 16) | (a[14] << 16))&mask;
  out[24] = (a[14] >> 2)&mask;
  out[25] = ((a[14] >> 20) | (a[15] << 12))&mask;
  out[26] = (a[15] >> 6)&mask;
  out[27] = ((a[15] >> 24) | (a[16] << 8))&mask;
  out[28] = (a[16] >> 10)&mask;
  out[29] = ((a[16] >> 28) | (a[17] << 4))&mask;
  out[30] = (a[17] >> 14)&mask;
  return a[0] & mask;
}

/**
 * Pack simulated waveform
 * assuming 18 bit ADC precision
 */
void EclConfiguration::pack(unsigned int* out, int cellid, const unsigned int* a)
{
  const unsigned int mask = (1 << 18) - 1;
  out[ 0] = (a[0] << 18) | (cellid & mask);
  out[ 1] = (a[0] >> 14) | (a[1] << 4) | (a[2] << 22);
  out[ 2] = (a[2] >> 10) | (a[3] << 8) | (a[4] << 26);
  out[ 3] = (a[4] >> 6) | (a[5] << 12) | (a[6] << 30);
  out[ 4] = (a[6] >> 2) | (a[7] << 16);
  out[ 5] = (a[7] >> 16) | (a[8] << 2) | (a[9] << 20);
  out[ 6] = (a[9] >> 12) | (a[10] << 6) | (a[11] << 24);
  out[ 7] = (a[11] >> 8) | (a[12] << 10) | (a[13] << 28);
  out[ 8] = (a[13] >> 4) | (a[14] << 14);
  out[ 9] = (a[15] << 0) | (a[16] << 18);
  out[10] = (a[16] >> 14) | (a[17] << 4) | (a[18] << 22);
  out[11] = (a[18] >> 10) | (a[19] << 8) | (a[20] << 26);
  out[12] = (a[20] >> 6) | (a[21] << 12) | (a[22] << 30);
  out[13] = (a[22] >> 2) | (a[23] << 16);
  out[14] = (a[23] >> 16) | (a[24] << 2) | (a[25] << 20);
  out[15] = (a[25] >> 12) | (a[26] << 6) | (a[27] << 24);
  out[16] = (a[27] >> 8) | (a[28] << 10) | (a[29] << 28);
  out[17] = (a[29] >> 4) | (a[30] << 14);
}
