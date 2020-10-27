#include <math.h>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <complex>
#include "ME-pair.hh"
using namespace std;

double dilog(double X)
{
// CERN      C304      VERSION    29/07/71 DILOG        59                C
  double Z, T, S, Y, B, A, di;
  Z = -1.64493406684822;
  if (X < -1.0) goto case1;
  if (X <= 0.5) goto case2;
  if (X == 1.0) goto case3;
  if (X <= 2.0) goto case4;
  Z = 3.2898681336964;
case1:
  T = 1.0 / X;
  S = -0.5;
  Z = Z - 0.5 * log(abs(X)) * log(abs(X));
  goto case5;
case2:
  T = X;
  S = 0.5;
  Z = 0.0;
  goto case5;
case3:
  di = 1.64493406684822;
  return di;
case4:
  T = 1.0 - X;
  S = -0.5;
  Z = 1.64493406684822 - log(X) * log(abs(T));
case5:
  Y = 2.66666666666666 * T + 0.66666666666666;
  B =      0.000000000000001;
  A = Y * B  + 0.000000000000004;
  B = Y * A - B + 0.000000000000011;
  A = Y * B - A + 0.000000000000037;
  B = Y * A - B + 0.000000000000121;
  A = Y * B - A + 0.000000000000398;
  B = Y * A - B + 0.000000000001312;
  A = Y * B - A + 0.000000000004342;
  B = Y * A - B + 0.000000000014437;
  A = Y * B - A + 0.000000000048274;
  B = Y * A - B + 0.000000000162421;
  A = Y * B - A + 0.000000000550291;
  B = Y * A - B + 0.000000001879117;
  A = Y * B - A + 0.000000006474338;
  B = Y * A - B + 0.000000022536705;
  A = Y * B - A + 0.000000079387055;
  B = Y * A - B + 0.000000283575385;
  A = Y * B - A + 0.000001029904264;
  B = Y * A - B + 0.000003816329463;
  A = Y * B - A + 0.000014496300557;
  B = Y * A - B + 0.000056817822718;
  A = Y * B - A + 0.000232002196094;
  B = Y * A - B + 0.001001627496164;
  A = Y * B - A + 0.004686361959447;
  B = Y * A - B + 0.024879322924228;
  A = Y * B - A + 0.166073032927855;
  A = Y * A - B + 1.93506430086996;
  di = S * T * (A - B) + Z;
  return di;
}
//**************************************************************************
// demo pi- e- e+ ME, based on paper arxiv.org/abs/1306.1732               *
//**************************************************************************
void eepi_(const float* pt, const float* pn, const float* p1, const float* p2, const float* p3, float& amplit, float* hv)
{

  double Mtau = 1.777;
  double me = 0.000511;
  double Gf = 0.0000116637;
  double fpi = 0.0922;
  double vud = 0.9742;

  float p23[4]; // momentum of e- e+ pair
  p23[3] = p3[3] + p2[3];
  p23[2] = p3[2] + p2[2];
  p23[1] = p3[1] + p2[1];
  p23[0] = p3[0] + p2[0];
  float m2ee = p23[3] * p23[3] - p23[2] * p23[2] - p23[1] * p23[1] - p23[0] * p23[0];

  float col[4];
  col[3] = p1[3] * p23[3];
  col[2] = p1[2] * p23[2];
  col[1] = p1[1] * p23[1];
  col[0] = p1[0] * p23[0];
  float pk = col[3] - col[2] - col[1] - col[0];

  float ptk[4];
  ptk[3] = pt[3] * p23[3];
  ptk[2] = pt[2] * p23[2];
  ptk[1] = pt[1] * p23[1];
  ptk[0] = pt[0] * p23[0];
  float ptp23 = ptk[3] - ptk[2] - ptk[1] - ptk[0];

  float kpn[4];
  kpn[3] = pn[3] * p23[3];
  kpn[2] = pn[2] * p23[2];
  kpn[1] = pn[1] * p23[1];
  kpn[0] = pn[0] * p23[0];
  float pnk = kpn[3] - kpn[2] - kpn[1] - kpn[0];

  float ptq[4];
  ptq[3] = pn[3] * pt[3];
  ptq[2] = pn[2] * pt[2];
  ptq[1] = pn[1] * pt[1];
  ptq[0] = pn[0] * pt[0];
  float ptpn = ptq[3] - ptq[2] - ptq[1] - ptq[0];

  float pp[4];
  pp[3] = p2[3] * p3[3];
  pp[2] = p2[2] * p3[2];
  pp[1] = p2[1] * p3[1];
  pp[0] = p2[0] * p3[0];
  float p2p3 = pp[3] - pp[2] - pp[1] - pp[0];

  float tmp[4];
  tmp[3] = p2[3] * pt[3];
  tmp[2] = p2[2] * pt[2];
  tmp[1] = p2[1] * pt[1];
  tmp[0] = p2[0] * pt[0];
  float p2pt = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p3[3] * pt[3];
  tmp[2] = p3[2] * pt[2];
  tmp[1] = p3[1] * pt[1];
  tmp[0] = p3[0] * pt[0];
  float p3pt = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p2[3] * pn[3];
  tmp[2] = p2[2] * pn[2];
  tmp[1] = p2[1] * pn[1];
  tmp[0] = p2[0] * pn[0];
  float p2pn = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p3[3] * pn[3];
  tmp[2] = p3[2] * pn[2];
  tmp[1] = p3[1] * pn[1];
  tmp[0] = p3[0] * pn[0];
  float p3pn = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p2[3] * p1[3];
  tmp[2] = p2[2] * p1[2];
  tmp[1] = p2[1] * p1[1];
  tmp[0] = p2[0] * p1[0];
  float p2p1 = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p3[3] * p1[3];
  tmp[2] = p3[2] * p1[2];
  tmp[1] = p3[1] * p1[1];
  tmp[0] = p3[0] * p1[0];
  float p3p1 = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p1[3] * pn[3];
  tmp[2] = p1[2] * pn[2];
  tmp[1] = p1[1] * pn[1];
  tmp[0] = p1[0] * pn[0];
  float p1pn = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p1[3] * pt[3];
  tmp[2] = p1[2] * pt[2];
  tmp[1] = p1[1] * pt[1];
  tmp[0] = p1[0] * pt[0];
  float p1pt = tmp[3] - tmp[2] - tmp[1] - tmp[0];

  float ptsq = pt[3] * pt[3] - pt[2] * pt[2] - pt[1] * pt[1] - pt[0] * pt[0];
  float p1sq = p1[3] * p1[3] - p1[2] * p1[2] - p1[1] * p1[1] - p1[0] * p1[0];

  // eq. 31 of https://arxiv.org/pdf/1306.1732.pdf
  float d1, d2;  //denominator parts
  d1 = m2ee - 2.*ptp23;
  d2 = m2ee + 2.*pk;
  float c1, c2, c3, c4, c5, c6, c7, c8, c9, a; // 9 contractions needed

  a = (p2p3 + me * me); // part of l_munu, used in all contractions

  c1 = p2pt * p3pn + p3pt * p2pn - p2p3 * ptpn - ptpn * a;
  c1 = -(c1 * 2. + a * ptpn) * m2ee / d1 / d1;

  c2 = p2p1 * p3pn + p2pn * p3p1 - p1pn * a;
  c2 = c2 * 4.*ptp23 / d1 / d2;

  c3 = p2pt * p3pn + p2pn * p3pt - ptpn * a;
  c3 = c3 * 4.*ptp23 / d1 / d1;

  c4 = 2.*p2p3 - a;
  c4 = -c4 * 2.*pnk * ptp23 / d1 / d1;

  c5 = p2p1 * p3pt + p2pt * p3p1 - p1pt * a;
  c5 = -c5 * 4.*pnk / d1 / d2;

  c6 = 2.*p2pt * p3pt - ptsq * a;
  c6 = -c6 * 4.*pnk / d1 / d1;

  c7 = p2p1 * p3pt + p2pt * p3p1 - p1pt * a;
  c7 = c7 * 8.*ptpn / d1 / d2;

  c8 = 2.*p2p1 * p3p1 - p1sq * a;
  c8 = c8 * 4.*ptpn / d2 / d2;

  c9 = 2.*p2pt * p3pt - ptsq * a;
  c9 = c9 * 4.*ptpn / d1 / d1;

  float ce;
  ce = 16.*Gf * Gf * Mtau * Mtau * fpi * fpi * vud * vud * (16.*M_PI * M_PI / 137. / 137.);

  amplit = (c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9) / m2ee / m2ee;
  amplit = amplit * ce;

  hv[0] = 1.0;
  hv[1] = 0.0;
  hv[2] = 0.0;
  hv[3] = 0.0;

}

//**************************************************************************
// demo pi- mu- mu+ ME, based on paper arxiv.org/abs/1306.1732             *
//**************************************************************************
void mumupi_(const float* pt, const float* pn, const float* p1, const float* p2, const float* p3, float& amplit, float* hv)
{

  double Mtau = 1.777;
  double me = 0.1056584;
  double Gf = 0.0000116637;
  double fpi = 0.0922;
  double vud = 0.9742;

  float p23[4]; // momentum of pair
  p23[3] = p3[3] + p2[3];
  p23[2] = p3[2] + p2[2];
  p23[1] = p3[1] + p2[1];
  p23[0] = p3[0] + p2[0];
  float m2ee = p23[3] * p23[3] - p23[2] * p23[2] - p23[1] * p23[1] - p23[0] * p23[0];

  float col[4];
  col[3] = p1[3] * p23[3];
  col[2] = p1[2] * p23[2];
  col[1] = p1[1] * p23[1];
  col[0] = p1[0] * p23[0];
  float pk = col[3] - col[2] - col[1] - col[0];

  float ptk[4];
  ptk[3] = pt[3] * p23[3];
  ptk[2] = pt[2] * p23[2];
  ptk[1] = pt[1] * p23[1];
  ptk[0] = pt[0] * p23[0];
  float ptp23 = ptk[3] - ptk[2] - ptk[1] - ptk[0];

  float kpn[4];
  kpn[3] = pn[3] * p23[3];
  kpn[2] = pn[2] * p23[2];
  kpn[1] = pn[1] * p23[1];
  kpn[0] = pn[0] * p23[0];
  float pnk = kpn[3] - kpn[2] - kpn[1] - kpn[0];

  float ptq[4];
  ptq[3] = pn[3] * pt[3];
  ptq[2] = pn[2] * pt[2];
  ptq[1] = pn[1] * pt[1];
  ptq[0] = pn[0] * pt[0];
  float ptpn = ptq[3] - ptq[2] - ptq[1] - ptq[0];

  float pp[4];
  pp[3] = p2[3] * p3[3];
  pp[2] = p2[2] * p3[2];
  pp[1] = p2[1] * p3[1];
  pp[0] = p2[0] * p3[0];
  float p2p3 = pp[3] - pp[2] - pp[1] - pp[0];

  float tmp[4];
  tmp[3] = p2[3] * pt[3];
  tmp[2] = p2[2] * pt[2];
  tmp[1] = p2[1] * pt[1];
  tmp[0] = p2[0] * pt[0];
  float p2pt = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p3[3] * pt[3];
  tmp[2] = p3[2] * pt[2];
  tmp[1] = p3[1] * pt[1];
  tmp[0] = p3[0] * pt[0];
  float p3pt = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p2[3] * pn[3];
  tmp[2] = p2[2] * pn[2];
  tmp[1] = p2[1] * pn[1];
  tmp[0] = p2[0] * pn[0];
  float p2pn = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p3[3] * pn[3];
  tmp[2] = p3[2] * pn[2];
  tmp[1] = p3[1] * pn[1];
  tmp[0] = p3[0] * pn[0];
  float p3pn = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p2[3] * p1[3];
  tmp[2] = p2[2] * p1[2];
  tmp[1] = p2[1] * p1[1];
  tmp[0] = p2[0] * p1[0];
  float p2p1 = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p3[3] * p1[3];
  tmp[2] = p3[2] * p1[2];
  tmp[1] = p3[1] * p1[1];
  tmp[0] = p3[0] * p1[0];
  float p3p1 = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p1[3] * pn[3];
  tmp[2] = p1[2] * pn[2];
  tmp[1] = p1[1] * pn[1];
  tmp[0] = p1[0] * pn[0];
  float p1pn = tmp[3] - tmp[2] - tmp[1] - tmp[0];
  tmp[3] = p1[3] * pt[3];
  tmp[2] = p1[2] * pt[2];
  tmp[1] = p1[1] * pt[1];
  tmp[0] = p1[0] * pt[0];
  float p1pt = tmp[3] - tmp[2] - tmp[1] - tmp[0];

  float ptsq = pt[3] * pt[3] - pt[2] * pt[2] - pt[1] * pt[1] - pt[0] * pt[0];
  float p1sq = p1[3] * p1[3] - p1[2] * p1[2] - p1[1] * p1[1] - p1[0] * p1[0];

  // eq. 31 of https://arxiv.org/pdf/1306.1732.pdf
  float d1, d2;  //denominator parts
  d1 = m2ee - 2.*ptp23;
  d2 = m2ee + 2.*pk;
  float c1, c2, c3, c4, c5, c6, c7, c8, c9, a; // 9 contractions needed

  a = (p2p3 + me * me); // part of l_munu, used in all contractions

  c1 = p2pt * p3pn + p3pt * p2pn - p2p3 * ptpn - ptpn * a;
  c1 = -(c1 * 2. + a * ptpn) * m2ee / d1 / d1;

  c2 = p2p1 * p3pn + p2pn * p3p1 - p1pn * a;
  c2 = c2 * 4.*ptp23 / d1 / d2;

  c3 = p2pt * p3pn + p2pn * p3pt - ptpn * a;
  c3 = c3 * 4.*ptp23 / d1 / d1;

  c4 = 2.*p2p3 - a;
  c4 = -c4 * 2.*pnk * ptp23 / d1 / d1;

  c5 = p2p1 * p3pt + p2pt * p3p1 - p1pt * a;
  c5 = -c5 * 4.*pnk / d1 / d2;

  c6 = 2.*p2pt * p3pt - ptsq * a;
  c6 = -c6 * 4.*pnk / d1 / d1;

  c7 = p2p1 * p3pt + p2pt * p3p1 - p1pt * a;
  c7 = c7 * 8.*ptpn / d1 / d2;

  c8 = 2.*p2p1 * p3p1 - p1sq * a;
  c8 = c8 * 4.*ptpn / d2 / d2;

  c9 = 2.*p2pt * p3pt - ptsq * a;
  c9 = c9 * 4.*ptpn / d1 / d1;

  float ce;
  ce = 16.*Gf * Gf * Mtau * Mtau * fpi * fpi * vud * vud * (16.*M_PI * M_PI / 137. / 137.);

  amplit = (c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9) / m2ee / m2ee;
  amplit = amplit * ce;

  hv[0] = 1.0;
  hv[1] = 0.0;
  hv[2] = 0.0;
  hv[3] = 0.0;

}

//**************************************************************************
// demo mu nu_mu ee nu_tau ME, based on paper by S.Antropov https://arxiv.org/pdf/1912.11376.pdf               *
//**************************************************************************
void eemu_(const float* pt, const float* pn, const float* p1, const float* p2, const float* p3, const float* p4, float& amplit,
           float* hv)
{
//we assume order: neutrino, muon, electron, positron

  double p34[4];
  p34[3] = p3[3] + p4[3];
  p34[2] = p3[2] + p4[2];
  p34[1] = p3[1] + p4[1];
  p34[0] = p3[0] + p4[0];

  double col[4];
  col[3] = p2[3] * p34[3];
  col[2] = p2[2] * p34[2];
  col[1] = p2[1] * p34[1];
  col[0] = p2[0] * p34[0];

  double pk = col[3] - col[2] - col[1] - col[0];
  double m2ee = p34[3] * p34[3] - p34[2] * p34[2] - p34[1] * p34[1] - p34[0] * p34[0];

  double ptk[4];
  ptk[3] = pt[3] * p34[3];
  ptk[2] = pt[2] * p34[2];
  ptk[1] = pt[1] * p34[1];
  ptk[0] = pt[0] * p34[0];

  double ptp34 = ptk[3] * ptk[3] - ptk[2] * ptk[2] - ptk[1] * ptk[1] - ptk[0] * ptk[0];
  ptp34 = ptk[3] - ptk[2] - ptk[1] - ptk[0];

  double PP[4];
  PP[3] = p2[3] / (pk + m2ee / 2.) - pt[3] / (ptp34 - m2ee / 2.);
  PP[2] = p2[2] / (pk + m2ee / 2.) - pt[2] / (ptp34 - m2ee / 2.);
  PP[1] = p2[1] / (pk + m2ee / 2.) - pt[1] / (ptp34 - m2ee / 2.);
  PP[0] = p2[0] / (pk + m2ee / 2.) - pt[0] / (ptp34 - m2ee / 2.);

  double PPsq;
  PPsq = PP[3] * PP[3] - PP[2] * PP[2] - PP[1] * PP[1] - PP[0] * PP[0];

  double p3PP;
  p3PP = p3[3] * PP[3] - p3[2] * PP[2] - p3[1] * PP[1] - p3[0] * PP[0];

  double p4PP;
  p4PP = p4[3] * PP[3] - p4[2] * PP[2] - p4[1] * PP[1] - p4[0] * PP[0];


  double ampser, ce;
  ce = 4.*(16.*M_PI * M_PI / 137. / 137.);
  ampser = (4.*p3PP * p4PP - m2ee * PPsq) / (2.*m2ee * m2ee) * ce;

  double ak0 = 0.001 * 1.777;
  hv[3] = 1.0;
  hv[0] = 0.0;
  hv[1] = 0.0;
  hv[2] = 0.0;
  amplit = (float)nunul_(p2, pn, p1, ak0, hv) * ampser;

}

double nunul_(const float* pl, const float* pnu, const float* pnl, float ak0, float* hv)
{

  double Mtau = 1.777;

  double Mnu = 0.0; // 0.01
  double Gf = 1.16637E-5;
  double alpha = 137.03604;
  double Msq = Mtau * Mtau;
  double pr[4];
  double rxnl[3], rxnu[3], rxl[3];
  pr[0] = 0.0;
  pr[1] = 0.0;
  pr[2] = 0.0;
  pr[3] = Mtau;
  for (int i = 0; i < 3; i++) {
    rxnl[i] = pr[3] * pnl[3] - pr[2] * pnl[2] - pr[1] * pnl[1] - pr[0] * pnl[0];
    rxnu[i] = pr[3] * pnu[3] - pr[2] * pnu[2] - pr[1] * pnu[1] - pr[0] * pnu[0];
    rxl[i] = pr[3] * pl[3] - pr[2] * pl[2] - pr[1] * pl[1] - pr[0] * pl[0];
  }
//     QUASI TWO-BODY VARIABLES
  double u0, u3, w3, w0, up, um, wp, wm, yu, yw, eps2, eps, y, al;
  u0 = pl[3] / Mtau;
  u3 = sqrt(pl[0] * pl[0] + pl[1] * pl[1] + pl[2] * pl[2]) / Mtau;
  w3 = u3;
  w0 = (pnu[3] + pnl[3]) / Mtau; //does this require ee pair energy?
  up = u0 + u3;
  um = u0 - u3;
  wp = w0 + w3;
  wm = w0 - w3;
  yu = (up / um) > 0 ? log(up / um) / 2. : 0;
  yw = (wp / wm) > 0 ? log(wp / wm) / 2. : 0;
  eps2 = u0 * u0 - u3 * u3;
  eps = eps2 > 0 ? sqrt(eps2) : 0;
  y = w0 * w0 - w3 * w3;
  al = ak0 / Mtau;

//     FORMFACTORS
  double f0, fp, fm, f3;
  f0 = 2.*u0 / u3 * (dilog(1.0 - (um * wm / (up * wp))) - dilog(1.0 - wm / wp)
                     + dilog(1.0 - um / up) - 2.*yu + 2.*log(up) * (yw + yu))
       + 1.0 / y * (2.*u3 * yu + (1.0 - eps2 - 2 * y) * log(eps)) + 2.0 - 4.*(u0 / u3 * yu - 1.0) * log(2.*al);
  fp = yu / (2 * u3) * (1. + (1. - eps2) / y) + log(eps) / y;
  fm = yu / (2 * u3) * (1. - (1. - eps2) / y) - log(eps) / y;
  f3 = eps2 * (fp + fm) / 2.;

//     SCALAR PRODUCTS OF FOUR-MOMENTA
  double lxnl, lxnu, nuxnl, mxnl, mxnu, mxl;
  lxnu = pl[3] * pnu[3] - pl[2] * pnu[2] - pl[1] * pnu[1] - pl[0] * pnu[0];
  lxnl = pl[3] * pnl[3] - pl[2] * pnl[2] - pl[1] * pnl[1] - pl[0] * pnl[0];
  nuxnl = pnu[3] * pnl[3] - pnu[2] * pnl[2] - pnu[1] * pnl[1] - pnu[0] * pnl[0];

  mxnu = Mtau * pnu[3];
  mxnl = Mtau * pnl[3];
  mxl = Mtau * pl[3];
//     DECAY DIFFERENTIAL WIDTH WITHOUT AND WITH POLARIZATION
  double c3, am3, xm3;
  c3 = 1. / (2.*alpha * M_PI) * 64.*Gf * Gf;
  xm3 = -(f0 * lxnu * mxnl + fp * eps2 * mxnu * mxnl + fm * lxnu * lxnl + f3 * Msq * nuxnl);
  am3 = 0.0 * xm3 * c3;
  if (isnan(xm3)) {
    cout << up << " " << um << " " << yu << endl;
    cout << wp << " " << wm << " " << yw << endl;
    cout << eps2 << " " << f0 << " " << fp << " " << fm << " " << f3 << " " << c3 << endl;
    am3 = 0.0;
  }

//     V-A  AND  V+A COUPLINGS, BUT IN THE BORN PART ONLY
  double brak, gv, ga, born, xm3pol[3], am3pol[3], bornp[3];
  gv = 1.;
  ga = -1.;
  brak = (gv + ga) * (gv + ga) * mxl * nuxnl
         + (gv - ga) * (gv - ga) * mxnl * lxnu
         - (gv * gv - ga * ga) * Mtau * Mnu * lxnl;
  born = 32.*(Gf * Gf / 2.0) * brak;
  for (int i = 0; i < 3; i++) {
    xm3pol[i] = -(f0 * lxnu * rxnl[i] + fp * eps2 * mxnu * rxnl[i]
                  + fm * lxnu * (lxnl + (rxnl[i] * mxl - mxnl * rxl[i]) / Msq)
                  + f3 * (Msq * nuxnl + mxnu * rxnl[i] - rxnu[i] * mxnl));
    am3pol[i] = 0.0 * xm3pol[i] * c3;
    if (isnan(xm3pol[i])) am3pol[i] = 0.0;
//     V-A  AND  V+A COUPLINGS, BUT IN THE BORN PART ONLY
    bornp[i] = born + ((gv + ga) * (gv + ga) * Mtau * nuxnl * pl[i]
                       - (gv + ga) * (gv + ga) * Mtau * lxnu * pnu[i]
                       + (gv * gv - ga * ga) * Mnu * mxnl * pl[i]
                       - (gv * gv - ga * ga) * Mnu * mxl * pnu[i]) * 32.*(Gf * Gf / 2.0);
    hv[i] = (bornp[i] + am3pol[i]) / (born + am3) - 1.0;
  }

  double tbh;
  tbh = born + am3;
  if (isnan(tbh)) {
    cout << f0 << yw << wm << wp << " " << w0 << " " << w3 << " " << xm3pol[3] << endl;
    tbh = 0.0;
    return tbh;
  }
  if (tbh / born < 0.1) {
    cout << "ERROR in nunul (matrix element)" << tbh / born << endl;
    tbh = 0.0;
    return tbh;
  }
  // if(tbh!=0.0) cout<<"amplit= "<< tbh << endl;
  return tbh;
}

