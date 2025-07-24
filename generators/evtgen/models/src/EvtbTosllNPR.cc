/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <framework/logging/Logger.h>

#include "generators/evtgen/EvtGenModelRegister.h"
#include "generators/evtgen/models/EvtbTosllNPR.h"

#include "EvtGenBase/EvtGenKine.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtScalarParticle.hh"
#include "EvtGenBase/EvtRandom.hh"

#include "EvtGenBase/EvtAmp.hh"
#include "EvtGenBase/EvtDiracSpinor.hh"
#include "EvtGenBase/EvtId.hh"
#include "EvtGenBase/EvtPDL.hh"
#include "EvtGenBase/EvtParticle.hh"
#include "EvtGenBase/EvtTensor4C.hh"
#include "EvtGenBase/EvtVector4C.hh"

#include "EvtGenModels/EvtbTosllAmp.hh"
#include "EvtGenModels/EvtbTosllFF.hh"

#include <algorithm>
#include <complex>

/** register the model in EvtGen */
B2_EVTGEN_REGISTER_MODEL(EvtbTosllNPR);

using std::endl;
using namespace std::complex_literals;
std::string EvtbTosllNPR::getName()
{
  return "BTOSLLNPR";
}

EvtDecayBase* EvtbTosllNPR::clone()
{
  return new EvtbTosllNPR;
}

struct EvtPointf {
  float x, y;
};

static void EvtLeptonVandACurrents(EvtVector4C&, EvtVector4C&, const EvtDiracSpinor&, const EvtDiracSpinor&);

struct EvtLinSample {
  EvtLinSample() {}
  EvtLinSample(const std::vector<EvtPointf>& _v) { init(_v); }
  void init(const std::vector<EvtPointf>& _v);
  std::pair<double, double> operator()(double) const;

  std::vector<EvtPointf> m_v;
  std::vector<double> m_I;
};

static double PhaseSpacePole2(double M, double m1, double m2, double m3, EvtVector4R* p4, const EvtLinSample* ls)
{
  const EvtLinSample& g = *ls;
  double m13sqmin = (m1 + m3) * (m1 + m3), m13sqmax = (M - m2) * (M - m2);
  double d13 = m13sqmax - m13sqmin;
  double M2 = M * M, m32 = m3 * m3, m12 = m1 * m1, m22 = m2 * m2;
  double c0 = M2 - m32, c1 = m12 - m22, c2 = m32 + m12;
  double m12sq, m13sq, weight;
  do {
    double z0 = EvtRandom::random(), z1 = EvtRandom::random();
    m13sq = m13sqmin + z0 * d13;
    auto t = g(z1);
    m12sq = t.first;
    weight = t.second;
    double E3s = c0 - m12sq, E1s = m12sq + c1;
    double w = 2 * m12sq, e = 4 * m12sq;
    double A = (m13sq - c2) * w - E3s * E1s;
    double B = (E3s * E3s - e * m32) * (E1s * E1s - e * m12);
    if (A * A < B)
      break;
  } while (true);
  double iM = 0.5 / M;
  double E2 = (M2 + m22 - m13sq) * iM;
  double E3 = (M2 + m32 - m12sq) * iM;
  double E1 = M - E2 - E3;
  double p1 = sqrt(E1 * E1 - m12);
  double p3 = sqrt(E3 * E3 - m32);
  double cost13 = (2.0 * E1 * E3 + (m12 + m32 - m13sq)) / (2.0 * p1 * p3);

  double px = p1 * cost13;
  double v0x = px;
  double v1x = -px - p3;
  double py = p1 * sqrt(1.0 - cost13 * cost13);
  double v2x = p3;

  double phi = (EvtRandom::random() - 0.5) * (2 * M_PI);
  double x = cos(phi), y = sin(phi);
  phi = (EvtRandom::random() - 0.5) * (2 * M_PI);
  double c = cos(phi), s = sin(phi);
  double u = EvtRandom::random() * 2, z = u - 1, t = sqrt(1 - z * z);
  double Rx = s * y - c * x, Ry = c * y + s * x, ux = u * x, uy = u * y;
  double R00 = ux * Rx + c, R01 = s - ux * Ry, R10 = uy * Rx - s,
         R11 = c - uy * Ry, R20 = -t * Rx, R21 = t * Ry;
  double pyx = R01 * py, pyy = R11 * py, pyz = R21 * py;

  p4[0].set(E1, R00 * v0x + pyx, R10 * v0x + pyy, R20 * v0x + pyz);
  p4[1].set(E2, R00 * v1x - pyx, R10 * v1x - pyy, R20 * v1x - pyz);
  p4[2].set(E3, R00 * v2x, R10 * v2x, R20 * v2x);

  return weight;
}

void EvtbTosllNPR::decay(EvtParticle* p)
{
  static EvtVector4R p4[3];
  static double mass[3];
  double m_b = p->mass();
  for (int i = 0; i < 3; i++)
    mass[i] = p->getDaug(i)->mass();
  EvtId* daughters = getDaugs();
  double weight = PhaseSpacePole2(m_b, mass[2], mass[1], mass[0], p4, m_ls);
  p->getDaug(0)->init(daughters[0], p4[2]);
  p->getDaug(1)->init(daughters[1], p4[1]);
  p->getDaug(2)->init(daughters[2], p4[0]);

  setWeight(weight);
  CalcAmp(p, _amp2);
}

void EvtLinSample::init(const std::vector<EvtPointf>& _v)
{
  m_v = _v;
  m_I.push_back(0);
  for (unsigned i = 0; i < m_v.size() - 1; i++)
    m_I.push_back((m_v[i + 1].y + m_v[i + 0].y) * (m_v[i + 1].x - m_v[i + 0].x) + m_I.back());
  double norm = 1 / m_I.back();
  for (unsigned i = 0; i < m_v.size(); i++)
    m_I[i] *= norm;
}

std::pair<double, double> EvtLinSample::operator()(double r) const
{
  int j = upper_bound(m_I.begin(), m_I.end(), r) - m_I.begin();
  double dI = m_I[j] - m_I[j - 1];
  r = (r - m_I[j - 1]) / dI;
  double f0 = m_v[j - 1].y, f1 = m_v[j].y, x0 = m_v[j - 1].x, x1 = m_v[j].x,
         df = f1 - f0, dx = x1 - x0, z;
  if (fabs(df) > f0 * 1e-3) {
    z = (f1 * x0 - x1 * f0 + dx * sqrt(df * (f0 + f1) * r + f0 * f0)) / df;
  } else {
    if (f0 > f1) {
      z = x0 + (r * dx) * (f1 - r * df) / f0;
    } else {
      z = x1 - (r * dx) * (f0 + r * df) / f1;
    }
  }
  return std::make_pair(z, f0 + (df / dx) * (z - x0));
}

struct BW_t {
  double Mv2, Gv2, A;

  BW_t(double m, double gtot, double ghad, double bll, double alpha = 1 / 137.035999084)
  {
    //        const double alpha = 1 / 137.035999084;    // (+-21) PDG 2021
    Mv2 = m * m;
    Gv2 = gtot * gtot;
    A = (9 / (alpha * alpha)) * bll * gtot * ghad;
  }

  double R(double s) const
  {
    double z = s - Mv2, z2 = z * z;
    return A * s / (z2 + Mv2 * Gv2);
  }

  double Rp(double s) const
  {
    double z = s - Mv2, z2 = z * z, t = 1 / (z2 + Mv2 * Gv2);
    t *= 1 - 2 * s * z * t;
    return A * t;
  }

  double Rpp(double s) const
  {
    double z = s - Mv2, z2 = z * z, t = 1 / (z2 + Mv2 * Gv2);
    t = t * t * (8 * s * z2 * t - 4 * z - 2 * s);
    return A * t;
  }

  // dispersion relation indefinite integral: s is the parmeter, sp is
  // the integration variable
  double DRIntegral(double s, double sp) const
  {
    double zp = sp - Mv2, zp2 = zp * zp;
    double z = s - Mv2, z2 = z * z;
    double GM2 = Mv2 * Gv2, GM = sqrt(GM2);
    double ds = sp - s;
    return A * (0.5 * log(ds * ds / (zp2 + GM2)) - z * atan(zp / GM) / GM) / (z2 + GM2);
  }

  void addknots(std::vector<double>& en) const
  {
    double Mv = sqrt(Mv2), Gv = sqrt(Gv2);
    double w = 0.6;
    double smin = (Mv - w * Gv) * (Mv - w * Gv);
    double smax = (Mv + w * Gv) * (Mv + w * Gv);
    en.push_back(Mv * Mv);
    for (int i = 0, n = 30; i < n; i++) {
      double s = smin + (smax - smin) * (i + 0.5) / n;
      en.push_back(s);
    }
    w = 2;
    double smin1 = (Mv - w * Gv) * (Mv - w * Gv);
    double smax1 = (Mv + w * Gv) * (Mv + w * Gv);
    for (int i = 0, n = 30; i < n; i++) {
      double s = smin1 + (smax1 - smin1) * (i + 0.5) / n;
      if (s >= smin && s <= smax)
        continue;
      en.push_back(s);
    }
    w = 8;
    double smin2 = (Mv - w * Gv) * (Mv - w * Gv);
    double smax2 = (Mv + w * Gv) * (Mv + w * Gv);
    for (int i = 0, n = 30; i < n; i++) {
      double s = smin2 + (smax2 - smin2) * (i + 0.5) / n;
      if (s >= smin1 && s <= smax1)
        continue;
      en.push_back(s);
    }
    w = 30;
    double smin3 = (Mv - w * Gv) * (Mv - w * Gv);
    double smax3 = (Mv + w * Gv) * (Mv + w * Gv);
    for (int i = 0, n = 30; i < n; i++) {
      double s = smin3 + (smax3 - smin3) * (i + 0.5) / n;
      if (s >= smin2 && s <= smax2)
        continue;
      en.push_back(s);
    }
    w = 100;
    double smin4 = (Mv - w * Gv) * (Mv - w * Gv);
    double smax4 = (Mv + w * Gv) * (Mv + w * Gv);
    for (int i = 0, n = 20; i < n; i++) {
      double s = smin4 + (smax4 - smin4) * (i + 0.5) / n;
      if (s >= smin3 && s <= smax3)
        continue;
      en.push_back(s);
    }
  }
};

static double Rcont(double s)
{
  //    const double mb = 4.18 /* +0.03 -0.02 [GeV] */, mb2 = mb * mb;
  const double mb = 4.8 /* +0.03 -0.02 [GeV] */, mb2 = mb * mb;
  if (s < 0.6 * mb2) return 0;
  if (s < 0.69 * mb2)
    return (1.02 / ((0.69 - 0.6) * mb2)) * (s - 0.6 * mb2);
  return 1.02;
}

BW_t jpsi(3.0969, 0.0926 / 1000, 0.0926 / 1000 * 0.877, 0.05971);
BW_t psi2s(3.6861, 0.294 / 1000, 0.294 / 1000 * 0.9785, 0.00793);
BW_t psi3770(3773.7 / 1000, 27.2 / 1000, 27.2 / 1000, 1e-5);
BW_t psi4040(4039 / 1000., 80 / 1000., 80 / 1000., 1.07e-5);
BW_t psi4160(4191 / 1000., 70 / 1000., 70 / 1000., 6.9e-6);
BW_t psi4230(4220 / 1000., 60 / 1000., 60 / 1000., 1e-5);

static double uR(double s, double wjpsi, double wpsi2s, double wrest)
{
  return Rcont(s) + wjpsi * jpsi.R(s) + wpsi2s * psi2s.R(s) + wrest * (psi3770.R(s) + psi4040.R(s) + psi4160.R(s) + psi4230.R(s));
}

static double linint(double a, double b, double s, double sp)
{
  double x = std::abs(sp - s);
  return ((a * s - b) * log(x) + b * log(sp)) / s;
}

static double cnstint(double c, double s, double sp)
{
  double x = std::abs(sp - s);
  return c * (log(x) - log(sp)) / s;
}

static double DRIcont(double s)
{
  // !!! check mb value!!!
  //  const double mb = 4.18/* +0.03 -0.02 [GeV] */, mb2 = mb*mb;
  const double mb = 4.8/* +0.03 -0.02 [GeV] */, mb2 = mb * mb;
  double s0 = 0.6 * mb2, s1 = 0.69 * mb2;
  double c = 1.02, a = c / (s1 - s0), b = c * s0 / (s1 - s0);
  return (linint(a, b, s, s1) - linint(a, b, s, s0)) - cnstint(c, s, s1);
}

static double DRInt(double s, double wjpsi, double wpsi2s, double wrest)
{
  const double Mpi = 1.3957061e-01, ct = 4 * Mpi * Mpi;
  auto Integ = [ct, s](const BW_t & r) {return r.DRIntegral(s, 1e8) - r.DRIntegral(s, ct);};
  return DRIcont(s) + wjpsi * Integ(jpsi) + wpsi2s * Integ(psi2s) + wrest * (Integ(psi3770) + Integ(psi4040) + Integ(psi4160) + Integ(
           psi4230));
}

static std::vector<double> getgrid(int reso)
{
  const double m_c = 1.3, m_s = 0.2, m_e = 0.511e-3, MD0 = 1864.84 / 1000;

  std::vector<double> en;
  if (reso) {
    std::vector<BW_t> v = { jpsi, psi2s, psi3770, psi4040, psi4160, psi4230 };
    for (const auto& t : v)
      t.addknots(en);

    en.push_back(jpsi.Mv2 + 0.1);
    en.push_back(jpsi.Mv2 - 0.1);
    en.push_back(jpsi.Mv2 + 0.08);
    en.push_back(jpsi.Mv2 - 0.08);
    en.push_back(jpsi.Mv2 + 0.065);
    en.push_back(jpsi.Mv2 - 0.065);
  }

  double smax = 25;    //4.37*4.37;
  for (unsigned i = 0, n = 200; i < n; i++) {
    double s = smax / n * (i + 0.5);
    en.push_back(s);
  }

  double t0;
  for (t0 = 4 * m_e * m_e; t0 < 0.15; t0 *= 1.5) {
    en.push_back(t0);
  }

  for (t0 = 4 * m_e * m_e; t0 < 0.5; t0 *= 1.5) {
    en.push_back(4 * MD0 * MD0 + t0);
    en.push_back(4 * MD0 * MD0 - t0);
  }

  en.push_back(4 * m_c * m_c);
  for (t0 = 0.00125; t0 < 0.05; t0 *= 1.5) {
    en.push_back(4 * m_c * m_c + t0);
    en.push_back(4 * m_c * m_c - t0);
  }

  en.push_back(4 * m_s * m_s);
  for (t0 = 0.00125; t0 < 0.1; t0 *= 1.5) {
    en.push_back(4 * m_s * m_s + t0);
    en.push_back(4 * m_s * m_s - t0);
  }
  en.push_back(0x1.27397cfb13b38p + 0);
  en.push_back(10);
  std::sort(en.begin(), en.end());

  for (std::vector<double>::iterator it = en.begin(); it != en.end(); it++) {
    if (*it > smax) {
      en.erase(it, en.end());
      break;
    }
  }
  return en;
}

void EvtbTosllNPR::initProbMax()
{
  EvtId pId = getParentId(), mId = getDaug(0), l1Id = getDaug(1), l2Id = getDaug(2);

  std::vector<double> v;
  std::vector<EvtPointf> pp;

  EvtScalarParticle* scalar_part;
  EvtParticle* root_part;

  scalar_part = new EvtScalarParticle;

  //includge to avoid generating random numbers!
  scalar_part->noLifeTime();

  EvtVector4R p_init;

  p_init.set(EvtPDL::getMass(pId), 0.0, 0.0, 0.0);
  scalar_part->init(pId, p_init);
  root_part = (EvtParticle*)scalar_part;
  root_part->setDiagonalSpinDensity();

  EvtParticle* daughter, *lep1, *lep2;

  EvtAmp amp;

  EvtId listdaug[3];
  listdaug[0] = mId;
  listdaug[1] = l1Id;
  listdaug[2] = l2Id;

  amp.init(pId, 3, listdaug);

  root_part->makeDaughters(3, listdaug);
  daughter = root_part->getDaug(0);
  lep1 = root_part->getDaug(1);
  lep2 = root_part->getDaug(2);

  //cludge to avoid generating random numbers!
  daughter->noLifeTime();
  lep1->noLifeTime();
  lep2->noLifeTime();

  //Initial particle is unpolarized, well it is a scalar so it is trivial
  EvtSpinDensity rho;
  rho.setDiag(root_part->getSpinStates());

  double mass[3];

  double m = root_part->mass();

  EvtVector4R p4meson, p4lepton1, p4lepton2, p4w;

  double maxprobfound = 0.0;

  mass[1] = EvtPDL::getMeanMass(l1Id);
  mass[2] = EvtPDL::getMeanMass(l2Id);
  std::vector<double> mH;
  mH.push_back(EvtPDL::getMeanMass(mId));
  //if the particle is narrow dont bother with changing the mass.
  double g = EvtPDL::getWidth(mId);
  if (g > 0) {
    mH.push_back(EvtPDL::getMinMass(mId));
    mH.push_back(
      std::min(EvtPDL::getMaxMass(mId), m - mass[1] - mass[2]));
    mH.push_back(mH.front() - g);
    mH.push_back(mH.front() + g);
  }

  double q2min = (mass[1] + mass[2]) * (mass[1] + mass[2]);

  double m0 = EvtPDL::getMinMass(mId);
  double q2max = (m - m0) * (m - m0);
  v = getgrid(m_flag);
  m0 = mH[0];
  //loop over q2
  for (double q2 : v) {
    // want to avoid picking up the tail of the photon propagator
    if (!(q2min <= q2 && q2 < q2max))
      continue;
    double Erho = (m * m + m0 * m0 - q2) / (2.0 * m);
    if (Erho < m0) {
      m0 = EvtPDL::getMinMass(mId);
      Erho = (m * m + m0 * m0 - q2) / (2.0 * m);
    }
    double Prho = sqrt((Erho - m0) * (Erho + m0));

    p4meson.set(Erho, 0.0, 0.0, -Prho);
    p4w.set(m - Erho, 0.0, 0.0, Prho);

    //This is in the W rest frame
    double Elep = sqrt(q2) * 0.5,
           Plep = sqrt((Elep - mass[1]) * (Elep + mass[1]));

    const int nj = 3 + 2 + 4 + 8 + 32;
    double cmin = -1, cmax = 1, dc = (cmax - cmin) / (nj - 1);
    double maxprob = 0;
    for (int j = 0; j < nj; j++) {
      double c = cmin + dc * j;

      //These are in the W rest frame. Need to boost out into the B frame.
      double Py = Plep * sqrt(1.0 - c * c), Pz = Plep * c;
      p4lepton1.set(Elep, 0.0, Py, Pz);
      p4lepton2.set(Elep, 0.0, -Py, -Pz);

      p4lepton1 = boostTo(p4lepton1, p4w);
      p4lepton2 = boostTo(p4lepton2, p4w);

      //Now initialize the daughters...
      daughter->init(mId, p4meson);
      lep1->init(l1Id, p4lepton1);
      lep2->init(l2Id, p4lepton2);
      CalcAmp(root_part, amp);
      double prob = rho.normalizedProb(amp.getSpinDensity());
      maxprob = std::max(maxprob, prob);
    }
    pp.push_back({ (float)q2, (float)maxprob });
    maxprobfound = std::max(maxprobfound, maxprob);
  }
  root_part->deleteTree();

  m_ls->init(pp);
  maxprobfound *= 8e-8 * 1.1;
  maxprobfound *= 1.3;
  maxprobfound *= 1.5; // large eta
  maxprobfound = std::min(20., maxprobfound);
  B2WARNING("Max prob " << maxprobfound << std::flush);
  setProbMax(maxprobfound);
}

static EvtId IdB0, IdaB0, IdBp, IdBm, IdBs, IdaBs, IdRhop, IdRhom, IdRho0,
       IdOmega, IdKst0, IdaKst0, IdKstp, IdKstm, IdK0, IdaK0, IdKp, IdKm;
static bool cafirst = true;

void EvtbTosllNPR::init()
{
  if (cafirst) {
    cafirst = false;
    IdB0 = EvtPDL::getId("B0");
    IdaB0 = EvtPDL::getId("anti-B0");
    IdBp = EvtPDL::getId("B+");
    IdBm = EvtPDL::getId("B-");
    IdBs = EvtPDL::getId("B_s0");
    IdaBs = EvtPDL::getId("anti-B_s0");
    IdRhop = EvtPDL::getId("rho+");
    IdRhom = EvtPDL::getId("rho-");
    IdRho0 = EvtPDL::getId("rho0");
    IdOmega = EvtPDL::getId("omega");
    IdKst0 = EvtPDL::getId("K*0");
    IdaKst0 = EvtPDL::getId("anti-K*0");
    IdKstp = EvtPDL::getId("K*+");
    IdKstm = EvtPDL::getId("K*-");
    IdK0 = EvtPDL::getId("K0");
    IdaK0 = EvtPDL::getId("anti-K0");
    IdKp = EvtPDL::getId("K+");
    IdKm = EvtPDL::getId("K-");
  }

  // First choose format of NP coefficients from the .DEC file
  // Cartesian(x,y)(0) or Polar(R,phase)(1)
  int n = getNArg();
  checkNDaug(3);

  //We expect the parent to be a scalar
  //and the daughters to be K(*) lepton+ lepton-

  checkSpinParent(EvtSpinType::SCALAR);
  checkSpinDaughter(1, EvtSpinType::DIRAC);
  checkSpinDaughter(2, EvtSpinType::DIRAC);

  EvtId mId = getDaug(0);
  if (mId != IdKst0 && mId != IdaKst0 && mId != IdKstp && mId != IdKstm &&
      mId != IdK0 && mId != IdaK0 && mId != IdKp && mId != IdKm) {
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "EvtbTosllNPR generator expected a K(*) 1st daughter, found: "
        << EvtPDL::name(getDaug(0)) << endl;
    EvtGenReport(EVTGEN_ERROR, "EvtGen")
        << "Will terminate execution!" << endl;
    ::abort();
  }

  auto getInteger = [this](int i) -> int {
    double a = getArg(i);
    if (a - static_cast<int>(a) != 0)
    {
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "Parameters is not integer in the BTOSLLNPR decay model: " << i
          << " " << a << endl;
      EvtGenReport(EVTGEN_ERROR, "EvtGen")
          << "Will terminate execution!" << endl;
      ::abort();
    }
    return static_cast<int>(a);
  };

  double phi = 0.0, n1 = 0.0, d1 = 1.0, eta = 0.0, theta_Jpsi = 0.0, theta_psi2S = 0.0, Nr = 1.0;
  if (n > 0) {      // parse arguments
    int i = 0, coordsyst = getInteger(i++);
    auto getcomplex = [this, &i, coordsyst]() -> EvtComplex {
      double a0 = getArg(i++);
      double a1 = getArg(i++);
      return (coordsyst) ? EvtComplex(a0 * cos(a1), a0 * sin(a1))
      : EvtComplex(a0, a1);
    };
    auto getreal = [this, &i]() -> double { return getArg(i++); };
    while (i < n) {
      int id = getInteger(i++);      // New Physics cooeficient Id
      if (id == 0)
        m_dc7 = getcomplex();    // delta C_7eff
      if (id == 1)
        m_dc9 = getcomplex();    // delta C_9eff
      if (id == 2)
        m_dc10 = getcomplex();    // delta C_10eff
      if (id == 3)
        m_c7p = getcomplex();    // C'_7eff -- right hand polarizations
      if (id == 4)
        m_c9p = getcomplex();    // C'_9eff -- right hand polarizations
      if (id == 5)
        m_c10p = getcomplex();    // c'_10eff -- right hand polarizations
      if (id == 6)
        m_cS = getcomplex();    // (C_S - C'_S) -- scalar right and left polarizations
      if (id == 7)
        m_cP = getcomplex();    // (C_P - C'_P) -- pseudo-scalar right and left polarizations
      if (id == 10)
        m_flag = getInteger(i++); // include resonances or not
      if (id == 11)
        phi = getreal(); // phase of the non-factorizable contribution
      if (id == 12)
        n1 = getreal(); // amplitude of the non-factorizable contribution
      if (id == 13)
        d1 = getreal(); // width of the non-factorizable contribution
      if (id == 14)
        eta = getreal(); // Eq. 47
      if (id == 15)
        theta_Jpsi = getreal(); // Eq. 47
      if (id == 16)
        theta_psi2S = getreal(); // Eq. 47
      if (id == 17)
        Nr = getreal(); // Eq. 47
    }
  }
  m_ls = new EvtLinSample;
  if (!m_flag) return;

  std::vector<double> v = getgrid(m_flag);
  std::complex<double> eiphi = exp(1i * phi), pJpsi = Nr * exp(1i * theta_Jpsi), ppsi2S = Nr * exp(1i * theta_psi2S);
  // constants have to be synchronized with src/EvtGenModels/EvtbTosllVectorAmpNP.cpp
  const double C1 = -0.257, C2 = 1.009, C3 = -0.005, C5 = 0;
  double sc = 1.0 / (4.0 / 3.0 * C1 + C2 + 6.0 * C3 + 60.0 * C5);
  const double m_b = 4.8, m_c = 1.3;
  for (auto t : v) {
    // Eq. 13, 14, and 15 in Emi's note. For the default theta_Jpsi=0 and theta_psi2S=0 it is identical to the Rahul's dispersion relation
    double re = -8.0 / 9.0 * log(m_c / m_b) - 4.0 / 9.0 +
                t / 3.0 * DRInt(t, real(pJpsi), real(ppsi2S), Nr) - M_PI / 3 * uR(t, imag(pJpsi), imag(ppsi2S), 0);
    double im =
      t / 3.0 * DRInt(t, imag(pJpsi), imag(ppsi2S), 0) + M_PI / 3 * uR(t, real(pJpsi), real(ppsi2S), Nr);
    std::complex<double> r(re, im);
    if (eta == 0.0) {
      double z = t - jpsi.Mv2;
      r += eiphi * (n1 * sc / (z * z + d1));    // bump under J/psi as a non-factorizable contribution
    }
    m_reso.push_back(std::make_pair(t, r));
  }

}

static std::complex<double> h(double q2, double mq)
{
  const double mu = 4.8;    // mu = m_b = 4.8 GeV
  if (mq == 0.0)
    return std::complex<double>(8. / 27 + 4. / 9 * log(mu * mu / q2), 4. / 9 * M_PI);
  double z = 4 * mq * mq / q2;
  std::complex<double> t;
  if (z > 1) {
    t = atan(1 / sqrt(z - 1));
  } else {
    t = std::complex<double>(log((1 + sqrt(1 - z)) / sqrt(z)), -M_PI / 2);
  }
  return -4 / 9. * (2 * log(mq / mu) - 2 / 3. - z) - (4 / 9. * (2 + z) * sqrt(fabs(z - 1))) * t;
}

static EvtComplex C9(double q2, std::complex<double> hReso = std::complex<double>(0, 0))
{
  double m_b = 4.8;    // quark masses
  double C1 = -0.257, C2 = 1.009, C3 = -0.005, C4 = -0.078, C5 = 0, C6 = 0.001;
  std::complex<double> Y = (hReso) *
                           (4 / 3. * C1 + C2 + 6 * C3 + 60 * C5);
  Y -= 0.5 * h(q2, m_b) * (7 * C3 + 4 / 3. * C4 + 76 * C5 + 64 / 3. * C6);
  Y -= 0.5 * h(q2, 0.0) * (C3 + 4 / 3. * C4 + 16 * C5 + 64 / 3. * C6);
  Y += 4 / 3. * C3 + 64 / 9. * C5 + 64 / 27. * C6;
  return EvtComplex(4.211 + real(Y), imag(Y));
}

static std::complex<double> interpol(const hvec_t& v, double s)
{
  double m_c = 1.3;    // quark masses
  if (!v.size()) return h(s, m_c);
  int j = std::lower_bound(v.begin(), v.end(), s, [](const helem_t& a, double b) { return a.first < b; }) - v.begin();

  double dx = v[j].first - v[j - 1].first;
  auto dy = v[j].second - v[j - 1].second;
  return v[j - 1].second + (s - v[j - 1].first) * (dy / dx);
}

void EvtbTosllNPR::CalcAmp(EvtParticle* parent, EvtAmp& amp)
{
  EvtId dId = parent->getDaug(0)->getId();
  if (dId == IdKst0 || dId == IdaKst0 || dId == IdKstp || dId == IdKstm) {
    CalcVAmp(parent, amp);
  }
  if (dId == IdK0 || dId == IdaK0 || dId == IdKp || dId == IdKm) {
    CalcSAmp(parent, amp);
  }
}

static inline double poly(double x, int n, const double* c)
{
  double t = c[--n];
  while (n--) t = c[n] + x * t;
  return t;
}

static void getVectorFF(double q2, double mB, double mV, double& a1, double& a2, double& a0, double& v, double& t1, double& t2,
                        double& t3)
{
  static const double alfa[7][4] = {
    // coefficients are from  https://arxiv.org/src/1503.05534v3/anc/BKstar_LCSR-Lattice.json
    //  m_res,       c0,        c1,       c2
    { 5.415000, 0.376313, -1.165970, 2.424430 },    // V
    { 5.366000, 0.369196, -1.365840, 0.128191 },    // A0
    { 5.829000, 0.297250,  0.392378, 1.189160 },    // A1
    { 5.829000, 0.265375,  0.533638, 0.483166 },    // A12
    { 5.415000, 0.312055, -1.008930, 1.527200 },    // T1
    { 5.829000, 0.312055,  0.496846, 1.614310 },    // T2
    { 5.829000, 0.667412,  1.318120, 3.823340 }     // T12
  };

  double mBaV = mB + mV, mBsV = mB - mV;
  double tp = mBaV * mBaV;    // t_{+} = (m_B + m_V)^2
  double s0 = sqrt(2 * mBaV * sqrt(mB * mV));       // sqrt(t_{+} - t_{+}*(1 - sqrt(1 - t_{-}/t_{+})))
  double z0 = (mBaV - s0) / (mBaV + s0);        // (sqrt(t_{+}) - s0)/(sqrt(t_{+}) + s0)

  double s = sqrt(tp - q2), z = (s - s0) / (s + s0), dz = z - z0, ff[7];
  for (int j = 0; j < 7; j++) {
    double mR = alfa[j][0], mR2 = mR * mR;
    ff[j] = (mR2 / (mR2 - q2)) * poly(dz, 3, alfa[j] + 1);
  }

  // Källén-function
  // arXiv:1503.05534 Eq. D.3
  double lambda = (mBaV * mBaV - q2) * (mBsV * mBsV - q2);

  v = ff[0];
  a0 = ff[1];
  a1 = ff[2];
  // Eq. D.5 arXiv:1503.05534
  // A12 = (mBaV*mBaV*(mBaV*mBsV - q2)*A1 - lambda(q2)*A2)/(16*mB*mV*mV*mBaV);
  double a12 = ff[3];
  a2 = mBaV * ((mBaV * (mBaV * mBsV - q2)) * a1 - (16 * mB * mV * mV) * a12) / lambda;
  t1 = ff[4];
  t2 = ff[5];
  // Eq. D.5 arXiv:1503.05534
  // T23 = mBaV*mBsV*(mB*mB + 3*mV*mV - q2)*T2 - lambda(q2)*T3)/(8*mB*mV*mV*mBsV);
  double t23 = ff[6];
  t3 = mBsV * (mBaV * (mB * mB + 3 * mV * mV - q2) * t2 - (8 * mB * mV * mV) * t23) / lambda;
}

static EvtTensor4C asymProd(const EvtVector4R& a, const EvtVector4R& b)
{
  // t_{ij} = eps_{ijkl} a^{k} b^{l}
  // eps_{ijkl} is the Levi-Civita symbol -- antisymmetric tensor

  EvtTensor4C res;
  double t01 = a.get(2) * b.get(3) - a.get(3) * b.get(2);
  res.set(0, 1, t01);
  res.set(1, 0, -t01);
  double t02 = a.get(3) * b.get(1) - a.get(1) * b.get(3);
  res.set(0, 2, t02);
  res.set(2, 0, -t02);
  double t03 = a.get(1) * b.get(2) - a.get(2) * b.get(1);
  res.set(0, 3, t03);
  res.set(3, 0, -t03);
  double t12 = a.get(0) * b.get(3) - a.get(3) * b.get(0);
  res.set(1, 2, t12);
  res.set(2, 1, -t12);
  double t13 = a.get(2) * b.get(0) - a.get(0) * b.get(2);
  res.set(1, 3, t13);
  res.set(3, 1, -t13);
  double t23 = a.get(0) * b.get(1) - a.get(1) * b.get(0);
  res.set(2, 3, t23);
  res.set(3, 2, -t23);
  return res;
}


/*
 * add scaled tensor
 */
static void addScaled(EvtTensor4C& out, EvtComplex scale, const EvtTensor4C& in)
{
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++) {
      out.set(i, j, out.get(i, j) + scale * in.get(i, j));
    }
}

/*
 * conj(c1)*c2 product
 */
static EvtComplex cprod(const EvtComplex& c1, const EvtComplex& c2)
{
  return EvtComplex(real(c1) * real(c2) + imag(c1) * imag(c2),
                    real(c1) * imag(c2) - imag(c1) * real(c2));
}

/*
 * return v = \bar{x} * gamma_mu * y and a = \bar{x} * gamma_mu * gamma_5 * y currents
 */
static void EvtLeptonVandACurrents(EvtVector4C& v, EvtVector4C& a, const EvtDiracSpinor& x, const EvtDiracSpinor& y)
{
  EvtComplex w0 = cprod(x.get_spinor(0), y.get_spinor(0)), w1 = cprod(x.get_spinor(1), y.get_spinor(1)), w2 = cprod(x.get_spinor(2),
                  y.get_spinor(2)), w3 = cprod(x.get_spinor(3), y.get_spinor(3));
  EvtComplex w02 = w0 + w2, w13 = w1 + w3;
  EvtComplex W1 = w02 + w13, W2 = w02 - w13;
  EvtComplex q0 = cprod(x.get_spinor(0), y.get_spinor(2)), q1 = cprod(x.get_spinor(1), y.get_spinor(3)), q2 = cprod(x.get_spinor(2),
                  y.get_spinor(0)), q3 = cprod(x.get_spinor(3), y.get_spinor(1));
  EvtComplex q02 = q0 + q2, q13 = q1 + q3;
  EvtComplex Q1 = q02 + q13, Q2 = q02 - q13;
  EvtComplex e0 = cprod(x.get_spinor(0), y.get_spinor(3)), e1 = cprod(x.get_spinor(1), y.get_spinor(2)), e2 = cprod(x.get_spinor(2),
                  y.get_spinor(1)), e3 = cprod(x.get_spinor(3), y.get_spinor(0));
  EvtComplex e20 = e0 + e2, e13 = e1 + e3;
  EvtComplex E1 = e13 + e20, E2 = e13 - e20;
  v.set(W1, E1, EvtComplex(-imag(E2), real(E2)), Q2);
  EvtComplex t0 = cprod(x.get_spinor(0), y.get_spinor(1)), t1 = cprod(x.get_spinor(1), y.get_spinor(0)), t2 = cprod(x.get_spinor(2),
                  y.get_spinor(3)), t3 = cprod(x.get_spinor(3), y.get_spinor(2));
  EvtComplex t20 = t0 + t2, t13 = t1 + t3;
  EvtComplex T1 = t13 + t20, T2 = t13 - t20;
  a.set(Q1, T1, EvtComplex(-imag(T2), real(T2)), W2);
}

void EvtbTosllNPR::CalcVAmp(EvtParticle* parent, EvtAmp& amp)
{
  // Add the lepton and neutrino 4 momenta to find q2
  EvtId pId = parent->getId();

  EvtVector4R q = parent->getDaug(1)->getP4() + parent->getDaug(2)->getP4();
  double q2 = q.mass2();
  double mesonmass = parent->getDaug(0)->mass();
  double parentmass = parent->mass();

  double a1, a2, a0, v, t1, t2, t3;    // form factors
  getVectorFF(q2, parentmass, mesonmass, a1, a2, a0, v, t1, t2, t3);

  const EvtVector4R& p4meson = parent->getDaug(0)->getP4();
  double pmass = parent->mass(), ipmass = 1 / pmass;
  const EvtVector4R p4b(pmass, 0.0, 0.0, 0.0);
  EvtVector4R pbhat = p4b * ipmass;
  EvtVector4R qhat = q * ipmass;
  EvtVector4R pkstarhat = p4meson * ipmass;
  EvtVector4R phat = pbhat + pkstarhat;

  EvtComplex c7 = -0.304;
  EvtComplex c9 = C9(q2, interpol(m_reso, q2));
  EvtComplex c10 = -4.103;
  c7 += m_dc7;
  c9 += m_dc9;
  c10 += m_dc10;

  EvtComplex I(0.0, 1.0);

  double mb = 4.8 /*GeV/c^2*/ * ipmass, ms = 0.093 /*GeV/c^2*/ * ipmass;
  double mH = mesonmass * ipmass, oamH = 1 + mH, osmH = 1 - mH,
         osmH2 = oamH * osmH, iosmH2 = 1 / osmH2;    // mhatkstar
  double is = pmass * pmass / q2;                    // 1/shat
  a1 *= oamH;
  a2 *= osmH;
  a0 *= 2 * mH;
  double cs0 = (a1 - a2 - a0) * is;
  a2 *= iosmH2;
  v *= 2 / oamH;

  EvtComplex a = (c9 + m_c9p) * v + (c7 + m_c7p) * (4 * mb * is * t1);
  EvtComplex b = (c9 - m_c9p) * a1 +
                 (c7 - m_c7p) * (2 * mb * is * osmH2 * t2);
  EvtComplex c = (c9 - m_c9p) * a2 +
                 (c7 - m_c7p) * (2 * mb * (t3 * iosmH2 + t2 * is));
  EvtComplex d = (c9 - m_c9p) * cs0 - (c7 - m_c7p) * (2 * mb * is * t3);
  EvtComplex e = (c10 + m_c10p) * v;
  EvtComplex f = (c10 - m_c10p) * a1;
  EvtComplex g = (c10 - m_c10p) * a2;
  EvtComplex h = (c10 - m_c10p) * cs0;
  double sscale = a0 / (mb + ms);
  EvtComplex hs = m_cS * sscale, hp = m_cP * sscale;

  int charge1 = EvtPDL::chg3(parent->getDaug(1)->getId());

  EvtParticle* lepPos = parent->getDaug(2 - (charge1 > 0));
  EvtParticle* lepNeg = parent->getDaug(1 + (charge1 > 0));

  EvtDiracSpinor lp0(lepPos->spParent(0)), lp1(lepPos->spParent(1));
  EvtDiracSpinor lm0(lepNeg->spParent(0)), lm1(lepNeg->spParent(1));

  EvtVector4C l11, l12, l21, l22, a11, a12, a21, a22;
  EvtComplex s11, s12, s21, s22, p11, p12, p21, p22;
  EvtTensor4C tt0 = asymProd(pbhat, pkstarhat);

  EvtTensor4C T1(tt0), T2(tt0);
  const EvtTensor4C& gmn = EvtTensor4C::g();
  EvtTensor4C tt1(EvtGenFunctions::directProd(pbhat, phat));
  EvtTensor4C tt2(EvtGenFunctions::directProd(pbhat, qhat));

  b *= I;
  c *= I;
  d *= I;
  f *= I;
  g *= I;
  h *= I;
  if (pId == IdBm || pId == IdaB0 || pId == IdaBs) {
    T1 *= a;
    addScaled(T1, -b, gmn);
    addScaled(T1, c, tt1);
    addScaled(T1, d, tt2);
    T2 *= e;
    addScaled(T2, -f, gmn);
    addScaled(T2, g, tt1);
    addScaled(T2, h, tt2);

    EvtLeptonVandACurrents(l11, a11, lp0, lm0);
    EvtLeptonVandACurrents(l21, a21, lp1, lm0);
    EvtLeptonVandACurrents(l12, a12, lp0, lm1);
    EvtLeptonVandACurrents(l22, a22, lp1, lm1);

    s11 = EvtLeptonSCurrent(lp0, lm0);
    p11 = EvtLeptonPCurrent(lp0, lm0);
    s21 = EvtLeptonSCurrent(lp1, lm0);
    p21 = EvtLeptonPCurrent(lp1, lm0);
    s12 = EvtLeptonSCurrent(lp0, lm1);
    p12 = EvtLeptonPCurrent(lp0, lm1);
    s22 = EvtLeptonSCurrent(lp1, lm1);
    p22 = EvtLeptonPCurrent(lp1, lm1);
  } else if (pId == IdBp || pId == IdB0 || pId == IdBs) {
    T1 *= -a;
    addScaled(T1, -b, gmn);
    addScaled(T1, c, tt1);
    addScaled(T1, d, tt2);
    T2 *= -e;
    addScaled(T2, -f, gmn);
    addScaled(T2, g, tt1);
    addScaled(T2, h, tt2);

    EvtLeptonVandACurrents(l11, a11, lp1, lm1);
    EvtLeptonVandACurrents(l21, a21, lp0, lm1);
    EvtLeptonVandACurrents(l12, a12, lp1, lm0);
    EvtLeptonVandACurrents(l22, a22, lp0, lm0);

    s11 = EvtLeptonSCurrent(lp1, lm1);
    p11 = EvtLeptonPCurrent(lp1, lm1);
    s21 = EvtLeptonSCurrent(lp0, lm1);
    p21 = EvtLeptonPCurrent(lp0, lm1);
    s12 = EvtLeptonSCurrent(lp1, lm0);
    p12 = EvtLeptonPCurrent(lp1, lm0);
    s22 = EvtLeptonSCurrent(lp0, lm0);
    p22 = EvtLeptonPCurrent(lp0, lm0);
  } else {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Wrong lepton number\n";
    T1.zero();
    T2.zero();    // Set all tensor terms to zero.
  }
  for (int i = 0; i < 3; i++) {
    EvtVector4C eps = parent->getDaug(0)->epsParent(i).conj();

    EvtVector4C E1 = T1.cont1(eps);
    EvtVector4C E2 = T2.cont1(eps);

    EvtComplex epsq = I * (eps * q), epsqs = epsq * hs, epsqp = epsq * hp;

    amp.vertex(i, 0, 0, l11 * E1 + a11 * E2 - s11 * epsqs - p11 * epsqp);
    amp.vertex(i, 0, 1, l12 * E1 + a12 * E2 - s12 * epsqs - p12 * epsqp);
    amp.vertex(i, 1, 0, l21 * E1 + a21 * E2 - s21 * epsqs - p21 * epsqp);
    amp.vertex(i, 1, 1, l22 * E1 + a22 * E2 - s22 * epsqs - p22 * epsqp);
  }
}

void getScalarFF(double q2, double& fp, double& f0, double& fT)
{
  // The form factors are taken from:
  // B -> K and D -> K form factors from fully relativistic lattice QCD
  // W. G. Parrott, C. Bouchard, C. T. H. Davies
  // https://arxiv.org/abs/2207.12468
  static const double MK = 0.495644, MB = 5.279495108051249,
                      MBs0 = 5.729495108051249, MBsstar = 5.415766151925566,
                      logsB = 1.3036556717286227;
  static const int N = 3;
  static const double a0B[] = { 0.2546162729845652,  0.211016713841977,
                                0.02690776720598433, 0.2546162729845652,
                                -0.7084710010870424, 0.3096901516968882,
                                0.2549078414069112,  -0.6549384905373116,
                                0.36265904141973127
                              },
                              *apB = a0B + N, *aTB = apB + N;

  double pole0 = 1 / (1 - q2 / (MBs0 * MBs0));
  double polep = 1 / (1 - q2 / (MBsstar * MBsstar));
  double B = MB + MK, A = sqrt(B * B - q2), z = (A - B) / (A + B),
         zN = z * z * z / N, zn = z;
  f0 = a0B[0];
  fp = apB[0];
  fT = aTB[0];
  for (int i = 1; i < N; i++) {
    f0 += a0B[i] * zn;
    fp += apB[i] * zn;
    fT += aTB[i] * zn;
    double izN = i * zN;
    if ((i - N) & 1) {
      fp += apB[i] * izN;
      fT += aTB[i] * izN;
    } else {
      fp -= apB[i] * izN;
      fT -= aTB[i] * izN;
    }
    zn *= z;
  }
  f0 *= pole0 * logsB;
  fp *= polep * logsB;
  fT *= polep * logsB;
}

void EvtbTosllNPR::CalcSAmp(EvtParticle* parent, EvtAmp& amp)
{
  // Add the lepton and neutrino 4 momenta to find q2
  EvtId pId = parent->getId();

  EvtVector4R q = parent->getDaug(1)->getP4() + parent->getDaug(2)->getP4();
  double q2 = q.mass2();
  double dmass = parent->getDaug(0)->mass();

  double fp, f0, ft;    // form factors
  getScalarFF(q2, fp, f0, ft);
  const EvtVector4R& k = parent->getDaug(0)->getP4();
  double pmass = parent->mass();
  const EvtVector4R p(pmass, 0.0, 0.0, 0.0);
  EvtVector4R pk = p + k;

  EvtComplex c7 = -0.304;
  EvtComplex c9 = C9(q2, interpol(m_reso, q2));
  EvtComplex c10 = -4.103;
  c7 += m_dc7;
  c9 += m_dc9;
  c10 += m_dc10;

  double mb = 4.8 /*GeV/c^2*/, ms = 0.093 /*GeV/c^2*/;

  int charge1 = EvtPDL::chg3(parent->getDaug(1)->getId());

  EvtParticle* lepPos = (charge1 > 0) ? parent->getDaug(1)
                        : parent->getDaug(2);
  EvtParticle* lepNeg = (charge1 < 0) ? parent->getDaug(1)
                        : parent->getDaug(2);

  EvtDiracSpinor lp0(lepPos->spParent(0)), lp1(lepPos->spParent(1));
  EvtDiracSpinor lm0(lepNeg->spParent(0)), lm1(lepNeg->spParent(1));

  EvtVector4C l11, l12, l21, l22, a11, a12, a21, a22;
  EvtComplex s11, s12, s21, s22, p11, p12, p21, p22;

  if (pId == IdBm || pId == IdaB0 || pId == IdaBs) {
    EvtLeptonVandACurrents(l11, a11, lp0, lm0);
    EvtLeptonVandACurrents(l21, a21, lp1, lm0);
    EvtLeptonVandACurrents(l12, a12, lp0, lm1);
    EvtLeptonVandACurrents(l22, a22, lp1, lm1);

    s11 = EvtLeptonSCurrent(lp0, lm0);
    p11 = EvtLeptonPCurrent(lp0, lm0);
    s21 = EvtLeptonSCurrent(lp1, lm0);
    p21 = EvtLeptonPCurrent(lp1, lm0);
    s12 = EvtLeptonSCurrent(lp0, lm1);
    p12 = EvtLeptonPCurrent(lp0, lm1);
    s22 = EvtLeptonSCurrent(lp1, lm1);
    p22 = EvtLeptonPCurrent(lp1, lm1);
  } else if (pId == IdBp || pId == IdB0 || pId == IdBs) {
    EvtLeptonVandACurrents(l11, a11, lp1, lm1);
    EvtLeptonVandACurrents(l21, a21, lp0, lm1);
    EvtLeptonVandACurrents(l12, a12, lp1, lm0);
    EvtLeptonVandACurrents(l22, a22, lp0, lm0);

    s11 = EvtLeptonSCurrent(lp1, lm1);
    p11 = EvtLeptonPCurrent(lp1, lm1);
    s21 = EvtLeptonSCurrent(lp0, lm1);
    p21 = EvtLeptonPCurrent(lp0, lm1);
    s12 = EvtLeptonSCurrent(lp1, lm0);
    p12 = EvtLeptonPCurrent(lp1, lm0);
    s22 = EvtLeptonSCurrent(lp0, lm0);
    p22 = EvtLeptonPCurrent(lp0, lm0);
  } else {
    EvtGenReport(EVTGEN_ERROR, "EvtGen") << "Wrong lepton number\n";
  }
  double dm2 = pmass * pmass - dmass * dmass, t0 = dm2 / q2,
         t1 = 2 * mb * ft / (pmass + dmass);
  c7 += m_c7p;
  c9 += m_c9p;
  c10 += m_c10p;
  EvtVector4C E1 = (c9 * fp + c7 * t1) * pk +
                   (t0 * (c9 * (f0 - fp) - c7 * t1)) * q;
  EvtVector4C E2 = (c10 * fp) * pk + (t0 * (f0 - fp)) * q;
  double s = dm2 / (mb - ms) * f0;
  amp.vertex(0, 0, l11 * E1 + a11 * E2 + (m_cS * s11 + m_cP * p11) * s);
  amp.vertex(0, 1, l12 * E1 + a12 * E2 + (m_cS * s12 + m_cP * p12) * s);
  amp.vertex(1, 0, l21 * E1 + a21 * E2 + (m_cS * s21 + m_cP * p21) * s);
  amp.vertex(1, 1, l22 * E1 + a22 * E2 + (m_cS * s22 + m_cP * p22) * s);
}
