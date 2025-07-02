/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <ecl/digitization/shaperdspUp.h>
#include <ecl/digitization/EclConfigurationUp.h>
#include <cmath>
#include <iostream>
//#include <TMath.h>

using namespace std;
using namespace Belle2::ECL;

const double t_adc_up = Belle2::ECL::EclConfigurationUp::s_clock / 508.876; // ADC clock period in us
const double time_to_units_up = Belle2::ECL::EclConfigurationUp::m_step / t_adc_up; // Convert time to units of ADC clock

// tau_d, tau_0, omega_0, tau_1, omega_1, tau_i, f_i, tau_s, tau_s1, alpha, t_shift, eta, a_norm, toff
const double ShaperDSPUp_t::_defs[] = {0.242 * time_to_units_up, 0.2111 * time_to_units_up, time_to_units_up / 0.1767, 0.2441 * time_to_units_up, time_to_units_up / 0.0929, 11.134 * time_to_units_up, 0.0872, 0.966 * time_to_units_up, 6.975 * time_to_units_up, 0.603, 0.378 * time_to_units_up, 0.720, 0.70235, 0.0}; // default parameters


ShaperDSPUp_t::sfshift_t& ShaperDSPUp_t::sfshift_t::operator +=(const ShaperDSPUp_t::sfshift_t& r)
{

  t += r.t;

  double sm0r = sm0 * r.c0 + cm0 * r.s0;
  double sm1r = sm1 * r.c1 + cm1 * r.s1;
  double cm0r = cm0 * r.c0 - sm0 * r.s0;
  double cm1r = cm1 * r.c1 - sm1 * r.s1;
  sm0 = sm0r;
  sm1 = sm1r;
  cm0 = cm0r;
  cm1 = cm1r;

  double si0r = si0 * r.c0 + ci0 * r.s0;
  double si1r = si1 * r.c1 + ci1 * r.s1;
  double ci0r = ci0 * r.c0 - si0 * r.s0;
  double ci1r = ci1 * r.c1 - si1 * r.s1;
  si0 = si0r;
  si1 = si1r;
  ci0 = ci0r;
  ci1 = ci1r;

  double sm01r = sm01 * r.c0 + cm01 * r.s0;
  double sm11r = sm11 * r.c1 + cm11 * r.s1;
  double cm01r = cm01 * r.c0 - sm01 * r.s0;
  double cm11r = cm11 * r.c1 - sm11 * r.s1;
  sm01 = sm01r;
  sm11 = sm11r;
  cm01 = cm01r;
  cm11 = cm11r;

  double si01r = si01 * r.c0 + ci01 * r.s0;
  double si11r = si11 * r.c1 + ci11 * r.s1;
  double ci01r = ci01 * r.c0 - si01 * r.s0;
  double ci11r = ci11 * r.c1 - si11 * r.s1;
  si01 = si01r;
  si11 = si11r;
  ci01 = ci01r;
  ci11 = ci11r;

  // needed for operator +
  double s0r = s0 * r.c0 + c0 * r.s0;
  double s1r = s1 * r.c1 + c1 * r.s1;
  double c0r = c0 * r.c0 - s0 * r.s0;
  double c1r = c1 * r.c1 - s1 * r.s1;
  s0 = s0r;
  s1 = s1r;
  c0 = c0r;
  c1 = c1r;

  e0 *= r.e0;
  e1 *= r.e1;
  es *= r.es;
  es1 *= r.es1;
  ed *= r.ed;
  ei *= r.ei;
  return *this;

}

ShaperDSPUp_t::sfshift_t ShaperDSPUp_t::sfshift_t::operator +(const ShaperDSPUp_t::sfshift_t& r) const
{
  sfshift_t a;
  a.t = t + r.t;

  a.sm0 = sm0 * r.c0 + cm0 * r.s0;
  a.sm1 = sm1 * r.c1 + cm1 * r.s1;
  a.cm0 = cm0 * r.c0 - sm0 * r.s0;
  a.cm1 = cm1 * r.c1 - sm1 * r.s1;

  a.si0 = si0 * r.c0 + ci0 * r.s0;
  a.si1 = si1 * r.c1 + ci1 * r.s1;
  a.ci0 = ci0 * r.c0 - si0 * r.s0;
  a.ci1 = ci1 * r.c1 - si1 * r.s1;

  a.sm01 = sm01 * r.c0 + cm01 * r.s0;
  a.sm11 = sm11 * r.c1 + cm11 * r.s1;
  a.cm01 = cm01 * r.c0 - sm01 * r.s0;
  a.cm11 = cm11 * r.c1 - sm11 * r.s1;

  a.si01 = si01 * r.c0 + ci01 * r.s0;
  a.si11 = si11 * r.c1 + ci11 * r.s1;
  a.ci01 = ci01 * r.c0 - si01 * r.s0;
  a.ci11 = ci11 * r.c1 - si11 * r.s1;

  // needed for operator +
  a.s0 = s0 * r.c0 + c0 * r.s0;
  a.s1 = s1 * r.c1 + c1 * r.s1;
  a.c0 = c0 * r.c0 - s0 * r.s0;
  a.c1 = c1 * r.c1 - s1 * r.s1;

  a.e0 = e0 * r.e0;
  a.e1 = e1 * r.e1;
  a.es = es * r.es;
  a.es1 = es1 * r.es1;
  a.ed = ed * r.ed;
  a.ei = ed * r.ei;
  return a;
}

void ShaperDSPUp_t::init(const double* s, double unitscale)
{
  double dd = 1 / s[0];
  double d0 = 1 / s[1];
  double w0 = s[2];
  double d1 = 1 / s[3];
  double w1 = s[4];

  double di = 1 / s[5];
  double ds = 1 / s[7];
  double ds1 = 1 / s[8];

  Sf_init(dd, d0, w0, d1, w1, di, ds, ds1);

  _fi = s[6];
  _alpha = s[9];
  _tshift = s[10];
  _eta = s[11];
  _anorm = s[12];
  _toff = s[13];

  _an *= unitscale;
  _ai *= unitscale;
  _an1 *= unitscale;
  _ai1 *= unitscale;

}

void ShaperDSPUp_t::init(const double* s)
{
  init(s, -1);
}

void ShaperDSPUp_t::init(const std::vector<double>& s, double unitscale)
{
  if (s.size() == 14)
    init(s.data(), unitscale);
  else
    init(_defs, unitscale);
}


void ShaperDSPUp_t::Sf_init(double dd, double d0, double w0, double d1, double w1, double di, double ds, double ds1)
{

  double dd01, dd0s, dd1s, dd0d, dd1d,
         dd0i, dd1i, ddsd, ddsi, dddi,
         dd0s1, dd1s1, dds1d, dds1i;


  dd01 = d0 - d1; // 1/tau_0 - 1/tau_1
  dd0s = d0 - ds; // 1/tau_0 - 1/tau_s
  dd1s = d1 - ds; // 1/tau_1 - 1/tau_s
  dd0d = d0 - dd; // 1/tau_0 - 1/tau_d
  dd1d = d1 - dd; // 1/tau_1 - 1/tau_d
  dd0i = d0 - di; // 1/tau_0 - 1/tau_i
  dd1i = d1 - di; // 1/tau_1 - 1/tau_i
  ddsd = ds - dd; // 1/tau_s - 1/tau_d
  ddsi = ds - di; // 1/tau_s - 1/tau_i
  dddi = dd - di; // 1/tau_d - 1/tau_i

  dd0s1 = d0 - ds1; // 1/tau_0 - 1/tau_s1
  dd1s1 = d1 - ds1; // 1/tau_1 - 1/tau_s1
  dds1d = ds1 - dd; // 1/tau_s1 - 1/tau_d
  dds1i = ds1 - di; // 1/tau_s1 - 1/tau_i

  double r01, phi01, phi10,
         r0s, phi0s, r1s, phi1s,
         r0d, phi0d, r1d, phi1d,
         r0i, phi0i, r1i, phi1i,
         r0s1, phi0s1, r1s1, phi1s1;

  _an = -(pow(w0, 2) + pow(d0, 2)) * (pow(w1, 2) + pow(d1, 2)) * (ds * dd);
  _ai = - _an * di;

  _an1 = -(pow(w0, 2) + pow(d0, 2)) * (pow(w1, 2) + pow(d1, 2)) * (ds1 * dd);
  _ai1 = - _an1 * di;

  r01 = sqrt(pow(pow(w0, 2) + pow(w1, 2) + pow(dd01, 2), 2) - pow(2 * w0 * w1, 2));
  phi01 = atan2(2 * w0 * dd01, pow(w1, 2) - pow(w0, 2) + pow(dd01, 2));
  phi10 = atan2(-2 * w1 * dd01, pow(w0, 2) - pow(w1, 2) + pow(dd01, 2));
  r0s = sqrt(pow(w0, 2) + pow(dd0s, 2));
  phi0s = atan2(-dd0s, w0);
  r1s = sqrt(pow(w1, 2) + pow(dd1s, 2));
  phi1s = atan2(-dd1s, w1);
  r0d = sqrt(pow(w0, 2) + pow(dd0d, 2));
  phi0d = atan2(-dd0d, w0);
  r1d = sqrt(pow(w1, 2) + pow(dd1d, 2));
  phi1d = atan2(-dd1d, w1);
  r0i = sqrt(pow(w0, 2) + pow(dd0i, 2));
  phi0i = atan2(-dd0i, w0);
  r1i = sqrt(pow(w1, 2) + pow(dd1i, 2));
  phi1i = atan2(-dd1i, w1);

  r0s1 = sqrt(pow(w0, 2) + pow(dd0s1, 2));
  phi0s1 = atan2(-dd0s1, w0);
  r1s1 = sqrt(pow(w1, 2) + pow(dd1s1, 2));
  phi1s1 = atan2(-dd1s1, w1);

  _w0 = w0;
  _w1 = w1;
  _d0 = d0;
  _d1 = d1;
  _di = di;
  _ds = ds;
  _dd = dd;
  _ds1 = ds1;

  _phi0 = phi01 + phi0s + phi0d;
  _phi1 = phi10 + phi1s + phi1d;
  _phi0i = _phi0 + phi0i;
  _phi1i = _phi1 + phi1i;

  _phi01 = phi01 + phi0s1 + phi0d;
  _phi11 = phi10 + phi1s1 + phi1d;
  _phi0i1 = _phi01 + phi0i;
  _phi1i1 = _phi11 + phi1i;

  _cs0 = 1. / (w0 * r01 * r0s * r0d);
  _cs1 = 1. / (w1 * r01 * r1s * r1d);
  _cc0 = _cs0 / r0i;
  _cc1 = _cs1 / r1i;
  _cems = 1. / (pow(r0s, 2) * pow(r1s, 2) * ddsd);
  _cemd = 1. / (pow(r0d, 2) * pow(r1d, 2) * ddsd);
  _ceis = _cems / ddsi;
  _ceid = _cemd / dddi;
  _ceii = 1. / (pow(r0i, 2) * pow(r1i, 2) * (-dddi) * (-ddsi));

  _cs01 = 1. / (w0 * r01 * r0s1 * r0d);
  _cs11 = 1. / (w1 * r01 * r1s1 * r1d);
  _cc01 = _cs01 / r0i;
  _cc11 = _cs11 / r1i;
  _cems1 = 1. / (pow(r0s1, 2) * pow(r1s1, 2) * dds1d);
  _cemd1 = 1. / (pow(r0d, 2) * pow(r1d, 2) * dds1d);
  _ceis1 = _cems1 / dds1i;
  _ceid1 = _cemd1 / dddi;
  _ceii1 = 1. / (pow(r0i, 2) * pow(r1i, 2) * (-dddi) * (-dds1i));


}

void ShaperDSPUp_t::sfshift_t::init(double _t, const ShaperDSPUp_t& _p)
{
  t = _t;

  sincos(t * _p._w0 + _p._phi0, &sm0, &cm0);
  sincos(t * _p._w1 + _p._phi1, &sm1, &cm1);
  sincos(t * _p._w0 + _p._phi0i, &si0, &ci0);
  sincos(t * _p._w1 + _p._phi1i, &si1, &ci1);

  sincos(t * _p._w0 + _p._phi01, &sm01, &cm01);
  sincos(t * _p._w1 + _p._phi11, &sm11, &cm11);
  sincos(t * _p._w0 + _p._phi0i1, &si01, &ci01);
  sincos(t * _p._w1 + _p._phi1i1, &si11, &ci11);

  // needed for operator +
  sincos(t * _p._w0, &s0, &c0);
  sincos(t * _p._w1, &s1, &c1);

  e0 = exp(-t * _p._d0);
  e1 = exp(-t * _p._d1);
  es = exp(-t * _p._ds);
  ed = exp(-t * _p._dd);
  ei = exp(-t * _p._di);
  es1 = exp(-t * _p._ds1);
}

double ShaperDSPUp_t::Sf(const sfshift_t& c) const
{
  if (c.isPedestal(0.))
    return 0.0;

  // Fm and Fi are the Fm and Fi parts of the response function
  double Fm = _an * (_cs0 * c.e0 * c.sm0 + _cs1 * c.e1 * c.sm1 + _cems * c.es - _cemd * c.ed);
  double Fi = _ai * (_cc0 * c.e0 * c.ci0 + _cc1 * c.e1 * c.ci1 + _ceis * c.es - _ceid * c.ed + _ceii * c.ei);

  double Fm1 = _an1 * (_cs01 * c.e0 * c.sm01 + _cs11 * c.e1 * c.sm11 + _cems1 * c.es1 - _cemd1 * c.ed);
  double Fi1 = _ai1 * (_cc01 * c.e0 * c.ci01 + _cc11 * c.e1 * c.ci11 + _ceis1 * c.es1 - _ceid1 * c.ed + _ceii1 * c.ei);

  return (Fm - _fi * Fi) + _alpha * (Fm1 - _fi * Fi1);
}

double ShaperDSPUp_t::ddSf(const sfshift_t& c) const
{
  if (c.isPedestal(0.))
    return 0.0;

  // Fm and Fi are the Fm and Fi parts of the response function
  double ddFm = _an * (_cs0 * c.e0 * (_w0 * c.cm0 - _d0 * c.sm0) + _cs1 * c.e1 * (_w1 * c.cm1 - _d1 * c.sm1) - _cems * _ds * c.es +
                       _cemd * _dd * c.ed);
  double ddFi = _ai * (_cc0 * c.e0 * (- _w0 * c.si0 - _d0 * c.ci0) + _cc1 * c.e1 * (- _w1 * c.si1 - _d1 * c.ci1) - _ceis * _ds * c.es
                       + _ceid * _dd * c.ed - _ceii * _di * c.ei);

  double ddFm1 = _an1 * (_cs01 * c.e0 * (_w0 * c.cm01 - _d0 * c.sm01) + _cs11 * c.e1 * (_w1 * c.cm11 - _d1 * c.sm11) - _cems1 * _ds1 *
                         c.es1 + _cemd1 * _dd * c.ed);
  double ddFi1 = _ai1 * (_cc01 * c.e0 * (- _w0 * c.si01 - _d0 * c.ci01) + _cc11 * c.e1 * (- _w1 * c.si11 - _d1 * c.ci11) - _ceis1 *
                         _ds1 * c.es1 + _ceid1 * _dd * c.ed - _ceii1 * _di * c.ei);

  return (ddFm - _fi * ddFi) + _alpha * (ddFm1 - _fi * ddFi1);
}

double ShaperDSPUp_t::ShaperDSPUp(const sfshift_t& t0) const
{

  double f = Sf(t0);

  ShaperDSPUp_t::sfshift_t t0_shift;
  t0_shift.init(t0.t - _tshift, *this);
  double fshift = Sf(t0_shift);

  return (f - _eta * fshift) / _anorm ;
}

dd_t ShaperDSPUp_t::ddShaperDSPUp(const sfshift_t& t0) const
{
  const sfshift_t& t00 = static_cast<const ShaperDSPUp_t::sfshift_t&>(t0);
  double f = Sf(t00);
  double fp = ddSf(t00);

  ShaperDSPUp_t::sfshift_t t00_shift;
  t00_shift.init(t00.t - _tshift, *this);

  double fshift = Sf(t00_shift);
  double fpshift = ddSf(t00_shift);

  return dd_t((f - _eta * fshift) / _anorm, (fp - _eta * fpshift) / _anorm);
}

void ShaperDSPUp_t::settimestride(double dt)
{
  _tstride.init(dt, *this);
}

void ShaperDSPUp_t::setseedoffset(double dt)
{
  _toffset.init(dt, *this);
}

void ShaperDSPUp_t::settimeseed(double t0)
{
  _tzero.init(t0 - _toff, *this);
}

void ShaperDSPUp_t::nextseed()
{
  _tzero += _toffset;
}

void ShaperDSPUp_t::fillarray(int n, double* s) const
{
  sfshift_t t0 = _tzero;

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ShaperDSPUp(t0);
  }
}

void ShaperDSPUp_t::fillarray(int n, dd_t* s) const
{
  sfshift_t t0 = _tzero;

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ddShaperDSPUp(t0);
  }
}

void ShaperDSPUp_t::fillarray(double t, int n, double* s) const
{
  sfshift_t t0 = sfshift_t(t  - _toff, *this);

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ShaperDSPUp(t0);
  }
}

void ShaperDSPUp_t::fillarray(double t, int n, dd_t* s) const
{
  sfshift_t t0 = sfshift_t(t  - _toff, *this);

  for (int i = 0; i < n; i++, t0 += _tstride) {
    s[i] = ddShaperDSPUp(t0);
  }
}

void ShaperDSPUp_t::fillvector(std::vector<double>& s) const
{
  fillarray(s.size(), s.data());
}

void ShaperDSPUp_t::fillvector(std::vector<dd_t>& s) const
{
  fillarray(s.size(), s.data());
}

void ShaperDSPUp_t::fillvector(double t, std::vector<double>& s) const
{
  fillarray(t, s.size(), s.data());
}

void ShaperDSPUp_t::fillvector(double t, std::vector<dd_t>& s) const
{
  fillarray(t, s.size(), s.data());
}
