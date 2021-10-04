/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/digitization/shaperdsp.h>
#include <cmath>
#include <iostream>

using namespace std;
using namespace Belle2::ECL;

const double ShaperDSP_t::_defs[] = {0.5, 0.6483, 0.4017, 0.3741, 0.8494, 0.00144547, 4.7071, 0.8156, 0.5556, 0.2752}; // default parameters

ostream& operator<<(ostream& o, const ShaperDSP_t::sv123shift_t& s)
{
  o << s.t << " " << s.c0 << " " << s.c0 << " " << s.c1 << " " << s.s1 << " " << s.e0 << " " << s.e1 << " " << s.es << " " << s.ed;
  return o;
}

ostream& operator<<(ostream& o, const ShaperDSP_t::shaperdspshift_t& s)
{
  const ShaperDSP_t::sv123shift_t& s0 = static_cast<const ShaperDSP_t::sv123shift_t&>(s);
  o << s0 << " " << s.et0 << " " << s.et1;
  return o;
}

void ShaperDSP_t::sv123shift_t::init(double _t, const ShaperDSP_t& _p)
{
  t = _t;
  sincos(t * _p._dw0, &s0, &c0);
  sincos(t * _p._dw1, &s1, &c1);
  e0 = exp(-t * _p._dks0);
  e1 = exp(-t * _p._dks1);
  es = exp(-t * _p._ds);
  ed = exp(-t * _p._dd);
}

ShaperDSP_t::sv123shift_t& ShaperDSP_t::sv123shift_t::operator +=(const ShaperDSP_t::sv123shift_t& r)
{
  t += r.t;

  double c0r = r.c0 * c0 - r.s0 * s0;
  double s0r = r.c0 * s0 + r.s0 * c0;
  c0 = c0r;
  s0 = s0r;

  double c1r = r.c1 * c1 - r.s1 * s1;
  double s1r = r.c1 * s1 + r.s1 * c1;
  c1 = c1r;
  s1 = s1r;

  e0 *= r.e0;
  e1 *= r.e1;
  es *= r.es;
  ed *= r.ed;
  return *this;
}

ShaperDSP_t::sv123shift_t ShaperDSP_t::sv123shift_t::operator +(const ShaperDSP_t::sv123shift_t& r) const
{
  sv123shift_t a;
  a.t = t + r.t;

  a.c0 = r.c0 * c0 - r.s0 * s0;
  a.s0 = r.c0 * s0 + r.s0 * c0;
  a.c1 = r.c1 * c1 - r.s1 * s1;
  a.s1 = r.c1 * s1 + r.s1 * c1;

  a.e0 = e0 * r.e0;
  a.e1 = e1 * r.e1;
  a.es = es * r.es;
  a.ed = ed * r.ed;
  return a;
}

void ShaperDSP_t::shaperdspshift_t::init(double _t, const ShaperDSP_t& _p)
{
  sv123shift_t::init(_t, _p);
  et0 = exp(-t * _p._dt0);
  et1 = exp(-t * _p._dt1);
}

ShaperDSP_t::shaperdspshift_t& ShaperDSP_t::shaperdspshift_t::operator +=(const ShaperDSP_t::shaperdspshift_t& r)
{
  const sv123shift_t& r0 = static_cast<const ShaperDSP_t::sv123shift_t&>(r);

  sv123shift_t::operator +=(r0);
  et0 *= r.et0;
  et1 *= r.et1;
  return *this;
}

ShaperDSP_t::shaperdspshift_t ShaperDSP_t::shaperdspshift_t::operator +(const ShaperDSP_t::shaperdspshift_t& r) const
{
  const sv123shift_t& r0 = static_cast<const ShaperDSP_t::sv123shift_t&>(r);
  shaperdspshift_t a;
  sv123shift_t& a0 = static_cast<ShaperDSP_t::sv123shift_t&>(a);

  a0 = sv123shift_t::operator+(r0);
  a.et0 = et0 * r.et0;
  a.et1 = et1 * r.et1;
  return a;
}

ShaperDSP_t::shaperdspshift_t operator +(const ShaperDSP_t::shaperdspshift_t& c0, const ShaperDSP_t::shaperdspshift_t& c1)
{
  ShaperDSP_t::shaperdspshift_t a = c0.operator + (c1);
  return a;
}

dd_t operator +(const dd_t& c0, const dd_t& c1)
{
  return dd_t(c0.first + c1.first, c0.second + c1.second);
}

dd_t operator *(double a, const dd_t& c)
{
  return dd_t(a * c.first, a * c.second);
}

void ShaperDSP_t::Sv123_init(double t01, double tb1, double t02, double tb2, double td1, double ts1)
{
  double  dks0, dks1, dksm,
          dw0, dw1, dwp, dwm, das1, dac1, das0, dac0, dzna, dksm2, ds, dd,
          dcs0, dsn0, dzn0, td, ts, dr,
          dcs0s, dsn0s, dcs0d, dsn0d, dcs1s, dsn1s, dcs1d, dsn1d;

  dr = (ts1 - td1) / td1;
  if (std::abs(dr) >= 0.0000001) {
    td = td1;
    ts = ts1;
  } else {
    td = td1;
    if (ts1 > td1) {
      ts = td1 * 1.00001;
    } else {
      ts = td1 * 0.99999;
    }
  }

  dr = ((t01 - t02) * (t01 - t02) + (tb1 - tb2) * (tb1 - tb2)) / (t01 * t01 + tb1 * tb1);
  dks0 = 1 / t01;
  dks1 = 1 / t02;
  if (dr < 0.0000000001) {
    if (dks0 > dks1) {
      dks0 = dks1 * 1.00001;
    } else {
      dks0 = dks1 * 0.99999;
    }
  }

  dksm = dks1 - dks0;

  ds = 1 / ts;
  dd = 1 / td;

  dw0 = 1 / tb1;
  dw1 = 1 / tb2;
  dwp = dw0 + dw1;
  dwm = dw1 - dw0;

  dksm2 = dksm * dksm;

  dzna = (dksm2 + dwm * dwm) * (dksm2 + dwp * dwp);

  das0 = dw1 * (dksm2 + dwp * dwm);
  dac0 = -2 * dksm * dw0 * dw1;
  das1 = dw0 * (dksm2 - dwp * dwm);
  dac1 = -dac0;

  dsn0 = (ds - dks0);
  dcs0 = -dw0;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn0s = (dsn0 * das0 - dcs0 * dac0) / dzn0;
  dcs0s = (dcs0 * das0 + dsn0 * dac0) / dzn0;

  dsn0 = (ds - dks1);
  dcs0 = -dw1;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn1s = (dsn0 * das1 - dcs0 * dac1) / dzn0;
  dcs1s = (dcs0 * das1 + dsn0 * dac1) / dzn0;

  dsn0 = (dd - dks0);
  dcs0 = -dw0;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn0d = (dsn0 * das0 - dcs0 * dac0) / dzn0;
  dcs0d = (dcs0 * das0 + dsn0 * dac0) / dzn0;

  dsn0 = (dd - dks1);
  dcs0 = -dw1;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn1d = (dsn0 * das1 - dcs0 * dac1) / dzn0;
  dcs1d = (dcs0 * das1 + dsn0 * dac1) / dzn0;

  _dw0  = dw0;
  _dw1  = dw1;
  _dks0 = dks0;
  _dks1 = dks1;
  _ds   = ds;
  _dd   = dd;

  double sc = 1 / (dzna * (ts - td));
  _cs0 = sc * (dsn0s - dsn0d);
  _cc0 = sc * (dcs0s - dcs0d);
  _cs1 = sc * (dsn1s - dsn1d);
  _cc1 = sc * (dcs1s - dcs1d);
  _ces = sc * (dcs0s + dcs1s);
  _ced = sc * (dcs0d + dcs1d);
}

double ShaperDSP_t::Sv123(const sv123shift_t& c) const
{
  double f0 = (_cs0 * c.s0 + _cc0 * c.c0) * c.e0;
  double f1 = (_cs1 * c.s1 + _cc1 * c.c1) * c.e1;
  double fs =  _ces * c.es;
  double fd =  _ced * c.ed;

  return (f0 + f1) + (fd - fs);
}

dd_t ShaperDSP_t::ddSv123(const sv123shift_t& c) const
{
  double f0 = (_cs0 * c.s0 + _cc0 * c.c0) * c.e0;
  double f1 = (_cs1 * c.s1 + _cc1 * c.c1) * c.e1;
  double fs =  _ces * c.es;
  double fd =  _ced * c.ed;

  double f0p = (_dw0 * c.e0) * (_cs0 * c.c0 - _cc0 * c.s0) - (_dks0 * f0);
  double f1p = (_dw1 * c.e1) * (_cs1 * c.c1 - _cc1 * c.s1) - (_dks1 * f1);
  double fsp =  _ds * fs;
  double fdp =  _dd * fd;

  double f  = (f0  + f1) + (fd  - fs);
  double fp = (f0p + f1p) - (fdp - fsp);
  return dd_t(f, fp);
}

double ShaperDSP_t::Sv123_filtered(const sv123shift_t& t0) const
{
  if (t0.validshift(_tp)) {
    double ft0p = Sv123(t0 + _tp), ft0 = 0, ft0m = 0;
    if (t0.t > 0) {
      ft0 = Sv123(t0);
      if (t0.validshift(_tm))
        ft0m = Sv123(t0 + _tm);
    }
    return _w0 * ft0 + _w1 * (ft0p + ft0m);
  }
  return 0;
}

dd_t ShaperDSP_t::ddSv123_filtered(const sv123shift_t& t0) const
{
  if (t0.validshift(_tp)) {
    dd_t ft0p = ddSv123(t0 + _tp), ft0 = dd_t(0, 0), ft0m = dd_t(0, 0);
    if (t0.t > 0) {
      ft0 = ddSv123(t0);
      if (t0.validshift(_tm))
        ft0m = ddSv123(t0 + _tm);
    }
    return _w0 * ft0 + _w1 * (ft0p + ft0m);
  }
  return dd_t(0, 0);
}

double ShaperDSP_t::ShaperDSP(const shaperdspshift_t& t0) const
{
  const sv123shift_t& t00 = static_cast<const ShaperDSP_t::sv123shift_t&>(t0);
  double f = Sv123_filtered(t00);
  if (t0.t > 0) {
    double z = t0.t * _dt1;
    double z2 = z * z;
    double odd = 1 + z2 * (1 / 6. + z2 * (1 / 120.));
    double evn = 1 + z2 * (1 / 2. + z2 * (1 / 24.));
    double texp = evn + z * odd;
    double df = _ccc * t0.et0 * (1 - t0.et1 * texp);
    f -= df;
  }
  return f;
}

dd_t ShaperDSP_t::ddShaperDSP(const shaperdspshift_t& t0) const
{
  const sv123shift_t& t00 = static_cast<const ShaperDSP_t::sv123shift_t&>(t0);
  dd_t f = ddSv123_filtered(t00);
  if (t0.t > 0) {
    double z = t0.t * _dt1;
    double z2 = z * z;
    double odd = 1 + z2 * (1 / 6. + z2 * (1 / 120.));
    double evn = 1 + z2 * (1 / 2. + z2 * (1 / 24.));
    double texp = evn + z * odd;
    double u = t0.et0 * (1 - t0.et1 * texp);
    f.first -= _ccc * u;

    double up = -_dt0 * u + ((_dt1 * t0.et0) * (t0.et1 * z2)) * ((z2 * z) * (1 / 120.));
    f.second -= _ccc * up;
  }
  return f;
}

void ShaperDSP_t::init(const double* s, double unitscale)
{
  double t01 = s[2];
  double tb1 = s[3];
  double t02 = s[7];
  double tb2 = s[8];
  double td1 = s[1];
  double ts1 = s[4];
  Sv123_init(t01, tb1, t02, tb2, td1, ts1);
  _dt0 = 1 / s[6];
  _dt1 = 1 / s[2];

  _toff = s[0];
  _w0   = 1.0 - s[9];
  _w1   = 0.5 * s[9];
  _ccc  = s[5];

  if (unitscale < 0) {
    /**
     * Calculate unitscale from shape parameters based on an
     * approximated formula.
     *
     * Slower alternative is to iterate over tabulated values
     * of the signal shape function and set unitscale to the
     * maximum value.
     */
    unitscale = 1.0
                / (-.109  + .919    * t01 - .261 * t01 * t01)
                / (-.109  + .919    * t02 - .261 * t02 * t02)
                / (.262   + .174    * tb1 - .208 * tb1 * tb1)
                / (.262   + .174    * tb2 - .208 * tb2 * tb2)
                / (4.56   - 1.58    * td1)
                / (1.391  - 0.434   * ts1)
                / (1.06   - 0.578   * (t01 - tb1) * (t01 - tb1))
                / (1.06   - 0.578   * (t02 - tb2) * (t02 - tb2))
                / (1.2140 - 0.79645 * t01 + 0.63440 * t01 * t01)
                / (1.2140 - 0.79645 * t02 + 0.63440 * t02 * t02);
  }

  _cs0 *= unitscale;
  _cc0 *= unitscale;
  _cs1 *= unitscale;
  _cc1 *= unitscale;
  _ces *= unitscale;
  _ced *= unitscale;

  _tp.init(_filterdt, *this);
  _tm.init(-_filterdt, *this);
}

void ShaperDSP_t::init(const double* s)
{
  init(s, -1);
}

void ShaperDSP_t::init(const std::vector<double>& s, double unitscale)
{
  if (s.size() == 10)
    init(s.data(), unitscale);
  else
    init(_defs, unitscale);
}

double ShaperDSP_t::operator()(double t) const
{
  shaperdspshift_t t0 = shaperdspshift_t(t  - _toff, *this);
  return ShaperDSP(t0);
}

double ShaperDSP_t::operator()(double* x, double*)
{
  return (*this)(x[0]);
}

void ShaperDSP_t::settimestride(double dt)
{
  _tstride.init(dt , *this);
}

void ShaperDSP_t::setseedoffset(double dt)
{
  _toffset.init(dt , *this);
}

void ShaperDSP_t::settimeseed(double t0)
{
  _tzero.init(t0 - _toff, *this);
}

void ShaperDSP_t::nextseed()
{
  _tzero += _toffset;
}

void ShaperDSP_t::fillarray(int n, double* s) const
{
  shaperdspshift_t t0 = _tzero;

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ShaperDSP(t0);
  }
}

void ShaperDSP_t::fillarray(int n, dd_t* s) const
{
  shaperdspshift_t t0 = _tzero;

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ddShaperDSP(t0);
  }
}

void ShaperDSP_t::fillarray(double t, int n, double* s) const
{
  shaperdspshift_t t0 = shaperdspshift_t(t  - _toff, *this);

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ShaperDSP(t0);
  }
}

void ShaperDSP_t::fillarray(double t, int n, dd_t* s) const
{
  shaperdspshift_t t0 = shaperdspshift_t(t  - _toff, *this);

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ddShaperDSP(t0);
  }
}

void ShaperDSP_t::fillvector(std::vector<double>& s) const
{
  fillarray(s.size(), s.data());
}

void ShaperDSP_t::fillvector(std::vector<dd_t>& s) const
{
  fillarray(s.size(), s.data());
}

void ShaperDSP_t::fillvector(double t, std::vector<double>& s) const
{
  fillarray(t, s.size(), s.data());
}

void ShaperDSP_t::fillvector(double t, std::vector<dd_t>& s) const
{
  fillarray(t, s.size(), s.data());
}
